#ifndef QPARSER_H
#define QPARSER_H

#include "qtv.h"

class QParser : public QObject
{
	Q_OBJECT

public:

signals:
	void parsed_laser_data(LDP ld);

public slots:

	void new_line(QString);


};

#endif
