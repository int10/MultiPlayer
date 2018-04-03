#ifndef CONFIGXML_H
#define CONFIGXML_H

#include <QFile>
#include <QList>
#include <QRegExp>

typedef struct {
	QString file;
	QString desc;
}sAudioInfo;

typedef struct {
	QString file;
	QString desc;
	int order;
}sVideoInfo;

class ConfigXml
{
public:
	ConfigXml();
	~ConfigXml();
	bool ParseXml(QString filepath, QList<sAudioInfo> &audioinfolist, QList<sVideoInfo> &videoinfolist);
};

#endif // CONFIGXML_H
