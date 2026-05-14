#include "ids_camera.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
    // 启动 Qt 应用程序环境
    QApplication a(argc, argv);

    // 实例化主窗口界面
    iDS_Camera w;

    // 将主窗口显示出来
    w.show();

    // 进入 Qt 的事件循环（让程序一直运行，等待用户点击按钮等操作）
    return a.exec();
}