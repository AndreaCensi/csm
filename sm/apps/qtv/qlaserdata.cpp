#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>

#include "qlaserdata.h"
#include "qlaserdata.moc"


QLaserData::QLaserData(LDP ld): ld(ld)
{
	this->countour = createCountour();
	double *p = ld->estimate;
	setPos(QPointF(p[0],p[1]));
	rotate(CSM::rad2deg(p[2]));
}

QRectF QLaserData::boundingRect() const
{
	return countour.boundingRect();
}

QPainterPath QLaserData::shape() const
{
	return countour;
}

QPainterPath QLaserData::createCountour() {
	ld_compute_cartesian(ld);
	struct stroke_sequence sequence[ld->nrays];
	compute_stroke_sequence(ld, sequence, 100, 0.1);
	
	QPainterPath path;
	for(int i=0;i<ld->nrays;i++) {
		if(sequence[i].valid==0) continue;
		double *p = sequence[i].p;
		if(sequence[i].begin_new_stroke)
			path.moveTo(p[0], p[1]);
		else
			path.lineTo(p[0], p[1]);
	}
	return path;
}

void QLaserData::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	 // Body
//		painter->setBrush(color);
/*	 painter->drawEllipse(-10, -20, 20, 40);

	 // Eyes
	 painter->setBrush(Qt::white);
	 painter->drawEllipse(-10, -17, 8, 8);
	 painter->drawEllipse(2, -17, 8, 8);

	 // Nose
	 painter->setBrush(Qt::black);
	 painter->drawEllipse(QRectF(-2, -22, 4, 4));


	 // Ears
		painter->setBrush(scene()->collidingItems(this).isEmpty() ? Qt::darkYellow : Qt::red);
	 painter->drawEllipse(-17, -12, 16, 16);
	 painter->drawEllipse(1, -12, 16, 16);
*/
	 // Tail
	 painter->setPen(Qt::black);
	 painter->setBrush(Qt::NoBrush);
/*	 painter->setBrush(Qt::red);*/
	 painter->drawPath(countour);
}

void QLaserData::timerEvent(QTimerEvent *)
{

  //	rotate(dx);
//		setPos(mapToParent(0, -(3 + sin(speed) * 3)));
}
