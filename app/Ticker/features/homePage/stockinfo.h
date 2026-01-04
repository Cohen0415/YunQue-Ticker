#ifndef STOCKINFO_H
#define STOCKINFO_H

#include <QString>

// 股票价格标志位
enum stockPriceFlag {
    PRICE_NORMAL = 0,     // 价格不变
    PRICE_RISE,           // 价格上涨
    PRICE_FALL,           // 价格下跌
};

// 股票状态信息
enum stockStatus {
    STATUS_NORMAL = 0,    // 正常交易
    STATUS_SUSPEND,       // 停牌
    STATUS_DELISTED,      // 退市
};

struct StockInfo {
    QString code;         // 股票代码
    QString name;         // 股票名称
    double currentPrice;  // 当前价格
    double previousClose; // 昨日收盘价
    double risePrice;     // 涨跌价格
    double risePct;       // 涨跌幅%
    stockPriceFlag isRise;// 股票价格标志位
    QString marketDate;   // 市场日期
    stockStatus status;   // 股票状态

    StockInfo()
        : code(""), name(""),
        currentPrice(0), previousClose(0), risePrice(0), risePct(0), isRise(PRICE_NORMAL), marketDate(""), status(STATUS_NORMAL)
    {}
    explicit StockInfo(const QString& c)
        : code(c), name(""),
        currentPrice(0), previousClose(0), risePrice(0), risePct(0), isRise(PRICE_NORMAL), marketDate(""), status(STATUS_NORMAL)
    {}
};

#endif // STOCKINFO_H
