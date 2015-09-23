/* Tyler Hardin
 * 4/4/2015
 */

#ifndef VEC_FIELD_WIDGET_HPP
#define VEC_FIELD_WIDGET_HPP

#include "Frame.hpp"

#include <QGLWidget>

#include <boost/optional.hpp>

/**
 * Widget that draws the vector field.
 */
class VecFieldWidget : public QGLWidget {
	Q_OBJECT

	int elapsed;
	bool _drawVectors = true;

	boost::optional<Frame> frame;
	
	// Range of x and y in world coords
	float range_x;
	float range_y;

	// Offset of viewport in pixel coords
	int vp_x_off;
	int vp_y_off;

	// Size in pixel coords.
	int vp_height;
	int vp_width;

protected:
	void mousePressEvent(QMouseEvent *event) override;

public:
	VecFieldWidget(QWidget* parent = nullptr);

	int getRow(int pixel);
	int getCol(int pixel);

	/**
	 * Gives QT a hint for the initial size of this widget.
	 */
	QSize sizeHint() const override;
	
	/**
	 * Initializes GL context.
	 */
	void initializeGL() override;
	
	/**
	 * Sets up for painting.
	 */
	void paintGL() override;
	
	/**
	 * Scales viewport when window is resized.
	 * 
	 * @param width		the width in pixels of the new viewport
	 * @param height	the height in pixels of the new viewport
	 */
	void resizeGL(int width, int height) override;
	
	/**
	 * Draws arrows.
	 */
	void draw();
	
	/**
	 * Sets the data to use for drawing.
	 * 
	 * @param frame	the data to draw
	 */
	void setData(const Frame& frame);

	void setDrawVectors(bool);

signals:
	void mousePressed(QMouseEvent*);
};

#endif // VEC_FIELD_WIDGET_HPP
