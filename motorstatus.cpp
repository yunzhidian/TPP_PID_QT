#include "motorstatus.h"

MotorStatus::MotorStatus(QWidget *pParentW, QWidget *sParentW)
{
    plotCurve1 = new QwtPlot(pParentW);
    plotCurve2 = new QwtPlot(sParentW);

    dataCurve1 = new QwtPlotCurve("position");
    dataCurve2 = new QwtPlotCurve("speed");

    initPlot(plotCurve1, pParentW, dataCurve1, POSITION_TITLE);
    initPlot(plotCurve2, sParentW, dataCurve2, SPEED_TITLE);
}

void MotorStatus::initPlot(QwtPlot *pPlot, QWidget *pParantW, QwtPlotCurve *_dataCurve, bool titleFlag)
{
    int width = pParantW->width()+940;
    int height = pParantW->height()+245;

//    qDebug()<<width<<height;

    //设置QwtPlot控件的大小
//    pPlot->move(0,0);
    pPlot->resize(width,height);
    pPlot->setAxisScale(QwtPlot::yLeft,0,100,10);
    pPlot->setAxisScale(QwtPlot::xBottom,0,10);
    pPlot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignHCenter | Qt::AlignBottom);

    //设置字体
    QFont fontAxisTitle("Helvetica", 12);
    title.setFont(fontAxisTitle);
    //设置颜色
    title.setColor(QColor(0,0,0));
    title.setText(QObject::tr("时间(s)"));    //设置标题名
    pPlot->setAxisTitle(QwtPlot::xBottom, title);
    if(titleFlag == POSITION_TITLE)          //设置标题名
        title.setText(QObject::tr("实际位置"));
    else
        title.setText(QObject::tr("实际速度"));
    pPlot->setAxisTitle(QwtPlot::yLeft, title);

    QFont fontAxis("Helvetica", 12);//Helvetica [Cronyx]
    pPlot->setAxisFont(QwtPlot::xBottom,fontAxis);
    pPlot->setAxisFont(QwtPlot::yLeft,fontAxis);

    //加入网格
    QwtPlotGrid *pPlotGrid;
    pPlotGrid = new QwtPlotGrid();
    pPlotGrid->setPen(QColor(222,222,222),1);
    pPlotGrid->attach(pPlot);

    //设置画笔
    _dataCurve->setPen(QColor(255,0,0),2,Qt::SolidLine);
    //抗锯齿
    _dataCurve->setRenderHint(QwtPlotItem::RenderAntialiased,true);
    //把曲线附加到qwtPlot上
    _dataCurve->attach(pPlot);
    QwtPlotZoomer* zoomer = new QwtPlotZoomer( pPlot->canvas() );
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt::RightButton );
}

void MotorStatus::updatePlot(long systemTime, int motorID, double newData1, double newData2)
{
    int deltaT1 = systemTime - systemTimeBefore1;
//    qDebug("System Time: %ld\n", systemTime);
    if(points1.size() != 0)//points内有点时
    {
        if(removeNum1 == 0 || points1.last().x() < 10)//前10s内，包括10s
        {
            startSystemTime1 = startSystemTime1;
        }
        else
        {
            startSystemTime1 = startSystemTime1 + deltaT1;
        }
    }
    else
    {
        startSystemTime1 = systemTime;//没接收点时
    }

    if(removeNum1 != 0)//10s之后接收的点每次要向左移deltaT，所以之前接收到的点横坐标也要相应减去deltaT
    {
        for(int i = 0;i<points1.size();i++)
        {
            points1[i].setX(points1[i].x() - deltaT1/1000.0);
        }
    }
    points1<<QPointF((systemTime-startSystemTime1)/1000.0, newData1/axisResolution1);
    plotCurve1->setAxisScale(QwtPlot::xBottom, 0, 10);
    dataCurve1->setSamples(points1);
    dataCurve1->attach(plotCurve1);
    plotCurve1->replot();
//    qDebug() <<" points1.size() : "<<points1.size()<<" removeNum1 : "<<removeNum1<<" startSystemTime1 : "<<startSystemTime1<< " X:Y"<< axisX << axisY<<" points1[0].x():"<<points1[0].x()<<" points1.last():"<<points1.last().x()<<"\n";
    if(removeNum1 == 0)//第1次超过10s时除去第一个点
    {
        while(points1.size() > 1 && points1.last().x()-points1[0].x()>10000/1000.0)
        {
    //        qDebug() <<"points1.remove:"<<points1.first().x()<<"\n";
            points1.removeFirst();
            removeNum1 = removeNum1 + 1;
        }
    }
    else//10s之后每当第二个点的横坐标在0左侧时除去第一个点，保证画出的线起点一定在0左侧
    {
        while(points1.size() > 1 && points1[1].x()<=0)
        {
    //        qDebug() <<"points1.remove:"<<points1.first().x()<<"\n";
            points1.removeFirst();
            removeNum1 = removeNum1 + 1;
        }
    }
    systemTimeBefore1 = systemTime;
//    qDebug() <<" points1.size() : "<<points1.size()<<"\n";



    int deltaT2 = systemTime - systemTimeBefore2;
    if(points2.size() != 0)
    {
        if(removeNum2 == 0 || points2.last().x() < 10)//前10s内，包括10s
        {
            startSystemTime2 = startSystemTime2;
        }
        else
        {
            startSystemTime2 = startSystemTime2 + deltaT2;
        }
    }
    else
    {
        startSystemTime2 = systemTime;
    }

    if(removeNum2 != 0)
    {
        for(int j = 0;j<points2.size();j++)
        {
            points2[j].setX(points2[j].x() - deltaT2/1000.0);
        }
    }
    points2<<QPointF((systemTime-startSystemTime2)/1000.0, newData2/axisResolution2);
    plotCurve2->setAxisScale(QwtPlot::xBottom, 0, 10);
    dataCurve2->setSamples(points2);
    dataCurve2->attach(plotCurve2);
    plotCurve2->replot();
    if(removeNum2 == 0)
    {
        while(points2.size() > 1 && points2.last().x()-points2[0].x()>10000/1000.0)
        {
    //        qDebug() <<"points1.remove:"<<points1.first().x()<<"\n";
            points2.removeFirst();
            removeNum2 = removeNum2 + 1;
        }
    }
    else
    {
        while(points2.size() > 1 && points2[1].x()<=0)
        {
    //        qDebug() <<"points1.remove:"<<points1.first().x()<<"\n";
            points2.removeFirst();
            removeNum2 = removeNum2 + 1;
        }
    }
    systemTimeBefore2 = systemTime;

}
