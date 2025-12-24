#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include "features/pagelifecycleaware.h"

namespace Ui {
class HomePage;
}

class HomePage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;
    void onPageLeave() override;

private:

    Ui::HomePage *ui;

};

#endif // HOMEPAGE_H
