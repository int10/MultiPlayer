#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QSlider>
#include <QSettings>
#include <QMessageBox>
#define VERSION "1.8"

using namespace QtAV;
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_unit = 1000;
	m_playergroup = NULL;
	m_dstindex = 0;
	m_curindex = 0;
	m_xmlfilepath = "";
	m_fullscreenindex = -1;
	m_singlevideooutput = NULL;
	m_controlpanel = NULL;
	m_showcontrolpanel = false;
	m_ready = true;
	m_videoout.clear();
	this->setWindowTitle(QString("Multi player ") + VERSION);

	/// 设置控件stylesheet ///
	QFile qssfile(QApplication::applicationDirPath() + "/skin.qss");
	if(qssfile.open(QIODevice::ReadOnly)){
		qApp->setStyleSheet(qssfile.readAll());
		qssfile.close();
	}

	m_audiobtngroup = new QButtonGroup(this);
	m_audiobtngroup->addButton(ui->btnAudio1, 0);
	m_audiobtngroup->addButton(ui->btnAudio2, 1);
	m_audiobtngroup->addButton(ui->btnAudio3, 2);
	m_audiobtngroup->addButton(ui->btnAudio4, 3);
	connect(m_audiobtngroup, SIGNAL(buttonClicked(int)), this, SLOT(Slot_ClickBtnGroup(int)));
	foreach(QAbstractButton * btn, m_audiobtngroup->buttons()) {
		btn->setCheckable(true);
	}

	//SetStopState();
	ui->btnIdle->setVisible(false);
	ui->btnOpen->setVisible(false);
	ui->sliVolume->setMaximum(100);
	ui->sliVolume->setValue(100);

	setAcceptDrops(true);

	if(PrepareFileList()){
		InitVideoInterface();
		SetStopState();
		//Play();
	}
}

MainWindow::~MainWindow()
{
	foreach (sVideoWindow * vo, m_videoout) {
		if(vo->label) {
			delete vo->label;
			vo->label = NULL;
		}
		if(vo->output) {
			delete vo->output;
			vo->output = NULL;
		}
		delete vo;
	}
	m_videoout.clear();

	if(m_audiobtngroup) {
		delete m_audiobtngroup;
		m_audiobtngroup = NULL;
	}
	if(m_singlevideooutput) {
		delete m_singlevideooutput;
		m_singlevideooutput = NULL;
	}
	if(m_controlpanel) {
		delete m_controlpanel;
		m_controlpanel = NULL;
	}
	delete ui;
}

