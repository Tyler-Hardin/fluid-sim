#include "VizWidget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDataStream>
#include <QFile>
#include <QFormLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

VizWidget::VizWidget(QWidget* parent) : QWidget(parent) {	
	// Create slider and attach handler.
	_slider = new QSlider(Qt::Horizontal);
	connect(_slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
	
	_vecWidget = new VecFieldWidget();
	_configWidget = initConfigWidget();

	auto splitterWidget = new QSplitter();
	splitterWidget->addWidget(_vecWidget);
	splitterWidget->addWidget(_configWidget);
	splitterWidget->setStretchFactor(0,1);
	splitterWidget->setStretchFactor(1,0);
	
	// Create buttons and attach handlers.
	auto playButton = new QPushButton("Play");
	connect(playButton, SIGNAL(released()), this, SLOT(playReleased()));
	auto pauseButton = new QPushButton("Pause");
	connect(pauseButton, SIGNAL(released()), this, SLOT(pauseReleased()));
	
	auto sliderHBox = new QHBoxLayout();
	sliderHBox->addWidget(_slider);

	auto buttonHBox = new QHBoxLayout();
	buttonHBox->addWidget(playButton);
	buttonHBox->addWidget(pauseButton);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(splitterWidget);
	vbox->addLayout(sliderHBox);
	vbox->addLayout(buttonHBox);
	
	this->setLayout(vbox);
	
	// Connect the play timer to the play event handler. The handler is called every time the
	// timer fires.
	connect(&_playTimer, SIGNAL(timeout()), this, SLOT(playEvent()));
}

QWidget* VizWidget::initConfigWidget() {
	auto formLayout = new QFormLayout;

	auto vectorCheckbox = new QCheckBox();
	vectorCheckbox->setChecked(true);
	connect(vectorCheckbox, SIGNAL(toggled(bool)),
			this, SLOT(vectorToggled(bool)));

	formLayout->addRow("Show Vectors:", vectorCheckbox);
	auto configWidget = new QWidget;
	configWidget->setLayout(formLayout);
	return configWidget;
}

void VizWidget::loadFile(QString filename){
	int height, width;
	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	
	// Clear frame buffer in case a file is already loaded.
	_frames.clear();
	
	QDataStream in(&file);
	
	// Input files are in big endian and floats in it are single precision. Notify input stream so
	// it reads correctly.
	in.setByteOrder(QDataStream::BigEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
	
	in >> height;
	in >> width;
	
	// Read barrier data.
	unsigned char b;
	barrier.clear();
	for(int i = 0;i < height;i++){
		for(int j = 0;j < width;j++){
			in >> b;
			barrier.push_back(b);
		}
	}
	
	// Read vector fields.
	while(!in.atEnd()){
		_frames.push_back(VecField::read(in, height, width));
	}
	
	if(_frames.size() > 0){
		setFrame(0);
		_slider->setMaximum(_frames.size() - 1);
	}
	else{
		_fileLoaded = false;
	}
}

int VizWidget::numFrames(){
	return _frames.size();
}

void VizWidget::nextFrame(int skip){
	if(_curFrame + skip < (int)_frames.size()){
		setFrame(_curFrame + skip);
	}
}

void VizWidget::setFrame(int i){
	// Ignore requests to set frame higher than the number of frames we have.
	if(i >= numFrames())
		return;

	_curFrame = i;
	_slider->setValue(i);
	
	_vecWidget->setData(barrier, _frames[i]);
	
	_vecWidget->resizeGL(_vecWidget->width(), _vecWidget->height());
	_vecWidget->update();
}

void VizWidget::sliderMoved(int val) {
	setFrame(val);
}

void VizWidget::playEvent(){
	int skip = _frames.size() / 1000;
	if(_curFrame + skip < (int)_frames.size())
		nextFrame(skip);
	else
		_playTimer.stop();
}

void VizWidget::playReleased(){
	_playTimer.start(50);
}

void VizWidget::pauseReleased(){
	_playTimer.stop();
}

void VizWidget::vectorToggled(bool checked) {
	_vecWidget->setDrawVectors(checked);
}
