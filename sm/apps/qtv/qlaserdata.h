#ifndef QLASERDATA_H
#define QLASERDATA_H

#include "qtv.h"

class QLaserData : public QObject, public QGraphicsItem
{
	 Q_OBJECT

public:
	 QLaserData(LDP ld);

	QPainterPath createCountour();

	 QRectF boundingRect() const;
	 QPainterPath shape() const;
	 void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
					QWidget *widget);

protected:
	 void timerEvent(QTimerEvent *event);

private:
	LDP ld;
	QPainterPath countour;
};

#endif
