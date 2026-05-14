#include "ids_camera.h"
#include <QMetaObject>

iDS_Camera::iDS_Camera(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//按键使能
	ui.releaseCamBtn->setEnabled(cam1Flag);
	ui.exposureTimeSlider->setEnabled(cam1Flag);
	ui.camGainSlider->setEnabled(cam1Flag);
	ui.continuousAcquireBtn->setEnabled(cam1Flag);
	ui.setTriggerBtn->setEnabled(cam1Flag);
	ui.setPicPathBtn->setEnabled(cam1Flag);
	ui.savePicBtn->setEnabled(cam1Flag);
	ui.picPathLEdit->setEnabled(cam1Flag);

	ui.releaseCamBtn_2->setEnabled(cam2Flag);
	ui.exposureTimeSlider_2->setEnabled(cam2Flag);
	ui.camGainSlider_2->setEnabled(cam2Flag);
	ui.continuousAcquireBtn_2->setEnabled(cam2Flag);
	ui.setTriggerBtn_2->setEnabled(cam2Flag);
	ui.setPicPathBtn_2->setEnabled(cam2Flag);
	ui.savePicBtn_2->setEnabled(cam2Flag);
	ui.picPathLEdit_2->setEnabled(cam2Flag);

	ui.cam1TriggerAcquireBtn->setEnabled(boolprojectorFlag);
	ui.cam2TriggerAcquireBtn->setEnabled(boolprojectorFlag);
	ui.cam12TriggerAcquireBtn->setEnabled(boolprojectorFlag);
	ui.projectorTriggerBtn->setEnabled(boolprojectorFlag);
	ui.closeProjectorBtn->setEnabled(boolprojectorFlag);

	ui.picPathLEdit->setReadOnly(true);//信息栏只读
	ui.picPathLEdit_2->setReadOnly(true);//信息栏只读
	ui.textEdit->setReadOnly(true);//信息栏只读

	//cam1
	connect(ui.initCamBtn, SIGNAL(clicked()), this, SLOT(initCamBtnClicked()));							//连接相机
	connect(ui.exposureTimeSlider, SIGNAL(valueChanged(int)), this, SLOT(updateExposureTime(int)));		//曝光条
	connect(ui.camGainSlider, SIGNAL(valueChanged(int)), this, SLOT(updateCamGain(int)));				//增益条
	connect(ui.releaseCamBtn, SIGNAL(clicked()), this, SLOT(releaseCamBtnClicked()));					//释放相机
	connect(ui.setTriggerBtn, SIGNAL(clicked()), this, SLOT(setTriggerBtnClicked()));					//触发获取模式
	connect(ui.continuousAcquireBtn, SIGNAL(clicked()), this, SLOT(continuousAcquireBtnClicked()));		//连续获取模式
	connect(ui.setPicPathBtn, SIGNAL(clicked()), this, SLOT(setPicPathBtnClicked()));					//浏览图像保存路径
	connect(ui.savePicBtn, SIGNAL(clicked()), this, SLOT(savePicBtnClicked()));							//保存当前图像

	//创建子线程cam1
	camThread = new CamThread;
	// 接收相机1发来的图像，并让 ui.cameraLabel 显示它
	connect(camThread, &CamThread::sendImage, this, [=](QImage img) {
		ui.cameraLabel->setPixmap(QPixmap::fromImage(img).scaled(ui.cameraLabel->size(), Qt::KeepAspectRatio));
		});
	connect(ui.continuousAcquireBtn, &QPushButton::clicked, this, [=]()
		{
			// 启动子线程
			camThread->getCamAddress(&cam1);
			camThread->start();

		});

	//cam2
	connect(ui.initCamBtn_2, SIGNAL(clicked()), this, SLOT(initCamBtnClicked_2()));
	connect(ui.exposureTimeSlider_2, SIGNAL(valueChanged(int)), this, SLOT(updateExposureTime_2(int)));
	connect(ui.camGainSlider_2, SIGNAL(valueChanged(int)), this, SLOT(updateCamGain_2(int)));
	connect(ui.releaseCamBtn_2, SIGNAL(clicked()), this, SLOT(releaseCamBtnClicked_2()));
	connect(ui.setTriggerBtn_2, SIGNAL(clicked()), this, SLOT(setTriggerBtnClicked_2()));
	connect(ui.continuousAcquireBtn_2, SIGNAL(clicked()), this, SLOT(continuousAcquireBtnClicked_2()));		//连续获取模式
	connect(ui.setPicPathBtn_2, SIGNAL(clicked()), this, SLOT(setPicPathBtnClicked_2()));
	connect(ui.savePicBtn_2, SIGNAL(clicked()), this, SLOT(savePicBtnClicked_2()));


	//创建子线程cam2
	camThread2 = new CamThread;
	// 接收相机2发来的图像，并让 ui.cameraLabel_2 显示它
	connect(camThread2, &CamThread::sendImage, this, [=](QImage img) {
		ui.cameraLabel_2->setPixmap(QPixmap::fromImage(img).scaled(ui.cameraLabel_2->size(), Qt::KeepAspectRatio));
		});
	connect(ui.continuousAcquireBtn_2, &QPushButton::clicked, this, [=]()
		{
			// 启动子线程
			camThread2->start();
			camThread2->getCamAddress(&cam2);
		});

	//projector,链接ui界面投影仪按钮initProjectorBtn与底层代码
	connect(ui.initProjectorBtn, SIGNAL(clicked()), this, SLOT(initProjectorBtnClicked()));				//连接投影仪
	connect(ui.projectorTriggerBtn, SIGNAL(clicked()), this, SLOT(projectorTriggerBtnClicked()));		//开始投影
	connect(ui.closeProjectorBtn, SIGNAL(clicked()), this, SLOT(closeProjectorBtnClicked()));			//停止投影
	connect(ui.cam1TriggerAcquireBtn, SIGNAL(clicked()), this, SLOT(cam1TriggerAcquireBtnClicked()));	//触发相机1
	connect(ui.cam2TriggerAcquireBtn, SIGNAL(clicked()), this, SLOT(cam2TriggerAcquireBtnClicked()));	//触发相机2
	connect(ui.cam12VideoAcquireBtn, SIGNAL(clicked()), this, SLOT(cam12VideoAcquireBtnClicked()));		//录像相机1和相机2
	connect(ui.cam12TriggerAcquireBtn, SIGNAL(clicked()), this, SLOT(cam12TriggerAcquireBtnClicked()));	//触发相机1和相机2

	connect(ui.cam12VideoAcquireBtn, &QPushButton::clicked, this, [this]()
		{
			if (cam1Flag == true && cam2Flag == true)
			{
				// 获取界面上填写的保存路径
				QString path1 = ui.picPathLEdit->text();
				QString path2 = ui.picPathLEdit_2->text();

				cam1VideoFlag = !cam1VideoFlag;		//翻转录像状态开关
				if (cam1VideoFlag == true)
				{
					textMessage("相机1和相机2已开始保存视频帧……");
					ui.cam12VideoAcquireBtn->setText("停止录像");		//把按钮上的字改成“停止录像”
					// 命令后台线程开始存图
					camThread->setRecordState(true, path1, 1);
					camThread2->setRecordState(true, path2, 2);
				}
				else
				{
					textMessage("相机1和相机2已结束保存视频帧！");
					ui.cam12VideoAcquireBtn->setText("视频帧获取");		//录像结束后，把按钮文字恢复
					// 命令后台线程停止存图
					camThread->setRecordState(false, "", 1);
					camThread2->setRecordState(false, "", 2);
				}

				ui.savePicBtn->click();
				ui.savePicBtn_2->click();

			}
			else
			{
				QMessageBox::warning(this, "警告", "请先连接相机1和相机2！");
				textMessage("请先连接相机1和相机2！");
			}

		});

	textMessage("已打开iDS相机调用软件");

}

