#include "MainWindow.hpp"
#include "NewDialog.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
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
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    statusBar()->setFont(font);

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

    // Generate frames if we don't have enough.
    while(frameNum >= _state->numFrames()) {
        if(_state) {
			_state->step();
        } else {
			return;
        }
	}

	_curFrame = frameNum;
    _slider->setMaximum(_state->numFrames());
	_slider->setValue(_curFrame + 1);
    _displayWidget->setData(_state->getFrame(frameNum));

    _displayWidget->updateGL();
    updateSubdisplay();
}

/**
 * @brief Sets a new SimState an resets the state of MainWindow for a new state.
 * @param s new SimState
 */
void MainWindow::setState(SimState s) {
	_play = false;
	_playTimer.stop();

    _curFrame = 0;

	_slider->setMaximum(1);
	_slider->setMinimum(0);
	_slider->setSliderPosition(_slider->maximum());

	_mode = EDIT;
	_saveInitialAction->setEnabled(true);

    _subdisplayWidget->hide();

    _state = s;
	setFrame(0);
}

/**
 * @brief Creates the widget that holds controls on the right
 * @param parent    parent widget
 * @return the initialized config widget
 */
QWidget* MainWindow::setupConfigWidget(QWidget* parent) {
    auto formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->setSizeConstraint(QLayout::SetNoConstraint);

	auto vectorCheckbox = new QCheckBox();
	vectorCheckbox->setChecked(true);
	connect(vectorCheckbox, SIGNAL(toggled(bool)),
			this, SLOT(vectorToggled(bool)));

    auto heatmapComboBox = new QComboBox();
    heatmapComboBox->addItems(QStringList{"Density", "Speed", "X Velocity", "Y Velocity"});
    heatmapComboBox->setCurrentIndex(1);
    connect(heatmapComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(heatmapChanged(QString)));


    _subdisplayWidget = new DisplayWidget;
    _subdisplayWidget->hide();
    connect(_subdisplayWidget, SIGNAL(hover(QString)), this, SLOT(displayHover(QString)));

	formLayout->addRow("Show Vectors:", vectorCheckbox);
    formLayout->addRow("Heatmap:", heatmapComboBox);

    auto vbox = new QVBoxLayout;
    vbox->addLayout(formLayout, 0);
    vbox->addWidget(_subdisplayWidget, 1);

	auto configWidget = new QWidget(parent);
    configWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    configWidget->setLayout(vbox);
	return configWidget;
}

/**
 * @brief Adds items to menu and connects slots to handle them.
 */
