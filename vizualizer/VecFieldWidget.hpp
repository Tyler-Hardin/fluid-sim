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
	 * 
	 * @param width		the width in pixels of the new viewport
	 * @param height	the height in pixels of the new viewport
	 */
	void resizeGL(int width, int height);
	
	/**
	 * Draws arrows.
	 */
	void draw();
	
	/**
	 * Sets the data to use for drawing.
	 * 
	 * @param height	the height of the vector field 2D array
	 * @param width		the width of the vector field 2D array
	 * @param u			the horizontal components of the vector field
	 * @param v			the vertical components of the vector field
	 */
	void setData(int height, int width, const QVector<float>& u, const QVector<float>& v);
};

#endif // VEC_FIELD_WIDGET_HPP
