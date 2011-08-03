#ifndef CVINTERFACE_H
#define CVINTERFACE_H

#include <QObject>
#include <QTcpSocket>

class cvInterface : public QObject
{
    Q_OBJECT

public:
    explicit cvInterface(QObject *parent = 0);
    ~cvInterface();

    void connectToCV(QString host, quint16 port);
    void disconnectFromCV();

    enum cvState {
        cvConnecting,
        cvConnected,
        cvDisconnecting,
        cvDisconnected,
        cvError,
        cvUnknown = -1
    };

private:
    QTcpSocket* cvSocket;

signals:
    void cvStateChanged(cvInterface::cvState state);

public slots:
    void slotHostFounded();
    void slotConnected();
    void slotReadyRead();
    void slotWritten(qint64 bytes);
    void slotDisconnected();
    void displayError(QAbstractSocket::SocketError socketError);

};

#endif // CVINTERFACE_H
