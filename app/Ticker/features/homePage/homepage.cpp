#include "homepage.h"
#include "ui_homepage.h"
#include "utils/log/logger.h"
#include "utils/qssload/qssloader.h"
#include "utils/confirmDialog/confirmdialog.h"
#include "features/pagemsgmanager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QScrollBar>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QSaveFile>
#include <QScroller>
#include <QAbstractItemView>

static QString getAllPortfolioFilePath()
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(baseDir); // 确保路径存在
    return baseDir + "/" + PORTFOLIO_SAVE_FILE_NAME;
}

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
    , m_quoteUpdateTimer(new QTimer(this))
{
    ui->setupUi(this);

    m_currentPortfolio = nullptr;

    // 连接行情更新定时器槽函数
    connect(m_quoteUpdateTimer, &QTimer::timeout, this, &HomePage::onQuoteUpdateTimerTimeout);
}

HomePage::~HomePage()
{
    delete ui;
}

// 页面进入回调
void HomePage::onPageEnter()
{
    LOG_DEBUG("HomePage entered.");

    emitFetchQuotesInCurPortfolio(); // 立即请求一次
    if (!m_quoteUpdateTimer->isActive())
    {
        LOG_DEBUG("start m_quoteUpdateTimer");
        m_quoteUpdateTimer->start(1000); // 1秒更新一次
    }
}

// 页面离开回调
void HomePage::onPageLeave()
{
    LOG_DEBUG("HomePage left.");
    if (m_quoteUpdateTimer->isActive())
    {
        LOG_DEBUG("stop m_quoteUpdateTimer");
        m_quoteUpdateTimer->stop();
    }
}

// 页面初始化
void HomePage::init()
{
    // UI 初始化
    uiInit();
    /*
     *  在 uiInit() 中，会初始化 combobox，从而导致 currentIndexChanged 信号触发，
     *  而 currentIndexChanged 里会获取最新行情，但此时还没有更新 combobox。
    */
    uiInitFlag = 1;

    // 安装滚动拖拽过滤器
    installScrollDragFilters();

    // 初始化行情提供者
    m_quoteProvider = new SinaQuoteProvider(this);
    connect(m_quoteProvider, &SinaQuoteProvider::quotesReady,
            this, &HomePage::onQuoteProviderUpdateQuotes);
    connect(m_quoteProvider, &SinaQuoteProvider::quoteError,
            this, &HomePage::onQuoteProviderError);

    // 连接 wifi 状态变化信号槽
    connect(PageMsgManager::getInstance(), &PageMsgManager::wifiStatusChanged,
            this, &HomePage::onWifiStatusChanged);

    // 从本地获取组合数据
    loadAllPortfoliosFromLocal();

    // 更新组合下拉框
    ui->portfolioComboBox->clear();
    for (int i = 0; i < m_portfolioList.size(); ++i)
    {
        const StockPortfolio* portfolio = m_portfolioList[i];
        ui->portfolioComboBox->addItem(portfolio->name());
    }

    // 自动切换到第一个组合
    if (!m_portfolioList.isEmpty())
    {
        m_currentPortfolio = m_portfolioList[0];
        ui->portfolioComboBox->setCurrentIndex(0);
    }
    else
    {
        m_currentPortfolio = nullptr;
    }

    // 初始时，交易日状态未知
    m_isOpenMarket = OPEN_STATUS_UNKNOWN;
}

