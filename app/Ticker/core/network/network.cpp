#include "network.h"
#include "utils/log/logger.h"

#include <QDebug>               
#include <QDataStream>
#include <QTimer>

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
    // LOG_DEBUG("Network Destructor called, cleaning up.");
}

bool Network::connectToServer(const QString &socketPath)
{
    if (m_isConnected)
    {
        LOG_WARN("Network Already connected to server at: %s", m_serverPath.toLocal8Bit().constData());
        return true;
    }

    // 保存服务器路径
    m_serverPath = socketPath;

    // LOG_DEBUG("Network Connecting to server at: %s", socketPath.toLocal8Bit().constData());

    // 连接到本地服务器
    m_socket->connectToServer(socketPath);
    // 等待连接建立，设置超时时间为 3 秒
    if (m_socket->waitForConnected(3000)) 
    {
        LOG_INFO("Network Connected to server at: %s", socketPath.toLocal8Bit().constData());
        return true;
    } 
    else 
    {
        LOG_ERROR("Network Failed to connect to server at: %s. Error: %s",
                  socketPath.toLocal8Bit().constData(),
                  m_socket->errorString().toLocal8Bit().constData());
        return false;
    }

    return true;
}

void Network::disconnectFromServer()
{
    if (m_isConnected)
    {
        LOG_INFO("Network Disconnecting from server at: %s", m_serverPath.toLocal8Bit().constData());
        // 断开连接
        m_socket->abort();
    }
    else
    {
       LOG_DEBUG("Network Not connected, no need to disconnect.");
    }
}

bool Network::isConnected() const
{
    return m_isConnected;
}

void Network::sendData(const QByteArray &data)
{
    if (!m_isConnected || !m_socket) 
    {
        LOG_WARN("Network Not connected to server. Cannot send data.");
        return; 
    }

    // 协议：[qint32 长度][实际数据]
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    // 设置小端字节序
    out.setByteOrder(QDataStream::LittleEndian);

    // 先写入数据长度
    out << static_cast<quint32>(data.size());
    // 再写入实际数据
    out.writeRawData(data.data(), data.size());

    // LOG_DEBUG("Network Sending data to server, size: %d bytes.", block.size());
    // LOG_DEBUG("Send: >>>>> %s", QString(block).toLocal8Bit().

    // 使用互斥锁保护写操作
    QMutexLocker locker(&m_sendMutex);
    qint64 bytesWritten = m_socket->write(block);
    if (bytesWritten == -1) 
    {
        LOG_WARN("Network Failed to write data to socket. Error: %s",
                 m_socket->errorString().toLocal8Bit().constData());
        return; 
    }
}

void Network::onConnected()
{
    m_isConnected = true;
    emit connectionStatusChanged(true);
}

void Network::onDisconnected()
{
    m_isConnected = false;
    emit connectionStatusChanged(false);
}

void Network::onSocketError(QLocalSocket::LocalSocketError socketError)
{
    QString errorString = m_socket->errorString();
    LOG_ERROR("Network Socket error occurred: %s (Error code: %d)",
              errorString.toLocal8Bit().constData(),
              static_cast<int>(socketError));

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

void Network::onReadyRead()
{
    // qDebug() << "[Network] Data ready to read.";
    // 将可用的所有数据追加到读取缓冲区
    m_readBuffer.append(m_socket->readAll());
    // 处理缓冲区中的数据
    processData();
}

void Network::processData()
{
    // qDebug() << "[Network] Processing buffer, size:" << m_readBuffer.size();
    // 循环处理，因为一次 readyRead 可能包含多个完整包
    while (m_readBuffer.size() >= static_cast<int>(sizeof(quint32))) 
    { 
        // 如果还没有确定当前包的大小，则先读取头部
        if (m_expectedDataSize == -1) 
        {
            // 从缓冲区创建一个临时的只读数据流来窥探头部
            QDataStream peekStream(m_readBuffer);
            peekStream.setByteOrder(QDataStream::LittleEndian);

            quint32 size;
            peekStream >> size;

            // 检查大小是否合理，防止恶意数据或错误
            if (size > MAX_MESSAGE_SIZE) 
            {
                LOG_ERROR("Network Received data size %u exceeds maximum allowed %d. Disconnecting.",
                          size, MAX_MESSAGE_SIZE);
                m_readBuffer.clear(); // 清空缓冲区
                m_expectedDataSize = -1;
                m_socket->disconnectFromServer(); // 断开连接
                return;
            }

            m_expectedDataSize = static_cast<qint32>(size);
            // qDebug() << "[Network] Expecting data packet of size:" << m_expectedDataSize;
        }

        // 检查缓冲区中是否有足够的数据来构成一个完整的包 ([头部] + [数据])
        int headerSize = sizeof(quint32);
        if (m_readBuffer.size() >= headerSize + m_expectedDataSize) 
        {
            // 有足够的数据，提取出来

            // 跳过头部，读取实际数据
            QByteArray messageData = m_readBuffer.mid(headerSize, m_expectedDataSize);

            // 从缓冲区中移除已处理的部分 ([头部] + [数据])
            m_readBuffer.remove(0, headerSize + m_expectedDataSize);

            // 重置状态，准备处理下一个包
            m_expectedDataSize = -1;

            // qDebug() << "[Network] Emitting complete data packet of size:" << messageData.size();
            // 发射接收到完整数据包的信号
            emit dataReceived(messageData);
        } 
        else 
        {
            // 数据还不完整，等待更多数据
            // qDebug() << "[Network] Incomplete packet, waiting for more data. Buffer size:"
            //          << m_readBuffer.size() << ", Expected total:" << (headerSize + m_expectedDataSize);
            break; // 退出循环，等待下次 onReadyRead
        }
    }
}
