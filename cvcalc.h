#ifndef CVCALC_H
#define CVCALC_H

#include <QObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include <QStringList>
#include <qmath.h>

class FlightData
{
public:
    FlightData() {}
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

class Attitude
{
public:
    Attitude() {}
    quint32 time;           // sim timer, ms
    QVector3D attitude;     // euler angles (roll, pith, yaw), deg
    QVector3D attitude2;
    QVector3D gyro;         // rates, deg/sec
    QVector3D accel;        // accelerations, m/s^2
    QVector3D speedNED;     // speed in NED coord, m/s
    qreal groundspeed;      // ground speed , m/s
    qreal airspeed;         // air speed, m/s
    QVector3D position;     // position in world, m
    qint8 controls[6];      // controls, +/-64
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

    Attitude* att;

private:
    static const qreal RAD2DEG;
    static const qreal DEG2RAG;
    quint8 fd_ptr; //number of copy and counter
    FlightData* fd;

    void cvMatrix2quaternion(const QMatrix4x4& M, QQuaternion& Q);
    void cvMatrix2quaternion_2(const QMatrix4x4& M, QQuaternion& Q);
    void cvMatrix2rpy(const QMatrix4x4& M, QVector3D& rpy);
    void cvMatrix2rpy_2(const QMatrix4x4& M, QVector3D& rpy);
    void quaternion2rpy(const QQuaternion& Q, QVector3D& rpy);
    void quaternion2rpy_2(const QQuaternion& Q, QVector3D& rpy);
};

#endif // CVCALC_H
