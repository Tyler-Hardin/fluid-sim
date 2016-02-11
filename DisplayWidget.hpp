/* Tyler Hardin
 * 4/4/2015
 */

#ifndef DISPLAYWIDGET_HPP
#define DISPLAYWIDGET_HPP

#include "Frame.hpp"

#include <QGLWidget>

#include <boost/optional.hpp>

/**
 * Widget that draws the vector field.
 */
class DisplayWidget : public QGLWidget {
	Q_OBJECT

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

    // Whether the user is currently selecting a subdisplay or toggling barriers.
    enum {HOVER, NONE, SELECT, TOGGLE} interactionMode = NONE;

    int start_row = -1;
    int start_col = -1;
    int cur_row = -1;
    int cur_col = -1;

    int prev_frame_rows = -1;
    int prev_frame_cols = -1;

public:
    enum HeatmapType {DENSITY, SPEED, X_VEL, Y_VEL} heatmapType = SPEED;

protected:
    void emitHover();

    void leaveEvent(QEvent*) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public:

	DisplayWidget(QWidget* parent = nullptr);

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
    void setHeatmapType(HeatmapType t);

signals:
    void hover(QString);
    void selected(int x, int y, int w, int h);
    void toggle(int row, int col);
};

#endif // DISPLAYWIDGET_HPP
