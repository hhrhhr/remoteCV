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
            qDebug() << "CVSimulator::processUpdate: previous command OK";
        } else {
            qDebug() << "CVSimulator::processUpdate: small packet detected:\n" << telemetry;
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

    // parse data
    {
        fd[N].simTime = in.at(0).toInt();

        fd[N].M = QMatrix4x4(
                      in.at( 1).toDouble(), in.at( 2).toDouble(), in.at( 3).toDouble(), in.at( 4).toDouble(),
                      in.at( 5).toDouble(), in.at( 6).toDouble(), in.at( 7).toDouble(), in.at( 8).toDouble(),
                      in.at( 9).toDouble(), in.at(10).toDouble(), in.at(11).toDouble(), in.at(12).toDouble(),
                      in.at(13).toDouble(), in.at(14).toDouble(), in.at(15).toDouble(), in.at(16).toDouble());

        fd[N].rate = QVector3D(in.at(17).toDouble() * RAD2DEG,
                               in.at(18).toDouble() * RAD2DEG,
                               in.at(19).toDouble() * RAD2DEG);

        fd[N].step = in.at(20).toFloat();

        fd[N].speed = QVector3D(in.at(21).toDouble(),
                                in.at(22).toDouble(),
                                in.at(23).toDouble());

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

    FlightData& f = fd[N];

    att->time = f.simTime;

    att->position = QVector3D(f.M(0,3),
                              f.M(1,3),
                              f.M(2,3));

    att->gyro = QVector3D(f.rate.x(),
                          f.rate.y(),
                          f.rate.z());

    att->speed = QVector3D(f.speed.x(),
                           f.speed.y(),
                           f.speed.z());

    QVector3D rpy;
    cvMatrix2rpy(f.M, rpy);
//    qDebug() << "rpy      " << rpy;

    QQuaternion Q;
    cvMatrix2quaternion(f.M, Q);
//    qDebug() << "quat     " << Q;

    // just for check
    QVector3D rpy2;
    quaternion2rpy(Q, rpy2);
//    qDebug() << "rpy2     " << rpy2 << "\n";

    att->attitude = QVector3D(rpy.x(),
                              rpy.y(),
                              rpy.z());

    att->attitude2 = QVector3D(rpy2.x(),
                              rpy2.y(),
                              rpy2.z());

    att->controls[0] = f.controls[0];
    att->controls[1] = f.controls[1];
    att->controls[2] = f.controls[2];
    att->controls[3] = f.controls[3];
    att->controls[4] = f.controls[4];
    att->controls[5] = f.controls[5];
}

// private

/*
 * all conversion code from http://www.euclideanspace.com
 */

void CVcalc::cvMatrix2rpy(const QMatrix4x4 &M, QVector3D &rpy)
{
    qreal roll;
    qreal pitch;
    qreal yaw;

    if (M(1, 0) > 0.998d) { // ~86.3°
        // north pole gimbal lock
        qDebug() << "NORTH 111";
        roll  = 0.0d;
        pitch = M_PI_2;
        yaw   = qAtan2(M(0, 2), M(2, 2));
    } else if (M(1, 0) < -0.998d) {
        // south pole gimbal lock
        qDebug() << "SOUTH 111";
        roll  = 0.0d;
        pitch = -M_PI_2;
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

    // TODO: check "_copysign" on other platforms (win ok)
    x = _copysign(x, (M(2, 1) - M(1, 2)));
    y = _copysign(y, (M(0, 2) - M(2, 0)));
    z = _copysign(z, (M(1, 0) - M(0, 1)));

    Q.setX(x);
    Q.setY(y);
    Q.setZ(z);
    Q.setScalar(w);
}

void CVcalc::quaternion2rpy(const QQuaternion &Q, QVector3D &rpy)
{
    qreal roll, pitch, yaw;

    qreal test = Q.x() * Q.y() + Q.z() * Q.scalar();
    if (qFabs(test) > 0.499d) {
        // north/south pole gimbal lock
        if (test > 0.499d)
            qDebug() << "NORTH 222";
        if (test < -0.499d)
            qDebug() << "SOUTH 222";
        roll = 0.0d;
        pitch = _copysign(M_PI_2, test);
        yaw = 2.0d * qAtan2(Q.x(), Q.scalar());
        yaw = _copysign(yaw, test);
    } else {
        qreal qxx = Q.x() * Q.x();  //
        qreal qyy = Q.y() * Q.y();  //
        qreal qzz = Q.z() * Q.z();  //

        qreal r1 = 2.0d * Q.x() * Q.scalar() - 2.0d * Q.y() * Q.z();
        qreal y1 = 2.0d * Q.y() * Q.scalar() - 2.0d * Q.x() * Q.z();

        qreal r2 = 1.0d - 2.0d * qxx - 2.0d * qzz;
        qreal y2 = 1.0d - 2.0d * qyy - 2.0d * qzz;

        roll = qAtan2(r1, r2);
        pitch = qAsin(2.0d * test);
        yaw = qAtan2(y1, y2);
    }

    rpy.setX(roll  * RAD2DEG);
    rpy.setY(pitch * RAD2DEG);
    rpy.setZ(yaw   * RAD2DEG);
}
