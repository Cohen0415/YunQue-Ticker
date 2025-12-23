#ifndef WIFICONNPAGE_H
#define WIFICONNPAGE_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class WifiConnPage;
}

class WifiConnPage : public QWidget
{
    Q_OBJECT

public:

    explicit WifiConnPage(QWidget *parent = nullptr);
    ~WifiConnPage();

    void Init();

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

private:

    // UI 初始化
    void UIInit(void);
    // 用户输入框内容检测
    void inputLineInspect(void);

private:

    Ui::WifiConnPage *ui;

    QTimer *m_statusTimer = nullptr;    // 轮询定时器
    int m_elapsed = 0;                  // 已用时ms
    int m_checkInterval = 700;          // 检测间隔ms
    int m_timeoutMs = 8000;             // 超时ms，可调

};

#endif // WIFICONNPAGE_H
