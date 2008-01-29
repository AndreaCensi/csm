#include "qscene.h"
#include "qscene.moc"

#include "qlaserdata.h"

QScene::QScene() {
	setSceneRect(-20, -20, 20, 20);
	setItemIndexMethod(QGraphicsScene::NoIndex);
}


void QScene::new_laser_data(LDP ld) {
	addItem(new QLaserData(ld));
}

void QScene::adjust() {
	setSceneRect(itemsBoundingRect());
}


