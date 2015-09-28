/* Tyler Hardin
 * 4/4/2015
 */
 
#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "DisplayWidget.hpp"
#include "Frame.hpp"
#include "SimState.hpp"

#include <QMainWindow>

#include <QString>
#include <QPair>
#include <QSlider>
#include <QTimer>
#include <QWidget>

#include <boost/optional.hpp>

#include <vector> // Need std vector because of deleted copy constructor on VecField

#include <boost/optional.hpp>

/**
 * @brief Main window of the project. Contains a menu, controls and a display widget.
 */
class MainWindow : public QMainWindow{
	Q_OBJECT

	QWidget* _configWidget = nullptr;
	DisplayWidget* _displayWidget = nullptr;

	// Set when vizualizer is playing/animating.
	bool _play = false;

	// Store the index of the current frame.
	int _curFrame = 0;

	// Set the number of frames to skip forward when animating.
	int _skip = 10;

	/**
	 * Mode of operation.
	 * STARTED -	Window just created, no state created or loaded yet.
	 * EDIT    -	State loaded but not yet .step()'ed.
	 * RUN     -	State loaded and .step()'ed.
	 */
	enum Mode {STARTED, EDIT, RUN} _mode = STARTED;

	// Copy of current _state, without .step() ever having been called.
	boost::optional<SimState> _savedState;

	boost::optional<SimState> _state;

	QTimer _playTimer;

	QSlider* _slider = nullptr;
	std::vector<Frame> _frames;

	QAction* _editAction;
	QAction* _saveInitialAction;

	QWidget* setupCentralWidget();
	QWidget* setupConfigWidget(QWidget* parent = nullptr);
	void setupMenu();
	void setupUI();
	
public:
	MainWindow(QWidget* parent = nullptr);

	void setFrame(int i);
	void setState(SimState);
	
private slots:
	void displayMousePressed(QMouseEvent* event);
	void editTriggered();
	void newTriggered();
	void playReleased();
	void pauseReleased();
	void playEvent();
	void saveInitialTriggered();
	void sliderMoved(int);
	void vectorToggled(bool checked);
};

#endif // MAIN_WINDOW_HPP
