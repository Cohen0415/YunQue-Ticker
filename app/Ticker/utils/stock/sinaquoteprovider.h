#ifndef SINAQUOTEPROVIDER_H
#define SINAQUOTEPROVIDER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include "features/homePage/stockinfo.h"
#include "stockquoteprovider.h"

// SinaQuoteProvider 利用新浪财经API拉取实时行情
class SinaQuoteProvider : public StockQuoteProvider
{
    Q_OBJECT

public:

    explicit SinaQuoteProvider(QObject *parent = nullptr);

    void fetchQuote(const QString &stockCode) override;
    void fetchQuotes(const QList<QString> &stockCodes) override;

private slots:

    void onReplyFinished(QNetworkReply *reply);

private:

    QNetworkAccessManager *m_net;
    // 缓存发起请求的股票代码，便于多请求并发时解析
    QMap<QNetworkReply*, QStringList> m_replyCodes;

    static QString codeToSinaSymbol(const QString& code); // 如600000.SH->sh600000
    static StockInfo parseSinaLine(const QString &line, const QString &code);

    static QString codeToDisplay(const QString& sinaSymbol); // sh600183 -> 600183.SH

};

#endif // SINAQUOTEPROVIDER_H