void MainWindow::InitVideoInterface()
{
	QStringList videolabel;
	QString labelstyple("QLabel{font: 15pt \"微软雅黑\"; color: rgb(255, 255, 255);  }");
	videolabel<<"律師  ADVOGADO"<<"法官  JUIZ"<<"檢察官  MINISTÉRIO PÚBLICO"<<"嫌犯  ARGUIDO"<<"證人  TESTEMUNHA"<<"證人  TESTEMUNHA"<<"證人  TESTEMUNHA";
	m_videoposmap.clear();
	for(int i = 0; i < videolabel.size() - 1; i++) {//隐蔽证人在后面特殊处理
		sVideoWindow * vo = new sVideoWindow;
		vo->label = new QLabel;
		vo->output = new (QtAV::VideoOutput);
		QVBoxLayout *vl =  new QVBoxLayout();
		vo->label->setAlignment(Qt::AlignHCenter);
		vo->label->setStyleSheet(labelstyple);
		vo->label->setText(videolabel.at(i));
		vl->addWidget(vo->label);
		vl->addWidget(vo->output->widget());
		vl->setStretch(0,0);
		vl->setStretch(1,1);
		int row = (i < 3)?0:1;
		int column = i%3;
		ui->loVideoList->addLayout(vl, row, column);
		m_videoposmap[row * 10 + column] = i;
		m_videoout.append(vo);
	}
	if(m_videolist.size() >= videolabel.size()){	//有隐蔽证人
		sVideoWindow * vo = new sVideoWindow;
		vo->label = new QLabel;
		vo->output = new (QtAV::VideoOutput);
		QVBoxLayout *vl =  new QVBoxLayout();
		vo->label->setAlignment(Qt::AlignHCenter);
		vo->label->setStyleSheet(labelstyple);
		vo->label->setText(videolabel.at(videolabel.size() - 1));
		vl->addWidget(vo->label);
		vl->addWidget(vo->output->widget());
		vl->setStretch(0,0);
		vl->setStretch(1,1);
		int row = 0;
		int column = 3;
		ui->loVideoList->addLayout(vl, row, column);
		m_videoposmap[row * 10 + column] = videolabel.size() - 1;
		m_videoout.append(vo);
	}

	m_singlevideooutput = new (QtAV::VideoOutput);
	ui->loSinVideo->addWidget(m_singlevideooutput->widget());
	m_controlpanel = new FormControlPanel(ui->page_2);
	connect(m_controlpanel, SIGNAL(SignalFb()), this, SLOT(on_btnFb_clicked()));
	connect(m_controlpanel, SIGNAL(SignalFf()), this, SLOT(on_btnFf_clicked()));
	connect(m_controlpanel, SIGNAL(SignalStop()), this, SLOT(on_btnStop_clicked()));
	connect(m_controlpanel, SIGNAL(SignalPause()), this, SLOT(on_btnPause_clicked()));
	connect(m_controlpanel, SIGNAL(SignalPlay()), this, SLOT(on_btnPlay_clicked()));
	connect(m_controlpanel, SIGNAL(SignalProcessMove(int)), this, SLOT(on_sliProcess_sliderMoved(int)));
	connect(m_controlpanel, SIGNAL(SignalUpdateVolume(int)), this, SLOT(Slot_UpdateVolume(int)));

	//从第一个视频文件名里拿录视频的日期，时间。
	QRegExp rxdt("^Court_(\\d+)-(\\d+)-(\\d+).asf");
	QString strdt;
	foreach(QString dt, m_videolist) {
		QString filename = dt.mid(dt.lastIndexOf("/") + 1);
		if(filename.contains(rxdt)){
			strdt = rxdt.cap(3);
			break;
		}
	}
	strdt = strdt.left(14);
	m_recordtime = QDateTime::fromString(strdt, "yyyyMMddhhmmss");
	ui->lbRecordTime->setText(m_recordtime.toString("yyyy-MM-dd hh:mm:ss"));
	ui->lbCurTime->setText(m_recordtime.toString("hh:mm:ss"));

	//设置mouse trace，激活全屏时鼠标移动事件
	m_controlpanel->setMouseTracking(true);
	m_singlevideooutput->widget()->setMouseTracking(true);
	ui->page_2->setMouseTracking(true);
	ui->stackedWidget->setMouseTracking(true);
	ui->centralwidget->setMouseTracking(true);
	setMouseTracking(true);
}

void MainWindow::seekBySlider(int value)
{
	if(m_playergroup && m_playergroup->IsPlaying()){
		m_playergroup->Seek(qint64(value * m_unit));
	}
}

void MainWindow::seekBySlider()
{
	seekBySlider(ui->sliProcess->value());
}

void MainWindow::on_btnOpen_clicked()
{
//	QString file = QFileDialog::getOpenFileName(0, tr("Open a xml"), "", "xml file(*.xml)");
//	if (file.isEmpty()) {
//		return;
//	}
//	Play(file);
}

void MainWindow::on_btnPlay_clicked()
{
	if(!m_playergroup) {	//如果当前是stop状态，play。
		Play();
		return;
	}
	if(m_playergroup->IsPlaying()) return;
	if(m_dstindex == m_curindex) {
		m_playergroup->PlayPause();
		ui->btnPlay->setVisible(false);
		ui->btnPause->setVisible(true);
		m_controlpanel->SetPlayPause(true);
	} else {
		SwitchAudio();
	}
}

void MainWindow::on_btnPause_clicked()
{
	if(!m_playergroup->IsPlaying()) return;
	m_playergroup->PlayPause();
	ui->btnPlay->setVisible(true);
	ui->btnPause->setVisible(false);
	m_controlpanel->SetPlayPause(false);
}

void MainWindow::on_btnStop_clicked()
{
	m_playergroup->Stop();
	SetStopState();
	delete m_playergroup;
	m_playergroup = NULL;

}

