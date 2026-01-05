#include "sinaquoteprovider.h"
#include "utils/log/logger.h"
#include <QNetworkRequest>
#include <QTextCodec>
#include <QRegularExpression>
#include <QDebug>

SinaQuoteProvider::SinaQuoteProvider(QObject *parent)
    : StockQuoteProvider(parent)
{
    m_net = new QNetworkAccessManager(this);
    connect(m_net, &QNetworkAccessManager::finished, this, &SinaQuoteProvider::onReplyFinished);
}

// 股票代码转换成新浪财经适用的，例如 600000.SH -> sh600000
QString SinaQuoteProvider::codeToSinaSymbol(const QString& code)
{
    // 支持 SH/SZ/BJ 三市代码转换
    if (code.endsWith(".SH", Qt::CaseInsensitive))
        return "sh" + code.left(6);
    if (code.endsWith(".SZ", Qt::CaseInsensitive))
        return "sz" + code.left(6);
    if (code.endsWith(".BJ", Qt::CaseInsensitive))
        return "bj" + code.left(6);
    // 纯六位也兼容（比如“600000”）
    if (code.length() == 6)
        return "sh" + code;
    return code; // 默认原样返回
}

// 新浪财经股票代码转成要显示的，例如 sh600183 -> 600183.SH
QString SinaQuoteProvider::codeToDisplay(const QString& sinaSymbol)
{
    // 逆向转换
    if (sinaSymbol.startsWith("sh") && sinaSymbol.length() == 8)
        return sinaSymbol.mid(2,6) + ".SH";
    if (sinaSymbol.startsWith("sz") && sinaSymbol.length() == 8)
        return sinaSymbol.mid(2,6) + ".SZ";
    if (sinaSymbol.startsWith("bj") && sinaSymbol.length() == 8)
        return sinaSymbol.mid(2,6) + ".BJ";
    return sinaSymbol;
}

// 获取单只股票行情
void SinaQuoteProvider::fetchQuote(const QString &stockCode)
{
    fetchQuotes({stockCode});
}

// 获取多只股票行情
void SinaQuoteProvider::fetchQuotes(const QList<QString> &stockCodes)
{
    QStringList urlCodes;
    for (const QString& code : stockCodes)
    {
        urlCodes << codeToSinaSymbol(code);
    }
    QString fullCodes = urlCodes.join(',');
    QUrl url(SINA_API_URL + fullCodes);  // 新浪行情接口，例如 http://hq.sinajs.cn/list=sh600000,sz000001

    QNetworkRequest req(url);
    req.setRawHeader("referer", "https://finance.sina.com.cn/");    // 新浪接口要加这个 header

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    req.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");
#else
    req.setRawHeader("User-Agent", "Mozilla/5.0");
#endif

    QNetworkReply* reply = m_net->get(req);     // 发起异步 HTTP GET 请求，reply 为 HTTP GET 请求所得到的结果对象
    m_replyCodes[reply] = urlCodes;             // 记录本次归属的所有 code，用于回调时解析
}

// 处理网络回复
void SinaQuoteProvider::onReplyFinished(QNetworkReply *reply)
{
    // 对应代码
    QStringList urlCodes = m_replyCodes.take(reply);

    // 网络错误检查
    if (reply->error() != QNetworkReply::NoError)
    {
        QString err = reply->errorString();
        for (const QString& code : urlCodes)
        {
            emit quoteError(code, err); // 全部标记为网络异常
        }
        reply->deleteLater();
        return;
    }

    // 新浪返回GBK编码
    QByteArray data = reply->readAll();     // 读取数据
    QTextCodec* codec = QTextCodec::codecForName("GBK");    // 获取GBK编码器
    QString text = codec->toUnicode(data);  // 转换为Unicode字符串
    reply->deleteLater();                   // 释放reply

    QList<StockInfo> infos;

    // 每行一只股票数据
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    for (int i = 0; i < lines.size(); ++i)
    {
        const QString &line = lines[i];
        // 匹配 var hq_str_sz000001="...csv...";
        int eqIdx = line.indexOf('=');
        if (eqIdx < 0)
            continue;
        int quoteIdx = line.indexOf('"', eqIdx);
        int quote2Idx = line.lastIndexOf('"');
        if (quoteIdx < 0 || quote2Idx <= quoteIdx)
            continue;

        QString dataStr = line.mid(quoteIdx+1, quote2Idx-quoteIdx-1);
        QString codeStr;
        // 获取代码，如 hq_str_sh600183
        QRegularExpression re("hq_str_([a-z]{2}\\d{6})");
        QRegularExpressionMatch m = re.match(line);
        if (m.hasMatch())
            codeStr = codeToDisplay(m.captured(1));

        if (dataStr.isEmpty() || codeStr.isEmpty())
            continue;

        StockInfo info = parseSinaLine(dataStr, codeStr);
        infos.append(info);
        emit quoteReady(info);
    }

    if (!infos.isEmpty())
        emit quotesReady(infos);
}

