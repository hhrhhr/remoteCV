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

    accelUseSpeed = TRUE;
}

CVcalc::~CVcalc()
{
    qDebug() << "~CVcalc";
    delete att;
    delete[] fd;
}

// public
QString CVcalc::parseTelemetry()
{
    FlightData& f  = fd[fd_ptr]; // current data
    Attitude* a = att;
    QString out = "";
    out.append("sim time\t" + QString::number(f.simTime) + "\n");
    out.append("matrix:\t"
               + QString::number(f.M(0,0), 'f', 6) + "\t"
               + QString::number(f.M(0,1), 'f', 6) + "\t"
               + QString::number(f.M(0,2), 'f', 6) + "\t"
               + QString::number(f.M(0,3), 'f', 6) + "\n\t"
               + QString::number(f.M(1,0), 'f', 6) + "\t"
               + QString::number(f.M(1,1), 'f', 6) + "\t"
               + QString::number(f.M(1,2), 'f', 6) + "\t"
               + QString::number(f.M(1,3), 'f', 6) + "\n\t"
               + QString::number(f.M(2,0), 'f', 6) + "\t"
               + QString::number(f.M(2,1), 'f', 6) + "\t"
               + QString::number(f.M(2,2), 'f', 6) + "\t"
               + QString::number(f.M(2,3), 'f', 6) + "\n\t"
               + QString::number(f.M(3,0), 'f', 6) + "\t"
               + QString::number(f.M(3,1), 'f', 6) + "\t"
               + QString::number(f.M(3,2), 'f', 6) + "\t"
               + QString::number(f.M(3,3), 'f', 6) + "\n\n");
    out.append("quat:\t"
               + QString::number(a->quat.scalar(), 'f', 6) + "\t"
               + QString::number(a->quat.x(), 'f', 6) + "\t"
               + QString::number(a->quat.y(), 'f', 6) + "\t"
               + QString::number(a->quat.z(), 'f', 6) + "\n");
    out.append("rpy1:\t"
               + QString::number(a->attitude.x(), 'f', 6) + "\t"
               + QString::number(a->attitude.y(), 'f', 6) + "\t"
               + QString::number(a->attitude.z(), 'f', 6) + "\n");
    out.append("rpy2:\t"
               + QString::number(a->attitude2.x(), 'f', 6) + "\t"
               + QString::number(a->attitude2.y(), 'f', 6) + "\t"
               + QString::number(a->attitude2.z(), 'f', 6) + "\n\n");
    out.append("rate:\t"
               + QString::number(f.rate.x(), 'f', 6) + "\t"
               + QString::number(f.rate.y(), 'f', 6) + "\t"
               + QString::number(f.rate.z(), 'f', 6) + "\n\n");
    out.append("speed:\t"
               + QString::number(f.speed.x(), 'f', 6) + "\t"
               + QString::number(f.speed.y(), 'f', 6) + "\t"
               + QString::number(f.speed.z(), 'f', 6) + "\n\n");

    return out;
}

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

    // swap structures
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

    FlightData& fn = fd[N]; // new data
    FlightData& fo = fd[O]; // old data

    // parse data start
    {
        fn.simTime = in.at(0).toInt();

        fn.M = QMatrix4x4(
                  in.at( 1).toDouble(), in.at( 2).toDouble(), in.at( 3).toDouble(), in.at( 4).toDouble(),
                  in.at( 5).toDouble(), in.at( 6).toDouble(), in.at( 7).toDouble(), in.at( 8).toDouble(),
                  in.at( 9).toDouble(), in.at(10).toDouble(), in.at(11).toDouble(), in.at(12).toDouble(),
                  in.at(13).toDouble(), in.at(14).toDouble(), in.at(15).toDouble(), in.at(16).toDouble());

        fn.rate = QVector3D(in.at(17).toDouble(), in.at(18).toDouble(), in.at(19).toDouble());

        fn.step = in.at(20).toFloat();

        fn.speed = QVector3D(in.at(21).toDouble(), in.at(22).toDouble(), in.at(23).toDouble());

        fn.crash = in.at(24).toInt();
        fn.stop  = in.at(25).toInt();
        fn.model = in.at(26).toInt();

        fn.controls[0] = in.at(27).toFloat();
        fn.controls[1] = in.at(28).toFloat();
        fn.controls[2] = in.at(29).toFloat();
        fn.controls[3] = in.at(30).toFloat();
        fn.controls[4] = in.at(31).toFloat();
        fn.controls[5] = in.at(32).toFloat();

        fn.cr = in.at(33).toInt();
    }
    // parse end

    // conversion start
    {
        att->time       = fn.simTime;
        att->position   = QVector3D(fn.M(0, 3), fn.M(1, 3), fn.M(2, 3));
        att->gyro       = fn.rate * -RAD2DEG;
        att->speedNED   = fn.speed;
        att->airspeed   = fn.speed.length();
        att->groundspeed= QVector3D(fn.speed.x(), 0.0, fn.speed.z()).length();

        // roll, pitch, yaw from matrix
        QVector3D rpy1;
        cvMatrix2rpy(fn.M, rpy1);
        // mirror the vector
        rpy1 *= -1;
        att->attitude  = rpy1;

        // quaternion from matrix
        QQuaternion Q;
        cvMatrix2quaternion(fn.M, Q);
        att->quat = Q;

        // roll, pitch, yaw from quaternion
        QVector3D rpy2;
        quaternion2rpy(Q, rpy2);
        // mirror the vector
        rpy2 *= -1;
        att->attitude2 = rpy2;

        // time delta
        qreal dt = (fn.simTime - fo.simTime);
        dt /= 1000;

        // speed delta
        QVector3D dv;
        if (accelUseSpeed) {
            dv = fn.speed - fo.speed;
        } else {
            QVector3D sn = QVector3D(fn.M(0, 3), fn.M(1, 3), fn.M(2, 3));
            QVector3D so = QVector3D(fo.M(0, 3), fo.M(1, 3), fo.M(2, 3));
            QVector3D ds = sn - so;
            // current speed
            QVector3D v = ds / dt;
            fn.speed2 = v;
            att->speedNED2 = v;
            dv = v - fo.speed2;
        }
        // acceleration
        QVector3D a = dv / dt;
        // add gravity
        a.setY(a.y() - 9.81d);
        // rotate world to model
//        a = fn.M.inverted().mapVector(a);
        QQuaternion Qinv = Q.conjugate();
        QQuaternion A = QQuaternion(0, a);
        a = (Qinv * A * Q).vector();
        att->accel = a;

        // control channels
        for (int i = 0; i < 6; ++i)
            att->controls[i] = qint8(64 * fn.controls[i]);
    }
    // conversion end
}

// private

// all conversion code from http://www.euclideanspace.com
// TODO: check "copysign" function on other platforms (win ok, ubuntu ok...)

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
    qreal w, x, y, z;

    w = qSqrt(qMax(0.0d,   1.0d + M(0, 0)  + M(1, 1)  + M(2, 2))) / 2.0d;
    x = qSqrt(qMax(0.0d,  (1.0d + M(0, 0)) - M(1, 1)  - M(2, 2))) / 2.0d;
    y = qSqrt(qMax(0.0d, ((1.0d - M(0, 0)) + M(1, 1)) - M(2, 2))) / 2.0d;
    z = qSqrt(qMax(0.0d,  (1.0d - M(0, 0)  - M(1, 1)) + M(2, 2))) / 2.0d;

    x = copysign(x, (M(2, 1) - M(1, 2)));
    y = copysign(y, (M(0, 2) - M(2, 0)));
    z = copysign(z, (M(1, 0) - M(0, 1)));

    Q.setScalar(w);
    Q.setX(x);
    Q.setY(y);
    Q.setZ(z);
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
