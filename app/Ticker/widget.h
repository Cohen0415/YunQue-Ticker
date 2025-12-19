#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include "features/homePage/homepage.h"
#include "features/settingPage/settingpage.h"
#include "features/sysinfoPage/sysinfopage.h"
#include "features/wifiPage/wifipage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:

    Widget(QWidget *parent = nullptr);
    ~Widget();

private:

    // UI 相关
    void UiInit(void);
    void MenuBarUIInit(void);
    void BJTimeUIInit(void);

    // stackedWidget 页面初始化
    void StackedWidgetPageInit(void);

    // 信号槽函数初始化
    void ConnectSignalAndSlot(void);

private slots:

    void onHomePageBtnClicked(void);
    void onSysinfoPageBtnClicked(void);
    void onSettingPageBtnClicked(void);
    void onWifiPageBtnClicked(void);

private:

    Ui::Widget *ui;

    QPushButton *m_homePageBtn;
    QPushButton *m_sysinfoPageBtn;
    QPushButton *m_settingPageBtn;
    QPushButton *m_wifiPageBtn;

    HomePage *m_homePageWidget;
    SettingPage *m_settingPageWidget;
    SysinfoPage *m_sysinfoPageWidget;
    WifiPage *m_wifiPageWidget;

};
#endif // WIDGET_H