void MainWindow::updateSlider(qint64 value)
{
	ui->sliProcess->setRange(0, int(m_playergroup->duration()/m_unit));
	ui->sliProcess->setValue(int(value/m_unit));

	//m_sliderprocess2->setRange(0, int(m_playergroup->duration()/m_unit));
	//m_sliderprocess2->setValue(int(value/m_unit));

	int total_sec = m_playergroup->duration()/1000;
	int min = total_sec/60;
	int sec = total_sec - min * 60;

	int ptotal_sec = value/1000;
	int pmin = ptotal_sec/60;
	int psec = ptotal_sec - pmin * 60;
	QString str;
	str.sprintf("%0d:%02d/%0d:%02d", pmin, psec, min, sec);
	ui->lbProcess->setText(str);
	QDateTime playtime = m_recordtime.addSecs(ptotal_sec);
	ui->lbCurTime->setText(playtime.toString("hh:mm:ss"));
	m_controlpanel->UpdateSlider(int(m_playergroup->duration()/m_unit), int(value/m_unit), str);
}

void MainWindow::updateSlider()
{
	updateSlider(m_playergroup->position());
}

void MainWindow::updateSliderUnit()
{
	m_unit = m_playergroup->notifyInterval();
	updateSlider();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list")) {
		event->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *event)
{
//	QList<QUrl> urls = event->mimeData()->urls();

//	if (urls.isEmpty()) {
//		return;
//	}

//	QString fileName = urls.first().toLocalFile();
//	if (fileName.isEmpty()) {
//		return;
//	}
//	if(!fileName.endsWith(".xml")){
//		return;
//	}
//	Play(fileName);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(event->button() != Qt::LeftButton) return;
	if(NULL == m_playergroup) return;
	if(!m_playergroup->IsPlaying() && -1 == m_fullscreenindex) return;	//pause下不全屏

	if(-1 == m_fullscreenindex){
		int index = 0;
		for(int row = 0; row < ui->loVideoList->rowCount(); row++){
			for(int column = 0; column < ui->loVideoList->columnCount(); column++) {
				if(ui->loVideoList->cellRect(row, column).contains(event->pos())){
					index = m_videoposmap.value(row * 10 + column, -1);	//从videomap中获取双击的窗口对应视频的index;
					if(-1 != index) {
						PlayFullScreen(index);
					}
					break;
				}
			}
		}
	} else {
		ExitFullScreen(m_fullscreenindex);
	}
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
//	int startx = 0;
//	int starty = m_singlevideooutput->widget()->height() - m_singlevideooutput->widget()->y() - m_controlpanel->height();
//	int w = m_singlevideooutput->widget()->width();
//	int h = m_controlpanel->height();
//	if(-1 != m_fullscreenindex) {
//		if(m_showcontrolpanel){
//			m_controlpanel->hide();
//			m_showcontrolpanel = false;
//		} else {
//			m_controlpanel->setGeometry(startx, starty, w, h);
//			m_controlpanel->show();
//			m_showcontrolpanel = true;
//		}
//	}
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	int startx = 0;
	int starty = m_singlevideooutput->widget()->height() - m_singlevideooutput->widget()->y() - m_controlpanel->height();
	int w = m_singlevideooutput->widget()->width();
	int h = m_controlpanel->height();
	if(-1 != m_fullscreenindex) {
		if(event->pos().y() > starty) {
			if(m_showcontrolpanel == false){
				m_controlpanel->setGeometry(startx, starty, w, h);
				m_controlpanel->show();
				m_showcontrolpanel = true;
			}
		} else {
			if(m_showcontrolpanel == true){
				m_controlpanel->hide();
				m_showcontrolpanel = false;
			}
		}
	}
}

//void MainWindow::Play(QString xmlfilename)
//{
//	QFileInfo fileinfo(xmlfilename);
//	QList<sAudioInfo> audiolist;
//	QList<sVideoInfo> videolist;
//	ConfigXml xml;
//	if(!xml.ParseXml(xmlfilename, audiolist, videolist)) {
//		QMessageBox::critical(this, "error", "Can't parse the xml!");
//		return;
//	}