iDS_Camera::~iDS_Camera()
{
	if (camThread != nullptr)
	{
		camThread->stopThread(); // 喊相机1线程结束
		camThread->wait();       // 等相机1退出（退出run函数）
		delete camThread;        // 销毁线程内存
		camThread = nullptr;
	}

	if (camThread2 != nullptr)
	{
		camThread2->stopThread(); // 喊相机2线程结束
		camThread2->wait();       // 等相机2退出（退出run函数）
		delete camThread2;
		camThread2 = nullptr;
	}
	// 如果没有点“释放相机”按钮就直接关了软件，则兜底释放相机
	if (cam1Flag)
		cam1.exitCamera();
	if (cam2Flag)
		cam2.exitCamera();
	//投影仪兜底释放
	if (boolprojectorFlag == true)
	{
		Projector::closeProjector();
	}
};


void iDS_Camera::textMessage(QString message)
{
	// 获取当前时间戳
	QDateTime currentTime = QDateTime::currentDateTime();
	QString timestamp = currentTime.toString("yyyy/MM/dd  hh:mm:ss  z");
	// 创建带有时间戳的文件名
	//ui.textEdit->append("  ");
	ui.textEdit->append(message);
	ui.textEdit->append(timestamp);
}


//**************************** 相机1 *******************************************//
void iDS_Camera::updateExposureTime(int)
{
	// 1. 获取滑动条的数值（曝光值原逻辑是除以10，保留）
	double exposureTime = ui.exposureTimeSlider->value() / 10.0;
	double camGain = ui.camGainSlider->value(); // 同时获取当前的增益值

	// 2. 更新界面上Label显示的数字
	ui.exposureTimeLable->setText(QString::number(exposureTime));

	// 3. 调用大恒相机的新接口进行设置
	cam1.setCamera(exposureTime, camGain);
}

