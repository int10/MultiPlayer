#include "ConfigXml.h"
#include <QDebug>

ConfigXml::ConfigXml()
{

}

ConfigXml::~ConfigXml()
{

}

bool ConfigXml::ParseXml(QString filepath, QList<sAudioInfo> &audioinfolist, QList<sVideoInfo> &videoinfolist)
{
	QFile xmlfile(filepath);
	if(!xmlfile.open(QIODevice::ReadOnly)) return false;
	QByteArray xmldata = xmlfile.readAll();
	QString xmlstr = QString::fromUtf8(xmldata);

	QRegExp rxvideo("<video>(.+)</video>");
	QRegExp rxaudio("<wave>(.+)</wave>");
	QRegExp rxfile("<file>(.+)</file>");
	QRegExp rxdesc("<desc>(.+)</desc>");
	QRegExp rxorder("<display_order>(.+)</display_order>");

	rxvideo.setMinimal(true);
	rxaudio.setMinimal(true);
	int index = 0;
	while(1) {
		index = rxvideo.indexIn(xmlstr, index);
		if(index == -1) break;
		QString videostr = rxvideo.cap(1);
		index += rxvideo.cap(0).size();
		sVideoInfo vinfo;
		if(videostr.contains(rxfile)){
			vinfo.file = rxfile.cap(1);
		}
		if(videostr.contains(rxdesc)){
			vinfo.desc = rxdesc.cap(1);
		}
		if(videostr.contains(rxorder)){
			vinfo.order = rxorder.cap(1).toInt();
		}
		if(vinfo.file != "" && vinfo.desc != ""){
			videoinfolist.append(vinfo);
		}
	}
	index = 0;
	while(1) {
		index = rxaudio.indexIn(xmlstr, index);
		if(index == -1) break;
		QString audiostr = rxaudio.cap(1);
		index += rxaudio.cap(0).size();
		sAudioInfo ainfo;
		if(audiostr.contains(rxfile)){
			ainfo.file = rxfile.cap(1);
		}
		if(audiostr.contains(rxdesc)){
			ainfo.desc = rxdesc.cap(1);
		}
		if(ainfo.file != "" && ainfo.desc != ""){
			audioinfolist.append(ainfo);
		}
	}
	if(audioinfolist.size() && videoinfolist.size())
		return true;
	else
		return false;
}
