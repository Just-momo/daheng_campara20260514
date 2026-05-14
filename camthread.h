#pragma once

#include <QThread>
#include "dahengcam.h"

class CamThread : public QThread
{
	Q_OBJECT
public:
	explicit CamThread(QObject* parent = nullptr);
	void stopThread();		//让主程序呼叫停止的函数

protected:
	void run();

private:
	DahengCam* m_pCam = nullptr; // 【修改处】改成指针，并且默认置空

	bool m_isRecording = false;  // 是否正在录像
	QString m_savePath = "";     // 保存路径
	int m_camId = 1;             // 相机编号（用于区分文件命名）
	int m_frameIndex = 0;        // 录像帧序号（001, 002...）
	bool m_bStop = false;		//停止标志位

signals:
	// 自定义信号, 传递数据
	// 【新增】这是一个用来发货（发送图像）的信号
	void sendImage(QImage img);

public slots:
	//主线程传递相机类给子线程
	void getCamAddress(DahengCam* p);
	void setRecordState(bool isRecording, QString savePath, int camId);		//用来接收主界面发来的录像指令
};
