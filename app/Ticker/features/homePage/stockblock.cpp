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

    m_longPressTimer.setSingleShot(true);   // 单次触发
    m_longPressTimer.setInterval(LONG_PRESS_THRESHOLD_MS); // 长按阈值
    connect(&m_longPressTimer, &QTimer::timeout, this, &StockBlock::onLongPress);
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
    m_deleteBtn->show();
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

    if (priceChanged)
        breathOpacityOn(ui->priceLabel, m_priceOpacityEff);
    if (riseChanged)
        breathOpacityOn(ui->risePriceLabel, m_riseOpacityEff);
    if (pctChanged)
        breathOpacityOn(ui->risePctLabel, m_pctOpacityEff);
    if (priceChanged || riseChanged || pctChanged)
        breathOpacityOn(ui->riseIconLabel, m_iconOpacityEff);

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

    if (m_stockInfo.isRise == CALL_AUCTION)
    {
        ui->risePriceLabel->setText("0.00");
        ui->risePctLabel->setText("0.00");
        
        ui->priceLabel->setStyleSheet("color: #FFA500;"); // 橙色
        ui->risePriceLabel->setStyleSheet("color: #FFA500;");
        ui->risePctLabel->setStyleSheet("color: #FFA500;");
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/orangeCir.png"));
        return;
    }

    QString prefixSymbol = "";
    switch (m_stockInfo.isRise)
    {
        case NORMAL: prefixSymbol = "";  break;
        case RISE:   prefixSymbol = "+"; break;
        case FALL:   prefixSymbol = "-"; break;
    }
    ui->risePriceLabel->setText(QString("%1 %2").arg(prefixSymbol, QString::number(m_stockInfo.risePrice, 'f', 2)));
    ui->risePctLabel->setText(QString("%1 %2%").arg(prefixSymbol, QString::number(m_stockInfo.risePct, 'f', 2)));

    // 根据涨跌状态设置颜色
    if (m_stockInfo.isRise == RISE)     // 上涨
    {
        ui->priceLabel->setStyleSheet("color: #E33C64;");
        ui->risePriceLabel->setStyleSheet("color: #E33C64;");
        ui->risePctLabel->setStyleSheet("color: #E33C64;");

        // 上涨显示红色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/redCir.png"));
    }
    else if (m_stockInfo.isRise == FALL)    // 下跌
    {
        ui->priceLabel->setStyleSheet("color: #43CF7C;");
        ui->risePriceLabel->setStyleSheet("color: #43CF7C;");
        ui->risePctLabel->setStyleSheet("color: #43CF7C;");

        // 上涨显示绿色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/greenCir.png"));
    }
    else if (m_stockInfo.isRise == NORMAL)  // 不变
    {
        ui->priceLabel->setStyleSheet("color: #707070;");
        ui->risePriceLabel->setStyleSheet("color: #707070;");
        ui->risePctLabel->setStyleSheet("color: #707070;");

        // 不变显示灰色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/grayCir.png"));
    }
}

// 呼吸灯效果
void StockBlock::breathOpacityOn(QWidget *label, QGraphicsOpacityEffect*& eff)
{
    // 懒加载
    if (!eff)
    {
        eff = new QGraphicsOpacityEffect(label);
        label->setGraphicsEffect(eff);
    }
    eff->setOpacity(1.0);

    QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setKeyValueAt(0.0, 1.0);
    anim->setKeyValueAt(0.3, 0.2);  // 低透明
    anim->setKeyValueAt(0.7, 0.2);  // 低透明
    anim->setKeyValueAt(1.0, 1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 动画结束后恢复原状
    connect(anim, &QPropertyAnimation::finished, label, [eff]() {
        eff->setOpacity(1.0);
    });
}
