#include "DisplayWidget.hpp"

#include <QMouseEvent>

#include <cmath>
#include <iostream>
#include <sstream>

#include <QDebug>

static constexpr float MARGIN = .025;

DisplayWidget::DisplayWidget(QWidget* parent) : QGLWidget(parent) {
	setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qDebug() << maximumSize();
    setMouseTracking(true);


    resizeGL(size().width(), size().height());
    update();
}

/**
 * @brief Handles when the mouse leaves the widget.
 */
void DisplayWidget::leaveEvent(QEvent*) {
    if(interactionMode == HOVER) {
        interactionMode = NONE;
        emit hover("");
    }
}

/**
 * @brief Emits a hover signal with the raw data from the grid point where the mouse is.
 */
void DisplayWidget::emitHover() {
    if(frame) {
        int row = cur_row;
        int col = cur_col;

        if(row == -1 || col == -1) {
            return;
        }

        float ux = frame->ux.at(row, col);
        float uy = frame->uy.at(row, col);
        float density = frame->density.at(row, col);
        float speed = sqrt(ux*ux + uy*uy);

        auto s = QString("");
        QTextStream stream(&s);
        stream.setNumberFlags(QTextStream::ForceSign);
        stream.setRealNumberNotation(QTextStream::FixedNotation);
        stream << "ux = " << ux << ", uy = " << uy << ", speed = " << speed << ", density = " << density;

        emit hover(stream.readAll());
    }
}

/**
 * @brief Handles mouse movements.
 * @param event
 */
void DisplayWidget::mouseMoveEvent(QMouseEvent* event) {
    int row =  getRow(event->y());
    int col = getCol(event->x());

    if(interactionMode == NONE || interactionMode == HOVER) {
        interactionMode = HOVER;
        cur_row = row;
        cur_col = col;
        emitHover();
    }
    else if(interactionMode == SELECT) {
        cur_row = row;
        cur_col = col;
        update();
    } else if(interactionMode == TOGGLE){
        if(row != cur_row || col != cur_col) {
            cur_row = row;
            cur_col = col;
            emit toggle(row, col);
        }
    }
}

/**
 * @brief Handles mouse presses.
 * @param event
 */
void DisplayWidget::mousePressEvent(QMouseEvent* event) {
    start_row = getRow(event->y());
    start_col = getCol(event->x());
    cur_row = start_row;
    cur_col = start_col;

    if(event->modifiers() == Qt::ShiftModifier) {
        interactionMode = SELECT;
        update();
    } else {
        interactionMode = TOGGLE;
        emit toggle(cur_row, cur_col);
    }
}

/**
 * @brief Handles mouse releases.
 * @param event
 */
void DisplayWidget::mouseReleaseEvent(QMouseEvent* event) {
    if(interactionMode == SELECT) {
        cur_row = getRow(event->y());
        cur_col = getCol(event->x());
        update();

        int r1 = start_row;
        int r2 = cur_row;
        int c1 = start_col;
        int c2 = cur_col;

        int start_r = std::min({r1, r2});
        int start_c = std::min({c1, c2});
        int height = std::max({r1, r2}) - start_r + 1;
        int width = std::max({c1, c2}) - start_c + 1;

        if(height <= 1 || width <= 1)
            return;

        emit selected(start_r, start_c, height, width);
        resizeGL(size().width(), size().height());
        update();
    }

    interactionMode = NONE;
}

/**
 * @brief DisplayWidget::getRow
 * @param pixel         the y pixel of the mouse click
 * @return the row in the simulation grid cooresponding to the clicked pixel
 */
int DisplayWidget::getRow(int pixel) {
	// Determine drawing box coordinates.
	float miny = range_y * MARGIN;
	float maxy = range_y * (1 - MARGIN);

	// Check that the vector field has been set.
	if(!frame)
		return -1;

	// Determine the step size of the grid.
	//float stepy = (maxy - miny) / frame->height;

	double point = (((double)(pixel - vp_y_off) / vp_height * range_y) - miny) / (maxy - miny);
    int index = point * frame->height;
	if(point >= 0 && point <= 1) {
		return index;
	}
	else {
		return -1;
	}
}

/**
 * @brief DisplayWidget::getCol
 * @param pixel         the x pixel of the mouse click
 * @return the col in the simulation grid cooresponding to the clicked pixel
 */
int DisplayWidget::getCol(int pixel) {
	// Determine drawing box coordinates.
	float minx = range_x * MARGIN;
	float maxx = range_x * (1 - MARGIN);

	// Check that the vector field has been set.
	if(!frame)
		return -1;

	// Determine the step size of the grid.
	//float stepx = (maxx - minx) / frame->width;

	double point = (((double)(pixel - vp_x_off) / vp_width * range_x) - minx) / (maxx - minx);
    int index = point * frame->width;
	if(point >= 0 && point <= 1) {
		return index;
	}
	else {
		return -1;
	}
}

