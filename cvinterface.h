#ifndef CVINTERFACE_H
#define CVINTERFACE_H

#include <QObject>
#include <QTcpSocket>
#include <cvcalc.h>
#include <QString>

class CVInterface : public QObject
{
    Q_OBJECT

public:
    explicit CVInterface(QObject *parent = 0);
    ~CVInterface();

    void connectToCV(QString host, quint16 port);
    void disconnectFromCV();

    void sendCommand(QString command);

    enum cvState {
        cvConnecting,
        cvHostFound,
        cvConnected,
        cvDisconnecting,
        cvDisconnected,
        cvError,
        cvUnknown = -1
    };

private:
    QTcpSocket *cvSocket;
    CVcalc *m_calc;


signals:
    void cvStateChanged(CVInterface::cvState state);
    void cvStateError(QString socketError);

private slots:
    void slotHostFounded();
    void slotConnected();
    void slotReadyRead();
    void slotWritten(qint64 bytes);
    void slotDisconnected();
    void displayError(QAbstractSocket::SocketError socketError);

};

#endif // CVINTERFACE_H