bool HomePage::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *relevantWidget = qobject_cast<QWidget*>(obj);
    QWidget *contentWidget = ui->scrollArea->widget();

    bool isRelevant = (relevantWidget &&
                       (relevantWidget == contentWidget ||
                        relevantWidget->isAncestorOf(contentWidget) ||
                        contentWidget->isAncestorOf(relevantWidget)));

    if (isRelevant && m_horizontalScrollBar)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                m_isHDragging = false;
                m_isHContentDragging = false;
                m_hDragStartPos = mouseEvent->globalPos();
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            if (!m_hDragStartPos.isNull())
            {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                QPoint currentPos = mouseEvent->globalPos();
                int distance = std::abs(currentPos.x() - m_hDragStartPos.x());

                // 横向判别阈值
                if (!m_isHDragging && distance > m_hDragThreshold)
                {
                    m_isHDragging = true;
                    m_isHContentDragging = true;
                }

                if (m_isHContentDragging)
                {
                    int deltaX = currentPos.x() - m_hDragStartPos.x();
                    int newValue = m_horizontalScrollBar->value() - deltaX;
                    m_horizontalScrollBar->setValue(newValue);
                    // 更新起点，做“拖到哪跟到哪”的手感
                    m_hDragStartPos = currentPos;
                    return true;
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                bool wasDragging = m_isHContentDragging;
                m_isHDragging = false;
                m_isHContentDragging = false;
                m_hDragStartPos = QPoint();

                if (wasDragging)
                    return true;
            }
        }
        else if (event->type() == QEvent::Leave)
        {
            if (!m_hDragStartPos.isNull())
            {
                m_isHDragging = false;
                m_isHContentDragging = false;
                m_hDragStartPos = QPoint();
            }
        }
    }

    // 其他方向或事件，走默认parent逻辑
    return QWidget::eventFilter(obj, event);
}

// UI 初始化
void HomePage::uiInit()
{
    // 默认不显示输入错误提示语
    ui->inputErrHintLabel->setVisible(false);

    // 用户输入框初始化
    ui->inputLineEdit->setText("");

    // 默认清空标题语
    ui->titleLabel->setText("");
    ui->titleLabel->setVisible(false);

    // 默认开启显示网络错误提示
    ui->networkErrLabel->setVisible(true);
    startBreathAnimation(ui->networkErrLabel, m_networkErrAnimation);

    // 清除组合下拉框
    ui->portfolioComboBox->clear();
    ui->portfolioComboBox->setMaxVisibleItems(5);
    ui->portfolioComboBox->setStyleSheet("QComboBox{combobox-popup:0;}");
    QAbstractItemView *view = ui->portfolioComboBox->view();
    // 滚动条
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 触摸滑动
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    // 惯性滑动
    QScroller::grabGesture(view, QScroller::LeftMouseButtonGesture);

    // 按钮不显示聚焦框
    ui->addNewPortfolioBtn->setFocusPolicy(Qt::NoFocus);
    ui->addNewStockBtn->setFocusPolicy(Qt::NoFocus);
    ui->delPortfolioBtn->setFocusPolicy(Qt::NoFocus);

    // 设置水平布局
    QWidget *contentWidget = ui->scrollArea->widget();
    stockBlocksLayout = new QHBoxLayout(contentWidget);
    stockBlocksLayout->setSpacing(18);
    stockBlocksLayout->setContentsMargins(18, 5, 18, 5);    // left top right bottom
    stockBlocksLayout->setAlignment(Qt::AlignLeft);

    // 去除边框和阴影
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->scrollArea->setFrameShadow(QFrame::Plain);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 加载样式表
    QString qss = QssLoader::load(":/res/qss/pageQss/homePage.qss");
    if (!qss.isEmpty())
        this->setStyleSheet(qss);
}

// 获取股票代码合法性，合法返回 0，非法返回 -1 或 -2
int HomePage::isStockCode(const QString& rawInput, QString& outNormalizedCode)
{
    QString code = rawInput.trimmed();

    // 必须是6位纯数字
    if (code.length() != 6 || !QRegularExpression(R"(^\d+$)").match(code).hasMatch())
    {
        return STOCKCODE_ERR_LEN_INVALID;
    }

    // 判断市场类型并生成后缀
    if (code.startsWith("60") || code.startsWith("688") || code.startsWith("689"))
    {
        outNormalizedCode = code + ".SH";   // 上海交易所
    }
    else if (code.startsWith("00") || code.startsWith("30"))
    {
        outNormalizedCode = code + ".SZ";   // 深圳交易所
    }
    else if (code.startsWith("83") || code.startsWith("87") || code.startsWith("88"))
    {
        outNormalizedCode = code + ".BJ";   // 北京交易所
    }
    else
    {
        return STOCKCODE_ERR_ILLEGAL;
    }

    return 0;
}

