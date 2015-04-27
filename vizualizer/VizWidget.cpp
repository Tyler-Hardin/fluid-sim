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
	_slider = new QSlider(Qt::Horizontal);
	connect(_slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
	
	_vecWidget = new VecFieldWidget();
	
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
	
	connect(&_playTimer, SIGNAL(timeout()), this, SLOT(playEvent()));
}

void VizWidget::loadFile(QString filename){
	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	
	_frames.clear();
	
	QDataStream in(&file);
	in.setByteOrder(QDataStream::BigEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
	in >> _height;
	in >> _width;
	
	std::cout << _height << std::endl;
	std::cout << _width << std::endl;
	
	unsigned char b;
	barrier.clear();
	for(int i = 0;i < _height;i++){
		for(int j = 0;j < _width;j++){
			in >> b;
			barrier.push_back(b);
		}
	}
	
	QVector<float> u;
	QVector<float> v;
	u.resize(_height * _width);
	v.resize(_height * _width);
	while(!in.atEnd()){
		for(int i = 0;i < _height * _width;i++){
			in >> u[i];
			in >> v[i];
			//std::cout << a.a[i] << "\t" << b.a[i] << std::endl;
		}
		_frames.append(qMakePair(u, v));
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
	if(_curFrame + skip < _frames.size()){
		setFrame(_curFrame + skip);
	}
}

void VizWidget::setFrame(int i){
	_curFrame = i;
	_slider->setValue(i);
	
	_vecWidget->setData(_height, _width, barrier, _frames[i].first, _frames[i].second);
	_vecWidget->update();
}

void VizWidget::sliderMoved(int val) {
	std::cout << "slider moved " << val << std::endl;
	setFrame(val);
}

void VizWidget::playEvent(){
	int skip = _frames.size() / 1000;
	if(_curFrame + skip < _frames.size())
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
