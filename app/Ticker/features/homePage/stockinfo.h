#ifndef STOCKINFO_H
#define STOCKINFO_H

#include <QString>

// 股票价格标志位
enum stockPriceFlag {
    NORMAL = 0,     // 价格不变
    CALL_AUCTION,   // 集合竞价
    RISE,           // 价格上涨
    FALL,           // 价格下跌
};

struct StockInfo {
    QString code;         // 股票代码
    QString name;         // 股票名称
    double currentPrice;  // 当前价格
    double previousClose; // 昨日收盘价
    double risePrice;     // 涨跌价格
    double risePct;       // 涨跌幅%
    stockPriceFlag isRise;// 股票价格标志位

    StockInfo()
        : code(""), name(""),
        currentPrice(0), previousClose(0), risePrice(0), risePct(0), isRise(NORMAL)
    {}
    explicit StockInfo(const QString& c)
        : code(c), name(""),
        currentPrice(0), previousClose(0), risePrice(0), risePct(0), isRise(NORMAL)
    {}
};

#endif // STOCKINFO_H
