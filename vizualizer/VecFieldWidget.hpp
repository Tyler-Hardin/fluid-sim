/* Tyler Hardin
 * 4/4/2015
 */

#ifndef VEC_FIELD_WIDGET_HPP
#define VEC_FIELD_WIDGET_HPP

#include "VecField.hpp"

#include <QGLWidget>

/**
 * Widget that draws the vector field.
 */
class VecFieldWidget : public QGLWidget {
	Q_OBJECT

	int elapsed;
	bool _drawVectors = true;
	
	const QVector<bool>* barriers = nullptr;
	const VecField* vecField = nullptr;
	
	float range_x;
	float range_y;
    
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
	 * @param barriers	the array of where barriers are
	 * @param vecField	the vector field
	 */
	void setData(const QVector<bool>& barriers, const VecField& vecField);

	void setDrawVectors(bool);
};

#endif // VEC_FIELD_WIDGET_HPP
