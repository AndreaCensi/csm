#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H


#include <QtGui/QGraphicsView>

#include <QtOpenGL/QtOpenGL>


class QMapView : public QGraphicsView
{
    Q_OBJECT

public:
    QMapView();

protected:
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void scaleView(qreal scaleFactor);

};

#endif
