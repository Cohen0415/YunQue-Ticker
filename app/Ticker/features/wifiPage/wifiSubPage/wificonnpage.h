#ifndef WIFICONNPAGE_H
#define WIFICONNPAGE_H

#include <QWidget>
#include <QTimer>
#include "features/pagelifecycleaware.h"

#define CHECK_WIFI_CONN_INTERVAL_MS    1000    // 检测连接状态间隔
#define WIFI_CONN_TIMEOUT_MS           15000   // 连接超时

namespace Ui {
class WifiConnPageWidget;
}

class WifiConnPage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit WifiConnPage(QWidget *parent = nullptr);
    ~WifiConnPage();

    void Init();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;
    void onPageLeave() override;

signals:

    // 向父页面发送的请求信号
    void connectWifiRequested(const QString &ssid, const QString &password);
    void disconnectWifiRequested(void);
    void getWifiStatusRequested(void);

    // 向父页面发送切换到状态页面请求信号
    void switchToStaPageRequested(void);

public slots:

    // 接收父页面发送的结果
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

private slots:

    void on_connButton_clicked();
    void on_ssidLineEdit_textChanged(const QString &arg1);
    void on_pwdLineEdit_textChanged(const QString &arg1);

    // 定时器槽函数
    void onStatusTimerTimeout();

    void on_ssidClearButton_clicked();

    void on_pwdClearButton_clicked();

private:

    // UI 初始化
    void UIInit(void);
    // 用户输入框内容检测
    bool inputLineInspect(void);
    // 加载样式表
    QString LoadQssStyle(const QString &path);

private:

    Ui::WifiConnPageWidget *ui;

    QTimer *m_statusTimer = nullptr;    // 轮询定时器
    int m_elapsed = 0;                  // 已用时ms
    int m_checkInterval;                // 检测间隔ms
    int m_timeoutMs;                    // 超时ms，可调
};

#endif // WIFICONNPAGE_H
