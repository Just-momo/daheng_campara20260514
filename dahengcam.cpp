#include "dahengcam.h"
#include "GxIAPI.h"
#include "DxImageProc.h"
#include <QDebug>
#include <thread>
#include <QFile>
#include <QTextStream>

// 初始化大恒 SDK 的全局标志
static bool g_bIsGxInit = false;

DahengCam::DahengCam() : m_hDevice(nullptr), m_bIsOpen(false)
{
    // 如果是第一次实例化，初始化大恒底层库
    if (!g_bIsGxInit) {
        GXInitLib();
        g_bIsGxInit = true;
    }
}

DahengCam::~DahengCam()
{
    exitCamera();
}

int DahengCam::initCamera(int camIndex, HWND hWnd)
{
    uint32_t nDeviceNum = 0;
    GXUpdateDeviceList(&nDeviceNum, 1000); // 扫描可用相机

    if (nDeviceNum < camIndex) {
        return -1; // 找不到足够的相机
    }

    // 通过索引打开相机 (大恒索引从 1 开始)
    GX_STATUS status = GXOpenDeviceByIndex(camIndex, &m_hDevice);
    if (status != GX_STATUS_SUCCESS) return -1;

    m_bIsOpen = true;

    // 设置为连续采集模式
    GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    // 初始关闭触发模式
    GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);

    // 发送开采命令
    GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);

    return 0; // 0代表成功 (模拟原代码的 IS_SUCCESS)
}

int DahengCam::exitCamera()
{
    if (m_bIsOpen && m_hDevice != nullptr) {
        GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
        GXCloseDevice(m_hDevice);
        m_hDevice = nullptr;
        m_bIsOpen = false;
    }
    return 0; // 成功
}

void DahengCam::setCamera(double expTimeMs, double gain)        //这里的 expTimeMs 接收到了上面传过来的exposureTime的值(仅仅是传递数值)，gain接收到了上面camGain的值
{
    if (!m_bIsOpen) return;
    //强制关闭自动曝光和自动增益
    GXSetEnum(m_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
    GXSetEnum(m_hDevice, GX_ENUM_GAIN_AUTO, GX_GAIN_AUTO_OFF);

    // 大恒曝光单位是微秒(us)，根据滑块的设定需要换算
    //毫秒(ms) 转 微秒(us)
    double expTimeUs = expTimeMs * 1000.0;

    GXSetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, expTimeUs);
    GXSetFloat(m_hDevice, GX_FLOAT_GAIN, gain);
}

// 替换一：连续采集单帧图像函数
QImage DahengCam::acquireSingleFrame()
{
    if (!m_bIsOpen) return QImage(); // 相机没开，直接返回空图

    // 1. 查一下当前相机一帧图像需要多大的内存（字节）
    int64_t nPayloadSize = 0;
    GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &nPayloadSize);

    // 2. 准备一张“空白画布”（动态分配内存）
    uchar* pImgBuf = new uchar[nPayloadSize];

    // 3. 告诉相机往这张画布上写数据
    GX_FRAME_DATA stFrameData;
    stFrameData.pImgBuf = pImgBuf;

    // 等待相机出图，超时时间设为1000ms
    GX_STATUS status = GXGetImage(m_hDevice, &stFrameData, 1000);

    QImage resultImg; // 准备一个空的返回值

    // 如果成功拿到了画面
    if (status == GX_STATUS_SUCCESS && stFrameData.nStatus == GX_FRAME_STATUS_SUCCESS)
    {
        // 用拿到的裸数据生成 Qt 认识的图像格式 (MER2-230 是黑白相机)
        QImage img(pImgBuf, stFrameData.nWidth, stFrameData.nHeight, QImage::Format_Grayscale8);

        // 必须进行深拷贝！因为原始画布马上就要被我们销毁了
        m_currentImage = img.copy();
        resultImg = m_currentImage;
    }

    // 用完画布后必须立刻销毁，否则内存泄漏会导致电脑死机
    delete[] pImgBuf;

    return resultImg; // 将画面抛给界面显示
}
void DahengCam::saveSingleFrame(QString picPath)
{
    if (m_currentImage.isNull()) return;

    QDateTime currentTime = QDateTime::currentDateTime();
    QString filename = picPath + "/" + currentTime.toString("yyyyMMdd_HHmmss_zzz") + ".bmp";
    m_currentImage.save(filename, "BMP");
}

