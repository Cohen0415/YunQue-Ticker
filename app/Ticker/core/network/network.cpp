#include "network.h"            
#include <QDebug>               
#include <QDataStream>          
#include <QJsonParseError>      

Network::Network(QObject *parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this)) 
    , m_isConnected(false)             
    , m_expectedDataSize(-1)           
{
    // 连接信号和槽
    connect(m_socket, &QLocalSocket::connected, this, &Network::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &Network::onDisconnected);
    connect(m_socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
            this, &Network::onSocketError);
    
    connect(m_socket, &QLocalSocket::readyRead, this, &Network::onReadyRead);
}

Network::~Network()
{
    qDebug() << "Network object destroyed.";
}

bool Network::connectToServer(const QString &socketPath)
{
    if (m_isConnected)
    {
        qWarning() << "Network: Already connected to" << m_serverPath << ". Ignoring new request to" << socketPath;
        return true;
    }

    // 保存服务器路径
    m_serverPath = socketPath;

    qDebug() << "Network: Attempting to connect to server at:" << socketPath;

    // 连接到本地服务器
    m_socket->connectToServer(socketPath);

    return true;
}

void Network::disconnectFromServer()
{
    if (m_isConnected)
    {
        qDebug() << "Network: Disconnecting from server at:" << m_serverPath;
        // 断开连接
        m_socket->abort();
    }
    else
    {
        qDebug() << "Network: disconnectFromServer called, but not currently connected.";
    }
}

bool Network::isConnected() const
{
    return m_isConnected;
}

void Network::onConnected()
{
    qDebug() << "Network: Successfully connected to server at" << m_serverPath;
    m_isConnected = true;
    emit connectionStatusChanged(true);
}

void Network::onDisconnected()
{
    qDebug() << "Network: Disconnected from server.";
    m_isConnected = false;
    emit connectionStatusChanged(false);
}

void Network::onSocketError(QLocalSocket::LocalSocketError socketError)
{
    QString errorString = m_socket->errorString();
    qCritical() << "Network: Socket error occurred (" << socketError << "):" << errorString;

    if (m_isConnected &&
        (socketError == QLocalSocket::ServerNotFoundError ||
         socketError == QLocalSocket::ConnectionRefusedError ||
         socketError == QLocalSocket::PeerClosedError ||
         socketError == QLocalSocket::SocketResourceError))
    {
        m_isConnected = false;
        emit connectionStatusChanged(false);
    }
}

bool Network::sendMessage(const QJsonDocument &jsonDoc)
{
    if (!m_isConnected) 
    {
        qWarning() << "Network: Cannot send message, not connected.";
        return false;
    }

    // 将 JSON 文档转换为 UTF-8 字节数组
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact); 
    if (jsonData.isEmpty()) 
    { 
        qWarning() << "Network: Attempted to send empty or invalid JSON.";
        return false;
    }

    // 获取数据大小并验证
    quint32 dataSize = static_cast<quint32>(jsonData.size());
    if (dataSize == 0 || dataSize > MAX_JSON_SIZE) 
    {
        qWarning() << "Network: JSON data size is out of valid range (1-8192 bytes):" << dataSize;
        return false;
    }

    qDebug() << "Network: Sending JSON message of size:" << dataSize << "bytes.";

    // 准备数据帧: [4-byte LE length][N-byte JSON]
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian); // 使用小端字节序
    stream << dataSize;                             // 先发送长度
    frame.append(jsonData);                         // 再发送JSON数据

    // 发送数据
    qint64 bytesWritten = m_socket->write(frame);
    if (bytesWritten != frame.size()) 
    {
        qCritical() << "Network: Failed to write complete frame. Wrote" << bytesWritten << "of" << frame.size() << "bytes.";
        return false;
    }

    return true;
}

void Network::onReadyRead()
{
    // 读取所有可用数据并追加到缓冲区
    m_readBuffer.append(m_socket->readAll());

    // 处理缓冲区中的数据
    while (true) 
    {
        if (m_expectedDataSize == -1) 
        {
            if (m_readBuffer.size() < static_cast<int>(sizeof(quint32))) 
            {
                break;
            }

            QDataStream stream(m_readBuffer);
            // 设置数据流的字节序为小端序
            stream.setByteOrder(QDataStream::LittleEndian);
            // 从数据流中读取前 4 个字节
            stream >> m_expectedDataSize;

            // 检查数据大小的合理性
            if (m_expectedDataSize <= 0 || m_expectedDataSize > MAX_JSON_SIZE) 
            {
                qCritical() << "Network: Received invalid data size in header:" << m_expectedDataSize;
                
                m_readBuffer.clear();
                m_expectedDataSize = -1;

                break; 
            }

            qDebug() << "Network: Expecting JSON data of size:" << m_expectedDataSize << "bytes.";

            // 移除已处理的长度字段
            m_readBuffer.remove(0, sizeof(quint32));
        }

        if (m_expectedDataSize > 0) 
        {
            // 检查缓冲区中的数据是否足够构成完整的JSON载荷
            if (m_readBuffer.size() < m_expectedDataSize) 
            {
                break;
            }

            QByteArray jsonData = m_readBuffer.left(m_expectedDataSize); // 从缓冲区左边取出期望大小的数据
            m_readBuffer.remove(0, m_expectedDataSize);                  // 从缓冲区中移除已取出的数据
            m_expectedDataSize = -1;                                     // 重置状态，准备读取下一个消息的长度头

            qDebug() << "Network: Received complete JSON message of size:" << jsonData.size() << "bytes.";

            // 解析提取出的JSON数据
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error != QJsonParseError::NoError) 
            {
                qWarning() << "Network: Failed to parse received JSON data:" << parseError.errorString();
                continue; 
            }

            // JSON解析成功，发射消息接收信号
            emit messageReceived(jsonDoc);
        }
    }
}
