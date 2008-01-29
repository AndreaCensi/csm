#include "qparser.h"
#include "qparser.moc"

void QParser::new_line(QString qqs) {
	const char * s = strdup(qqs.toAscii().constData());
	LDP ld = ld_read_smart_string(s);
	if(ld) {
		emit parsed_laser_data(ld);
	}
}

