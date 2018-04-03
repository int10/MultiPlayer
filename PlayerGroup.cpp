#include "PlayerGroup.h"
#include <QDebug>

PlayerGroup::PlayerGroup(QStringList audiolist, QStringList videolist, QList<QtAV::VideoOutput *> videooutput)
{
	m_audiolist = audiolist;
	m_videolist = videolist;

	for(int i = 0; i < videolist.size(); i++) {
		if(videolist.at(i) != "") {
			QtAV::AVPlayer *player = new (QtAV::AVPlayer);
			m_playerlist.append(player);
			player->setRenderer(videooutput.at(i));
		} else {
			m_playerlist.append(NULL);
		}
	}
	m_isplaying = false;
	m_audiopos = 0;

#ifdef USE_QMEDIAPLAYER
	m_audioplayer = new QMediaPlayer;
	connect(m_audioplayer, SIGNAL(positionChanged(qint64)), SLOT(updateSlider(qint64)));
	//	connect(m_audioplayer, SIGNAL(started()), SLOT(updateSlider()));
	connect(m_audioplayer, SIGNAL(notifyIntervalChanged()), SLOT(updateSliderUnit()));
	connect(m_audioplayer, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(stateChanged(QtAV::AVPlayer::State)));

#else
	m_audioplayer = new QtAV::AVPlayer;
	connect(m_audioplayer, SIGNAL(positionChanged(qint64)), SLOT(updateSlider(qint64)));
	connect(m_audioplayer, SIGNAL(started()), SLOT(updateSlider()));
	connect(m_audioplayer, SIGNAL(notifyIntervalChanged()), SLOT(updateSliderUnit()));
	connect(m_audioplayer, SIGNAL(stateChanged(QtAV::AVPlayer::State)), SLOT(stateChanged(QtAV::AVPlayer::State)));

#endif
}

PlayerGroup::~PlayerGroup()
{
	Stop();
	foreach(QtAV::AVPlayer *player, m_playerlist) {
		if(player) {
			delete player;
		}
	}
	m_playerlist.clear();
	foreach(QtAV::VideoOutput *output, m_volist) {
		delete output;
	}
	m_volist.clear();
	if(m_audioplayer){
		delete m_audioplayer;
	}
}

void PlayerGroup::Play(int index)
{
	m_curaudioindex = index;
	for(int i = 0; i < m_videolist.size(); i++) {
		if(0 == i) {
		#ifdef USE_QMEDIAPLAYER
			m_audioplayer->setMedia(QUrl::fromLocalFile(m_audiolist.at(index)));
			m_audioplayer->play();
		#else
			m_audioplayer->play(m_audiolist.at(index));
		#endif
			if(m_playerlist.at(i)) {
				m_playerlist.at(i)->play(m_videolist.at(i));
			}

		} else {
			if(m_playerlist.at(i)) {
				m_playerlist.at(i)->play(m_videolist.at(i));
			}
		}
	}
	m_isplaying = true;
}

bool PlayerGroup::IsPlaying()
{
	return m_isplaying;
}

void PlayerGroup::PlayPause()
{
#ifdef USE_QMEDIAPLAYER
	if(QMediaPlayer::PausedState == m_audioplayer->state()) {
		m_audioplayer->play();
		foreach(QtAV::AVPlayer *player, m_playerlist) {
			if(player){
				player->play();
			}
		}
		m_isplaying = true;
		return;
	}
	foreach(QtAV::AVPlayer *player, m_playerlist) {
		if(player) {
			player->pause(!player->isPaused());
		}
	}
	m_audioplayer->pause();
	m_isplaying = false;

#else
	//好忧伤，原来理解错了，AVPlayer的isPlaying在pause状态下也是返回true的。。导致这个类的IsPlaying的返回值跟AVPlayer的返回值对不上，这里要留意一下。
	if (!m_audioplayer->isPlaying()) {
			foreach(QtAV::AVPlayer *player, m_playerlist) {
			if(player){
				player->play();
			}
		}
		m_audioplayer->play();
		m_isplaying = true;
		return;
	}

	foreach(QtAV::AVPlayer *player, m_playerlist) {
		if(player) {
			player->pause(!player->isPaused());
		}
	}
	m_audioplayer->pause(!m_audioplayer->isPaused());
	if(m_audioplayer->isPaused()) {
		m_isplaying = false;
	} else {
		m_isplaying = true;
	}

#endif
}

void PlayerGroup::SwitchAudio(int index)
{
	if(index < 0 || index >= m_audiolist.size() || index == m_curaudioindex){
		return;
	}

	m_curaudioindex = index;
#ifdef USE_QMEDIAPLAYER
	qint64 pos = m_audioplayer->position();
	m_audioplayer->stop();
	m_audioplayer->setMedia(QUrl::fromLocalFile(m_audiolist.at(index)));
	m_audioplayer->setPosition(pos);
	m_audioplayer->play();

	if(m_isplaying == false) {
		foreach(QtAV::AVPlayer *player, m_playerlist) {
			if(player) {
				player->play();
			}
		}
		m_isplaying = true;
	}
#else
	m_audiopos = m_audioplayer->position();
	m_audioplayer->stop();
	m_audioplayer->setFile(m_audiolist.at(index));
	m_audioplayer->play();
	//it will seek at slot state change
#endif
}

