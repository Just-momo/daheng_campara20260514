#pragma once

#include <QtWidgets/QWidget>
#include "ui_ids_camera.h"
#include "dahengcam.h"
#include "projector.h"
#include "camthread.h"
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

class iDS_Camera : public QWidget
{
	Q_OBJECT

public:
	iDS_Camera(QWidget* parent = nullptr);
	~iDS_Camera();

	//相机1参数
	DahengCam cam1;//相机1
	QString camPicFileName1 = "";
	//相机2参数
	DahengCam cam2;//相机1
	QString camPicFileName2 = "";

	int picNum = 0;

	bool cam1Flag = false;
	bool cam2Flag = false;
	bool cam1VideoFlag = false;
	bool cam2VideoFlag = false;
	bool boolprojectorFlag = false;

private:
	Ui::iDS_CameraClass ui;

private slots:
	void textMessage(QString message);//打印日志信息

	//cam1
	void updateExposureTime(int value);
	void updateCamGain(int value);
	void initCamBtnClicked();
	void releaseCamBtnClicked();
	void continuousAcquireBtnClicked();
	void setTriggerBtnClicked();
	void setPicPathBtnClicked();
	void savePicBtnClicked();

	//cam2
	void updateExposureTime_2(int value);
	void updateCamGain_2(int value);
	void initCamBtnClicked_2();
	void releaseCamBtnClicked_2();
	void continuousAcquireBtnClicked_2();
	void setTriggerBtnClicked_2();
	void setPicPathBtnClicked_2();
	void savePicBtnClicked_2();

	//projector
	void initProjectorBtnClicked();
	void projectorTriggerBtnClicked();
	void cam1TriggerAcquireBtnClicked();
	void cam2TriggerAcquireBtnClicked();
	void cam12TriggerAcquireBtnClicked();
	void closeProjectorBtnClicked();

private:
	CamThread* camThread = nullptr;
	CamThread* camThread2 = nullptr;
	
};
