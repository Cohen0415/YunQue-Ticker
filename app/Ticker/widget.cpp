#include "widget.h"
#include "ui_widget.h"

#include <QPushButton>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 初始化 UI
    UiInit();

    // 初始化 stackedWidget 页面
    StackedWidgetPageInit();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::UiInit()
{
    // 禁用标题栏
    // this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    // 不显示鼠标
    QWidget::setCursor(QCursor(Qt::BlankCursor));

    // 初始化菜单栏 UI
    MenuBarUIInit();

    // 初始化时间 UI
    BJTimeUIInit();

    // 初始化信号槽
    ConnectSignalAndSlot();
}

void Widget::MenuBarUIInit()
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

    // 添加 系统信息页面 按钮
    m_sysinfoPageBtn = new QPushButton(contentWidget);
    m_sysinfoPageBtn->setIcon(QIcon(":/res/icon/menuIcon/sysinfo.png"));
    m_sysinfoPageBtn->setIconSize(QSize(64, 64));
    m_sysinfoPageBtn->setFixedSize(64, 64);
    m_sysinfoPageBtn->setFlat(true);

    // 添加 设置信息页面 按钮
    m_settingPageBtn = new QPushButton(contentWidget);
    m_settingPageBtn->setIcon(QIcon(":/res/icon/menuIcon/setting.png"));
    m_settingPageBtn->setIconSize(QSize(64, 64));
    m_settingPageBtn->setFixedSize(64, 64);
    m_settingPageBtn->setFlat(true);

    // 添加 wifi页面 按钮
    m_wifiPageBtn = new QPushButton(contentWidget);
    m_wifiPageBtn->setIcon(QIcon(":/res/icon/menuIcon/wifi.png"));
    m_wifiPageBtn->setIconSize(QSize(64, 64));
    m_wifiPageBtn->setFixedSize(64, 64);
    m_wifiPageBtn->setFlat(true);

    // scrollWidget 添加按钮
    menuScrollWidgetLayout->addWidget(m_homePageBtn);
    menuScrollWidgetLayout->addWidget(m_sysinfoPageBtn);
    menuScrollWidgetLayout->addWidget(m_settingPageBtn);
    menuScrollWidgetLayout->addWidget(m_wifiPageBtn);

    // 去除边框和阴影
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->scrollArea->setFrameShadow(QFrame::Plain);
}

void Widget::BJTimeUIInit()
{
    ui->bjTimeLabel->setText("1219 - 13:53");
}

void Widget::StackedWidgetPageInit()
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
    ui->stackedWidget->setCurrentWidget(m_homePageWidget);
}

void Widget::ConnectSignalAndSlot()
{
    if (!m_homePageBtn || !m_sysinfoPageBtn || !m_settingPageBtn || !m_wifiPageBtn)
    {
        return;
    }

    connect(m_homePageBtn, &QPushButton::clicked, this, &Widget::onHomePageBtnClicked);
    connect(m_sysinfoPageBtn, &QPushButton::clicked, this, &Widget::onSysinfoPageBtnClicked);
    connect(m_settingPageBtn, &QPushButton::clicked, this, &Widget::onSettingPageBtnClicked);
    connect(m_wifiPageBtn, &QPushButton::clicked, this, &Widget::onWifiPageBtnClicked);
}

void Widget::onHomePageBtnClicked()
{
    if (m_homePageWidget)
        ui->stackedWidget->setCurrentWidget(m_homePageWidget);
}

void Widget::onSysinfoPageBtnClicked()
{
    if (m_sysinfoPageWidget)
        ui->stackedWidget->setCurrentWidget(m_sysinfoPageWidget);
}

void Widget::onSettingPageBtnClicked()
{
    if (m_settingPageWidget)
        ui->stackedWidget->setCurrentWidget(m_settingPageWidget);
}

void Widget::onWifiPageBtnClicked()
{
    if (m_wifiPageWidget)
        ui->stackedWidget->setCurrentWidget(m_wifiPageWidget);
}
