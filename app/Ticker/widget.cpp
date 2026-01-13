#include "widget.h"
#include "ui_widget.h"
#include "utils/log/logger.h"
#include "appcontext.h"
#include "features/pagemsgmanager.h"
#include "features/pagelifecycleaware.h"

#include <QPushButton>
#include <QScrollBar>
#include <QMouseEvent>
#include <QApplication>
#include <QWidget>
#include <QList>
#include <QLabel>
#include <QDateTime>
#include <QTimeZone>
#include <QScroller>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , bjTimeTimer(new QTimer(this))
{
    ui->setupUi(this);

    connect(bjTimeTimer, &QTimer::timeout, this, &Widget::onBjTimeUpdated);
    bjTimeTimer->start(1000); // 每秒更新一次北京时间
}

Widget::~Widget()
{
    delete ui;
}

// 只先初始化 UI，用于快速桌面显示
void Widget::init0()
{
    // 初始化 UI
    uiInit();
}

// 初始化函数
void Widget::init()
{
    // 初始化 stackedWidget 页面
    stackedWidgetPageInit();

    // 初始化信号槽
    connectSignalAndSlot();

    // 在初始化信号后，再初始化 pages，因为 pages 初始化时可能会发送信号
    pagesInit();
}

// 加载样式表
QString Widget::loadQssStyle(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return QString(); // 打开失败返回空字符串

    QTextStream in(&file);
    in.setCodec("UTF-8"); // 保证中文正常显示

    QString style = in.readAll();
    file.close();
    return style;
}

// 切换到指定页面
void Widget::switchToPage(QWidget *target)
{
    // 如果目标页等于当前页，直接返回
    if (ui->stackedWidget->currentWidget() == target)
        return;

    // 先通知上一个页面
    if (m_lastPageWidget)
    {
        auto lastAble = dynamic_cast<PageLifecycleAware *>(m_lastPageWidget);
        if (lastAble)
            lastAble->onPageLeave();
    }

    // 切换页面
    ui->stackedWidget->setCurrentWidget(target);

    // 再通知新页面
    auto newAble = dynamic_cast<PageLifecycleAware *>(target);
    if (newAble)
        newAble->onPageEnter();

    // 更新last
    m_lastPageWidget = target;
}

// UI 初始化
void Widget::uiInit()
{
    // 禁用标题栏
    // this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    // 不显示鼠标
    QWidget::setCursor(QCursor(Qt::BlankCursor));

    // 初始化菜单栏 UI
    menuBarUIInit();

    // 初始化时间 UI
    bjTimeUIInit();

    // 初始化状态栏 UI
    staBarUIInit();

    // 加载样式表
    QString qss = loadQssStyle(":/res/qss/mainFrameQss/mainFrame.qss");
    if (!qss.isEmpty())
        this->setStyleSheet(qss);
}

// 菜单栏 UI 初始化
void Widget::menuBarUIInit()
{
    // 获取 scrollArea 内部的内容 widget
    QWidget *contentWidget = ui->scrollArea->widget();

    // 设置垂直布局
    QVBoxLayout *menuScrollWidgetLayout = new QVBoxLayout(contentWidget);
    menuScrollWidgetLayout->setSpacing(15);
    menuScrollWidgetLayout->setContentsMargins(0, 15, 0, 15);    // left top right bottom
    menuScrollWidgetLayout->setAlignment(Qt::AlignHCenter);

    // 添加 主页 按钮
    m_homePageBtn = new QPushButton(contentWidget);
    m_homePageBtn->setIcon(QIcon(":/res/icon/menuIcon/home.png"));
    m_homePageBtn->setIconSize(QSize(64, 64));
    m_homePageBtn->setFixedSize(64, 64);
    m_homePageBtn->setFlat(true);    // 去掉按钮边框
    m_homePageBtn->setFocusPolicy(Qt::NoFocus);
    m_homePageBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"             // 移除边框（可选）
        "    background: transparent;"  // 背景透明
        "}"
        "QPushButton:pressed {"
        "    background: transparent;"  // 按下时背景也透明
        "}"
    );

    // 添加 系统信息页面 按钮
    m_sysinfoPageBtn = new QPushButton(contentWidget);
    m_sysinfoPageBtn->setIcon(QIcon(":/res/icon/menuIcon/sysinfo.png"));
    m_sysinfoPageBtn->setIconSize(QSize(64, 64));
    m_sysinfoPageBtn->setFixedSize(64, 64);
    m_sysinfoPageBtn->setFlat(true);
    m_sysinfoPageBtn->setFocusPolicy(Qt::NoFocus);
    m_sysinfoPageBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"             // 移除边框（可选）
        "    background: transparent;"  // 背景透明
        "}"
        "QPushButton:pressed {"
        "    background: transparent;"  // 按下时背景也透明
        "}"
    );

    // 添加 设置信息页面 按钮
    m_settingPageBtn = new QPushButton(contentWidget);
    m_settingPageBtn->setIcon(QIcon(":/res/icon/menuIcon/setting.png"));
    m_settingPageBtn->setIconSize(QSize(64, 64));
    m_settingPageBtn->setFixedSize(64, 64);
    m_settingPageBtn->setFlat(true);
    m_settingPageBtn->setFocusPolicy(Qt::NoFocus);
    m_settingPageBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"             // 移除边框（可选）
        "    background: transparent;"  // 背景透明
        "}"
        "QPushButton:pressed {"
        "    background: transparent;"  // 按下时背景也透明
        "}"
    );

    // 添加 wifi页面 按钮
    m_wifiPageBtn = new QPushButton(contentWidget);
    m_wifiPageBtn->setIcon(QIcon(":/res/icon/menuIcon/wifi.png"));
    m_wifiPageBtn->setIconSize(QSize(64, 64));
    m_wifiPageBtn->setFixedSize(64, 64);
    m_wifiPageBtn->setFlat(true);
    m_wifiPageBtn->setFocusPolicy(Qt::NoFocus);
    m_wifiPageBtn->setStyleSheet(
        "QPushButton {"
        "    border: none;"             // 移除边框（可选）
        "    background: transparent;"  // 背景透明
        "}"
        "QPushButton:pressed {"
        "    background: transparent;"  // 按下时背景也透明
        "}"
    );

    // scrollWidget 添加按钮
    menuScrollWidgetLayout->addWidget(m_homePageBtn);
    menuScrollWidgetLayout->addWidget(m_settingPageBtn);
    menuScrollWidgetLayout->addWidget(m_wifiPageBtn);
    menuScrollWidgetLayout->addWidget(m_sysinfoPageBtn);

    // 去除边框和阴影
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->scrollArea->setFrameShadow(QFrame::Plain);

    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏滚动条
    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);  // 触摸 / 惯性滚动
}

