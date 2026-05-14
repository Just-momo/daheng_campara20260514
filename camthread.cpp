#include "camthread.h"

// CamThread 构造函数的定义
CamThread::CamThread(QObject* parent) : QThread(parent)
{
}

// run 函数的定义（子线程在这个死循环里不断获取图像）
void CamThread::run()
{
    m_bStop = false; // 初始时，状态为不停止
    while (!m_bStop)
    {
        if (m_pCam != nullptr)
        {
            // 拿到相机传出来的图片
            QImage img = m_pCam->acquireSingleFrame();

            // 如果图片不是空的，就通过信号发射出去！
            if (!img.isNull())
            {
                emit sendImage(img);        // 把画面发给界面显示
                if (m_isRecording == true && !m_savePath.isEmpty())
                {
                    // 构造文件名，例如：D:/Images/Cam1_video_00001.bmp
                    QString filename = QString("%1/Cam%2_video_%3.bmp")
                        .arg(m_savePath)
                        .arg(m_camId)
                        .arg(m_frameIndex, 5, 10, QChar('0')); // 补齐5位前导零

                    img.save(filename, "BMP"); // 将当前帧存入硬盘
                    m_frameIndex++;            // 序号加1，准备存下一张
                }

            }
        }
        QThread::msleep(10);
    }
}

void CamThread::getCamAddress(DahengCam* p)
{
    m_pCam = p;// 【修改处】接收真实的内存地址
}

void CamThread::setRecordState(bool isRecording, QString savePath, int camId)
{
    m_isRecording = isRecording;
    m_savePath = savePath;
    m_camId = camId;

    // 如果是刚开始录像，把序号清零
    if (m_isRecording)
    {
        m_frameIndex = 0;
    }
}

void CamThread::stopThread()
{
    m_bStop = true; // 拉响下班铃声
}