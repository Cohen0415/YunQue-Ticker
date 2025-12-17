#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QLocalSocket>      
#include <QAbstractSocket>   
#include <QString>
#include <QMutex>

#define MAX_MESSAGE_SIZE            8192

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

public slots:

    // 发送数据到服务器
    void sendData(const QByteArray &data);

signals:

    // 接收到数据的信号，接收到完整的一帧数据时发射
    void dataReceived(const QByteArray &data);

    // 连接状态改变信号
    void connectionStatusChanged(bool isConnected);

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

    void processData();             // 处理缓冲区中的数据，组装完整包

    QLocalSocket *m_socket;         // 本地套接字对象
    QString m_serverPath;           // 服务器路径
    bool m_isConnected;             // 当前连接状态

    QByteArray m_readBuffer;        // 用于存储接收的数据缓冲区
    qint32 m_expectedDataSize;      // 预期接收的数据大小

    QMutex m_sendMutex;             // 互斥锁
};

#endif // NETWORK_H
