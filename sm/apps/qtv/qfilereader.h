#ifndef QFILEREADER_H
#define QFILEREADER_H

#include <QThread>
#include <QIODevice>

class QFileReader : public QThread
{
	Q_OBJECT

	QIODevice * source;
	
public:
	QFileReader(QIODevice * source);
	void run();

signals:
	void new_line(QString);
	void eof();
};

#endif
