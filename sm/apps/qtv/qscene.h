#ifndef QSCENE_H
#define QSCENE_H

#include "qtv.h"

class QScene : public QGraphicsScene
{
	Q_OBJECT


public:
	QScene();

signals:


public slots:

	void new_laser_data(LDP);
	void adjust();

};

#endif
