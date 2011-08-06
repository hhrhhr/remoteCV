#include "cvcalc.h"

const qreal CVcalc::RAD2DEG = (180.0/M_PI);
const qreal CVcalc::DEG2RAG = (M_PI/180.0);

///////////////////////////////////////////////////////

CVcalc::CVcalc(QObject *parent) :
    QObject(parent)
{
    qDebug() << "CVcalc";
    fd = new FlightData[2];
    fd_ptr = 0;

    att = new Attitude;
}

CVcalc::~CVcalc()
{
    qDebug() << "~CVcalc";
    delete att;
    delete[] fd;
}

// public

void CVcalc::getTelemetry(QString telemetry)
{
    QStringList in = telemetry.split(" ");
    if ( in.length() < 34 ) {
        if (in.length() == 1 && in.at(0) == "ok\n") {
            qDebug() << "previous command OK";
        } else {
            qDebug() << "small packet detected:\n" << telemetry;
        }
        return;
    }

    quint8 N; // new
    quint8 O; // old
    if (fd_ptr) {
        fd_ptr--;
        N = 1;
        O = 0;
    } else {
        fd_ptr++;
        N = 0;
        O = 1;
    }

    // parse data start
    {
        fd[N].simTime = in.at(0).toInt();

        fd[N].M = QMatrix4x4(
                  in.at( 1).toDouble(), in.at( 2).toDouble(), in.at( 3).toDouble(), in.at( 4).toDouble(),
                  in.at( 5).toDouble(), in.at( 6).toDouble(), in.at( 7).toDouble(), in.at( 8).toDouble(),
                  in.at( 9).toDouble(), in.at(10).toDouble(), in.at(11).toDouble(), in.at(12).toDouble(),
                  in.at(13).toDouble(), in.at(14).toDouble(), in.at(15).toDouble(), in.at(16).toDouble());

        fd[N].rate = QVector3D(in.at(17).toDouble(), in.at(18).toDouble(), in.at(19).toDouble());

        fd[N].step = in.at(20).toFloat();

        fd[N].speed = QVector3D(in.at(21).toDouble(), in.at(22).toDouble(), in.at(23).toDouble());

        fd[N].crash = in.at(24).toInt();
        fd[N].stop  = in.at(25).toInt();
        fd[N].model = in.at(26).toInt();

        fd[N].controls[0] = in.at(27).toFloat();
        fd[N].controls[1] = in.at(28).toFloat();
        fd[N].controls[2] = in.at(29).toFloat();
        fd[N].controls[3] = in.at(30).toFloat();
        fd[N].controls[4] = in.at(31).toFloat();
        fd[N].controls[5] = in.at(32).toFloat();

        fd[N].cr = in.at(33).toInt();
    }
    // parse end

    // conversion start
    {
        FlightData& f  = fd[N]; // new data
        FlightData& fo = fd[O]; // old data

        att->time       = f.simTime;
        att->position   = QVector3D(f.M(0, 3), f.M(1, 3), f.M(2, 3));
        att->gyro       = f.rate * -RAD2DEG;
        att->speedNED   = f.speed;
        att->airspeed   = f.speed.length();
        QVector3D gs = QVector3D(f.speed.x(), 0.0, f.speed.z());
        att->groundspeed= gs.length();

        // roll, pitch, yaw
        QVector3D rpy1;
        cvMatrix2rpy(f.M, rpy1);
        rpy1 *= -1; // mirror the vector
        att->attitude  = rpy1;

        // quaternion
        QQuaternion Q;
        cvMatrix2quaternion(f.M, Q);

        // second variant of rpy
        QVector3D rpy2;
        quaternion2rpy(Q, rpy2);
        rpy2 *= -1; // mirror the vector
        att->attitude2 = rpy2;


        // accelerations
        qreal dt = f.simTime - fo.simTime;
        QVector3D dv = f.speed - fo.speed;
        QVector3D a = dv * 1000 / dt;
        a.setX(a.x() * -1);
        a.setY((a.y() - 9.81d));
        a.setZ(a.z() * -1);
        Q = Q.conjugate();
        a = Q.rotatedVector(a);
        att->accel = a;

        // control channels
        for (int i = 0; i < 6; ++i)
            att->controls[i] = qint8(64 * f.controls[i]);
    }
    // conversion end
}

// private


// all conversion code from http://www.euclideanspace.com
// TODO: check "copysign" on other platforms (win ok, ubuntu ok...)

void CVcalc::cvMatrix2rpy(const QMatrix4x4 &M, QVector3D &rpy)
{
    qreal roll;
    qreal pitch;
    qreal yaw;

    if (qFabs(M(1, 0)) > 0.998d) { // ~86.3°
        // gimbal lock
        roll  = 0.0d;
        pitch = copysign(M_PI_2, M(1, 0));
        yaw   = qAtan2(M(0, 2), M(2, 2));
    } else {
        roll  = qAtan2(-M(1, 2), M(1, 1));
        pitch = qAsin ( M(1, 0));
        yaw   = qAtan2(-M(2, 0), M(0, 0));
    }

    rpy.setX(roll  * RAD2DEG);
    rpy.setY(pitch * RAD2DEG);
    rpy.setZ(yaw   * RAD2DEG);
}

void CVcalc::cvMatrix2quaternion(const QMatrix4x4 &M, QQuaternion &Q)
{
    qreal x, y, z, w;

    w = qSqrt(qMax(0.0d,   1.0d + M(0, 0)  + M(1, 1)  + M(2, 2))) / 2.0d;
    x = qSqrt(qMax(0.0d,  (1.0d + M(0, 0)) - M(1, 1)  - M(2, 2))) / 2.0d;
    y = qSqrt(qMax(0.0d, ((1.0d - M(0, 0)) + M(1, 1)) - M(2, 2))) / 2.0d;
    z = qSqrt(qMax(0.0d,  (1.0d - M(0, 0)  - M(1, 1)) + M(2, 2))) / 2.0d;

    x = copysign(x, (M(2, 1) - M(1, 2)));
    y = copysign(y, (M(0, 2) - M(2, 0)));
    z = copysign(z, (M(1, 0) - M(0, 1)));

    Q.setX(x);
    Q.setY(y);
    Q.setZ(z);
    Q.setScalar(w);
}

void CVcalc::quaternion2rpy(const QQuaternion &Q, QVector3D &rpy)
{
    qreal roll, pitch, yaw;
    qreal d2 = 2.0d;

    qreal test = d2 * (Q.x() * Q.y() + Q.z() * Q.scalar());
    if (qFabs(test) > 0.998d) {
        // gimbal lock
        roll = 0.0d;
        pitch = copysign(M_PI_2, test);
        yaw = d2 * qAtan2(Q.x(), Q.scalar());
        yaw = copysign(yaw, test);
    } else {
        qreal qxx = Q.x() * Q.x();
        qreal qyy = Q.y() * Q.y();
        qreal qzz = Q.z() * Q.z();

        qreal r1 = d2 * (Q.x() * Q.scalar() - Q.y() * Q.z());
        qreal r2 = 1.0d - (d2 * (qxx + qzz));

        qreal y1 = d2 * (Q.y() * Q.scalar() - Q.x() * Q.z());
        qreal y2 = 1.0d - (d2 * (qyy + qzz));

        roll = qAtan2(r1, r2);
        pitch = qAsin(test);
        yaw = qAtan2(y1, y2);
    }

    rpy.setX(roll  * RAD2DEG);
    rpy.setY(pitch * RAD2DEG);
    rpy.setZ(yaw   * RAD2DEG);
}
