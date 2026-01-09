#include "stockblock.h"
#include "ui_stockblock.h"
#include "utils/log/logger.h"
#include "utils/qssload/qssloader.h"
#include <QMouseEvent>

StockBlock::StockBlock(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StockBlock)
{
    ui->setupUi(this);

    // 加载样式表
    QString qss = QssLoader::load(":/res/qss/pageQss/stockBlock.qss");
    if (!qss.isEmpty())
        this->setStyleSheet(qss);

    m_longPressTimer.setSingleShot(true);                   // 单次触发
    m_longPressTimer.setInterval(LONG_PRESS_THRESHOLD_MS);  // 长按阈值
    connect(&m_longPressTimer, &QTimer::timeout, this, &StockBlock::onLongPress);

    ui->susDelistLabel->setText("");                        // 设置停牌/摘牌文本
    ui->susDelistLabel->setVisible(false);                  // 默认不可见
}

StockBlock::~StockBlock()
{
    delete ui;
}

// 创建删除按钮
void StockBlock::ensureDeleteBtn()
{
    if (m_deleteBtn)
        return;

    m_deleteBtn = new QPushButton("删除", this);
    m_deleteBtn->hide();        // 默认隐藏
    m_deleteBtn->show();        // 置顶显示

    m_deleteBtn->setFocusPolicy(Qt::NoFocus);   // 不显示聚焦框
    m_deleteBtn->setFixedWidth(60);             // 固定宽度
    m_deleteBtn->setStyleSheet("background:#fa5555;color:white;border:none;border-radius:5px;padding:3px 12px;");
    m_deleteBtn->raise();       // 置顶显示
    m_deleteBtn->move(width() - m_deleteBtn->width() - 15, 8); // 移动到右上角

    connect(m_deleteBtn, &QPushButton::clicked, this, &StockBlock::onDelBtnPress);
}

// 鼠标按下事件
void StockBlock::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_longPressTimer.start();
    }
    QWidget::mousePressEvent(event);
}

// 鼠标释放事件
void StockBlock::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_longPressTimer.isActive())
        {
            m_longPressTimer.stop();

            // 普通点击行为，可以添加点击行为
            // todo
        }
    }
    QWidget::mouseReleaseEvent(event);
}

// 鼠标离开事件
void StockBlock::leaveEvent(QEvent *event)
{
    // 鼠标一离开自己控件区域，自动隐藏删除按钮
    if (m_deleteBtn)
    {
        m_deleteBtn->hide();
    }

    if (m_longPressTimer.isActive())
    {
        m_longPressTimer.stop();
    }
    QWidget::leaveEvent(event);
}

// 长按处理槽函数
void StockBlock::onLongPress()
{
    ensureDeleteBtn();
    showDeleteBtnWithAnim();
}

// 删除按钮点击槽函数
void StockBlock::onDelBtnPress()
{
    m_deleteBtn->hide();
    emit deleteRequested(m_stockInfo.code);
}

// 设置股票信息并更新 UI
void StockBlock::setStockInfo(const StockInfo &info)
{
    bool priceChanged = (info.currentPrice != m_lastPrice);
    bool riseChanged = (info.risePrice != m_lastRise);
    bool pctChanged  = (info.risePct != m_lastRisePct);

    // 更新 UI
    m_stockInfo = info;
    updateUI();

    if (priceChanged && riseChanged && pctChanged && m_stockInfo.isRise == PRICE_RISE)
    {
        flashColorOn(ui->priceLabel, QColor("#FF8FA3"), QColor("#E33C64"));
        flashColorOn(ui->risePriceLabel, QColor("#FF8FA3"), QColor("#E33C64"));
        flashColorOn(ui->risePctLabel, QColor("#FF8FA3"), QColor("#E33C64"));
    }
    else if (priceChanged && riseChanged && pctChanged && m_stockInfo.isRise == PRICE_FALL)
    {
        flashColorOn(ui->priceLabel, QColor("#9FF0C8"),QColor("#43CF7C"));
        flashColorOn(ui->risePriceLabel, QColor("#9FF0C8"),QColor("#43CF7C"));
        flashColorOn(ui->risePctLabel, QColor("#9FF0C8"),QColor("#43CF7C"));
    }

    m_lastPrice = info.currentPrice;
    m_lastRise = info.risePrice;
    m_lastRisePct = info.risePct;
}

