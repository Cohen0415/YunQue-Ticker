#ifndef STOCKINFO_H
#define STOCKINFO_H

#include <QString>

struct StockInfo {
    QString code;         // 股票代码
    QString name;         // 股票名称
    double currentPrice;  // 当前价格
    double risePrice;     // 涨跌价格
    double risePct;       // 涨跌幅%
    bool isRise;          // 涨（true）/跌（false）标志

    StockInfo()
        : code(""), name(""),
        currentPrice(0), risePrice(0), risePct(0), isRise(false)
    {}
    explicit StockInfo(const QString& c)
        : code(c), name(""),
        currentPrice(0), risePrice(0), risePct(0), isRise(false)
    {}
};

#endif // STOCKINFO_H
