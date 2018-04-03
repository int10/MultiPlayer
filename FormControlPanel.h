#ifndef FORMCONTROLPANEL_H
#define FORMCONTROLPANEL_H

#include <QWidget>

namespace Ui {
class FormControlPanel;
}

class FormControlPanel : public QWidget
{
	Q_OBJECT

public:
	explicit FormControlPanel(QWidget *parent = 0);
	~FormControlPanel();
	void SetStopState();
	void SetPlayState();
	void SetPlayPause(bool play);
	void UpdateSlider(int max, int value, QString str);
	void SyncVolume(int volume);
signals:
	void SignalFb();
	void SignalFf();
	void SignalStop();
	void SignalPlay();
	void SignalPause();
	void SignalProcessMove(int position);
	void SignalUpdateVolume(int volume);
private slots:
	void on_btnFb_clicked();
	void on_btnFf_clicked();
	void on_btnPlay_clicked();
	void on_btnStop_clicked();
	void on_btnPause_clicked();
	void on_sliProcess_sliderMoved(int position);
	void on_sliVolume_sliderPressed();
	void on_sliVolume_valueChanged(int value);

private:
	Ui::FormControlPanel *ui;
};

#endif // FORMCONTROLPANEL_H
