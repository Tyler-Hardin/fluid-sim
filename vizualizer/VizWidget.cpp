#include "VizWidget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDataStream>
#include <QFile>
#include <QPushButton>


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
	
	// Create buttons and attach handlers.
	auto playButton = new QPushButton("Play");
	connect(playButton, SIGNAL(released()), this, SLOT(playReleased()));
	auto pauseButton = new QPushButton("Pause");
	connect(pauseButton, SIGNAL(released()), this, SLOT(pauseReleased()));
	
	auto hbox1 = new QHBoxLayout();
	hbox1->addWidget(_slider);

	auto hbox2 = new QHBoxLayout();
	hbox2->addWidget(playButton);
	hbox2->addWidget(pauseButton);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(_vecWidget);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);
	
	this->setLayout(vbox);
	
	// Connect the play timer to the play event handler. The handler is called every time the
	// timer fires.
	connect(&_playTimer, SIGNAL(timeout()), this, SLOT(playEvent()));
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
	_curFrame = i;
	_slider->setValue(i);
	
	_vecWidget->setData(barrier, _frames[i]);
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
