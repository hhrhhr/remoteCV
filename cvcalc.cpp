#include "cvcalc.h"

const float CVcalc::RAD2DEG = (180.0/M_PI);

FlightData::FlightData(QObject *parent) :
    QObject(parent)
{
    qDebug() << "FlightData";
}
FlightData::~FlightData()
{
    qDebug() << "~FlightData";
}

///////////////////////////////////////////////////////

CVcalc::CVcalc(QObject *parent) :
    QObject(parent)
{
    qDebug() << "CVcalc";
    fd_ptr = 2;
    fd = new FlightData[fd_ptr];
    fd_ptr = 0;
}

CVcalc::~CVcalc()
{
    qDebug() << "~CVcalc";
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

    fd[N].simTime = in.at(0).toInt();

    qDebug() << fd[N].simTime << fd[O].simTime;

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

// private

void CVcalc::matrix2quaternion(const QMatrix4x4 M, QQuaternion Q)
{

}

void CVcalc::matrix2rpy(const QMatrix4x4 M, QVector3D rpy)
{

}

void CVcalc::quaternion2rpy(const QQuaternion Q, QVector3D rpy)
{

}
