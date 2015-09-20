#include "VecFieldWidget.hpp"

#include <cmath>
#include <iostream>

VecFieldWidget::VecFieldWidget(QWidget* parent) : QGLWidget(parent) {
	elapsed = 0;
	setAutoFillBackground(true);
}

QSize VecFieldWidget::sizeHint() const
{
    return QSize(400, 400);
}

void VecFieldWidget::initializeGL()
{
    qglClearColor(Qt::white);
}

void VecFieldWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    draw();
}

void VecFieldWidget::resizeGL(int width, int height)
{
	if(vecField == nullptr)
		return;
		
	float aspect_ratio = (float)vecField->width / vecField->height;
    
	range_x = 2;
	range_y = 2;
	
	// Determine how to scale ortho.
	if(aspect_ratio > 1)
		range_x *= aspect_ratio;
	else
		range_y *= 1/aspect_ratio;
	
	// Use most of screen for the aspect ratio of the vector field.
	if((float)width / height >= aspect_ratio){
		// Extra width, so use all of height and center width.
    	glViewport(
    		(width - aspect_ratio * height) / 2, 
    		0, 
    		aspect_ratio * height, 
    		height);
	}
	else{
		// Extra height, so use all width and center height.
    	glViewport(
    		0, 
    		(height - width / aspect_ratio) / 2, 
    		width, 
    		width / aspect_ratio);
	}
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-range_x, range_x, -range_y, range_y, 1.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
    
    range_x *= .95;
    range_y *= .95;
}

/**
 * Draws an array with base at x,y.
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
	float minx = -range_x;
	float maxx =  range_x;
	float miny = -range_y;
	float maxy =  range_y;
	
	// Check that the vector field has been set.
	if(vecField == nullptr)
		return;
	
	// Determine the step size of the grid.
	float stepx = (maxx - minx) / vecField->width;
	float stepy = (maxy - miny) / vecField->height;
	
	// Scale of vectors.
	constexpr float scale = .075;
	
	// Find max and min lengths so we can normalize vector length.
	// Lengths are also used in speed heat map.
	QVector<float> lengths;
	lengths.resize(vecField->height * vecField->width); // Pre-resize vector. This is efficient.
	float max_len;
	float min_len;
	for(int i = 0;i < vecField->height * vecField->width;i++){
		float u = vecField->u.at(i);
		float v = vecField->v.at(i);
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
	for(int yi = 0;yi < vecField->height;yi++){
		for(int xi = 0;xi < vecField->width;xi++){
			float x = minx + stepx  * xi + stepx / 2;
			float y = miny + stepy  * yi + stepy / 2;
			float len = lengths[i];
			
			
    		if(!barriers->at(i)){
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
		for(int yi = 0;yi < vecField->height;yi++){
			for(int xi = 0;xi < vecField->width;xi++){
				float x = minx + stepx  * xi + stepx / 2;
				float y = miny + stepy  * yi + stepy / 2;
				float len = lengths[i];

				if(!barriers->at(i)){
					// If there's not a barrier here, draw the vector.
					arrow(x, y, scale * vecField->u.at(i) / len, scale * vecField->v.at(i) / len);
				}
				i++;
			}
		}
	}
}

void VecFieldWidget::setData(const QVector<bool>& barriers, const VecField& vecField){
	this->barriers = &barriers;
	this->vecField = &vecField;
}

void VecFieldWidget::setDrawVectors(bool b) {
	_drawVectors = b;
	update();
}
