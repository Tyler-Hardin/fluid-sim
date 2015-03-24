#include "VizWidget.hpp"

#include <mgl2/base.h>

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

void mgls_prepare2d(mglData *a, mglData *b=0, mglData *v=0)
{
	long i,j,n=50,m=40,i0;
	if(a) a->Create(n,m);   if(b) b->Create(n,m);
	if(v) { v->Create(9); v->Fill(-1,1);  }
	mreal x,y;
	for(i=0;i<n;i++)  for(j=0;j<m;j++)
	{
		x = i/(n-1.); y = j/(m-1.); i0 = i+n*j;
		if(a) a->a[i0] = 0.6*sin(2*M_PI*x)*sin(3*M_PI*y)+0.4*cos(3*M_PI*x*y);
		if(b) b->a[i0] = 0.6*cos(2*M_PI*x)*cos(3*M_PI*y)+0.4*cos(3*M_PI*x*y);
	}
}

int sample(mglGraph *gr)
{
	mglData a, b;
	mgls_prepare2d(&a, &b);
	
	gr->SubPlot(1,1,0,"");
	gr->Box();
	gr->Vect(a,b,"wk");
	
	return 0;
}

VizWidget::VizWidget(QWidget* parent) : QWidget(parent) {	
	_slider = new QSlider(Qt::Horizontal);
	connect(_slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
	
	_mgl = new QMathGL();
	_mgl->setDraw(&draw);
	
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
	vbox->addWidget(_mgl);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);
	
	this->setLayout(vbox);
	
	connect(&_playTimer, SIGNAL(timeout()), this, SLOT(playEvent()));
	connect(&_renderTimer, SIGNAL(timeout()), this, SLOT(renderEvent()));
	_renderTimer.start(250);
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
	
	mglData a(_width, _height);
	mglData b(_width, _height);
	while(!in.atEnd()){
		for(int i = 0;i < _height * _width;i++){
			in >> a.a[i];
			in >> b.a[i];
			//std::cout << a.a[i] << "\t" << b.a[i] << std::endl;
		}
		_frames.append(qMakePair(a, b));
	}
	
	if(_frames.size() > 0){
		_fileLoaded = true;
		_curFrame = 0;
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
	_changed = true;
	_slider->setValue(i);
}

int VizWidget::VizDraw::Draw(mglGraph* gr){
	gr->SubPlot(1,1,0,"");
	gr->Box();
	if(vizWidget->_fileLoaded){
		int frame = vizWidget->_curFrame;
		auto a = vizWidget->_frames[frame].first;
		auto b = vizWidget->_frames[frame].second;
		gr->Vect(a,b,"k");
	}
	
	return 0;
}

void VizWidget::sliderMoved(int val) {
	std::cout << "slider moved " << val << std::endl;
	setFrame(val);
}

void VizWidget::playEvent(){
	int skip = _frames.size() / 500;
	if(_curFrame + skip < _frames.size())
		nextFrame(skip);
	else
		_playTimer.stop();
}

void VizWidget::renderEvent(){
	std::atomic<bool> busy(false);
	if(_changed && !busy.load()){
		busy = true;
		_mgl->update();
		busy = false;
	}
	_changed = false;
}

void VizWidget::playReleased(){
	_playTimer.start(500);
}

void VizWidget::pauseReleased(){
	_playTimer.stop();
}
