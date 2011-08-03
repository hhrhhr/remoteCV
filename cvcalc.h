#ifndef CVCALC_H
#define CVCALC_H

#include <QObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

class CVcalc : public QObject
{
    Q_OBJECT

public:
    explicit CVcalc(QObject *parent = 0);

    void getTelemetry(QByteArray telemetry);

signals:

public slots:

private:
//    void setControl(QString control);
    void matrix2quaternion(const QMatrix4x4 M, QQuaternion Q);
    void matrix2rpy(const QMatrix4x4 M, QVector3D rpy);
    void quaternion2rpy(const QQuaternion Q, QVector3D rpy);

};

#endif // CVCALC_H