void iDS_Camera::updateCamGain(int)
{
	// 1. 获取滑动条的数值
	double exposureTime = ui.exposureTimeSlider->value() / 10.0; // 同时获取当前的曝光值
	double camGain = ui.camGainSlider->value();

	// 2. 更新界面上Label显示的数字
	ui.camGainLable->setText(QString::number(camGain));

	// 3. 调用大恒相机的新接口进行设置
	cam1.setCamera(exposureTime, camGain);
}
void iDS_Camera::initCamBtnClicked()//初始化相机按钮
{
	// 根据下拉框选择决定是几号相机 (大恒索引从 1 开始)
	int camIndex = (ui.camIdCBox->currentIndex() == 0) ? 1 : 2;

	// 调用新的大恒初始化函数
	if (cam1.initCamera(camIndex, (HWND)ui.cameraLabel->winId()) == 0) // 0 代表成功
	{
	

		cam1Flag = true;
		ui.releaseCamBtn->setEnabled(cam1Flag);
		ui.exposureTimeSlider->setEnabled(cam1Flag);
		ui.camGainSlider->setEnabled(cam1Flag);
		ui.continuousAcquireBtn->setEnabled(cam1Flag);
		ui.setTriggerBtn->setEnabled(cam1Flag);
		ui.setPicPathBtn->setEnabled(cam1Flag);
		ui.savePicBtn->setEnabled(cam1Flag);
		ui.picPathLEdit->setEnabled(cam1Flag);

		updateExposureTime(0);
		updateCamGain(0);

		ui.continuousAcquireBtn->click();
		textMessage("相机1连接成功！");
		QMessageBox::information(this, "信息", "相机连接成功！");

		//cam1.setImageSize();  // 大恒相机默认获取最大分辨率
		//cam1.setAOI();
		//cam1.setImageMemoryAOI();
		//cam1.setImageMemory();  // 内存由底层管理
	}
	else
	{
		QMessageBox::warning(this, "警告", "相机连接失败！");
		textMessage("相机1连接失败！");
	}
}

void iDS_Camera::releaseCamBtnClicked()//释放相机按钮
{
	if (camThread != nullptr)
	{
		camThread->stopThread(); //停止线程
		camThread->wait();       
	}

	if (cam1.exitCamera() == 0)
	{
		QMessageBox::information(this, "信息", "相机释放成功！");
		cam1Flag = false;
		ui.releaseCamBtn->setEnabled(cam1Flag);
		ui.exposureTimeSlider->setEnabled(cam1Flag);
		ui.camGainSlider->setEnabled(cam1Flag);
		ui.continuousAcquireBtn->setEnabled(cam1Flag);
		ui.setTriggerBtn->setEnabled(cam1Flag);
		ui.setPicPathBtn->setEnabled(cam1Flag);
		ui.savePicBtn->setEnabled(cam1Flag);
		ui.picPathLEdit->setEnabled(cam1Flag);
		textMessage("相机1释放成功！");
	}
	else
	{
		QMessageBox::warning(this, "警告", "相机释放失败！");
		textMessage("相机1释放失败！");
	}
}

