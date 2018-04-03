#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtAV.h"
#include "PlayerGroup.h"
#include <QLabel>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include "ConfigXml.h"
#include <QButtonGroup>
#include <QSlider>
#include "FormControlPanel.h"
#include <QMap>
#include <QTime>

namespace Ui {
class MainWindow;
}

typedef struct{
	QtAV::VideoOutput *output;
	QLabel *label;
}sVideoWindow;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	bool IsReady();
private slots:
	void on_btnOpen_clicked();
	void on_btnPlay_clicked();
	void on_btnPause_clicked();
	void on_btnStop_clicked();
	void seekBySlider(int value);
	void seekBySlider();
	void on_sliProcess_sliderMoved(int position);

	void updateSlider(qint64 value);
	void updateSlider();
	void updateSliderUnit();
	void Slot_ClickBtnGroup(int id);
	void Slot_StateChanged(QtAV::AVPlayer::State state);
//	void Slot_MediaStateChanged(QMediaPlayer::State state);
	void Slot_UpdateVolume(int volume);
	void on_btnIdle_clicked();
	void on_sliVolume_sliderPressed();
	void on_sliVolume_valueChanged(int value);
	void on_btnFb_clicked();
	void on_btnFf_clicked();
private:
//	void Play(QString xmlfilename);
	void Play();
	void PlayFullScreen(int index);
	void ExitFullScreen(int index);
	void SetStopState();
//	void SetPlayState(QList<sAudioInfo> audiocount);
	void SetPlayState(QStringList audiolist);
	bool PrepareFileList();
	void InitVideoInterface();
	void SetVolume();
	bool SwitchAudio();
protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
private:
	Ui::MainWindow *ui;
	QList<sVideoWindow *> m_videoout;
	QtAV::VideoOutput *m_singlevideooutput;
	PlayerGroup *m_playergroup;
	int m_unit;
	int m_dstindex, m_curindex;	//音频index，dst为要切换到的音频，cur为当前播放的音频。
	int m_fullscreenindex;
	qint64 m_position;
	QButtonGroup *m_audiobtngroup;
	QString m_xmlfilepath;
	//QSlider * m_sliderprocess2;	//single play slider
	QStringList m_audiolist;
	QStringList m_videolist;
	FormControlPanel *m_controlpanel;
	bool m_showcontrolpanel;
	QMap<int, int> m_videoposmap;
	QDateTime m_recordtime;
	bool m_ready;
};

#endif // MAINWINDOW_H
