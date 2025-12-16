#include <QApplication>
#include <QDebug>
#include <QTimer>

// 包含相关头文件
#include "services/servicemanager.h"
#include "services/backlight/backlightservice.h"

// #include "widget.h" // 如果你有 Widget 并想显示它

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "--- Starting Client Test ---";

    // 1. 创建 ServiceManager 实例
    ServiceManager serviceManager;

    // 2. 初始化 ServiceManager，指定服务器的 Unix Socket 路径
    // *** 请将下面的路径替换为你实际的服务器 Socket 路径 ***
    QString serverSocketPath = "/tmp/dev.sock"; // <--- 修改为你的实际路径
    if (!serviceManager.initialize(serverSocketPath)) 
    {
        qCritical() << "Failed to initialize ServiceManager with socket path:" << serverSocketPath;
        return -1; // 初始化失败，退出
    }

    // 3. 连接到服务器
    if (!serviceManager.connectToServer()) 
    {
        qCritical() << "Failed to connect to the server at" << serverSocketPath;
        return -1; // 连接失败，退出
    }
    qDebug() << "Successfully initialized and connected to server at" << serverSocketPath;


    // 4. 创建背光服务实例
    BacklightService backlightService;

    // 5. 将背光服务添加到服务管理器
    serviceManager.addService(&backlightService);

    // 6. 连接 BacklightService 的信号到测试处理逻辑
    QObject::connect(&backlightService, &BacklightService::brightnessGetResult,
                     [](bool success, int currentValue) {
                         qDebug() << "--- Received brightnessGetResult ---";
                         if (success) {
                             qDebug() << ">>> SUCCESS: Current brightness is" << currentValue;
                         } else {
                             qDebug() << ">>> FAILED to get brightness.";
                         }
                     });

    QObject::connect(&backlightService, &BacklightService::brightnessSetResult,
                     [](bool success, int newValue) {
                         qDebug() << "--- Received brightnessSetResult ---";
                         if (success) {
                             qDebug() << ">>> SUCCESS: Brightness set to" << newValue;
                         } else {
                             qDebug() << ">>> FAILED to set brightness.";
                         }
                     });

    // 可选：监听 ServiceManager 的连接状态变化
    QObject::connect(&serviceManager, &ServiceManager::connectionStatusChanged,
                     [](bool isConnected) {
                         qDebug() << "--- ServiceManager Connection Status Changed: " << (isConnected ? "Connected" : "Disconnected");
                         if(!isConnected) {
                              qWarning() << "Lost connection to server!";
                              // 这里可以添加重连逻辑或退出程序的逻辑
                         }
                     });


    // 7. 发送测试请求 (例如：获取亮度)
    // 给一点时间让连接稳定? 或者直接发送?
    // 最好是在确认连接建立之后再发送。可以通过监听 connectionStatusChanged 或者假设 connectToServer 成功后即可发送。
    qDebug() << "Sending 'brightness.get' request to the server...";
    backlightService.getBrightness();


    // 8. (可选) 稍后发送设置亮度的请求

    #include <QTimer> // 需要包含 QTimer
    QTimer::singleShot(2000, [&backlightService]() {
        qDebug() << "Sending 'brightness.set' request (value=128) to the server...";
        backlightService.setBrightness(128);
    });



    // 9. (可选) 显示主窗口 Widget
    // Widget w;
    // w.show();

    qDebug() << "--- Entering Application Event Loop ---";

    // 10. 启动事件循环，等待网络数据、处理信号等
    return app.exec();
}
