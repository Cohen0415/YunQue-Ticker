#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QLocalSocket>      
#include <QAbstractSocket>   
#include <QString>

#define MAX_JSON_SIZE           8192        // 最大JSON消息大小（字节）

class Network : public QObject
{
    Q_OBJECT

public:

    explicit Network(QObject *parent = nullptr);
    ~Network();

    // 连接到本地服务器
    bool connectToServer(const QString &socketPath);

    // 从本地服务器断开连接
    void disconnectFromServer();

    // 检查当前连接状态
    bool isConnected() const;

    // 发送JSON消息
    bool sendMessage(const QJsonDocument &jsonDoc);

signals:

    // 连接状态改变信号
    void connectionStatusChanged(bool isConnected);

    // 收到消息信号
    void messageReceived(const QJsonDocument &jsonDoc);

private slots:

    // 连接成功的处理
    void onConnected();

    // 断开连接的处理
    void onDisconnected();

    // 套接字错误的处理
    void onSocketError(QLocalSocket::LocalSocketError socketError);

    // 处理可读数据
    void onReadyRead();

private:
    QLocalSocket *m_socket;         // 本地套接字对象
    QString m_serverPath;           // 服务器路径
    bool m_isConnected;             // 当前连接状态

    QByteArray m_readBuffer;        // 用于存储接收的数据缓冲区
    qint32 m_expectedDataSize;      // 预期接收的数据大小
};

#endif // NETWORK_H
