#include "qtv.h"

int main(int argc, char*argv[]) {
	
	QApplication app(argc, argv);
   
	QFile *qin = new QFile();
	if(!qin->open(stdin, QIODevice::ReadOnly)) {
		printf("Could not open stding as QFile.\n");
		exit(1);
	}
	QFileReader *qfr = new QFileReader(qin);
	QParser *qp = new QParser;
	
	QScene * qscene = new QScene();
	
	QObject::connect(qfr, SIGNAL(new_line(QString)), qp,  SLOT(new_line(QString)));
	QObject::connect(qfr, SIGNAL(eof()), qscene,  SLOT(adjust()));

	QObject::connect(qp, SIGNAL(parsed_laser_data(LDP)), qscene,  SLOT(new_laser_data(LDP)));
	
	
	QMapView view;
	view.setScene(qscene);
   view.setCacheMode(QGraphicsView::CacheBackground);
   view.setDragMode(QGraphicsView::ScrollHandDrag);
	view.scale(10,10);
   view.show();
	qfr->start();
	
	return app.exec();
}