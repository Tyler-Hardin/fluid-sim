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
    int side = qMin(width, height);
    std::cout << width << std::endl;
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
	float range_x = 2;
	float range_y = 2;
	
//	if(_width > _height){
//		range_y *= ((float)_height / _width);
//	}
//	else{
//		range_x *= ((float)_width / _height);
//	}
	
    glOrtho(-range_x, range_x, -range_y, range_y, 1.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}

static void square(float x, float y, float w, float h){
    glBegin(GL_QUADS);
        glVertex2f(x-w/2,y-h/2);
        glVertex2f(x-w/2,y+h/2);
        glVertex2f(x+w/2,y+h/2);
        glVertex2f(x+w/2,y-h/2);
    glEnd();
}

static void circle(float x, float y, float r){
	constexpr int triangles = 10;
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(x,y);
	for(int i = 0;i <= triangles;i++){
		glVertex2f(x + r * cos(2 * M_PI / triangles * i),y + r * sin(2 * M_PI / triangles * i));
	}
	glEnd();
}

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
	
	//circle(x+u, y+v, .025);
}

void VecFieldWidget::draw(){
	float minx = -1.95;
	float maxx =  1.95;
	float miny = -1.95;
	float maxy =  1.95;
	
	if(_width > _height){
		maxy = maxy * ((float)_height / _width);
		miny = miny * ((float)_height / _width);
	}
	else{
		maxx = maxx * ((float)_width / _height);
		minx = minx * ((float)_width / _height);
	}
	
	float stepx = (maxx - minx) / _width;
	float stepy = (maxy - miny) / _height;
	
	constexpr float scale = .1;
	
	if(_u == nullptr || _v == nullptr)
		return;
	
	QVector<float> lengths;
	lengths.resize(_height * _width);
	
	float max_len;
	float min_len;
	for(int i = 0;i < _height * _width;i++){
		float u = _u->at(i);
		float v = _v->at(i);
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
	
	int i = 0;
	for(int yi = 0;yi < _height;yi++){
		for(int xi = 0;xi < _width;xi++){
			float x = minx + stepx  * xi + stepx / 2;
			float y = miny + stepy  * yi + stepy / 2;
			float len = lengths[i];
			
			
    		if(!_barriers->at(i)){
				int hue = 240 - len / max_len * 240;
				qglColor(QColor::fromHsv(hue, 240, 128));
				square(x, y, stepx, stepy);
			}
			else{
				qglColor(Qt::gray);
				square(x, y, stepx, stepy);
			}
			i++;
		}
	}
	
	i = 0;
	qglColor(Qt::black);
	for(int yi = 0;yi < _height;yi++){
		for(int xi = 0;xi < _width;xi++){
			float x = minx + stepx  * xi + stepx / 2;
			float y = miny + stepy  * yi + stepy / 2;
			float len = lengths[i];
			
    		if(!_barriers->at(i)){
				arrow(x, y, scale * _u->at(i) / len, scale * _v->at(i) / len);
			}
			i++;
		}
	}
}

void VecFieldWidget::setData(int height, int width, const QVector<bool>& barriers, 
	const QVector<float>& u, const QVector<float>& v){
	_height = height;
	_width = width;
	
	_barriers = &barriers;
	_u = &u;
	_v = &v;
}
