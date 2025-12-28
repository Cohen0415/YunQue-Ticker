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

    void fetchQuote(const QString &stockCode) override;         // 获取单只股票行情
    void fetchQuotes(const QList<QString> &stockCodes) override;// 获取多只股票行情

private slots:

    void onReplyFinished(QNetworkReply *reply);                 // 处理网络回复

private:

    QNetworkAccessManager *m_net;                               // 网络访问管理器
    QMap<QNetworkReply*, QStringList> m_replyCodes;             // 缓存发起请求的股票代码，便于多请求并发时解析

    static QString codeToSinaSymbol(const QString& code);       // 股票代码转换成新浪财经适用的，例如 600000.SH -> sh600000
    static QString codeToDisplay(const QString& sinaSymbol);    // 新浪财经股票代码转成要显示的，例如 sh600183 -> 600183.SH

    static StockInfo parseSinaLine(const QString &line, const QString &code);   // 解析新浪财经返回的一行股票数据
};

#endif // SINAQUOTEPROVIDER_H
