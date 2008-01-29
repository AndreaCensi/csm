#include "qfilereader.h"
#include "qfilereader.moc"

QFileReader::QFileReader(QIODevice * source) :
	source(source) {

}
	
void QFileReader::run() {
	printf("Running\n");
		
	while(1) {
		char buf[10024];
		qint64 lineLength = source->readLine(buf, sizeof(buf));
		if (lineLength == -1) break;
		emit new_line(QString(buf));
		
/*		source->waitForBytesWritten(-1);
		if(source->canReadLine()) {
			printf("can\n");
			QByteArray data = source->readLine();
			emit new_line(QString(data));
		} else {
			printf(".");
		}*/
	}
	
	emit eof();
}