void PlayerGroup::Stop()
{
	foreach(QtAV::AVPlayer * player, m_playerlist){
		if(player) {
			player->stop();
		}
	}
	m_audioplayer->stop();
	m_isplaying = false;
}

void PlayerGroup::Seek(qint64 value)
{
#ifdef USE_QMEDIAPLAYER
	m_audioplayer->setPosition(value);
	foreach(QtAV::AVPlayer * player, m_playerlist){
		if(player) {
			player->setPosition(value);
		}
	}
#else
	m_audioplayer->seek(value);
	foreach(QtAV::AVPlayer * player, m_playerlist){
		if(player) {
			player->seek(value);
		}
	}
#endif

}

void PlayerGroup::updateSliderUnit()
{
	emit Signal_UpdateSliderUnit();
}

void PlayerGroup::updateSlider(qint64 value)
{
	static qint64 tinysync = 0; //if a faster than v ,set a positive num , else negative num.
	qint64 pos = m_audioplayer->position();
	foreach(QtAV::AVPlayer * player, m_playerlist){
		if(player && player->isPlaying()) {
			if(pos > tinysync) {
				if(qAbs(pos + tinysync - player->position()) > 1000){	//seek in cast a-v bigger than 1s;
					player->setPosition(pos + tinysync);
				} else if(qAbs(pos + tinysync - player->position()) > 200) {	//sync in cast a-v bigger than 200ms;
					player->updateClock(pos + tinysync);
				}
			}
		}
	}

//	QList<int> poslist;
//	poslist.append(m_audioplayer->position());
//	foreach(QtAV::AVPlayer * player, m_playerlist){
//		if(player) {
//			poslist.append(player->position());
//		} else {
//			poslist.append(0);
//		}
//	}
//	qDebug()<<poslist;

	emit Signal_PositionChanged(value);
}

void PlayerGroup::updateSlider()
{
	emit Signal_Started();
}

int PlayerGroup::notifyInterval()
{
#ifdef USE_QMEDIAPLAYER
	return 1000;
#else
	return m_audioplayer->notifyInterval();
#endif
}

qint64 PlayerGroup::position()
{
#ifdef USE_QMEDIAPLAYER
	return m_audioplayer->position();
#else
	return m_audioplayer->position();
#endif
}

qint64 PlayerGroup::duration()
{
#ifdef USE_QMEDIAPLAYER
	return m_audioplayer->duration();
#else
	return m_audioplayer->duration();
#endif
}

void PlayerGroup::stateChanged(QtAV::AVPlayer::State state)
{
	if(QtAV::AVPlayer::PlayingState == state) {
		if(m_audiopos != 0) {
			m_audioplayer->seek(m_audiopos);
			m_audiopos = 0;
			if(m_isplaying == false) {
				foreach(QtAV::AVPlayer *player, m_playerlist) {
					if(player && player->isPaused()) {
						player->pause(!player->isPaused());
					}
				}
				m_isplaying = true;
			}
		}
	}
	emit Signal_StateChanged(state);
}

//void PlayerGroup::mediaStateChanged(QMediaPlayer::State state)
//{
//	emit Signal_mediaStateChanged(state);
//}

QList<QtAV::VideoOutput *> PlayerGroup::GetVideoOutput()
{
	return m_volist;
}

bool PlayerGroup::AddVideoOutput(int index, QtAV::VideoOutput * output)
{
	if(index >= m_playerlist.size()) return false;
	if(!m_playerlist.at(index)) return false;
	if(!m_playerlist.at(index)->isPlaying()) return false;
	m_playerlist.at(index)->addVideoRenderer(output);
	return true;
}

void PlayerGroup::RemoveVideoOutput(int index, QtAV::VideoOutput * output)
{
	if(index >= m_playerlist.size()) return;
	if(!m_playerlist.at(index)) return;
	if(!m_playerlist.at(index)->isPlaying()) return;
	m_playerlist.at(index)->removeVideoRenderer(output);
}

void PlayerGroup::SetVolume(int value)
{
	qreal kVolumeInterval = 0.04;
	QtAV::AudioOutput *ao = m_audioplayer ? m_audioplayer->audio() : 0;
	qreal v = qreal(value)*kVolumeInterval;
	if (ao) {
		if (qAbs(int(ao->volume()/kVolumeInterval) - value) >= int(0.1/kVolumeInterval)) {
			ao->setVolume(v);
		}
	}
}

void PlayerGroup::Fb()
{
	Seek(position() - 5000);
}

void PlayerGroup::Ff()
{
	Seek(position() + 5000);
}
