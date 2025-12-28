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

    // 清除组合下拉框
    ui->portfolioComboBox->clear();

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
        return -1;
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
        return -2;
    }

    return 0;
}

// 获取组合索引，找不到返回 -1
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
    if (ret == -1)
    {
        ui->inputErrHintLabel->setText("请输入6位数字代码！");
        ui->inputErrHintLabel->setVisible(true);
        return;
    }
    else if (ret == -2)
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
    if (portfolioName.length() > 7)
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
        ui->portfolioComboBox->setCurrentIndex(0);
        m_currentPortfolio = m_portfolioList[0];
    }
    else
    {
        m_currentPortfolio = nullptr;
    }

    // 保存所有组合到本地
    saveAllPortfoliosToLocal();
}

// 打印当前组合列表，调试用
void HomePage::printAllPortfolioList()
{
    // 打印组合列表的每个组合，及该组合下的股票代码
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

// 打印当前组合的股票列表，调试用
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
            s.isRise = st["isRise"].toBool();
            port->addStock(s);
        }
        m_portfolioList.append(port);
    }

    LOG_DEBUG("All portfolios loaded from local, path: %s", path.toStdString().c_str());
    printAllPortfolioList();
}

// 刷新股票信息显示
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

// 更新股票块显示
void HomePage::updateStockBlocks()
{
    // 清除原有所有股票Block
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

    // 遍历当前组合，生成新的StockBlock
    QList<StockInfo> stocks = m_currentPortfolio->stocks();
    for (int i = 0; i < stocks.size(); ++i)
    {
        const StockInfo &info = stocks[i];
        StockBlock *block = new StockBlock(ui->scrollArea->widget());
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
    
    // 更新当前组合内的股票数据
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
}

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