//	QStringList audiofilelist;
//	QStringList videofilelist;
//#if 0
//	audiolist.append("E:/SysFolder/Desktop/Court_1_20171017/1_20171017170849.mp3");
//	audiolist.append("E:/SysFolder/Desktop/Court_1_20171017/2_20171017170849.mp3");
//	audiolist.append("E:/SysFolder/Desktop/Court_1_20171017/3_20171017170849.mp3");
//	audiolist.append("E:/SysFolder/Desktop/Court_1_20171017/4_20171017170849.mp3");

//	videolist.append("E:/SysFolder/Desktop/Court_1_20171017/Court_1-1-20171017170848966.asf");
//	videolist.append("E:/SysFolder/Desktop/Court_1_20171017/Court_1-2-20171017170849018.asf");
//	videolist.append("E:/SysFolder/Desktop/Court_1_20171017/Court_1-3-20171017170849174.asf");
//	videolist.append("E:/SysFolder/Desktop/Court_1_20171017/Court_1-4-20171017170849207.asf");
//	videolist.append("E:/SysFolder/Desktop/Court_1_20171017/Court_1-5-20171017170848962.asf");
//#else
//	foreach(sAudioInfo ainfo, audiolist) {
//		audiofilelist.append(fileinfo.absolutePath() + "/" + ainfo.file);
//	}

//	for(int i = 0; i < videolist.size(); i++) {
//		videofilelist.append(fileinfo.absolutePath() + "/" + videolist.at(i).file);
//		m_videoout[i]->label->setText(videolist.at(i).desc);
//	}
////	foreach(sVideoInfo vinfo, videolist) {
////		videofilelist.append(fileinfo.absolutePath() + "/" + vinfo.file);
////	}

//#endif
//	if(m_playergroup) {
//		m_playergroup->Stop();
//		delete m_playergroup;
//	}
//	QList<QtAV::VideoOutput *> output;
//	foreach(sVideoWindow *vo, m_videoout) {
//		output.append(vo->output);
//	}

//	SetPlayState(audiolist);
//	m_playergroup = new PlayerGroup(audiofilelist, videofilelist, output);

//	connect(m_playergroup, SIGNAL(Signal_PositionChanged(qint64)), SLOT(updateSlider(qint64)));
//	connect(m_playergroup, SIGNAL(Signal_Started()), SLOT(updateSlider()));
//	connect(m_playergroup, SIGNAL(Signal_UpdateSliderUnit()), SLOT(updateSliderUnit()));
//	connect(m_playergroup, SIGNAL(Signal_mediaStateChanged(QMediaPlayer::State)), SLOT(Slot_MediaStateChanged(QMediaPlayer::State)));
//	m_index = 0;
//	m_fullscreenindex = -1;
//	m_playergroup->Play(m_index);
//	m_audiobtngroup->button(m_index)->setChecked(true);
//	m_xmlfilepath = xmlfilename;

//}

