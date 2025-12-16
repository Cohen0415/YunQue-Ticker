#include "network.h"            
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
    connect(m_socket, static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::errorOccurred),
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

    if (m_socket->waitForConnected(3000)) 
    {
        qDebug() << "[Network] Connected successfully.";
        return true;
    } 
    else 
    {
        qWarning() << "[Network] Connection failed:" << m_socket->errorString();
        return false;
    }

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

void Network::sendData(const QByteArray &data)
{
    if (!m_isConnected || !m_socket) 
    {
        qWarning() << "[Network] Cannot send data: Not connected.";
        return; 
    }

    // 协议：[qint32 长度][实际数据]
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    // 先写入数据长度
    out << static_cast<quint32>(data.size());
    // 再写入实际数据
    out.writeRawData(data.data(), data.size());

    qDebug() << "[Network] Sending data packet of size:" << data.size() << "bytes (total"
             << block.size() << "bytes with header).";

    // 使用互斥锁保护写操作
    QMutexLocker locker(&m_sendMutex);
    qint64 bytesWritten = m_socket->write(block);
    if (bytesWritten == -1) 
    {
        qCritical() << "[Network] Failed to write data to socket:" << m_socket->errorString();
        return; 
    }
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
            peekStream.setVersion(QDataStream::Qt_5_15);
            peekStream.setByteOrder(QDataStream::LittleEndian);

            quint32 size;
            peekStream >> size;

            // 检查大小是否合理，防止恶意数据或错误
            if (size > MAX_MESSAGE_SIZE) 
            {
                qCritical() << "[Network] Received packet size (" << size << " bytes) exceeds maximum allowed ("
                            << MAX_MESSAGE_SIZE << " bytes). Disconnecting.";
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