// 更新 UI 显示
void StockBlock::updateUI()
{
    ui->codeLabel->setText(m_stockInfo.code);   // 更新股票代码
    ui->nameLabel->setText(m_stockInfo.name);   // 更新股票名称

    ui->priceLabel->setText(QString::number(m_stockInfo.currentPrice, 'f', 2)); // 当前价格

    // 股票状态处理
    ui->susDelistLabel->setVisible(false);
    if (m_stockInfo.status != STATUS_NORMAL)
    {
        ui->risePriceLabel->setText("--");
        ui->risePctLabel->setText("--");
        ui->priceLabel->setStyleSheet("color: #707070;");
        ui->risePriceLabel->setStyleSheet("color: #707070;");
        ui->risePctLabel->setStyleSheet("color: #707070;");
        ui->susDelistLabel->setStyleSheet("color: #707070;");
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/grayCir.png"));
        ui->susDelistLabel->setVisible(true);
    }
    if (m_stockInfo.status == STATUS_SUSPEND)   // 停牌
    {
        ui->susDelistLabel->setText("停牌中");
        return;
    }
    else if (m_stockInfo.status == STATUS_DELISTED)     // 摘牌
    {
        ui->susDelistLabel->setText("已退市");
        return;
    }

    QString prefixSymbol = "";
    switch (m_stockInfo.isRise)
    {
        case PRICE_NORMAL: prefixSymbol = "";  break;
        case PRICE_RISE:   prefixSymbol = "+"; break;
        case PRICE_FALL:   prefixSymbol = "-"; break;
    }
    ui->risePriceLabel->setText(QString("%1 %2").arg(prefixSymbol, QString::number(m_stockInfo.risePrice, 'f', 2)));
    ui->risePctLabel->setText(QString("%1 %2%").arg(prefixSymbol, QString::number(m_stockInfo.risePct, 'f', 2)));

    // 根据涨跌状态设置颜色
    if (m_stockInfo.isRise == PRICE_RISE)     // 上涨 
    {
        ui->priceLabel->setStyleSheet("color: #E33C64;");
        ui->risePriceLabel->setStyleSheet("color: #E33C64;");
        ui->risePctLabel->setStyleSheet("color: #E33C64;");

        // 上涨显示红色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/redCir.png"));
    }
    else if (m_stockInfo.isRise == PRICE_FALL)    // 下跌
    {
        ui->priceLabel->setStyleSheet("color: #43CF7C;");
        ui->risePriceLabel->setStyleSheet("color: #43CF7C;");
        ui->risePctLabel->setStyleSheet("color: #43CF7C;");

        // 上涨显示绿色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/greenCir.png"));
    }
    else if (m_stockInfo.isRise == PRICE_NORMAL)  // 不变
    {
        ui->priceLabel->setStyleSheet("color: #707070;");
        ui->risePriceLabel->setStyleSheet("color: #707070;");
        ui->risePctLabel->setStyleSheet("color: #707070;");

        // 不变显示灰色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/grayCir.png"));
    }
}

// 删除按钮的缩放动画
void StockBlock::showDeleteBtnWithAnim()
{
    if (!m_deleteBtn)
        return;

    // 透明度效果（懒加载）
    if (!m_delOpacityEff)
    {
        m_delOpacityEff = new QGraphicsOpacityEffect(m_deleteBtn);
        m_deleteBtn->setGraphicsEffect(m_delOpacityEff);
    }

    // 初始状态
    m_delOpacityEff->setOpacity(0.0);

    const int btnW = m_deleteBtn->width();
    const int btnH = m_deleteBtn->height();
    const QPoint finalPos = m_deleteBtn->pos();

    // 90% 缩放的起始矩形
    QRect startRect(
        finalPos.x() + btnW * 0.05,
        finalPos.y() + btnH * 0.05,
        btnW * 0.9,
        btnH * 0.9
    );

    QRect endRect(finalPos, QSize(btnW, btnH));

    m_deleteBtn->setGeometry(startRect);
    m_deleteBtn->show();
    m_deleteBtn->raise();

    // 透明度动画
    m_delOpacityAnim = new QPropertyAnimation(m_delOpacityEff, "opacity", this);
    m_delOpacityAnim->setDuration(180);
    m_delOpacityAnim->setStartValue(0.0);
    m_delOpacityAnim->setEndValue(1.0);
    m_delOpacityAnim->setEasingCurve(QEasingCurve::OutCubic);

    // 缩放动画（geometry）
    m_delScaleAnim = new QPropertyAnimation(m_deleteBtn, "geometry", this);
    m_delScaleAnim->setDuration(220);
    m_delScaleAnim->setStartValue(startRect);
    m_delScaleAnim->setEndValue(endRect);
    m_delScaleAnim->setEasingCurve(QEasingCurve::OutBack);

    // 启动
    m_delOpacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    m_delScaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

// 文字颜色闪烁效果
void StockBlock::flashColorOn(QLabel *label, const QColor &flashColor, const QColor &normalColor, int durationMs)
{
    if (!label)
        return;

    // 使用 dynamic property 承载颜色
    label->setProperty("animColor", normalColor);

    auto *anim = new QPropertyAnimation(label, "animColor", label);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::InOutQuad);

    anim->setKeyValueAt(0.0, normalColor);
    anim->setKeyValueAt(0.3, flashColor);
    anim->setKeyValueAt(1.0, normalColor);

    // 颜色变化时更新样式
    connect(anim, &QPropertyAnimation::valueChanged, this,
            [label](const QVariant &value) {
                QColor c = value.value<QColor>();
                label->setStyleSheet(QString("color:%1;").arg(c.name()));
            });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
