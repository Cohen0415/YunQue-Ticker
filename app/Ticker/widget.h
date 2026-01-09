#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPoint>
#include <QScrollBar>
#include <QTimer>

#include "features/homePage/homepage.h"
#include "features/settingPage/settingpage.h"
#include "features/sysinfoPage/sysinfopage.h"
#include "features/wifiPage/wifipage.h"

#include "features/settingPage/settingpresenter.h"
#include "features/wifiPage/wifipresenter.h"
#include "features/sysinfoPage/sysinfopresenter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:

    Widget(QWidget *parent = nullptr);
    ~Widget();

    void init0();
    void init();

private:

    // UI 相关
    void uiInit();          // UI 初始化
    void menuBarUIInit();   // 菜单栏 UI 初始化
    void bjTimeUIInit();    // 北京时间 UI 初始化
    void staBarUIInit();    // 状态栏 UI 初始化
    
    void stackedWidgetPageInit();   // stackedWidget 页面初始化
    void pagesInit();               // pages 初始化
    void connectSignalAndSlot();    // 信号槽函数初始化

    QString loadQssStyle(const QString &path);  // 加载样式表
    void switchToPage(QWidget *target); // 切换到指定页面

private slots:

    void onHomePageButtonClicked();
    void onSysinfoPageButtonClicked();
    void onSettingPageButtonClicked();
    void onWifiPageButtonClicked();

    void onVolumeMuteStateChanged(bool isMuted);    // 音量静音状态改变
    void onWifiStatusChanged(bool connected);       // wifi 状态改变
    void onBjTimeUpdated();                         // 北京时间更新

private:

    Ui::Widget *ui;

    QPushButton *m_homePageBtn;         // 主页 按钮
    QPushButton *m_sysinfoPageBtn;      // 系统信息页 按钮
    QPushButton *m_settingPageBtn;      // 设置页 按钮
    QPushButton *m_wifiPageBtn;         // wifi设置页 按钮

    HomePage *m_homePageWidget;         // 主页 页面
    SettingPage *m_settingPageWidget;   // 设置页 页面
    SysinfoPage *m_sysinfoPageWidget;   // 系统信息页 页面
    WifiPage *m_wifiPageWidget;         // wifi设置页 页面

    QWidget *m_lastPageWidget = nullptr;// 记录上一个页面指针

    QTimer *bjTimeTimer;                // 北京时间刷新定时器

};
#endif // WIDGET_H
