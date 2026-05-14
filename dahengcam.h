#pragma once
#include <QString>
#include <QImage>
#include <QDateTime>
// 引入大恒相机的标准 C API 头文件
#include "GxIAPI.h" 
#include "DxImageProc.h"  


class DahengCam
{
public:
    DahengCam();
    ~DahengCam();

    // 基础功能接口 
    int initCamera(int camIndex, HWND hWnd);               // 初始化相机 (camIndex 传入 1 或 2)
    int exitCamera();                                     // 释放相机
    void setCamera(double exposuretime, double gain);     // 设置曝光和增益
    QImage acquireSingleFrame();                          // 将 void 改成 QImage，让它能返回图像
    void saveSingleFrame(QString picPath);                // 保存当前图像

    // 触发模式接口
    void setGpioTrigger();                                // 设置为硬件触发模式(接收投影仪信号)
    void triggerAcquireEvent(QString picPath, int count); // 等待触发信号并保存图像
    void setContinuousMode();                             // 恢复连续采集模式（关闭硬件触发）


private:
    GX_DEV_HANDLE m_hDevice;                              // 大恒相机设备句柄
    bool m_bIsOpen;                                       // 标记是否打开

    // 内部存图辅助变量
    QImage m_currentImage;                                // 缓存最新的一帧图像
};