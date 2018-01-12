#ifndef MOTORSTATUS_H
#define MOTORSTATUS_H

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <QComboBox>
#include <Qwt/qwt.h>
#include <qwt_plot_grid.h>
#include <Qwt/qwt_plot.h>
#include <Qwt/qwt_plot_curve.h>
#include <Qwt/qwt_samples.h>
#include <qwt_plot_zoomer.h>
#include <QPointF>
#include "seriallogic.h"

#define POSITION_TITLE 0
#define SPEED_TITLE    1



class MotorStatus : public QObject
{
    Q_OBJECT

private:
    int startSystemTime1;
    int startSystemTime2;
    int systemTimeBefore1;
    int systemTimeBefore2;

    int removeNum1= 0;
    int removeNum2= 0;

    int axisResolution1= 1;
    int axisResolution2= 1;

    QVector<QPointF> points1;
    QVector<QPointF> points2;

    QwtPlot* plotCurve1;
    QwtPlot* plotCurve2;

    QwtPlotCurve* dataCurve1;
    QwtPlotCurve* dataCurve2;

    QwtText title;

public:
    MotorStatus(QWidget *pParentW, QWidget *sParentW);
    void initPlot(QwtPlot *pPlot, QWidget *pParantW, QwtPlotCurve *_dataCurve, bool titleFlag);

private slots:
    void updatePlot(long systemTime, int motorID, double newData1, double newData2);
};

#endif // MOTORSTATUS_H
