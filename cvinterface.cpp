#include "cvinterface.h"

CVInterface::CVInterface(QObject *parent) :
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

    m_calc = new CVcalc(this);
}

CVInterface::~CVInterface()
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

void CVInterface::connectToCV(QString host, quint16 port)
{
    qDebug() << "connectToCV";
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

void CVInterface::disconnectFromCV()
{
    qDebug() << "disconnectFromCV";
    emit cvStateChanged(cvDisconnecting);
    cvSocket->disconnectFromHost();
}

void CVInterface::sendCommand(QString command)
{
    qDebug() << command;
    if (cvSocket->isValid()) {
        cvSocket->write(command.toAscii());
        cvSocket->flush();
    }
}

// public slots //////////////////////////////////////////////////////////////////////////////////

void CVInterface::slotHostFounded()
{
    qDebug("host lookup has succeeded");
    emit cvStateChanged(cvHostFound);
    emit cvStateError("host lookup has succeeded");
}

void CVInterface::slotConnected()
{
    qDebug("connection has been successfully established");
    emit cvStateChanged(cvConnected);
    emit cvStateError("connection has been successfully established");
}

void CVInterface::slotReadyRead()
{
    qDebug("ready read");
    QByteArray telemetry = cvSocket->readLine();
    qDebug() << telemetry;
    m_calc->getTelemetry(telemetry);
}

void CVInterface::slotWritten(qint64 bytes)
{
    qDebug() << "written" << bytes;
}

void CVInterface::slotDisconnected()
{
    qDebug("socket has been disconnected");
    emit cvStateChanged(cvDisconnected);
    emit cvStateError("socket has been disconnected");
}

void CVInterface::displayError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit cvStateChanged(cvError);
    emit cvStateError(cvSocket->errorString());
    qDebug() << "error" << cvSocket->errorString();
}
