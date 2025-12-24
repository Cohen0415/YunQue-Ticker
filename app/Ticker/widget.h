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

    void Init();

protected:

    bool eventFilter(QObject *obj, QEvent *event) override;

private:

    // UI 相关
    void UiInit(void);
    void MenuBarUIInit(void);
    void BJTimeUIInit(void);
    void StaBarUIInit(void);

    // stackedWidget 页面初始化
    void StackedWidgetPageInit(void);

    // pages 初始化
    void PagesInit(void);

    // 信号槽函数初始化
    void ConnectSignalAndSlot(void);

    // 初始化滚动
    void installScrollDragFilters();

    // 加载样式表
    QString LoadQssStyle(const QString &path);

    // 切换到指定页面
    void switchToPage(QWidget *target);

private slots:

    void onHomePageBtnClicked(void);
    void onSysinfoPageBtnClicked(void);
    void onSettingPageBtnClicked(void);
    void onWifiPageBtnClicked(void);

    void onVolumeMuteStateChanged(bool isMuted);
    void onWifiStatusChanged(bool connected);

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

    SettingPresenter *m_settingPresenter;

    bool m_isDragging = false;
    QPoint m_dragStartPos;              // 记录拖拽开始的位置
    QScrollBar *m_verticalScrollBar;
    int m_dragThreshold = 5;            // 拖拽阈值
    bool m_isContentDragging = false;

    QWidget *m_lastPageWidget = nullptr;
};
#endif // WIDGET_H