// 解析新浪单行数据
StockInfo SinaQuoteProvider::parseSinaLine(const QString &line, const QString &code)
{
    /*
        例如："生益科技,54.400,55.340,57.930,58.200,54.170,57.930,57.950,32971357,1871113251.000,4000,57.930,27800,57.920,2000,57.900,500,57.890,100,57.880,600,57.950,1700,57.960,700,57.970,400,57.980,500,57.990,2025-11-26,11:29:11,00,"

        解析：
            股票名称：生益科技
            今日开盘价：54.400
            昨日收盘价：55.340
            当前价格：57.930
            今日最高价：58.200
            今日最低价：54.170
            竞买价：57.930
            竞卖价：57.950
            成交量：32971357
            成交金额：1871113251.000
            买一量：4000
            买一价：57.930
            买二量：27800
            买二价：57.920
            买三量：2000
            买三价：57.900
            买四量：500
            买四价：57.890
            买五量：100
            买五价：57.880
            卖一量：600
            卖一价：57.950
            卖二量：1700
            卖二价：57.960
            卖三量：700
            卖三价：57.970
            卖四量：400
            卖四价：57.980
            卖五量：500
            卖五价：57.990
            日期：2025-11-26
            时间：11:29:11
            状态：00
    */
    
    LOG_DEBUG("stockLine = %s", line.toStdString().c_str());

    auto list = line.split(',');
    StockInfo info;
    info.code = code;
    if (list.size() < 32)
        return info;

    // 获取股票名称
    info.name = list[0];
    // 昨日收盘价
    info.previousClose = list[2].toDouble();            

    // 早盘集合竞价期间，全部设置为 0.00
    QTime currentTime = QTime::currentTime();
    int hour = currentTime.hour();
    int minute = currentTime.minute();
    if (hour == 9 && minute >= 15 && minute <= 24) 
    {
        info.currentPrice = 0.00;
        info.risePrice = 0.00;
        info.risePct = 0.00;
        info.isRise = PRICE_NORMAL;
    }
    else
    {
        // 获取当前价格
        info.currentPrice = list[3].toDouble();             

        // 计算涨跌价格
        double yestClose = info.previousClose;
        double risePrice = info.currentPrice - yestClose;

        // 计算涨跌幅
        info.risePrice = std::abs(risePrice);
        if (yestClose > 0.00001)
            info.risePct = std::abs(risePrice / yestClose) * 100.0;

        // 涨跌标志位
        if (risePrice > 0.00001)
            info.isRise = PRICE_RISE;
        else if (risePrice < -0.00001)
            info.isRise = PRICE_FALL;
        else
            info.isRise = PRICE_NORMAL;
    }
    
    // 市场日期
    info.marketDate = list[30];

    // 状态码判断股票状态
    QString statusCode = list[32];
    if (statusCode == SINA_STATUS_NORMAL)
        info.status = STATUS_NORMAL;
    else if (statusCode == SINA_STATUS_SUSPEND1 || statusCode == SINA_STATUS_SUSPEND2 ||
             statusCode == SINA_STATUS_SUSPEND3 || statusCode == SINA_STATUS_SUSPEND4 ||
             statusCode == SINA_STATUS_SUSPEND5 || statusCode == SINA_STATUS_PAUSE)
        info.status = STATUS_SUSPEND;
    else if (statusCode == SINA_STATUS_DELISTED1 || statusCode == SINA_STATUS_DELISTED2 ||
             statusCode == SINA_STATUS_DELISTED3)
        info.status = STATUS_DELISTED;

    return info;
}