void iDS_Camera::continuousAcquireBtnClicked()
{
	if (cam1Flag == true)	// 安全判断：如果相机1连着，让相机1解除触发，恢复连续模式
	{
		cam1.setContinuousMode();	// 向大恒相机发送指令，关闭硬件触发，恢复连续模式
		textMessage("相机1已设置连续获取模式");
	}
}

void iDS_Camera::setPicPathBtnClicked()
{
	iDS_Camera::camPicFileName1 = QFileDialog::getExistingDirectory(this, tr("请选择图片保存的文件夹（路径不能有中文和特殊符号）"), "");
	ui.picPathLEdit->setText(camPicFileName1);
	textMessage("相机1已设置图像保存路径");
}

void iDS_Camera::savePicBtnClicked()
{
	cam1.saveSingleFrame(camPicFileName1);
	textMessage("相机1已保存一张图像");
	//if(camPicFileName1.isEmpty()) {
	//	QMessageBox::warning(this, "警告", "请先设置相机1的保存路径！");
	//	return;
	//}
	//if (cam1VideoFlag == false)
	//{
	//	ui.savePicBtn->setText("结束保存图像帧");
	//	textMessage("相机1正在保存多张视频帧……");	
	//	cam1VideoFlag = true;
	//}
	//else
	//{
	//	ui.savePicBtn->setText("开始保存图像帧");
	//	textMessage("相机1结束保存图像帧！");
	//	cam1VideoFlag = false;
	//}		
	//std::thread t1([this]() {
	//	cam1.saveVideoFrame(camPicFileName1,cam1VideoFlag);
	//	});
	//t1.detach();
	// 等待两个线程完成

}

void iDS_Camera::setTriggerBtnClicked()
{
	cam1.setGpioTrigger();
	textMessage("相机1已设置硬件触发模式");
}

//**************************** 相机1 结束*******************************************//

//**************************** 相机2 *******************************************//
void iDS_Camera::updateExposureTime_2(int)
{
	//ui.exposureTimeLable->setText(QString::number(ui.exposureTimeSlider->value()/10.0 ));
	double exposureTime2 = ui.exposureTimeSlider_2->value() / 10.0;
	double camGain2 = ui.camGainSlider_2->value(); // 同时获取当前的增益值
	ui.exposureTimeLable_2->setText(QString::number(exposureTime2));
	cam2.setCamera(exposureTime2, camGain2);
}
void iDS_Camera::updateCamGain_2(int)
{
	//ui.camGainLable->setText(QString::number(ui.camGainSlider->value()));
	double exposureTime2 = ui.exposureTimeSlider_2->value() / 10.0;
	double camGain2 = ui.camGainSlider_2->value(); // 同时获取当前的增益值
	ui.camGainLable_2->setText(QString::number(camGain2));
	cam2.setCamera(exposureTime2, camGain2);
}

void iDS_Camera::initCamBtnClicked_2()		//初始化相机按钮
{
	int camIndex = (ui.camIdCBox_2->currentIndex() == 0) ? 2 : 1;
	
	if (cam2.initCamera(camIndex, (HWND)ui.cameraLabel_2->winId()) == 0)
	{
		QMessageBox::information(this, "信息", "相机连接成功！");
		cam2Flag = true;
		ui.releaseCamBtn_2->setEnabled(cam2Flag);
		ui.exposureTimeSlider_2->setEnabled(cam2Flag);
		ui.camGainSlider_2->setEnabled(cam2Flag);
		ui.continuousAcquireBtn_2->setEnabled(cam2Flag);
		ui.setTriggerBtn_2->setEnabled(cam2Flag);
		ui.setPicPathBtn_2->setEnabled(cam2Flag);
		ui.savePicBtn_2->setEnabled(cam2Flag);
		ui.picPathLEdit_2->setEnabled(cam2Flag);

		int currentExp2 = ui.exposureTimeSlider_2->value();
		updateExposureTime_2(currentExp2);

		//cam2.setImageSize();
		//cam2.setAOI();
		//cam2.setImageMemoryAOI();
		//cam2.setImageMemory();
		cam2.setCamera(ui.exposureTimeSlider_2->value(), ui.camGainSlider_2->value());

		ui.continuousAcquireBtn_2->click();
		textMessage("相机2连接成功！");
	}
	else
	{
		QMessageBox::warning(this, "警告", "相机连接失败！");
		textMessage("相机2连接失败！");
	}
}

