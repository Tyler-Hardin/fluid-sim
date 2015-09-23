#include "VizWidget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDataStream>
#include <QFile>
#include <QFormLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include <QDebug>

VizWidget::VizWidget(QWidget* parent) : QWidget(parent) {
	// Create slider and attach handler.
	_slider = new QSlider(Qt::Horizontal);
	_slider->setMaximum(1);
	_slider->setMinimum(0);
	_slider->setSliderPosition(_slider->maximum());
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
	connect(playButton, SIGNAL(released()),
			this, SLOT(playReleased()));
	auto pauseButton = new QPushButton("Pause");
	connect(pauseButton, SIGNAL(released()),
			this, SLOT(pauseReleased()));
	
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
	connect(&_playTimer, SIGNAL(timeout()),
			this, SLOT(playEvent()));

	// Connect mouse press event from display widget to this object so we can add barriers
	// when the user clicks.
	connect(_vecWidget, SIGNAL(mousePressed(QMouseEvent*)),
			this, SLOT(displayMousePressed(QMouseEvent*)));
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

int VizWidget::numFrames(){
	return _frames.size();
}

void VizWidget::setFrame(int i){
	// Ignore requests to set frame higher than the number of frames we have.
	while(i >= numFrames()) {
		if(_state) {
			_frames.push_back(_state->getFrame());
			_state->step();
		} else
			return;
	}

	_curFrame = i;
	_slider->setMaximum(numFrames());
	_slider->setValue(_curFrame + 1);
	_vecWidget->setData(_frames[i]);

	qDebug() << "i: " << i;
	qDebug() << "Val: " << _slider->value();
	qDebug() << "Max: " << _slider->maximum() << "\n\n";
	
	_vecWidget->resizeGL(_vecWidget->width(), _vecWidget->height());
	_vecWidget->update();
}

void VizWidget::setState(SimState s) {
	_play = false;
	_playTimer.stop();

	_curFrame = 0;
	_frames.clear();

	_slider->setMaximum(1);
	_slider->setMinimum(0);
	_slider->setSliderPosition(_slider->maximum());

	qDebug() << "Max: " << _slider->maximum();
	_state = s;
	_frames.push_back(_state->getFrame());
	setFrame(0);

}

void VizWidget::displayMousePressed(QMouseEvent *event) {
	int col = _vecWidget->getCol(event->x());
	int row = _vecWidget->getRow(event->y());

	// Click was not in grid.
	if(row < 0 || col < 0)
		return;

	if(_state)
		_state->toggleBarrier(row, col);

	_vecWidget->update();
}

void VizWidget::sliderMoved(int val) {
	if(val > 0)
		setFrame(val - 1);
}

void VizWidget::playEvent(){
	if(_play) {
		setFrame(_curFrame + _skip);
	}
}

void VizWidget::playReleased(){
	if(_state) {
		_play = true;
		_playTimer.start(100);
	}
}

void VizWidget::pauseReleased(){
	_playTimer.stop();
}

void VizWidget::vectorToggled(bool checked) {
	_vecWidget->setDrawVectors(checked);
}
