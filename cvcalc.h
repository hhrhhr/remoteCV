#ifndef CVCALC_H
#define CVCALC_H

#include <QObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include <QStringList>

class FlightData : public QObject
{
    Q_OBJECT

public:
    explicit FlightData(QObject *parent = 0);
    ~FlightData();

    quint32 simTime;    // ms
                        // m[row][column]
    QMatrix4x4 M;       // m[0][3] posX, m[1][3] posY, m[2][3] posZ; m
    QVector3D rate;     // roll, yaw, pitch; rad/sec
    float step;         // simulator step time; ms (default = 1/120)
    QVector3D speed;    // speed vector in world coord: X, Y, Z; m/sec
    quint8 crash;       // is crashed: 0-no, 1-yes
    quint8 stop;        // is engine stopped: 0-worked, 1-stop
    quint8 model;       // model: 0-heli, 1-plane
    float controls[6];  // ail, ele, thr, rud, thrhld/gear, iddle/flaps; -1...+1
    uchar cr;           // "\n" at end
};

/////////////////////////////////////////////////////////////////////////////////////////

class CVcalc : public QObject
{
    Q_OBJECT

public:
    explicit CVcalc(QObject *parent = 0);
    ~CVcalc();

    void getTelemetry(QString telemetry);
    void processTelemetry();

private:
    quint8 fd_ptr; //number of copy and counter
    FlightData* fd;

    void matrix2quaternion(const QMatrix4x4 M, QQuaternion Q);
    void matrix2rpy(const QMatrix4x4 M, QVector3D rpy);
    void quaternion2rpy(const QQuaternion Q, QVector3D rpy);
};

#endif // CVCALC_H
