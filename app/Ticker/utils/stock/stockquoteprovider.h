#ifndef STOCKQUOTEPROVIDER_H
#define STOCKQUOTEPROVIDER_H

#include <QObject>
#include <QString>
#include <QList>
#include "features/homePage/stockinfo.h"

class StockQuoteProvider : public QObject
{
    Q_OBJECT

public:

    explicit StockQuoteProvider(QObject *parent = nullptr);
    virtual ~StockQuoteProvider();

    // 获取单只股票最新行情
    virtual void fetchQuote(const QString &stockCode) = 0;
    // 获取多只股票的最新行情
    virtual void fetchQuotes(const QList<QString> &stockCodes) = 0;

signals:

    // 成功获取单只股票行情
    void quoteReady(const StockInfo &info);
    // 成功获取多只股票行情
    void quotesReady(const QList<StockInfo> &infos);
    // 错误
    void quoteError(const QString &stockCode, const QString &errReason);

};

#endif // STOCKQUOTEPROVIDER_H
