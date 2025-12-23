#ifndef WIFISTAPAGE_H
#define WIFISTAPAGE_H

#include <QWidget>

namespace Ui {
class WifiStaPage;
}

class WifiStaPage : public QWidget
{
    Q_OBJECT

public:

    explicit WifiStaPage(QWidget *parent = nullptr);
    ~WifiStaPage();

signals:

    // 向父页面发送的信号
    void disconnectWifiRequested(void);
    void getWifiStatusRequested(void);

public slots:

    // 接收父页面发送的结果
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

private:

    Ui::WifiStaPage *ui;

};

#endif // WIFISTAPAGE_H
