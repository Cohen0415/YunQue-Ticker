#ifndef SINAQUOTEPROVIDER_H
#define SINAQUOTEPROVIDER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include "features/homePage/stockinfo.h"
#include "stockquoteprovider.h"

#define SINA_API_URL               "http://hq.sinajs.cn/list="    // 新浪财经股票行情API基础URL

/* 状态码 
    00	正常
    01	停牌一小时
    02	停牌一天
    03	连续停牌
    04	盘中停牌
    05	停牌半天
    07	暂停
    -1	无该记录
    -2	未上市
    -3	退市
*/
#define SINA_STATUS_NORMAL        "00"    // 正常交易
#define SINA_STATUS_SUSPEND1      "01"    // 停牌一小时
#define SINA_STATUS_SUSPEND2      "02"    // 停牌一天
#define SINA_STATUS_SUSPEND3      "03"    // 连续停牌
#define SINA_STATUS_SUSPEND4      "04"    // 盘中停牌
#define SINA_STATUS_SUSPEND5      "05"    // 停牌半天
#define SINA_STATUS_PAUSE         "07"    // 暂停
#define SINA_STATUS_DELISTED1     "-1"    // 无该记录
#define SINA_STATUS_DELISTED2     "-2"    // 未上市
#define SINA_STATUS_DELISTED3     "-3"    // 退市

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