void iDS_Camera::releaseCamBtnClicked_2()	//释放相机按钮
{
	if (camThread2 != nullptr)
	{
		camThread2->stopThread();
		camThread2->wait();
	}

	if (cam2.exitCamera() == 0)
	{
		QMessageBox::information(this, "信息", "相机释放成功！");
		cam2Flag = false;
		ui.releaseCamBtn_2->setEnabled(cam2Flag);
		ui.exposureTimeSlider_2->setEnabled(cam2Flag);
		ui.camGainSlider_2->setEnabled(cam2Flag);
		ui.continuousAcquireBtn_2->setEnabled(cam2Flag);
		ui.setTriggerBtn_2->setEnabled(cam2Flag);
		ui.setPicPathBtn_2->setEnabled(cam2Flag);
		ui.savePicBtn_2->setEnabled(cam2Flag);
		ui.picPathLEdit_2->setEnabled(cam2Flag);
		textMessage("相机2释放成功！");
	}
	else
	{
		QMessageBox::warning(this, "警告", "相机释放失败！");
		textMessage("相机2释放失败!");
	}
}

void iDS_Camera::continuousAcquireBtnClicked_2()
{
	if (cam2Flag == true)	// 安全判断：如果相机1连着，让相机1解除触发，恢复连续模式
	{
		cam2.setContinuousMode();	// 向大恒相机发送指令，关闭硬件触发，恢复连续模式
		textMessage("相机2已设置连续获取模式");
	}
}

void iDS_Camera::setPicPathBtnClicked_2()
{
	iDS_Camera::camPicFileName2 = QFileDialog::getExistingDirectory(this, tr("请选择图片保存的文件夹（路径不能有中文和特殊符号）"), "");
	ui.picPathLEdit_2->setText(camPicFileName2);
	textMessage("相机2已设置图像保存路径");
}

void iDS_Camera::savePicBtnClicked_2()
{
	cam2.saveSingleFrame(camPicFileName2);
	textMessage("相机2已保持一张图像");
	//if(camPicFileName2.isEmpty()) {
	//	QMessageBox::warning(this, "警告", "请先设置相机2的保存路径！");
	//	return;
	//}
	//if (cam2VideoFlag == false)
	//{
	//	ui.savePicBtn_2->setText("结束保存图像帧");
	//	textMessage("相机2正在保存多张视频帧……");
	//	cam2VideoFlag = true;
	//}
	//else
	//{
	//	ui.savePicBtn_2->setText("开始保存图像帧");
	//	textMessage("相机2结束保存图像帧！");
	//	cam2VideoFlag = false;
	//}
	//std::thread t2([this]() {
	//	cam2.saveVideoFrame(camPicFileName2, cam2VideoFlag);
	//	});
	//t2.detach();
	//// 等待两个线程完成
}

void iDS_Camera::setTriggerBtnClicked_2()
{
	cam2.setGpioTrigger();
	textMessage("相机2已设置硬件触发模式");
}

//**************************** 相机2 结束*******************************************//

//****************************  投影仪   *******************************************//
void iDS_Camera::initProjectorBtnClicked()
{
	int projectorFlag = Projector::initProjector();
	if (projectorFlag == 0)
	{
		QMessageBox::information(this, "信息", "投影仪连接成功！");
		ui.projectorLabel->setText("已连接！");
		boolprojectorFlag = true;
		ui.initProjectorBtn->setEnabled(boolprojectorFlag);
		ui.projectorTriggerBtn->setEnabled(boolprojectorFlag);
		ui.cam1TriggerAcquireBtn->setEnabled(boolprojectorFlag);
		ui.cam2TriggerAcquireBtn->setEnabled(boolprojectorFlag);
		ui.cam12TriggerAcquireBtn->setEnabled(boolprojectorFlag);
		ui.closeProjectorBtn->setEnabled(boolprojectorFlag);
		textMessage("投影仪连接成功！");
	}
	else
	{
		QMessageBox::warning(this, "警告", "投影仪连接失败！");
		ui.projectorLabel->setText("未连接！");
		textMessage("投影仪连接失败！");
	}
}

