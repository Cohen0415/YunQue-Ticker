#ifndef SYSINFOPAGE_H
#define SYSINFOPAGE_H

#include <QWidget>

namespace Ui {
class SysinfoPage;
}

class SysinfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit SysinfoPage(QWidget *parent = nullptr);
    ~SysinfoPage();

private:
    Ui::SysinfoPage *ui;
};

#endif // SYSINFOPAGE_H
