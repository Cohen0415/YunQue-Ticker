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
    QUrl url("http://hq.sinajs.cn/list=" + fullCodes);  // 新浪行情接口，例如 http://hq.sinajs.cn/list=sh600000,sz000001

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
    LOG_DEBUG("stockLine = %s", line.toStdString().c_str());

    auto list = line.split(',');
    StockInfo info;
    info.code = code;
    if (list.size() < 32)
        return info;

    info.name = list[0];
    info.currentPrice = list[3].toDouble();

    double yestClose = list[2].toDouble();
    double risePrice = info.currentPrice - yestClose;

    // 这里 risePrice/risePct 直接保存为绝对值
    info.risePrice = std::abs(risePrice);
    if (yestClose > 0.00001)
        info.risePct = std::abs(risePrice / yestClose) * 100.0;
    if (risePrice > 0.00001)
        info.isRise = RISE;
    else if (risePrice < -0.00001)
        info.isRise = FALL;
    else
        info.isRise = NORMAL;

    return info;
}
