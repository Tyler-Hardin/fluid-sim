#include "VecFieldWidget.hpp"

#include <QMouseEvent>

#include <cmath>
#include <iostream>

#include <QDebug>

static constexpr float MARGIN = .025;

VecFieldWidget::VecFieldWidget(QWidget* parent) : QGLWidget(parent) {
	elapsed = 0;
	setAutoFillBackground(true);
}

void VecFieldWidget::mousePressEvent(QMouseEvent* event) {
	emit mousePressed(event);
}

int VecFieldWidget::getRow(int pixel) {
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
	/*qDebug() << "Pixel: " << pixel << ", Height: " << height();
	qDebug() << "X Off: " << vp_x_off << ", VP Width: " << vp_width << ", Width: " << width();
	qDebug() << "Y Off: " << vp_y_off << ", VP Height: " << vp_height << ", Height: " << height();
	qDebug() << "range_x: " << range_x << ", range_y: " << range_y << ", stepy: " << stepy;
	qDebug() << point;
	qDebug() << index;*/
	if(point >= 0 && point <= 1) {
		return index;
	}
	else {
		return -1;
	}
}

int VecFieldWidget::getCol(int pixel) {
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
	/*qDebug() << "Pixel: " << pixel << ", Height: " << height();
	qDebug() << "X Off: " << vp_x_off << ", VP Width: " << vp_width << ", Width: " << width();
	qDebug() << "Y Off: " << vp_y_off << ", VP Height: " << vp_height << ", Height: " << height();
	qDebug() << "range_x: " << range_x << ", range_y: " << range_y << ", stepy: " << stepy;
	qDebug() << point;
	qDebug() << index;*/
	if(point >= 0 && point <= 1) {
		return index;
	}
	else {
		return -1;
	}
}

QSize VecFieldWidget::sizeHint() const
{
    return QSize(400, 400);
}

void VecFieldWidget::initializeGL()
{
	//auto color = this->palette().color(QPalette::Background);
	//qglClearColor(color);
	qglClearColor(Qt::white);
}

void VecFieldWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
	glTranslatef(0.0, 0.0, -5.0);
    draw();
}

void VecFieldWidget::resizeGL(int width, int height)
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
	glViewport(
		vp_x_off,
		vp_y_off,
		vp_width,
		vp_height);
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0, range_x, range_y, 0, 1.0, 15.0);
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

void VecFieldWidget::draw(){
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
	
	
	// Draw the heat map.
	int i = 0;
	for(int xi = 0;xi < frame->width;xi++){
		for(int yi = 0;yi < frame->height;yi++){
			float x = minx + stepx  * xi + stepx / 2;
			float y = miny + stepy  * yi + stepy / 2;
			float len = lengths[i];
			
			
			if(!frame->barriers[yi * frame->width + xi]){
    			// If it's not a barrier, draw a color square.
				int hue = 240 - len / max_len * 240;
				qglColor(QColor::fromHsv(hue, 240, 200));
				square(x, y, stepx, stepy);
			}
			else{
				// If it's a barrier, draw gray.
				qglColor(Qt::gray);
				square(x, y, stepx, stepy);
			}
			i++;
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

				if(!frame->barriers[yi * frame->width + xi]){
					// If there's not a barrier here, draw the vector.
					arrow(x, y, scale * frame->ux.at(i) / len, scale * frame->uy.at(i) / len);
				}
				i++;
			}
		}
	}
}

void VecFieldWidget::setData(const Frame& frame){
	this->frame = frame;
}

void VecFieldWidget::setDrawVectors(bool b) {
	_drawVectors = b;
	update();
}
