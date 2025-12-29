#include "homepage.h"
#include "ui_homepage.h"
#include "utils/log/logger.h"
#include "utils/qssload/qssloader.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QScrollBar>
#include <QMouseEvent>

static QString getAllPortfolioFilePath()
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(baseDir); // 确保路径存在
    return baseDir + "/all_portfolios.json";
}

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
    , m_quoteUpdateTimer(new QTimer(this))
{
    ui->setupUi(this);

    m_currentPortfolio = nullptr;

    // 连接行情更新定时器槽函数
    connect(m_quoteUpdateTimer, &QTimer::timeout, this, &HomePage::on_quoteUpdateTimer_timeout);
}

HomePage::~HomePage()
{
    delete ui;
}

// 页面进入回调
void HomePage::onPageEnter()
{
    LOG_DEBUG("HomePage entered.");

    if (!m_quoteUpdateTimer->isActive())
    {
        m_quoteUpdateTimer->start(1000); // 1秒更新一次
    }
}

// 页面离开回调
void HomePage::onPageLeave()
{
    LOG_DEBUG("HomePage left.");
    if (m_quoteUpdateTimer->isActive())
    {
        m_quoteUpdateTimer->stop();
    }
}

// 页面初始化
void HomePage::init()
{
    // UI 初始化
    uiInit();

    //
    installScrollDragFilters();

    // 从本地获取组合数据
    loadAllPortfoliosFromLocal();
    updatePortfolioComboBox();

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

    // 初始化行情提供者
    m_quoteProvider = new SinaQuoteProvider(this);
    connect(m_quoteProvider, &SinaQuoteProvider::quotesReady,
            this, &HomePage::on_quoteProvider_updateQuotes);
    connect(m_quoteProvider, &SinaQuoteProvider::quoteError,
            this, &HomePage::on_quoteProvider_error);
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

    // 清除组合下拉框
    ui->portfolioComboBox->clear();
    ui->portfolioComboBox->setMaxVisibleItems(5);
    ui->portfolioComboBox->setStyleSheet("QComboBox{combobox-popup:0;}");

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

// 更新组合下拉框显示
void HomePage::updatePortfolioComboBox()
{
    ui->portfolioComboBox->clear();
    for (int i = 0; i < m_portfolioList.size(); ++i)
    {
        const StockPortfolio* portfolio = m_portfolioList[i];
        ui->portfolioComboBox->addItem(portfolio->name());
    }
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
    updatePortfolioComboBox();

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
}

// 删除组合按钮点击槽函数
void HomePage::on_delPortfolioBtn_clicked()
{
    int index = ui->portfolioComboBox->currentIndex();
    if (index < 0 || index >= m_portfolioList.size())
    {
        return;
    }

    // 删除组合
    StockPortfolio* toDelete = m_portfolioList[index];
    m_portfolioList.removeAt(index);
    delete toDelete;

    // 更新组合下拉框
    updatePortfolioComboBox();

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

// 保存所有组合到本地
void HomePage::saveAllPortfoliosToLocal()
{
    QJsonArray arr;
    for (int i = 0; i < m_portfolioList.size(); ++i)
    {
        StockPortfolio* portfolio = m_portfolioList[i];
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
            st["currentPrice"] = s.currentPrice;
            st["risePrice"] = s.risePrice;
            st["risePct"] = s.risePct;
            st["isRise"] = s.isRise;
            stockArray.append(st);
        }
        obj["stocks"] = stockArray;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QString path = getAllPortfolioFilePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    LOG_DEBUG("All portfolios saved to local, path: %s", path.toStdString().c_str());
}

// 从本地加载所有组合
void HomePage::loadAllPortfoliosFromLocal()
{
    QString path = getAllPortfolioFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray())
        return;

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
            s.currentPrice = st["currentPrice"].toDouble();
            s.risePrice = st["risePrice"].toDouble();
            s.risePct = st["risePct"].toDouble();
            s.isRise = static_cast<stockPriceFlag>(st["isRise"].toInt());
            port->addStock(s);
        }
        m_portfolioList.append(port);
    }

    LOG_DEBUG("All portfolios loaded from local, path: %s", path.toStdString().c_str());
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
    if (!m_currentPortfolio)
        return;

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

    // 遍历当前组合，生成新的股票块
    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    for (int i = 0; i < stocks.size(); ++i)
    {
        const StockInfo &info = stocks[i];
        StockBlock *block = new StockBlock(ui->scrollArea->widget());
        // 连接删除股票块请求槽函数
        connect(block, &StockBlock::deleteRequested, this, &HomePage::on_delStockBlock_requested);
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

// 行情更新槽函数
void HomePage::on_quoteProvider_updateQuotes(const QList<StockInfo> &infos)
{
    if (!m_currentPortfolio)
        return; 
    
    // 获取当前系统时间
    QTime currentTime = QTime::currentTime();
    int hour = currentTime.hour();
    int minute = currentTime.minute();

    // 如果在 9:15 到 9:25 之间，显示集合竞价，不更新数据
    if (hour == 9 && minute >= 15 && minute < 25)
    {
        LOG_DEBUG("During pre-market auction time (9:15-9:25), skipping quote update.");

        for (const StockInfo& updatedInfo : infos)
        {
            StockInfo* stock = m_currentPortfolio->getStock(updatedInfo.code);
            if (stock)
            {
                stock->name = updatedInfo.name;
                stock->currentPrice = updatedInfo.previousClose; // 显示昨日收盘价作为当前价
                stock->isRise = CALL_AUCTION; // 标记为集合竞价状态
            }
        }

        return;
    }

    // 除了每天的集合竞价 9.15~9.25 之外，更新当前组合内的股票数据
    for (const StockInfo& updatedInfo : infos)
    {
        StockInfo* stock = m_currentPortfolio->getStock(updatedInfo.code);
        if (stock)
        {
            stock->name = updatedInfo.name;
            stock->currentPrice = updatedInfo.currentPrice;
            stock->risePrice = updatedInfo.risePrice;
            stock->risePct = updatedInfo.risePct;
            stock->isRise = updatedInfo.isRise;
        }
    }

    // 刷新显示
    refreshStockInfoDisplay();

    // 更新标题语显示
    // 如果当前时间在 9:15 到 9:25 之间，显示集合竞价提示
    if (hour == 9 && minute >= 15 && minute < 25)
    {
        ui->titleLabel->setText("集合竞价中");
        // 设置橙色
        ui->titleLabel->setStyleSheet("color: #FFA500;");
    }
    // 如果在 11:30 到 13:00 之间，显示午休提示
    else if (hour == 11 && minute >= 30 || hour == 12 || (hour == 13 && minute < 0))
    {
        ui->titleLabel->setText("中午休盘");
        // 设置灰色
        ui->titleLabel->setStyleSheet("color: #808080;");
    }
    // 如果在 15:00 之后，显示今日收盘提示
    else if (hour >= 15)
    {
        ui->titleLabel->setText("今日已收盘");
        // 设置灰色
        ui->titleLabel->setStyleSheet("color: #808080;");
    }
    // 其它时间，不显示
    else
    {
        ui->titleLabel->setText("");
    }
}

// 行情错误槽函数
void HomePage::on_quoteProvider_error(const QString &stockCode, const QString &errReason)
{
    LOG_DEBUG("Quote error for %s: %s", stockCode.toStdString().c_str(), errReason.toStdString().c_str());
}

// 行情定时更新槽函数
void HomePage::on_quoteUpdateTimer_timeout()
{
    if (!m_currentPortfolio)
        return;

    // 收集当前组合内的股票代码
    QStringList codes;
    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    for (const StockInfo& stock : stocks)
    {
        codes.append(stock.code);
    }

    // 请求行情更新
    m_quoteProvider->fetchQuotes(codes);
}

// 股票块删除请求槽函数
void HomePage::on_delStockBlock_requested(const QString &code)
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
