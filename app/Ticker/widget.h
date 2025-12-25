#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPoint>
#include <QScrollBar>

#include "features/homePage/homepage.h"
#include "features/settingPage/settingpage.h"
#include "features/sysinfoPage/sysinfopage.h"
#include "features/wifiPage/wifipage.h"

#include "features/settingPage/settingpresenter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:

    Widget(QWidget *parent = nullptr);
    ~Widget();

    void init();

protected:

    // 事件过滤器重载，用于实现滚动拖拽
    bool eventFilter(QObject *obj, QEvent *event) override;

private:

    // UI 相关
    void uiInit();          // UI 初始化
    void menuBarUIInit();   // 菜单栏 UI 初始化
    void bjTimeUIInit();    // 北京时间 UI 初始化
    void staBarUIInit();    // 状态栏 UI 初始化
    
    void stackedWidgetPageInit();   // stackedWidget 页面初始化
    void pagesInit();               // pages 初始化
    void connectSignalAndSlot();    // 信号槽函数初始化

    void installScrollDragFilters();    // 初始化滚动

    QString loadQssStyle(const QString &path);  // 加载样式表
    void switchToPage(QWidget *target); // 切换到指定页面

private slots:

    void on_homePageButton_clicked();
    void on_sysinfoPageButton_clicked();
    void on_settingPageButton_clicked();
    void on_wifiPageButton_clicked();

    void onVolumeMuteStateChanged(bool isMuted);    // 音量静音状态改变
    void onWifiStatusChanged(bool connected);       // wifi 状态改变

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

    SettingPresenter *m_settingPresenter;   // 设置页 Presenter

    bool m_isDragging = false;
    QPoint m_dragStartPos;              // 记录拖拽开始的位置
    QScrollBar *m_verticalScrollBar;
    int m_dragThreshold = 5;            // 拖拽阈值
    bool m_isContentDragging = false;

    QWidget *m_lastPageWidget = nullptr;// 记录上一个页面指针

};
#endif // WIDGET_H
