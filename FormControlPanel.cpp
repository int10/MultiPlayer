#include "FormControlPanel.h"
#include "ui_FormControlPanel.h"
#include <QToolTip>
#include <QDebug>
#include <QTime>

FormControlPanel::FormControlPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FormControlPanel)
{
	ui->setupUi(this);
	ui->sliVolume->setMaximum(100);
	ui->sliVolume->setValue(100);
	this->setAutoFillBackground(true);
	QPalette p = this->palette();
	p.setColor(QPalette::Window,QColor(64, 64, 64, 128));
	this->setPalette(p);
	//ui->sliProcess->setMouseTracking(true);
	connect(ui->sliProcess, SIGNAL(onHover(int,int)), SLOT(onTimeSliderHover(int,int)));
}

FormControlPanel::~FormControlPanel()
{
	delete ui;
}

void FormControlPanel::SetStopState()
{
	ui->btnPlay->setEnabled(true);
	ui->btnPlay->setVisible(true);
	ui->btnPause->setEnabled(false);
	ui->btnPause->setVisible(false);
	ui->btnStop->setEnabled(false);
	ui->btnFb->setEnabled(false);
	ui->btnFf->setEnabled(false);
	ui->btnPre->setEnabled(false);
	ui->btnNext->setEnabled(false);
	ui->sliProcess->setEnabled(false);
	ui->sliVolume->setEnabled(false);
}

void FormControlPanel::SetPlayState()
{
	ui->btnPlay->setEnabled(true);
	ui->btnPlay->setVisible(false);
	ui->btnPause->setEnabled(true);
	ui->btnPause->setVisible(true);
	ui->btnStop->setEnabled(true);
	ui->btnFb->setEnabled(true);
	ui->btnFf->setEnabled(true);
	ui->sliProcess->setEnabled(true);
	ui->sliVolume->setEnabled(true);
}

void FormControlPanel::SetPlayPause(bool play)
{
	if(play) {
		ui->btnPlay->setVisible(false);
		ui->btnPause->setVisible(true);
	} else {
		ui->btnPlay->setVisible(true);
		ui->btnPause->setVisible(false);
	}
}

void FormControlPanel::UpdateSlider(int max, int value, QString str)
{
	ui->sliProcess->setRange(0, max);
	ui->sliProcess->setValue(value);
	ui->lbProcess->setText(str);
}

void FormControlPanel::on_btnFb_clicked()
{
	emit SignalFb();
}

void FormControlPanel::on_btnFf_clicked()
{
	emit SignalFf();
}

void FormControlPanel::on_btnPlay_clicked()
{
	emit SignalPlay();
}

void FormControlPanel::on_btnStop_clicked()
{
	emit SignalStop();
}

void FormControlPanel::on_btnPause_clicked()
{
	emit SignalPause();
}

void FormControlPanel::on_sliProcess_sliderMoved(int position)
{
	emit SignalProcessMove(position);
}

void FormControlPanel::on_sliVolume_sliderPressed()
{
	emit SignalUpdateVolume(ui->sliVolume->value());
}

void FormControlPanel::on_sliVolume_valueChanged(int value)
{
	emit SignalUpdateVolume(ui->sliVolume->value());
}

void FormControlPanel::SyncVolume(int volume)
{
	ui->sliVolume->setValue(volume);
}

void FormControlPanel::onTimeSliderHover(int pos, int value)
{
	QString timestr = QTime(0, 0, 0).addSecs(value).toString(QString::fromLatin1("HH:mm:ss"));
	ui->sliProcess->SetTipStr(timestr);
}

QPoint FormControlPanel::MapGlobal(QWidget *widget)
{
	QWidget *curwidget = widget;
	QPoint point = widget->pos();
	while(curwidget->parent()){
		point = ((QWidget *)curwidget->parent())->mapToParent(point);
		curwidget = (QWidget *)curwidget->parent();
	}
	return point;
}

void FormControlPanel::on_sliProcess_sliderPressed()
{
	emit SignalProcessMove(ui->sliProcess->value());
}

void FormControlPanel::SetProcessTipStr(QString tipstr)
{
	ui->sliProcess->SetTipStr(tipstr);
}