// 获取组合索引
int HomePage::getIndexOfPortfolio(const QString& name)
{
    for (int i = 0; i < m_portfolioList.size(); ++i)
    {
        if (m_portfolioList[i]->name() == name)
        {
            return i;
        }
    }
    return -1;
}

// 添加新股票按钮点击槽函数
void HomePage::on_addNewStockBtn_clicked()
{
    QString newCode;

    // 如果当前一个组合都没有，直接返回
    if (m_currentPortfolio == nullptr)
    {
        ui->inputErrHintLabel->setText("请先创建股票组合！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }

    // 检查股票代码合法性
    int ret = isStockCode(ui->inputLineEdit->text(), newCode);
    if (ret == STOCKCODE_ERR_LEN_INVALID)
    {
        ui->inputErrHintLabel->setText("请输入6位数字代码！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }
    else if (ret == STOCKCODE_ERR_ILLEGAL)
    {
        ui->inputErrHintLabel->setText("请输入合法代码！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }

    // 如果代码已存在于当前组合，提示错误
    if (m_currentPortfolio->getStock(newCode) != nullptr)
    {
        ui->inputErrHintLabel->setText("股票已存在当前组合！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }

    // 添加股票到当前组合
    m_currentPortfolio->addStock(StockInfo(newCode));
    // LOG_DEBUG("%s", m_currentPortfolio->getStock(newCode)->code.toStdString().c_str());

    // 清除输入框和错误提示
    ui->inputLineEdit->setText("");
    ui->inputErrHintLabel->setVisible(false);

    // 手动触发一次数据请求，避免因为在收盘时间内，导致新加股票不显示数据
    m_quoteProvider->fetchQuote(newCode);

    // 更新股票块显示
    updateStockBlocks();

    // 保存所有组合到本地
    saveAllPortfoliosToLocal();
}

// 添加组合按钮点击槽函数
void HomePage::on_addNewPortfolioBtn_clicked()
{
    // 检查用户输入，最多不可超过 7 个字符
    QString portfolioName = ui->inputLineEdit->text().trimmed();
    if (portfolioName.isEmpty())
    {
        ui->inputErrHintLabel->setText("组合名不能为空！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }
    if (portfolioName.length() > PORTFOLIO_NAME_MAX_LEN)
    {
        ui->inputErrHintLabel->setText("组合名不能超过7个字符！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }

    // 检查是否有已存在的组合
    int size = m_portfolioList.size();
    for (int i = 0; i < size; i++)
    {
        if (m_portfolioList[i]->name() == portfolioName)
        {
            ui->inputErrHintLabel->setText("组合名已存在！");
            ui->inputErrHintLabel->setVisible(true);
            return;
        }
    }

    // 创建新组合并添加到列表
    StockPortfolio* newPortfolio = new StockPortfolio(portfolioName, this);
    m_portfolioList.append(newPortfolio);

    // 更新组合下拉框
    ui->portfolioComboBox->addItem(portfolioName);

    // 切换到新创建的组合
    int index = getIndexOfPortfolio(portfolioName);
    ui->portfolioComboBox->setCurrentIndex(index);
    m_currentPortfolio = newPortfolio;

    // 清除输入框和错误提示
    ui->inputLineEdit->setText("");
    ui->inputErrHintLabel->setVisible(false);

    // 保存所有组合到本地
    saveAllPortfoliosToLocal();
}

// 组合下拉框索引变化槽函数
void HomePage::on_portfolioComboBox_currentIndexChanged(int index)
{
    // 根据下拉框选择切换当前组合
    if (index >= 0 && index < m_portfolioList.size())
    {
        m_currentPortfolio = m_portfolioList[index];
    }
    else
    {
        m_currentPortfolio = nullptr;
    }

    // 更新股票块显示
    updateStockBlocks();

    // 请求当前组合下所有股票的最新行情
    emitFetchQuotesInCurPortfolio();
}

// 删除组合按钮点击槽函数
void HomePage::on_delPortfolioBtn_clicked()
{
    // 如果当前没有组合，直接返回
    int index = ui->portfolioComboBox->currentIndex();
    if (index < 0 || index >= m_portfolioList.size())
    {
        return;
    }

    // 弹出确认对话框
    if (!ConfirmDialog::ask(this, "是否删除该组合？"))
    {
        // 用户取消删除
        return;
    }

    // 删除组合
    StockPortfolio* toDelete = m_portfolioList[index];
    m_portfolioList.removeAt(index);
    delete toDelete;

    // 更新组合下拉框
    ui->portfolioComboBox->removeItem(index);

    // 切换当前组合
    if (!m_portfolioList.isEmpty())
    {
        // 如果删除的是最后一个组合，切换到前一个
        if (index >= m_portfolioList.size())
        {
            index = m_portfolioList.size() - 1;
        }
        ui->portfolioComboBox->setCurrentIndex(index);
        m_currentPortfolio = m_portfolioList[index];
    }
    else
    {
        m_currentPortfolio = nullptr;
    }

    // 保存所有组合到本地
    saveAllPortfoliosToLocal();
}

// 重命名组合按钮槽函数
void HomePage::on_renameBtn_clicked()
{
    // 如果当前没有组合，直接返回
    int index = ui->portfolioComboBox->currentIndex();
    if (index < 0 || index >= m_portfolioList.size())
    {
        ui->inputErrHintLabel->setText("没有可重命名的组合！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }

    // 检查用户输入，最多不可超过 7 个字符
    QString newName = ui->inputLineEdit->text().trimmed();
    if (newName.isEmpty())
    {
        ui->inputErrHintLabel->setText("组合名不能为空！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }
    if (newName.length() > PORTFOLIO_NAME_MAX_LEN)
    {
        ui->inputErrHintLabel->setText("组合名不能超过7个字符！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }

    // 检查是否有已存在的组合
    int size = m_portfolioList.size();
    for (int i = 0; i < size; i++)
    {
        if (i != index && m_portfolioList[i]->name() == newName)
        {
            ui->inputErrHintLabel->setText("组合名已存在！");
            ui->inputErrHintLabel->setVisible(true);
            return;
        }
    }

    // 重命名当前组合
    StockPortfolio* currentPortfolio = m_portfolioList[index];
    currentPortfolio->setName(newName);

    // 更新组合下拉框显示
    ui->portfolioComboBox->setItemText(index, newName);

    // 清除输入框和错误提示
    ui->inputLineEdit->setText("");
    ui->inputErrHintLabel->setVisible(false);

    // 保存所有组合到本地
    saveAllPortfoliosToLocal();
}

// 打印所有组合名称及其下的股票代码
void HomePage::printAllPortfolioList()
{
    LOG_DEBUG("Current Portfolios:");
    for (int i = 0; i < m_portfolioList.size(); ++i)
    {
        const StockPortfolio* portfolio = m_portfolioList[i];
        LOG_DEBUG("Portfolio: %s", portfolio->name().toStdString().c_str());
        QList<StockInfo> stocks = portfolio->stocks();
        for (int j = 0; j < stocks.size(); ++j)
        {
            const StockInfo& stock = stocks[j];
            LOG_DEBUG("  Stock Code: %s", stock.code.toStdString().c_str());
        }
    }
}

// 打印当前组合名称及其下的股票列表
void HomePage::printCurrentPortfolioStocks()
{
    if (m_currentPortfolio == nullptr)
    {
        LOG_DEBUG("No current portfolio selected.");
        return;
    }
    LOG_DEBUG("Current Portfolio: %s", m_currentPortfolio->name().toStdString().c_str());
    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    for (int i = 0; i < stocks.size(); ++i)
    {
        const StockInfo& stock = stocks[i];
        LOG_DEBUG("  Stock Code: %s", stock.code.toStdString().c_str());
    }
}

// 保存所有组合到本地（原子写）
void HomePage::saveAllPortfoliosToLocal()
{
    QJsonArray arr;

    for (int i = 0; i < m_portfolioList.size(); ++i)
    {
        StockPortfolio* portfolio = m_portfolioList[i];
        if (!portfolio)
            continue;

        QJsonObject obj;
        obj["name"] = portfolio->name();

        QJsonArray stockArray;
        QList<StockInfo> stockList = portfolio->stocks();
        for (int j = 0; j < stockList.size(); ++j)
        {
            const StockInfo& s = stockList[j];
            QJsonObject st;
            st["code"] = s.code;
            st["name"] = s.name;
            stockArray.append(st);
        }

        obj["stocks"] = stockArray;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QString path = getAllPortfolioFilePath();

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG_WARN("Failed to open portfolio file: %s", path.toStdString().c_str());
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));

    if (!file.commit())
    {
        LOG_WARN("Failed to commit portfolio file: %s", path.toStdString().c_str());
        return;
    }

    LOG_INFO("All portfolios saved and synced, path: %s", path.toStdString().c_str());
}

// 从本地加载所有组合
void HomePage::loadAllPortfoliosFromLocal()
{
    QString path = getAllPortfolioFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_WARN("Failed to open portfolio file: %s", path.toStdString().c_str());
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray())
    {
        LOG_WARN("Failed to parse portfolio file: %s", path.toStdString().c_str());
        return;
    }

    QJsonArray arr = doc.array();

    // 先清空
    for (int i = 0; i < m_portfolioList.size(); ++i)
        delete m_portfolioList[i];
    m_portfolioList.clear();

    // 加载所有组合
    for (int i = 0; i < arr.size(); ++i)
    {
        QJsonObject obj = arr[i].toObject();
        QString pname = obj["name"].toString();
        if (pname.isEmpty()) continue;
        StockPortfolio* port = new StockPortfolio(pname, this);
        QJsonArray stockArray = obj["stocks"].toArray();
        for (int j = 0; j < stockArray.size(); ++j)
        {
            QJsonObject st = stockArray[j].toObject();
            StockInfo s;
            s.code = st["code"].toString();
            s.name = st["name"].toString();
            port->addStock(s);
        }
        m_portfolioList.append(port);
    }

    LOG_INFO("All portfolios loaded from local, path: %s", path.toStdString().c_str());
    printAllPortfolioList();
}

// 刷新当前组合下已有股票的信息显示
void HomePage::refreshStockInfoDisplay()
{
    if (!m_currentPortfolio)
        return;

    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    int n = std::min(stocks.size(), stockBlocksLayout->count());
    for (int i = 0; i < n; ++i)
    {
        QLayoutItem* item = stockBlocksLayout->itemAt(i);
        StockBlock *block = qobject_cast<StockBlock*>(item->widget());
        if (block)
            block->setStockInfo(stocks[stocks.size() - 1 - i]);  // 新在左
    }
}

// 更新当前组合下的股票块
void HomePage::updateStockBlocks()
{
    // 清除原有所有股票块
    QLayoutItem *item;
    while ((item = stockBlocksLayout->takeAt(0)) != nullptr)
    {
        QWidget *widget = item->widget();
        if (widget)
            delete widget;
        delete item;
    }
    stockBlocksLayout->update();

    if (!m_currentPortfolio)
        return;

    // 遍历当前组合，生成新的股票块
    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    for (int i = 0; i < stocks.size(); ++i)
    {
        const StockInfo &info = stocks[i];
        LOG_DEBUG("stock status: %d", info.status);
        StockBlock *block = new StockBlock(ui->scrollArea->widget());
        // 连接删除股票块请求槽函数
        connect(block, &StockBlock::deleteRequested, this, &HomePage::onDelStockBlockRequested);
        block->setStockInfo(info);
        stockBlocksLayout->insertWidget(0, block); // 新的在左
    }
}

// 安装滚动拖拽事件过滤器
void HomePage::installScrollDragFilters()
{
    m_horizontalScrollBar = ui->scrollArea->horizontalScrollBar();
    QWidget *contentWidget = ui->scrollArea->widget();

    if (!contentWidget || !m_horizontalScrollBar)
    {
        LOG_WARN("ScrollArea or its content widget is null, cannot install drag filters.");
        return;
    }

    contentWidget->installEventFilter(this);
    QList<QPushButton*> buttons = contentWidget->findChildren<QPushButton*>();
    for (QPushButton* button : qAsConst(buttons))
    {
        button->installEventFilter(this);
    }
}

// 接收最新行情槽函数
void HomePage::onQuoteProviderUpdateQuotes(const QList<StockInfo> &infos)
{
    if (!m_currentPortfolio)
        return; 

    // 更新当前组合内的股票数据
    StockInfo* stock = nullptr;
    for (const StockInfo& updatedInfo : infos)
    {
        stock = m_currentPortfolio->getStock(updatedInfo.code);
        if (stock)
        {
            stock->name = updatedInfo.name;
            stock->currentPrice = updatedInfo.currentPrice;
            stock->risePrice = updatedInfo.risePrice;
            stock->risePct = updatedInfo.risePct;
            stock->isRise = updatedInfo.isRise;
            stock->marketDate = updatedInfo.marketDate;     // 2025-12-31
            stock->status = updatedInfo.status;
        }

        // 如果该股票是停牌或退市，则不能以当前日期判断交易日状态
        if (stock && (stock->status == STATUS_SUSPEND || stock->status == STATUS_DELISTED))
        {
            // not todo
        }
        else 
        {
            // 交易日判断
            // 若当前日期与股票的最后获取时间一致，则说明今天是交易日
            QString currentDateStr = QDate::currentDate().toString("yyyy-MM-dd");
            if (stock && stock->marketDate == currentDateStr)
            {
                // LOG_DEBUG("Market is open today, date: %s", currentDateStr.toStdString().c_str());
                m_isOpenMarket = OPEN_STATUS_OPEN;
            }
            else
            {
                // LOG_DEBUG("Market is closed today, date: %s", currentDateStr.toStdString().c_str());
                m_isOpenMarket = OPEN_STATUS_CLOSED;
            }
        }
    }

    // 刷新显示
    refreshStockInfoDisplay();
}

// 行情错误槽函数
void HomePage::onQuoteProviderError(const QString &stockCode, const QString &errReason)
{
    LOG_DEBUG("Quote error for %s: %s", stockCode.toStdString().c_str(), errReason.toStdString().c_str());
}

// 行情定时请求更新槽函数，只要在 homepage 页，该函数会一直定时被执行
void HomePage::onQuoteUpdateTimerTimeout()
{
    // 刷新标题标签显示
    refreshTitleLabel();

    if (!m_currentPortfolio)
        return;

    /*  只在开盘时间段内请求数据
        上午：09:14 ~ 11:31（含 11:31 整）；上下多加1分钟缓冲，避免时间点切换时请求遗漏
        下午：12:59 ~ 15:01（含 15:01 整）；上下多加1分钟缓冲，避免时间点切换时请求遗漏
    */
    QTime currentTime = QTime::currentTime();
    // 定义有效时间段（包含端点）
    QTime tradingDayStart(9, 14);
    QTime tradingDayEnd(15, 1);   
    bool intradingDay = (currentTime >= tradingDayStart) && (currentTime <= tradingDayEnd);
    if (!intradingDay) // 不在有效时段
    {
        LOG_DEBUG("Not in trading time, skip fetching quotes.");
        m_isOpenMarket = OPEN_STATUS_UNKNOWN;
        return;
    }

    // 请求行情更新
    if (m_isOpenMarket != OPEN_STATUS_CLOSED)
    {
        LOG_DEBUG("fetching quotes ...");
        emitFetchQuotesInCurPortfolio();
    }
}

// 股票块删除请求槽函数
void HomePage::onDelStockBlockRequested(const QString &code)
{
    if (!m_currentPortfolio)
        return;

    // 从当前组合删除股票
    bool success = m_currentPortfolio->delStock(code);
    if (success)
    {
        // 更新股票块显示
        updateStockBlocks();

        // 保存所有组合到本地
        saveAllPortfoliosToLocal();

        // 打印
        printCurrentPortfolioStocks();
    }
}

// WiFi 状态变化槽函数
void HomePage::onWifiStatusChanged(bool isConnected)
{
    // 如果 wifi 状态变化了，才进行处理
    if ((isConnected && m_wifiLastStatus == 1) ||
        (!isConnected && m_wifiLastStatus == 0))
    {
        return;
    }

    m_wifiLastStatus = isConnected ? 1 : 0;

    if (isConnected)
    {
        ui->networkErrLabel->setVisible(false);
        // 停止网络错误动画
        stopBreathAnimation(ui->networkErrLabel, m_networkErrAnimation);
        // 立即请求一次行情
        emitFetchQuotesInCurPortfolio();
    }
    else
    {
        ui->networkErrLabel->setVisible(true);
        // 启动网络错误动画
        startBreathAnimation(ui->networkErrLabel, m_networkErrAnimation);
    }
}

// 启动呼吸灯动画
void HomePage::startBreathAnimation(QWidget *target, QSequentialAnimationGroup *&holder)
{
    if (holder)
        return;

    auto *effect = new QGraphicsOpacityEffect(target);
    target->setGraphicsEffect(effect);

    auto *fadeOut = new QPropertyAnimation(effect, "opacity", target);
    fadeOut->setDuration(1500);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    auto *fadeIn = new QPropertyAnimation(effect, "opacity", target);
    fadeIn->setDuration(1500);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    auto *seq = new QSequentialAnimationGroup(target);
    seq->addAnimation(fadeOut);
    seq->addAnimation(fadeIn);
    seq->setLoopCount(-1);
    seq->start();

    holder = seq;
}

// 停止呼吸灯动画
void HomePage::stopBreathAnimation(QWidget *target, QSequentialAnimationGroup *&holder)
{
    if (holder)
    {
        holder->stop();
        holder->deleteLater();
        target->setGraphicsEffect(nullptr);
        holder = nullptr;
    }
}

// 刷新标题标签显示
void HomePage::refreshTitleLabel()
{
    // 只在有组合和有股票时，显示 titleLabel
    if (!m_currentPortfolio || m_currentPortfolio->stocks().isEmpty())
    {
        ui->titleLabel->setVisible(false);
        return;
    }
    else
    {
        ui->titleLabel->setVisible(true);
    }

    if (m_isOpenMarket == OPEN_STATUS_OPEN)
    {
        // 获取当前系统时间
        QTime currentTime = QTime::currentTime();
        int hour = currentTime.hour();
        int minute = currentTime.minute();

        /*
            时间段划分：
            1、09:15 – 09:25：开盘集合竞价（橙色背景，呼吸灯动画）
            2、09:25 – 09:30：开市前休市（橙色背景，常亮）
            3、09:30 – 11:30：连续竞价（绿色背景，呼吸灯动画）
            4、11:30 – 13:00：午间休市（灰色背景，常亮）
            5、13:00 – 14:57：连续竞价（绿色背景，呼吸灯动画）
            6、14:57 – 15:00：收盘集合竞价（橙色背景，呼吸灯动画）
            7、15:00 起：已收盘（灰色背景，常亮）
        */

        // 1. 开盘集合竞价：09:15 – 09:24（含）
        if (hour == 9 && minute >= 15 && minute <= 24)
        {
            ui->titleLabel->setText("开盘集合竞价");
            ui->titleLabel->setStyleSheet("background-color: #FFA500; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            startBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        // 2. 开市前休市：09:25 – 09:29（含）
        else if (hour == 9 && minute >= 25 && minute <= 29)
        {
            ui->titleLabel->setText("开市前休市");
            ui->titleLabel->setStyleSheet("background-color: #FFA500; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            stopBreathAnimation(ui->titleLabel, m_titleBreathAnimation); // 常亮，不呼吸
        }
        // 3. 上午连续竞价：09:30 – 11:29（含）
        else if ((hour == 9 && minute >= 30) ||
                (hour == 10) ||
                (hour == 11 && minute <= 29))
        {
            ui->titleLabel->setText("交易进行中");
            ui->titleLabel->setStyleSheet("background-color: #28a745; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            startBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        // 4. 午间休市：11:30 – 12:59（含）
        else if ((hour == 11 && minute >= 30) ||
                (hour == 12))
        {
            ui->titleLabel->setText("午间休市");
            ui->titleLabel->setStyleSheet("background-color: #808080; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            stopBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        // 5. 下午连续竞价：13:00 – 14:56（含）
        else if ((hour == 13) ||
                (hour == 14 && minute <= 56))
        {
            ui->titleLabel->setText("交易进行中");
            ui->titleLabel->setStyleSheet("background-color: #28a745; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            startBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        // 6. 收盘集合竞价：14:57 – 14:59（含）
        else if (hour == 14 && minute >= 57 && minute <= 59)
        {
            ui->titleLabel->setText("收盘集合竞价");
            ui->titleLabel->setStyleSheet("background-color: #FFA500; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            startBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        // 7. 已收盘：15:00 起
        else if (hour >= 15)
        {
            ui->titleLabel->setText("已收盘");
            ui->titleLabel->setStyleSheet("background-color: #808080; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            stopBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        // 其他时间（如夜间、早盘前）：默认“已收盘”或可自定义
        else
        {
            ui->titleLabel->setText("已收盘");
            ui->titleLabel->setStyleSheet("background-color: #808080; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
            stopBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        }
        ui->titleLabel->setVisible(true);
    }
    else if (m_isOpenMarket == OPEN_STATUS_CLOSED)
    {
        ui->titleLabel->setText("已收盘");
        ui->titleLabel->setStyleSheet("background-color: #808080; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
        stopBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        ui->titleLabel->setVisible(true);
    }
    else if (m_isOpenMarket == OPEN_STATUS_UNKNOWN)
    {
        ui->titleLabel->setText("已收盘");
        ui->titleLabel->setStyleSheet("background-color: #808080; color: #ffffff; border-radius: 4px; padding: 4px 8px; font-size: 16px;");
        stopBreathAnimation(ui->titleLabel, m_titleBreathAnimation);
        ui->titleLabel->setVisible(true);
    }
}

// 触发行情请求
void HomePage::emitFetchQuotesInCurPortfolio()
{
    if (uiInitFlag == 0)
        return;

    if (!m_currentPortfolio)
        return;

    // 收集当前组合内的股票代码
    QStringList codes;
    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    for (const StockInfo& stock : stocks)
    {
        codes.append(stock.code);
    }

    m_quoteProvider->fetchQuotes(codes);
}
