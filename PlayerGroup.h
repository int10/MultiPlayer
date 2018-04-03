#ifndef PLAYERGROUP_H
#define PLAYERGROUP_H
#include <QStringList>
#include <QList>
#include <QTimer>
//#include <QMediaPlayer>
#include "QtAV.h"

//#define USE_QMEDIAPLAYER
#define MAX_AUDIO_FILE		(4)
class PlayerGroup : public QObject
{
	Q_OBJECT
public:
	PlayerGroup(QStringList audiolist, QStringList videolist, QList<QtAV::VideoOutput *> videooutput);
	~PlayerGroup();

	void Play(int index);
	void Play();
	void PlayPause();
	void Stop();
	void Seek(qint64 value);
	bool IsPlaying();
	int notifyInterval();
	qint64 position();
	qint64 duration();
	void SwitchAudio(int index);
	QList<QtAV::VideoOutput *> GetVideoOutput();
	bool AddVideoOutput(int index, QtAV::VideoOutput * output);
	void RemoveVideoOutput(int index, QtAV::VideoOutput * output);
	void SetVolume(int value);
	void Fb();
	void Ff();
signals:
	void Signal_UpdateSliderUnit();
	void Signal_PositionChanged(qint64 value);
	void Signal_Started();
	void Signal_StateChanged(QtAV::AVPlayer::State state);
//	void Signal_mediaStateChanged(QMediaPlayer::State state);
private slots:
	void updateSliderUnit();
	void updateSlider(qint64 value);
	void updateSlider();
	void stateChanged(QtAV::AVPlayer::State state);
	//void mediaStateChanged(QMediaPlayer::State state);

private:
	QStringList m_audiolist;
	QStringList m_videolist;
	QList<QtAV::VideoOutput *> m_volist;
	QList<QtAV::AVPlayer *> m_playerlist;
	QtAV::AVClock m_mainclock;
#ifdef USE_QMEDIAPLAYER
	QMediaPlayer * m_audioplayer;
#else
	QtAV::AVPlayer *m_audioplayer;
#endif
	bool m_isplaying;
	int m_curaudioindex;
	qint64 m_audiopos;
};

#endif // PLAYERGROUP_H
