#include "stockportfolio.h"

StockPortfolio::StockPortfolio(const QString &name, QObject *parent)
    : QObject{parent}
    , m_name{name}
{
    m_stockList.clear();
}

QString StockPortfolio::name() const
{
    return m_name;
}

void StockPortfolio::setName(const QString &n)
{
    m_name = n;
}

// 添加股票到组合
void StockPortfolio::addStock(const StockInfo &info)
{
    // 避免重复添加
    for (int i = 0; i < m_stockList.size(); ++i)
    {
        if (m_stockList[i].code == info.code)
        {
            return;
        }
    }
    m_stockList.append(info);
}

// 删除股票
bool StockPortfolio::delStock(const QString &code)
{
    for (int i = 0; i < m_stockList.size(); ++i)
    {
        if (m_stockList[i].code == code)
        {
            m_stockList.removeAt(i);
            return true;
        }
    }
    return false;
}

// 获取股票数据指针
StockInfo *StockPortfolio::getStock(const QString &code)
{
    for (StockInfo& stock : m_stockList)
    {
        if (stock.code == code)
        {
            return &stock;
        }
    }
    return nullptr;
}

// 获取组合列表
QList<StockInfo> StockPortfolio::stocks() const
{
    return m_stockList;
}

// 获取股票数量
int StockPortfolio::stockCount() const
{
    return m_stockList.size();
}
