/* Tyler Hardin
 * 4/4/2015
 */

#ifndef VEC_FIELD_WIDGET_HPP
#define VEC_FIELD_WIDGET_HPP

#include <QGLWidget>

/**
 * Widget that draws the vector field.
 */
class VecFieldWidget : public QGLWidget {
	Q_OBJECT

	int elapsed;
	
	int _height = 0, _width = 0;
	const QVector<float>* _u = nullptr;
	const QVector<float>* _v = nullptr;
    
public:
	VecFieldWidget(QWidget* parent = nullptr);
	
	/**
	 * Gives QT a hint for the initial size of this widget.
	 */
	QSize sizeHint() const;
	
	/**
	 * Initializes GL context.
	 */
	void initializeGL();
	
	/**
	 * Sets up for painting.
	 */
	void paintGL();
	
	/**
	 * Scales viewport when window is resized.
	 */
	void resizeGL(int width, int height);
	
	/**
	 * Draws arrows.
	 */
	void draw();
	
	/**
	 * Sets the data to use for drawing.
	 */
	void setData(int height, int width, const QVector<float>& u, const QVector<float>& v);
};

#endif // VEC_FIELD_WIDGET_HPP
