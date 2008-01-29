#include "qmapview.h"
#include "qmapview.moc"


#include <QDebug>
#include <QGraphicsScene>
#include <QWheelEvent>

#include <math.h>

QMapView::QMapView()
{
	setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	
	 setCacheMode(CacheBackground);
	 setRenderHint(QPainter::Antialiasing);
	 setTransformationAnchor(AnchorUnderMouse);
	 setResizeAnchor(AnchorViewCenter);

	 scale(0.8, 0.8);
	 setMinimumSize(400, 400);
	 setWindowTitle(tr("QMapView"));
}

void QMapView::keyPressEvent(QKeyEvent *event)
{
	 switch (event->key()) {
	 case Qt::Key_Up:
		  break;
	 case Qt::Key_Down:
		  break;
	 case Qt::Key_Left:
		  break;
	 case Qt::Key_Right:
		  break;
	 case Qt::Key_Plus:
		  scaleView(1.2);
		  break;
	 case Qt::Key_Minus:
		  scaleView(1 / 1.2);
		  break;
	 case Qt::Key_Space:
	 default:
		  QGraphicsView::keyPressEvent(event);
	 }
}


void QMapView::wheelEvent(QWheelEvent *event)
{
	 scaleView(pow((double)2, -event->delta() / 240.0));
}


void QMapView::scaleView(qreal scaleFactor)
{
	 qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	 if (factor < 0.07 || factor > 100)
		  return;

	 scale(scaleFactor, scaleFactor);
}
