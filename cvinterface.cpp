#include "cvinterface.h"

cvInterface::cvInterface(QObject *parent) :
    QObject(parent)
{
    qDebug("cvInterface()");
    cvSocket = new QTcpSocket(this);
    connect(cvSocket, SIGNAL(hostFound()), this, SLOT(slotHostFounded()));
    connect(cvSocket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(cvSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()), Qt::DirectConnection);
    connect(cvSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotWritten(qint64)),Qt::DirectConnection);
    connect(cvSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(cvSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displayError(QAbstractSocket::SocketError)));
}

cvInterface::~cvInterface()
{
    qDebug("~cvInterface()");
    disconnect(cvSocket, SIGNAL(hostFound()), this, SLOT(slotHostFounded()));
    disconnect(cvSocket, SIGNAL(connected()), this, SLOT(slotConnected()));
    disconnect(cvSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    disconnect(cvSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotWritten(qint64)));
    disconnect(cvSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    disconnect(cvSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displayError(QAbstractSocket::SocketError)));
    cvSocket->deleteLater();
}

// public //////////////////////////////////////////////////////////////////////////////////

void cvInterface::connectToCV(QString host, quint16 port)
{
    emit cvStateChanged(cvConnecting);
    if (cvSocket->state() == QAbstractSocket::UnconnectedState) {
        cvSocket->connectToHost(host, port);
//    if (cvSocket->waitForConnected(5000)) {
//        qDebug() << "wait for connect OK";
//    } else {
//        qDebug() << "wait for connect NOT OK";
//        emit cvStateChanged(cvError);
//    }
    }
}

void cvInterface::disconnectFromCV()
{
    emit cvStateChanged(cvDisconnecting);
    if (cvSocket->isValid() && cvSocket->state() == QAbstractSocket::ConnectedState) {
        cvSocket->disconnectFromHost();
    }
}

// public slots //////////////////////////////////////////////////////////////////////////////////

void cvInterface::slotHostFounded()
{
    qDebug("host found");
}

void cvInterface::slotConnected()
{
    emit cvStateChanged(cvConnected);
    qDebug("connected");
}

void cvInterface::slotReadyRead()
{
    qDebug("ready read");
}

void cvInterface::slotWritten(qint64 bytes)
{
    qDebug() << "written" << bytes;
}

void cvInterface::slotDisconnected()
{
    emit cvStateChanged(cvDisconnected);
    qDebug("disconnected");
}

void cvInterface::displayError(QAbstractSocket::SocketError socketError)
{
    emit cvStateChanged(cvError);
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug("remote host closed");
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug("host was not found");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug("connection was refused by the peer");
            break;
        default:
            qDebug() << "error" << cvSocket->errorString();
    }
}