// 北京时间 UI 初始化
void Widget::bjTimeUIInit()
{
    ui->bjTimeLabel->setText("xxxx - xx:xx");
}

// 状态栏 UI 初始化
void Widget::staBarUIInit()
{
    ui->soundLabelIcon->setPixmap(QPixmap(":/res/icon/staBarIcon/soundOff.png"));
    ui->wifiLabelIcon->setPixmap(QPixmap(":/res/icon/staBarIcon/wifiDisconnect.png"));
}

// stackedWidget 页面初始化
void Widget::stackedWidgetPageInit()
{
    // 主页 页面
    m_homePageWidget = new HomePage(this);
    ui->stackedWidget->addWidget(m_homePageWidget);

    // 系统信息 页面
    m_sysinfoPageWidget = new SysinfoPage(this);
    ui->stackedWidget->addWidget(m_sysinfoPageWidget);

    // 设置信息 页面
    m_settingPageWidget = new SettingPage(this);
    ui->stackedWidget->addWidget(m_settingPageWidget);

    // wifi 页面
    m_wifiPageWidget = new WifiPage(this);
    ui->stackedWidget->addWidget(m_wifiPageWidget);

    // 默认显示 主页 页面
    switchToPage(m_homePageWidget);
}

// pages 初始化
void Widget::pagesInit()
{
    // homePage init
    m_homePageWidget->init();
    LOG_INFO("HomePage initialized.");

    // settingPage init
    m_settingPageWidget->init();
    LOG_INFO("SettingPage initialized.");

    // wifiPage init
    m_wifiPageWidget->init();
    LOG_INFO("WifiPage initialized.");

    // sysinfoPage init
    m_sysinfoPageWidget->init();
    LOG_INFO("SysinfoPage initialized.");
}

