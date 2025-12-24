#ifndef SYSINFOPAGE_H
#define SYSINFOPAGE_H

#include <QWidget>
#include "features/pagelifecycleaware.h"

namespace Ui {
class SysinfoPage;
}

class SysinfoPage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit SysinfoPage(QWidget *parent = nullptr);
    ~SysinfoPage();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;
    void onPageLeave() override;

private:

    Ui::SysinfoPage *ui;
};

#endif // SYSINFOPAGE_H