bool MainWindow::PrepareFileList()
{
	QString strpath = QApplication::applicationDirPath();
	QDir curdir(strpath);
	QStringList filelist = curdir.entryList(QDir::Files);

	m_audiolist.clear();
	m_videolist.clear();
	//获取音频文件
	QList<QRegExp> audioreglist;
	audioreglist.append(QRegExp("^1_.+\\.mp3"));
	audioreglist.append(QRegExp("^2_.+\\.mp3"));
	audioreglist.append(QRegExp("^3_.+\\.mp3"));
	audioreglist.append(QRegExp("^4_.+\\.mp3"));

	foreach (QRegExp rx, audioreglist) {
		int i;
		for(i = 0; i < filelist.size(); i++) {
			QString filename = filelist.at(i);
			if(filename.contains(rx)){
				m_audiolist.append(strpath + "/" + filename);
				break;
			}
		}
		if(i >= filelist.size()){
			m_audiolist.append("");//if not found , add null string to it
		}
	}
	qDebug()<<"Audio list:"<<m_audiolist;
	bool noaudio = true;
	foreach(QString audioname, m_audiolist) {
		if(audioname != "") {
			noaudio = false;
			break;
		}
	}
	if(noaudio == true) {
		QMessageBox::information(this, "Error", "找不到音频文件！");
		m_ready = false;
		return false;
	}

	//获取视频文件
	//获取court number
	QString courtnumber;
	QRegExp rxcourt("^Court_(\\d+)-.*.asf");
	foreach (QString filename, filelist) {
		if(filename.contains(rxcourt)){
			courtnumber = rxcourt.cap(1);
		}
	}

	//从config文件中读取这个court对应的cam number
	QSettings camconfig(strpath + "/CamConfig.ini", QSettings::IniFormat);
	QStringList camlist =  camconfig.value("main/Court_" + courtnumber).toStringList();
	//根据cam num找到对应的文件
	foreach (QString c, camlist) {
		int i;
		for(i = 0; i < filelist.size(); i++) {
			QRegExp rxvideo("^Court_" + courtnumber + "-" + c + "-" + ".+.asf");
			QString filename = filelist.at(i);
			if(filename.contains(rxvideo)) {
				m_videolist.append(strpath + "/" + filename);
				break;
			}
		}
		if(i >= filelist.size()) {
			m_videolist.append("");
		}
	}
	qDebug()<<m_videolist;
	bool novideo = true;
	foreach(QString videoname, m_videolist) {
		if(videoname != "") {
			novideo = false;
			break;
		}
	}
	if(novideo == true) {
		QMessageBox::information(this, "Error", "找不到视频文件！");
		m_ready = false;
		return false;
	}

	return true;
}

void MainWindow::Play()
{
	if(m_playergroup) {
		m_playergroup->Stop();
		delete m_playergroup;
	}
	QList<QtAV::VideoOutput *> output;
	foreach(sVideoWindow *vo, m_videoout) {
		output.append(vo->output);
	}

	SetPlayState(m_audiolist);
	m_playergroup = new PlayerGroup(m_audiolist, m_videolist, output);

	connect(m_playergroup, SIGNAL(Signal_PositionChanged(qint64)), SLOT(updateSlider(qint64)));
	connect(m_playergroup, SIGNAL(Signal_Started()), SLOT(updateSlider()));
	connect(m_playergroup, SIGNAL(Signal_UpdateSliderUnit()), SLOT(updateSliderUnit()));
	//connect(m_playergroup, SIGNAL(Signal_mediaStateChanged(QMediaPlayer::State)), SLOT(Slot_MediaStateChanged(QMediaPlayer::State)));
	//connect(m_playergroup, SIGNAL(Signal_StateChanged(QtAV::AVPlayer::State)), SLOT(Slot_StateChanged(QtAV::AVPlayer::State)));
	m_dstindex = 0;
	m_curindex = 0;
	m_fullscreenindex = -1;
	m_playergroup->Play(m_curindex);
	m_audiobtngroup->button(m_curindex)->setChecked(true);
	//估计是btn group的bug了。。disable后，再setChecked，要set两次才正常。
	m_audiobtngroup->button(m_curindex)->setChecked(true);
	SetVolume();
}

void MainWindow::on_sliProcess_sliderMoved(int position)
{
	seekBySlider(position);
}

void MainWindow::Slot_ClickBtnGroup(int id)
{
	m_dstindex = id;
	foreach (QAbstractButton * btn, m_audiobtngroup->buttons()) {
		btn->setChecked(false);
	}
	m_audiobtngroup->button(id)->setChecked(true);
	QSettings setting(QApplication::applicationDirPath() + "/Config.ini", QSettings::IniFormat);
	int autoplay = setting.value("main/AutoPlaySwitchAudio", "0").toInt();
	if(1 == autoplay || m_playergroup->IsPlaying()){
		SwitchAudio();
	}
}

bool MainWindow::SwitchAudio()
{
	if(m_dstindex == m_curindex)	return false;	//不用切换
	m_curindex = m_dstindex;
	m_playergroup->SwitchAudio(m_curindex);
	SetPlayState(m_audiolist);
}

void MainWindow::PlayFullScreen(int index)
{
	if(m_playergroup->AddVideoOutput(index, m_singlevideooutput)) {
		ui->stackedWidget->setCurrentIndex(1);
		this->showFullScreen();
		m_fullscreenindex = index;
		m_controlpanel->hide();
		m_showcontrolpanel = false;
	}
}