// 信号槽函数初始化
void Widget::connectSignalAndSlot()
{
    if (!m_homePageBtn || !m_sysinfoPageBtn || !m_settingPageBtn || !m_wifiPageBtn)
    {
        return;
    }

    // 菜单栏按钮的信号槽连接
    connect(m_homePageBtn, &QPushButton::clicked, this, &Widget::onHomePageButtonClicked);
    connect(m_sysinfoPageBtn, &QPushButton::clicked, this, &Widget::onSysinfoPageButtonClicked);
    connect(m_settingPageBtn, &QPushButton::clicked, this, &Widget::onSettingPageButtonClicked);
    connect(m_wifiPageBtn, &QPushButton::clicked, this, &Widget::onWifiPageButtonClicked);

    // presenters 和各个 page 的信号槽连接
    // settingPresenter <--> settingPage
    SettingPresenter *settingPresenter = AppContext::getInstance()->settingPresenter();
    // backlight set
    connect(m_settingPageWidget, &SettingPage::setBacklightRequested, settingPresenter, &SettingPresenter::onBacklightSetChangeRequested);
    connect(settingPresenter, &SettingPresenter::backlightSetChangeResult, m_settingPageWidget, &SettingPage::onBacklightSetResult);
    // backlight get
    connect(m_settingPageWidget, &SettingPage::getBacklightRequested, settingPresenter, &SettingPresenter::onBacklightGetChangeRequested);
    connect(settingPresenter, &SettingPresenter::backlightGetChangeResult, m_settingPageWidget, &SettingPage::onBacklightGetResult);
    // audio set
    connect(m_settingPageWidget, &SettingPage::setVolumeRequested, settingPresenter, &SettingPresenter::onVolumeSetChangeRequested);
    connect(settingPresenter, &SettingPresenter::volumeSetChangeResult, m_settingPageWidget, &SettingPage::onVolumeSetResult);
    // audio get
    connect(m_settingPageWidget, &SettingPage::getVolumeRequested, settingPresenter, &SettingPresenter::onVolumeGetChangeRequested);
    connect(settingPresenter, &SettingPresenter::volumeGetChangeResult, m_settingPageWidget, &SettingPage::onVolumeGetResult);

    // wifiPresenter <--> wifiPage
    WifiPresenter *wifiPresenter = AppContext::getInstance()->wifiPresenter();
    // wifi connect
    connect(m_wifiPageWidget, &WifiPage::connectWifiRequested, wifiPresenter, &WifiPresenter::onConnectWifiRequested);
    connect(wifiPresenter, &WifiPresenter::connectWifiResult, m_wifiPageWidget, &WifiPage::onConnectWifiResult);
    // wifi disconnect
    connect(m_wifiPageWidget, &WifiPage::disconnectWifiRequested, wifiPresenter, &WifiPresenter::onDisconnectWifiRequested);
    connect(wifiPresenter, &WifiPresenter::disconnectWifiResult, m_wifiPageWidget, &WifiPage::onDisconnectWifiResult);
    // wifi get status
    connect(m_wifiPageWidget, &WifiPage::getWifiStatusRequested, wifiPresenter, &WifiPresenter::onGetWifiStatusRequested);
    connect(wifiPresenter, &WifiPresenter::getWifiStatusResult, m_wifiPageWidget, &WifiPage::onGetWifiStatusResult);

    // sysinfoPresenter <--> sysinfoPage
    SysinfoPresenter *sysinfoPresenter = AppContext::getInstance()->sysinfoPresenter();
    // cpu temp get
    connect(m_sysinfoPageWidget, &SysinfoPage::getCpuTempRequested, sysinfoPresenter, &SysinfoPresenter::onGetCpuTempRequested);
    connect(sysinfoPresenter, &SysinfoPresenter::cpuTempGetResult, m_sysinfoPageWidget, &SysinfoPage::onCpuTempGetResult);
    // bj time get
    connect(m_sysinfoPageWidget, &SysinfoPage::getBjTimeRequested, sysinfoPresenter, &SysinfoPresenter::onGetBjTimeRequested);
    connect(sysinfoPresenter, &SysinfoPresenter::bjTimeGetResult, m_sysinfoPageWidget, &SysinfoPage::onBjTimeGetResult);
    // system version get
    connect(m_sysinfoPageWidget, &SysinfoPage::getSysVersionRequested, sysinfoPresenter, &SysinfoPresenter::onGetSysVersionRequested);
    connect(sysinfoPresenter, &SysinfoPresenter::sysVersionGetResult, m_sysinfoPageWidget, &SysinfoPage::onSysVersionGetResult);


    // 订阅 PageMsgManager 的信号槽连接
    PageMsgManager *pageMsgManager = PageMsgManager::getInstance();
    // 音量静音状态变化信号
    connect(pageMsgManager, &PageMsgManager::volumeMuteStateChanged, this, &Widget::onVolumeMuteStateChanged);
    // wifi 状态变化信号
    connect(pageMsgManager, &PageMsgManager::wifiStatusChanged, this, &Widget::onWifiStatusChanged);
}

void Widget::onHomePageButtonClicked()
{
    if (m_homePageWidget)
        switchToPage(m_homePageWidget);
}

void Widget::onSysinfoPageButtonClicked()
{
    if (m_sysinfoPageWidget)
        switchToPage(m_sysinfoPageWidget);
}

void Widget::onSettingPageButtonClicked()
{
    if (m_settingPageWidget)
        switchToPage(m_settingPageWidget);
}

void Widget::onWifiPageButtonClicked()
{
    if (m_wifiPageWidget)
        switchToPage(m_wifiPageWidget);
}

// 收到 音量 状态变化信号
void Widget::onVolumeMuteStateChanged(bool isMuted)
{
    if (isMuted) // 静音
    {
        ui->soundLabelIcon->setPixmap(QPixmap(":/res/icon/staBarIcon/soundOff.png"));
    }
    else // 取消静音
    {
        ui->soundLabelIcon->setPixmap(QPixmap(":/res/icon/staBarIcon/soundOn.png"));
    }
}

// 收到 wifi 状态变化信号
void Widget::onWifiStatusChanged(bool connected)
{
    if (connected) // 已连接
    {
        ui->wifiLabelIcon->setPixmap(QPixmap(":/res/icon/staBarIcon/wifiConnect.png"));
    }
    else // 未连接
    {
        ui->wifiLabelIcon->setPixmap(QPixmap(":/res/icon/staBarIcon/wifiDisconnect.png"));
    }
}

// 收到北京时间更新
void Widget::onBjTimeUpdated()
{
    QDateTime bjTime = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Shanghai"));
    QString bjTimeStr = bjTime.toString("MMdd - HH:mm");
    ui->bjTimeLabel->setText(bjTimeStr);
}
