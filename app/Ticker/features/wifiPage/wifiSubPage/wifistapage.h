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

private:
    Ui::WifiStaPage *ui;
};

#endif // WIFISTAPAGE_H