void MainWindow::ExitFullScreen(int index)
{
	if(-1 != m_fullscreenindex) {
		this->showNormal();
		ui->stackedWidget->setCurrentIndex(0);
		if(m_playergroup) m_playergroup->RemoveVideoOutput(index, m_singlevideooutput);
		m_fullscreenindex = -1;
	}
}

void MainWindow::on_btnIdle_clicked()
{
	ui->stackedWidget->setCurrentIndex(1);
	m_singlevideooutput->widget()->setWindowFlags (Qt::Window);
	m_singlevideooutput->widget()->showFullScreen();
}

void MainWindow::SetStopState()
{
	foreach (QAbstractButton * btn, m_audiobtngroup->buttons()) {
		btn->setChecked(false);
		btn->setCheckable(false);
		btn->setEnabled(false);
	}
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
	m_controlpanel->SetStopState();


	ui->stackedWidget->setCurrentIndex(0);
	ExitFullScreen(m_fullscreenindex);
}

//void MainWindow::SetPlayState(QList<sAudioInfo> audiolist)
//{
//	for(int i = 0; i < MAX_AUDIO_FILE; i++) {
//		if(i < audiolist.size()) {
//			m_audiobtngroup->buttons().at(i)->setEnabled(true);
//			m_audiobtngroup->buttons().at(i)->setText(audiolist.at(i).desc);
//		} else {
//			m_audiobtngroup->buttons().at(i)->setEnabled(false);
//			m_audiobtngroup->buttons().at(i)->setText(QString::number(i+1));
//		}
//	}
//	ui->btnPlayPause->setEnabled(true);
//	ui->btnStop->setEnabled(true);
//	ui->sliProcess->setEnabled(true);
//}

void MainWindow::SetPlayState(QStringList audiolist)
{
	for(int i = 0; i < MAX_AUDIO_FILE; i++) {
		m_audiobtngroup->buttons().at(i)->setCheckable(true);
		if(m_audiolist.at(i) == ""){
			m_audiobtngroup->buttons().at(i)->setEnabled(false);
		} else {
			m_audiobtngroup->buttons().at(i)->setEnabled(true);
		}
	}
	ui->btnPlay->setEnabled(true);
	ui->btnPlay->setVisible(false);
	ui->btnPause->setEnabled(true);
	ui->btnPause->setVisible(true);
	ui->btnStop->setEnabled(true);
	ui->btnFb->setEnabled(true);
	ui->btnFf->setEnabled(true);
	ui->sliProcess->setEnabled(true);
	ui->sliVolume->setEnabled(true);
	m_controlpanel->SetPlayState();
}

void MainWindow::Slot_StateChanged(QtAV::AVPlayer::State state)
{
	if(QtAV::AVPlayer::StoppedState == state) {
		m_playergroup->Stop();
		delete m_playergroup;
		m_playergroup = NULL;
		SetStopState();
	}
}

//void MainWindow::Slot_MediaStateChanged(QMediaPlayer::State state)
//{
//	if(QMediaPlayer::StoppedState == state) {
//		m_playergroup->Stop();
//		delete m_playergroup;
//		m_playergroup = NULL;
//	}
//	SetStopState();
//}

void MainWindow::SetVolume()
{
	if(m_playergroup){
		m_playergroup->SetVolume(ui->sliVolume->value());
		m_controlpanel->SyncVolume(ui->sliVolume->value());
	}
}

void MainWindow::on_sliVolume_sliderPressed()
{
	SetVolume();
}

void MainWindow::on_sliVolume_valueChanged(int value)
{
	SetVolume();
}

void MainWindow::on_btnFb_clicked()
{
	if(m_playergroup){
		m_playergroup->Fb();
	}
}

void MainWindow::on_btnFf_clicked()
{
	if(m_playergroup){
		m_playergroup->Ff();
	}
}

void MainWindow::Slot_UpdateVolume(int volume)
{
	ui->sliVolume->setValue(volume);
	SetVolume();
}

bool MainWindow::IsReady()
{
	return m_ready;
}
