#ifndef VIZ_WIDGET_HPP
#define VIZ_WIDGET_HPP

#include <QString>
#include <QPair>
#include <QSlider>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <mgl2/qmathgl.h>
#include <mgl2/mgl.h>

class VizWidget : public QWidget {
	Q_OBJECT
	
	struct VizDraw : public mglDraw {
		VizDraw(VizWidget* t) : vizWidget(t) {}
		int Draw(mglGraph* gr);
		
	private:
		VizWidget* vizWidget;
	} draw{this};
	
	bool _changed = true;
	bool _fileLoaded = false;
	bool _play = false;
	int _height;
	int _width;
	int _curFrame;
	
	QTimer _playTimer;
	QTimer _renderTimer;
	
	QMathGL* _mgl;
	QSlider* _slider;
	QVector<QPair<mglData, mglData>> _frames;
	
public:
	VizWidget(QWidget* parent = nullptr);
	
	void loadFile(QString);
	
	int numFrames();
	void nextFrame(int skip = 1);
	void setFrame(int i);
	
private slots:
	void playReleased();
	void pauseReleased();
	void playEvent();
	void renderEvent();
	void sliderMoved(int);
};

#endif // VIZ_WIDGET_HPP
