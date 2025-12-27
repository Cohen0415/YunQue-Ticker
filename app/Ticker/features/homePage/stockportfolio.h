#ifndef STOCKPORTFOLIO_H
#define STOCKPORTFOLIO_H

#include "features/homePage/stockblock.h"
#include <QString>
#include <QList>
#include <QObject>

// 股票组合类
// 比如创建一个“我的自选股”组合，管理用户关注的股票列表
class StockPortfolio : public QObject
{
    Q_OBJECT

public:

    explicit StockPortfolio(const QString& name, QObject *parent = nullptr);

    QString name() const;                       // 获取组合名称
    void setName(const QString& n);             // 设置组合名称

    void addStock(const StockInfo& info);       // 添加股票到组合
    bool delStock(const QString& code);         // 删除股票

    StockInfo* getStock(const QString& code);   // 按股票代码获取数据指针
    QList<StockInfo> stocks() const;            // 获取组合内所有股票列表

    int stockCount() const;                     // 获取股票数量

private:

    QString m_name;                             // 组合名称
    QList<StockInfo> m_stockList;               // 股票列表

};

#endif // STOCKPORTFOLIO_H
