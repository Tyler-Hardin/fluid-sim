#include "MainWindow.hpp"
#include "NewDialog.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QKeySequence>
#include <QMenuBar>
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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	setupMenu();
	setupUI();
	setWindowTitle(tr("Vizualizer"));
}

/**
 * @brief Sets a new frame.
 *
 * @param frameNum	the frame number
 */
void MainWindow::setFrame(int frameNum){
	if(frameNum > 0) {
		_mode = RUN;
	}

	// Ignore requests to set frame higher than the number of frames we have.
	while(frameNum >= (int)_frames.size()) {
		if(_state) {
			_frames.push_back(_state->getFrame());
			_state->step();
		} else
			return;
	}

	_curFrame = frameNum;
	_slider->setMaximum(_frames.size());
	_slider->setValue(_curFrame + 1);
	_displayWidget->setData(_frames[frameNum]);

	_displayWidget->resizeGL(_displayWidget->width(), _displayWidget->height());
	_displayWidget->update();
}

void MainWindow::setState(SimState s) {
	_play = false;
	_playTimer.stop();

	_curFrame = 0;
	_frames.clear();

	_slider->setMaximum(1);
	_slider->setMinimum(0);
	_slider->setSliderPosition(_slider->maximum());

	_mode = EDIT;
	_state = s;
	_frames.push_back(_state->getFrame());
	setFrame(0);
}

QWidget* MainWindow::setupConfigWidget(QWidget* parent) {
	auto formLayout = new QFormLayout;

	auto vectorCheckbox = new QCheckBox();
	vectorCheckbox->setChecked(true);
	connect(vectorCheckbox, SIGNAL(toggled(bool)),
			this, SLOT(vectorToggled(bool)));

	formLayout->addRow("Show Vectors:", vectorCheckbox);
	auto configWidget = new QWidget(parent);
	configWidget->setLayout(formLayout);
	return configWidget;
}

void MainWindow::setupMenu(){
	auto menuBar = this->menuBar();
	
	QAction* exitAction = new QAction(tr("&Quit"), this);
	exitAction->setShortcuts(QKeySequence::Quit);
	exitAction->setStatusTip(tr("Exit application"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	QAction* newAction = new QAction(tr("&New"), this);
	newAction->setStatusTip(tr("Start new simulation"));
	connect(newAction, SIGNAL(triggered()), this, SLOT(newTriggered()));

	_editAction = new QAction(tr("&Edit"), this);
	_editAction->setStatusTip(tr("Edit (and reset) this simulation"));
	connect(_editAction, SIGNAL(triggered()), this, SLOT(editTriggered()));
	_editAction->setEnabled(false);

	_saveInitialAction = new QAction(tr("Save &Initial State"), this);
	_saveInitialAction->setStatusTip(tr("Save initial condition of the current simulation"));
	connect(_saveInitialAction, SIGNAL(triggered()), this, SLOT(saveInitialTriggered()));
	_saveInitialAction->setEnabled(false);

	auto fileMenu = menuBar->addMenu(tr("&File"));
	fileMenu->addAction(newAction);
	fileMenu->addAction(_editAction);
	fileMenu->addAction(_saveInitialAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);
}

void MainWindow::setupUI(){
	// Create slider and attach handler.
	_slider = new QSlider(Qt::Horizontal);
	_slider->setMaximum(1);
	_slider->setMinimum(0);
	_slider->setSliderPosition(_slider->maximum());
	connect(_slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));

	auto splitterWidget = new QSplitter();
	_displayWidget = new DisplayWidget(splitterWidget);
	_configWidget = setupConfigWidget(splitterWidget);
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

	// Connect the play timer to the play event handler. The handler is called every time the
	// timer fires.
	connect(&_playTimer, SIGNAL(timeout()),
			this, SLOT(playEvent()));

	// Connect mouse press event from display widget to this object so we can add barriers
	// when the user clicks.
	connect(_displayWidget, SIGNAL(mousePressed(QMouseEvent*)),
			this, SLOT(displayMousePressed(QMouseEvent*)));

	auto centralWidget = new QWidget;
	centralWidget->setLayout(vbox);
	setCentralWidget(centralWidget);
}

/**
 * @brief Slot called when the display widget is clicked
 * @param event mouse event
 */
void MainWindow::displayMousePressed(QMouseEvent *event) {
	int col = _displayWidget->getCol(event->x());
	int row = _displayWidget->getRow(event->y());

	// Click was not in grid.
	if(row < 0 || col < 0)
		return;

	if(_state && (unsigned long)_curFrame == _frames.size() - 1)
		_state->toggleBarrier(row, col);

	_displayWidget->update();
}

void MainWindow::editTriggered() {
	setState(*_savedState);
}

void MainWindow::newTriggered() {
	NewDialog* newDialog = new NewDialog();
	if(newDialog->exec()) {
		SimState s(newDialog->height, newDialog->width);
		_savedState.reset();
		setState(s);
	}
	delete newDialog;
}

/**
 * @brief Slot called when the pause button is released.
 */
void MainWindow::pauseReleased(){
	_playTimer.stop();
}

/**
 * @brief Timer event that skips forward when playing.
 */
void MainWindow::playEvent(){
	if(_play) {
		setFrame(_curFrame + _skip);
	}
}

/**
 * @brief Slot called when the play button is released.
 */
void MainWindow::playReleased(){
	if(_state) {
		_mode = RUN;
		if(!_savedState) {
			_savedState = _state;
			_editAction->setEnabled(true);
		}
		_play = true;
		_playTimer.start(100);
	}
}

/**
 * @brief Slot called when the "Save Initial State" action is triggered.
 */
void MainWindow::saveInitialTriggered() {
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save Initial State"),
													"",
													tr("Initial State (*.istate)"));
	Q_ASSERT(_mode != STARTED);

	QFile file(fileName);
	if(file.open(QFile::WriteOnly)) {
		QDataStream stream(&file);
		if(_mode == EDIT) {
			_state->save(stream);
		} else if(_mode == RUN) {
			_savedState->save(stream);
		}
		file.close();
	}

}

/**
 * @brief Slot called when slider is moved.
 * @param val	the value of the slider
 */
void MainWindow::sliderMoved(int val) {
	if(val > 0)
		setFrame(val - 1);
}

/**
 * @brief Slot called when vector checkbox is toggled.
 */
void MainWindow::vectorToggled(bool checked) {
	_displayWidget->setDrawVectors(checked);
}
