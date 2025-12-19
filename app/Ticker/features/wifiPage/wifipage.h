#ifndef WIFIPAGE_H
#define WIFIPAGE_H

#include <QWidget>

namespace Ui {
class WifiPage;
}

class WifiPage : public QWidget
{
    Q_OBJECT

public:
    explicit WifiPage(QWidget *parent = nullptr);
    ~WifiPage();

private:
    Ui::WifiPage *ui;
};

#endif // WIFIPAGE_H