void MainWindow::setupMenu(){
	auto menuBar = this->menuBar();
	
	QAction* exitAction = new QAction(tr("&Quit"), this);
	exitAction->setShortcuts(QKeySequence::Quit);
	exitAction->setStatusTip(tr("Exit application"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    QAction* newAction = new QAction(tr("&New"), this);
	connect(newAction, SIGNAL(triggered()), this, SLOT(newTriggered()));

    _editAction = new QAction(tr("&Edit"), this);
	connect(_editAction, SIGNAL(triggered()), this, SLOT(editTriggered()));
    _editAction->setEnabled(false);

    _saveInitialAction = new QAction(tr("Save Initial State"), this);
    connect(_saveInitialAction, SIGNAL(triggered()), this, SLOT(saveInitialTriggered()));
    _saveInitialAction->setEnabled(false);

    QAction* loadInitialAction = new QAction(tr("Load Initial State"), this);
    connect(loadInitialAction, SIGNAL(triggered()), this, SLOT(loadInitialTriggered()));

	auto fileMenu = menuBar->addMenu(tr("&File"));
	fileMenu->addAction(newAction);
	fileMenu->addAction(_editAction);
	fileMenu->addAction(_saveInitialAction);
    fileMenu->addAction(loadInitialAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);
}

/**
 * @brief Sets up widget hierarchy and connects slots.
 */
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
    connect(_displayWidget, SIGNAL(hover(QString)),
            this, SLOT(displayHover(QString)));
    connect(_displayWidget, SIGNAL(selected(int,int,int,int)),
            this, SLOT(subdiplaySelected(int,int,int,int)));
    connect(_displayWidget, SIGNAL(toggle(int,int)),
            this, SLOT(displayToggle(int,int)));

	auto centralWidget = new QWidget;
	centralWidget->setLayout(vbox);
	setCentralWidget(centralWidget);
}

/**
 * @brief Updates subdisplay when a new frame is set.
 */
void MainWindow::updateSubdisplay() {
    if(_subdisplayWidget->isHidden())
        return;

    static Frame f = _state->getFrame(_curFrame).getSubframe(subdisplayRow, subdisplayCol, subdisplayH, subdisplayW);
    f = _state->getFrame(_curFrame).getSubframe(subdisplayRow, subdisplayCol, subdisplayH, subdisplayW);
    _subdisplayWidget->setData(f);
    _subdisplayWidget->update();
}

/**
 * @brief Called by DisplayWidget instances to show raw data.
 * @param raw data string
 */
void MainWindow::displayHover(QString s) {
    statusBar()->showMessage(s);
}

/**
 * @brief Called by DisplayWidget when a barrier is clicked.
 * @param row
 * @param col
 */
void MainWindow::displayToggle(int row, int col) {
    // Click was not in grid.
    if(row < 0 || col < 0)
        return;

    if(_state && _curFrame == _state->numFrames() - 1) {
        qDebug() << "row,col: " << row << "," << col;
        _state->toggleBarrier(row, col);
    }

    _displayWidget->update();
    updateSubdisplay();
}

/**
 * @brief Called when the Edit menu option is clicked.
 */
void MainWindow::editTriggered() {
    setState(_state->initialState());
}

/**
 * @brief Called when heatmap dropdown is changed
 * @param value of heatmap dropdown
 */
void MainWindow::heatmapChanged(QString s) {
    if(s == "Density") {
        _displayWidget->setHeatmapType(DisplayWidget::HeatmapType::DENSITY);
        _subdisplayWidget->setHeatmapType(DisplayWidget::HeatmapType::DENSITY);
    } else if(s == "Speed") {
        _displayWidget->setHeatmapType(DisplayWidget::HeatmapType::SPEED);
        _subdisplayWidget->setHeatmapType(DisplayWidget::HeatmapType::SPEED);
    } else if(s == "X Velocity") {
        _displayWidget->setHeatmapType(DisplayWidget::HeatmapType::X_VEL);
        _subdisplayWidget->setHeatmapType(DisplayWidget::HeatmapType::X_VEL);
    } else if(s == "Y Velocity") {
        _displayWidget->setHeatmapType(DisplayWidget::HeatmapType::Y_VEL);
        _subdisplayWidget->setHeatmapType(DisplayWidget::HeatmapType::Y_VEL);
    }

    _displayWidget->update();
    _subdisplayWidget->update();
}

/**
 * @brief Called when Load is clicked in menu.
 */
void MainWindow::loadInitialTriggered() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load Initial State"),
                                                    "",
                                                    tr("Initial State (*.istate)"));

    QFile file(fileName);
    if(file.open(QFile::ReadOnly)) {
        QDataStream stream(&file);
        setState(SimState::load(stream));
        file.close();
    }
}

/**
 * @brief Called when New is clicked in menu.
 */
void MainWindow::newTriggered() {

	NewDialog* newDialog = new NewDialog();
    if(newDialog->exec()) {
        // Hide the subdisplay because a new selection must be made for the new state.
        _subdisplayWidget->hide();

        SimState s(newDialog->height, newDialog->width);
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
        _editAction->setEnabled(true);
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
        _state->initialState().save(stream);
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
 * @brief Called when a subdisplahy is selected in the main DisplayWidget
 * @param row
 * @param col
 * @param height
 * @param width
 */
void MainWindow::subdiplaySelected(int row, int col, int height, int width) {
    if(row < 0 || col < 0 || height < 2 || width < 2)
        return;

    subdisplayRow = row;
    subdisplayCol = col;
    subdisplayH = height;
    subdisplayW = width;

    _subdisplayWidget->show();
    updateSubdisplay();
    _subdisplayWidget->paintGL();
    _subdisplayWidget->resize(_subdisplayWidget->size());
    _subdisplayWidget->resizeGL(_subdisplayWidget->width() - 1, _subdisplayWidget->height());
    _subdisplayWidget->paintGL();
}

/**
 * @brief Slot called when vector checkbox is toggled.
 */
void MainWindow::vectorToggled(bool checked) {
    _displayWidget->setDrawVectors(checked);
    _subdisplayWidget->setDrawVectors(checked);
}