void DahengCam::setGpioTrigger()
{
    if (!m_bIsOpen) return;

    // 1、停止当前采集
    GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);

    //2、 打开硬件触发模式 (通常投影仪的同步信号接入 Line0)
    GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
    GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE2);

    // 设置触发极性：配置为下降沿触发 (Falling Edge)
    // 很多 DLP 光机平时是高电平，投图时会将电压拉低，所以默认先试下降沿，如果是上升沿就换成 RISING_EDGE
    GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_ACTIVATION, GX_TRIGGER_ACTIVATION_RISINGEDGE);

    // 3. 强制关闭自动曝光（防止相机自作主张）
    GXSetEnum(m_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_OFF);
    // 强制设置曝光模式为“固定时间 (Timed)”
    GXSetEnum(m_hDevice, GX_ENUM_EXPOSURE_MODE, GX_EXPOSURE_MODE_TIMED);
    // 【极其关键】将曝光时间死死锁在10000微秒！(与烧录软件里的exposure time一致)
    GXSetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, 10000.0);

    ///4、重新发送开采命令，此时相机进入等待触发信号的状态
    GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
}

//硬件触发事件,等待投影仪闪光并保存图像
void DahengCam::triggerAcquireEvent(QString picPath, int count)
{
    if (!m_bIsOpen) return;

    //查大小并分配画布
    // 1. 动态获取当前分辨率（适配 ROI 裁剪后的高度）
    int64_t nWidth = 0, nHeight = 0, nPayloadSize = 0;
    GXGetInt(m_hDevice, GX_INT_WIDTH, &nWidth);
    GXGetInt(m_hDevice, GX_INT_HEIGHT, &nHeight);
    GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &nPayloadSize);

    uchar* pImgBuf = new uchar[nPayloadSize];
    GX_FRAME_DATA stFrameData;
    stFrameData.pImgBuf = pImgBuf;

    // 准备一个内存队列，用来极速暂存图片
    QList<QImage> memoryImages;
    bool bSuccessAll = true;

    // 2. 连续等候 count 个脉冲
    for (int i = 0; i < count; ++i)
    {
        // 阻塞等待 1000ms
        GX_STATUS status = GXGetImage(m_hDevice, &stFrameData, 1000);

        if (status == GX_STATUS_SUCCESS && stFrameData.nStatus == GX_FRAME_STATUS_SUCCESS)
        {
            // 成功：静默保存图片
            QImage img(pImgBuf, (int)stFrameData.nWidth, (int)stFrameData.nHeight, QImage::Format_Grayscale8);
            memoryImages.append(img.copy());
        }
        else
        {
            // 失败：用 txt 文件留案底，标明是第几张图超时的
            QString errorFile = picPath + QString("/TIMEOUT_ERROR_AT_%1.txt").arg(i);
            QFile file(errorFile);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "Camera timeout at pattern index: " << i << "\n";
                file.close();
            }
            bSuccessAll = false;
            // 只要有一张超时，直接跳出循环，防止死锁
            break;
        }
    }

        for (int i = 0; i < memoryImages.size(); ++i) {
            QString fileName = picPath + QString("/%1.bmp").arg(i);
            memoryImages[i].save(fileName, "BMP");
        }
    // 3. 释放内存
    delete[] pImgBuf;
}
void DahengCam::setContinuousMode()
{
    if (!m_bIsOpen) return;

    // 向大恒相机发送“关闭触发模式”的命令
    GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
}
