/* Tyler Hardin
 * 4/4/2015
 */

#ifndef VIZ_WIDGET_HPP
#define VIZ_WIDGET_HPP

#include "VecField.hpp"
#include "VecFieldWidget.hpp"

#include <QString>
#include <QPair>
#include <QSlider>
#include <QTimer>
#include <QWidget>

#include <vector> // Need std vector because of deleted copy constructor on VecField

/**
 * Widget containing play/pause buttons, seek slider, and the GL widget that draws the vector field.
 */
class VizWidget : public QWidget {
	Q_OBJECT
	
	VecFieldWidget* _vecWidget = nullptr;
	QWidget* _configWidget = nullptr;

	bool _fileLoaded = false;
	bool _play = false;
	int _curFrame;
	
	QTimer _playTimer;
	
	QSlider* _slider = nullptr;
	QVector<bool> barrier;
	std::vector<VecField> _frames;

	QWidget* initConfigWidget();
	
public:
	VizWidget(QWidget* parent = nullptr);
	
	/**
	 * Loads a new data file.
	 * @param file		the filename
	 */
	void loadFile(QString file);
	
	/**
	 * Gets the number of frames.
	 * @return the number of frames
	 */
	int numFrames();
	
	/**
	 * Sets a new frame (and skips a number of frames).
	 *
	 * @param skip		the number of frames to skip
	 */
	void nextFrame(int skip = 1);
	
	/**
	 * Sets a new frame.
	 *
	 * @param frame		the frame number
	 */
	void setFrame(int i);
	
private slots:

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
