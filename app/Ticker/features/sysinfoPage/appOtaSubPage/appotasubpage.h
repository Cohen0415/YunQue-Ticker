#ifndef APPOTASUBPAGE_H
#define APPOTASUBPAGE_H

#include <QWidget>

namespace Ui {
class AppOtaSubPage;
}

class AppOtaSubPage : public QWidget
{
    Q_OBJECT

public:
    explicit AppOtaSubPage(QWidget *parent = nullptr);
    ~AppOtaSubPage();

private:
    Ui::AppOtaSubPage *ui;
};

#endif // APPOTASUBPAGE_H