/**
 * @brief Gives Qt a hint about the default size of this widget.
 * @return
 */
QSize DisplayWidget::sizeHint() const
{
    return QSize(400, 400);
}

/**
 * @brief Initializes GL state.
 */
void DisplayWidget::initializeGL()
{
	//auto color = this->palette().color(QPalette::Background);
	//qglClearColor(color);
    qglClearColor(Qt::white);
}

/**
 * @brief Calls OpenGL drawing functions.
 */
void DisplayWidget::paintGL()
{
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    draw();
}

/**
 * @brief Handles resize/scale of view when the widget is resized.
 * @param width
 * @param height
 */
void DisplayWidget::resizeGL(int width, int height)
{
	if(!frame)
		return;
		
	float aspect_ratio = (float)frame->width / frame->height;
    
	range_x = 1;
	range_y = 1;
	
	// Determine how to scale ortho.
	//	if(aspect_ratio > 1)
	//		range_x *= aspect_ratio;
	//	else
	//		range_y *= 1/aspect_ratio;
	if(aspect_ratio > 1)
		range_y *= 1/aspect_ratio;
	else
		range_x *= aspect_ratio;
	
	// Use most of screen for the aspect ratio of the vector field.
	if((float)width / height >= aspect_ratio){
		// Extra width, so use all of height and center width.
		vp_x_off = (width - aspect_ratio * height) / 2;
		vp_y_off = 0;
		vp_width = aspect_ratio * height;
		vp_height = height;
	}
	else{
		// Extra height, so use all width and center height.
		vp_x_off = 0;
		vp_y_off = (height - width / aspect_ratio) / 2;
		vp_width = width;
		vp_height = width / aspect_ratio;
    }

    qDebug() << "Resize" << this;/*
    qDebug() << range_x << range_y;
    qDebug() << vp_x_off << vp_y_off;
    qDebug() << vp_width << vp_height;*/
	
    glMatrixMode(GL_PROJECTION);
    glViewport(
        vp_x_off,
        vp_y_off,
        vp_width,
        vp_height);
    glLoadIdentity();
    glOrtho(0, range_x, range_y, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

/**
 * Draws an arrow with base at x,y.
 *
 * @param x				center x
 * @param y				center y
 * @param u				horizontal component
 * @param v				vertical componenet
 */
static void arrow(float x, float y, float u, float v, float s = .25){
	glBegin(GL_LINES);
		glVertex2f(x,y);
		glVertex2f(x+u*(1-s),y+v*(1-s));
	glEnd();
	
	glBegin(GL_TRIANGLES);
		glVertex2f(x + u, y + v);
		glVertex2f(x+u*(1-s) + v*s, y+v*(1-s) - u*s);
		glVertex2f(x+u*(1-s) - v*s, y+v*(1-s) + u*s);
	glEnd();
}

/**
 * Draw a square centered at x,y.
 *
 * @param x				center x
 * @param y				center y
 * @param w				width
 * @param h				height
 */
static void square(float x, float y, float w, float h){
    glBegin(GL_QUADS);
        glVertex2f(x-w/2,y-h/2);
        glVertex2f(x-w/2,y+h/2);
        glVertex2f(x+w/2,y+h/2);
        glVertex2f(x+w/2,y-h/2);
    glEnd();
}

/**
 * @brief Draws the vectors and heatmap based on the current Frame.
 */
void DisplayWidget::draw(){

    qDebug() << "Draw" << this;/*
    qDebug() << range_x << range_y;
    qDebug() << vp_x_off << vp_y_off;
    qDebug() << vp_width << vp_height;*/

	// Determine drawing box coordinates.
	float minx = range_x * MARGIN;
	float maxx = range_x * (1 - MARGIN);
	float miny = range_y * MARGIN;
	float maxy = range_y * (1 - MARGIN);
	
	// Check that the vector field has been set.
	if(!frame)
		return;
	
	// Determine the step size of the grid.
	float stepx = (maxx - minx) / frame->width;
	float stepy = (maxy - miny) / frame->height;
	
	// Scale of vectors.
	float scale = stepx / 3;
	
	// Find max and min lengths so we can normalize vector length.
	// Lengths are also used in speed heat map.
	QVector<float> lengths;
	lengths.resize(frame->height * frame->width); // Pre-resize vector. This is efficient.
	float max_len;
	float min_len;
	for(int i = 0;i < frame->height * frame->width;i++){
		float u = frame->ux.at(i);
		float v = frame->uy.at(i);
		float len = std::sqrt(u*u + v*v);
		lengths[i] = len;
		
		if(i == 0){
			max_len = len;
			min_len = len;
		}
		else if(len > max_len){
			max_len = len;
		}
		else if(len < min_len){
			min_len = len;
		}
	}
	
    auto getHue = [](float n) -> int {
        Q_ASSERT(n >= 0 && n <= 1);
        return (int)(240 - n * 240);
    };

	// Draw the heat map.
	int i = 0;
    float maxDensity = 0;
    float minDensity = 0;
    float maxSpeed = 0;
    float minSpeed = 0;
    float maxUx = 0;
    float minUx = 0;
    float maxUy = 0;
    float minUy = 0;


    if(heatmapType == DENSITY) {
        maxDensity = frame->density.max();
        minDensity = frame->density.min();

        if(maxDensity == minDensity) {
            maxDensity += .01;
        }
    } else if(heatmapType == SPEED) {
        maxSpeed = max_len;
        minSpeed = min_len;

        if(maxSpeed == minSpeed) {
            maxSpeed += .01;
        }
    } else if(heatmapType == X_VEL) {
        maxUx = frame->ux.max();
        minUx = frame->ux.min();

        if(maxUx == minUx) {
            maxUx += .01;
        }
    } else if(heatmapType == Y_VEL) {
        maxUy = frame->uy.max();
        minUy = frame->uy.min();

        if(maxUy == minUy) {
            maxUy += .01;
        }
    } else {
        Q_ASSERT(false);
    }

    for(int xi = 0;xi < frame->width;xi++){
        for(int yi = 0;yi < frame->height;yi++, i++){
            int hue;
			float x = minx + stepx  * xi + stepx / 2;
            float y = miny + stepy  * yi + stepy / 2;
			
			if(!frame->getBarrier(yi, xi)){
    			// If it's not a barrier, draw a color square.
                if(heatmapType == DENSITY) {
                    auto density = frame->density[i];
                    hue = getHue((density - minDensity) / (maxDensity - minDensity));
                } else if(heatmapType == SPEED) {
                    float len = lengths[i];
                    hue = getHue((len - minSpeed) / (maxSpeed - minSpeed));
                } else if(heatmapType == X_VEL) {
                    float ux = frame->ux[i];
                    hue = getHue((ux - minUx) / (maxUx - minUx));
                } else if(heatmapType == Y_VEL) {
                    float uy = frame->uy[i];
                    hue = getHue((uy - minUy) / (maxUy - minUy));
                } else {
                    Q_ASSERT(false);
                    hue = 0;
                }

                if(interactionMode == SELECT &&
                        xi >= std::min({cur_col, start_col}) &&
                        xi <= std::max({cur_col, start_col}) &&
                        yi >= std::min({cur_row, start_row}) &&
                        yi <= std::max({cur_row, start_row})) {
                    qglColor(QColor::fromHsv(hue, 160, 240));
                } else {
                    qglColor(QColor::fromHsv(hue, 240, 200));
                }
                square(x, y, stepx, stepy);
			}
			else{
				// If it's a barrier, draw gray.
				qglColor(Qt::gray);
				square(x, y, stepx, stepy);
			}
		}
	}
	
	// Draw the vectors.
	if(_drawVectors) {
		i = 0;
		qglColor(Qt::black);
		for(int xi = 0;xi < frame->width;xi++){
			for(int yi = 0;yi < frame->height;yi++){
				float x = minx + stepx  * xi + stepx / 2;
				float y = miny + stepy  * yi + stepy / 2;
				float len = lengths[i];

				if(!frame->getBarrier(yi, xi)){
					// If there's not a barrier here, draw the vector.
					arrow(x, y, scale * frame->ux.at(i) / len, scale * frame->uy.at(i) / len);
				}
				i++;
			}
		}
	}
}

/**
 * @brief Sets a new frame.
 * @param frame
 */
void DisplayWidget::setData(const Frame& frame){
    auto oldFrame = this->frame;
    this->frame = frame;
    if(!oldFrame || oldFrame->height != frame.height || oldFrame->width != frame.width) {
        resizeGL(width(), height());
        interactionMode = NONE;
    }

    if(interactionMode == HOVER) {
        emitHover();
    }
}

/**
 * @brief Disables or enables the drawing of vectors.
 * @param b
 */
void DisplayWidget::setDrawVectors(bool b) {
	_drawVectors = b;
	update();
}

/**
 * @brief Changes the heatmap type.
 * @param t
 */
void DisplayWidget::setHeatmapType(HeatmapType t) {
    heatmapType = t;
}
