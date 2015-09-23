/* Tyler Hardin
 * 4/4/2015
 */

#ifndef VIZ_WIDGET_HPP
#define VIZ_WIDGET_HPP

#include "Frame.hpp"
#include "SimState.hpp"
#include "VecFieldWidget.hpp"

#include <QString>
#include <QPair>
#include <QSlider>
#include <QTimer>
#include <QWidget>

#include <boost/optional.hpp>

#include <vector> // Need std vector because of deleted copy constructor on VecField

/**
 * Widget containing play/pause buttons, seek slider, and the GL widget that draws the vector field.
 */
class VizWidget : public QWidget {
	Q_OBJECT
	
	VecFieldWidget* _vecWidget = nullptr;
	QWidget* _configWidget = nullptr;

	// Set when vizualizer is playing/animating.
	bool _play = false;

	// Store the index of the current frame.
	int _curFrame = 0;

	// Set the number of frames to skip forward when animating.
	int _skip = 10;

	boost::optional<SimState> _state;
	
	QTimer _playTimer;
	
	QSlider* _slider = nullptr;
	QVector<bool> barrier;
	std::vector<Frame> _frames;

	QWidget* initConfigWidget();
	
public:
	VizWidget(QWidget* parent = nullptr);
	
	/**
	 * Gets the number of frames.
	 * @return the number of frames
	 */
	int numFrames();
	
	/**
	 * Sets a new frame.
	 *
	 * @param frame		the frame number
	 */
	void setFrame(int i);

	void setState(SimState);
	
private slots:

	/**
	 * Slot called when the display widget is clicked.
	 */
	void displayMousePressed(QMouseEvent* event);

	/**
	 * Slot called when the play button is released.
	 */
	void playReleased();
	
	/**
	 * Slot called when the pause button is released.
	 */
	void pauseReleased();
	
	/**
	 * Timer event that skips forward when playing.
	 */
	void playEvent();

	/**
	 * Slot called when slider is moved.
	 */
	void sliderMoved(int);

	/**
	 * Slot called when vector checkbox is toggled.
	 */
	void vectorToggled(bool checked);
};

#endif // VIZ_WIDGET_HPP
