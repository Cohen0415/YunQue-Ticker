#include <QApplication>
#include <QDebug>
#include <QTimer>

#include "utils/log/logger.h"
#include "services/servicemanager.h"
#include "services/backlight/backlightservice.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    installCustomLogger();

    LOG_DEBUG("Application started.");

    // 1. 创建 ServiceManager 实例
    ServiceManager serviceManager;

    // 2. 初始化 ServiceManager，指定服务器的 Unix Socket 路径
    QString serverSocketPath = "/tmp/dev.sock";
    if (!serviceManager.initialize(serverSocketPath))
    {
        LOG_ERROR("Failed to initialize ServiceManager with socket path: %s", serverSocketPath.toLocal8Bit().constData());
        return -1;
    }

    // 3. 连接到服务器
    if (!serviceManager.connectToServer())
    {
        LOG_ERROR("Failed to connect to server at socket path: %s", serverSocketPath.toLocal8Bit().constData());
        return -1;
    }
    LOG_INFO("Connected to server at socket path: %s", serverSocketPath.toLocal8Bit().constData());

    // 4. 创建背光服务实例
    BacklightService backlightService;

    // 5. 将背光服务添加到服务管理器
    serviceManager.addService(&backlightService);

    // 6. 连接 BacklightService 的信号到测试处理逻辑
    QObject::connect(&backlightService, &BacklightService::brightnessGetResult,
                     [](bool success, int currentValue) {
                         if (success) {
                            LOG_DEBUG("SUCCESS: Current brightness is %d", currentValue);
                         } else {
                            LOG_ERROR("FAILED to get brightness.");
                         }
                     });

    QObject::connect(&backlightService, &BacklightService::brightnessSetResult,
                     [](bool success, int newValue) {
                         if (success) {
                            LOG_DEBUG("SUCCESS: Brightness set to %d", newValue);
                         } else {
                            LOG_ERROR("FAILED to set brightness.");
                         }
                     });

    // 监听 ServiceManager 的连接状态变化
    /*
    QObject::connect(&serviceManager, &ServiceManager::connectionStatusChanged,
                     [](bool isConnected) {
                        LOG_DEBUG(">>> Connection status changed. Now connected: %s", isConnected ? "true" : "false");
                            if(!isConnected) {
                                LOG_WARN(">>> Disconnected from server!");
                            }
                     }); */

    // 7. 发送测试请求 (获取亮度)
    LOG_DEBUG("Sending 'brightness.get' request to the server...");
    backlightService.getBrightness();

    // 8. 稍后发送设置亮度的请求
    QTimer::singleShot(2000, [&backlightService]() {
        LOG_DEBUG("Sending 'brightness.set' request to the server...");
        backlightService.setBrightness(128);
    });

    return app.exec();
}