void iDS_Camera::projectorTriggerBtnClicked()
{
	Projector::ProjectorTriggerOnce();
	textMessage("开始投影...");
}

void iDS_Camera::cam1TriggerAcquireBtnClicked()			//对应ui界面上相机1触发采集
{
	if (cam1Flag == true)			//安全检查，若cam1Flag为true，则说明相机1已经连接上
	{
		Projector::ProjectorTriggerOnce();		//命令投影仪闪烁一次图案，并在闪烁的瞬间通过线缆发出一个电信号（触发信号）
		//cam1.triggerAcquire(picNum, camPicFileName1);
		cam1.triggerAcquireEvent(camPicFileName1, 4);		//让相机1进入“等候状态”，一旦接收到投影仪发出的电信号，就瞬间拍一张照片，并保存到 camPicFileName1 这个路径下
	}
	textMessage("投影仪投影触发相机1...");
}

void iDS_Camera::cam2TriggerAcquireBtnClicked()
{
	if (cam2Flag == true)
	{
		Projector::ProjectorTriggerOnce();
		//cam2.triggerAcquire(picNum, camPicFileName2);
		cam2.triggerAcquireEvent(camPicFileName2, 4);
	}
	textMessage("投影仪触发触发相机2...");
}

void iDS_Camera::cam12TriggerAcquireBtnClicked()
{
	//if (hCam1 != 0)
	//{
	//	Projector::ProjectorTriggerOnce();
	//	//cam1.triggerAcquire(picNum, camPicFileName1);
	//	cam1.triggerAcquireEvent(camPicFileName1);
	//}
	//textMessage("投影仪投影触发相机1...");
	//if (hCam2 != 0)
	//{
	//	Projector::ProjectorTriggerOnce();
	//	//cam2.triggerAcquire(picNum, camPicFileName2);
	//	cam2.triggerAcquireEvent(camPicFileName2);
	//}
	//textMessage("投影仪投影触发相机2...");

	// 检查保存路径是否设置
	if (camPicFileName1.isEmpty() || camPicFileName2.isEmpty()) {
		textMessage("请先设置两个相机的保存路径！");
		return;
	}
	if (cam1Flag == true && cam2Flag == true)
	{
		textMessage("准备双相机同步触发...");

		// 定义抓拍数量
		int patternCount = 4;

		// 开启一个后台总指挥线程，防止 UI 界面卡死
		std::thread masterThread([this, patternCount]() {

			// 启动两个相机的监听线程（注意 lambda 表达式要捕获 patternCount）
			std::thread t1([this, patternCount]() {
				cam1.triggerAcquireEvent(camPicFileName1, patternCount);
				});

			std::thread t2([this, patternCount]() {
				cam2.triggerAcquireEvent(camPicFileName2, patternCount);
				});

			// 缓冲时间：确保两个相机的底层 GXGetImage 已经彻底开始阻塞等待
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			// 投影仪发送同步触发信号
			Projector::ProjectorTriggerOnce();

			// 跨线程安全调用 UI
			QMetaObject::invokeMethod(this, [this]() {
				textMessage("投影仪同步触发双相机...");
				});

			// 阻塞等待两个相机的 4 张图全部拍完
			t1.join();
			t2.join();

			// 跨线程安全调用 UI
			QMetaObject::invokeMethod(this, [this]() 
				{
					textMessage("双相机同步采集完成！");
				});
			});

		// 让总指挥线程去后台独立运行，主界面恢复流畅
		masterThread.detach();
	}
}

void iDS_Camera::closeProjectorBtnClicked()
{
	Projector::closeProjector();
	textMessage("已停止投影");
}

//*************************** 投影仪结束 *******************************************//