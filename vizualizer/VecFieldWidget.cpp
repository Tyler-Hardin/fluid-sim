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
    qglClearColor(Qt::black);

}

void VecFieldWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    draw();
}

void VecFieldWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef QT_OPENGL_ES_1
    glOrthof(-2, +2, -2, +2, 1.0, 15.0);
#else
    glOrtho(-2, +2, -2, +2, 1.0, 15.0);
#endif
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

static void arrow(float x, float y, float u, float v){
	glBegin(GL_LINES);
	glVertex2f(x,y);
	glVertex2f(x+u,y+v);
	glEnd();
}

void VecFieldWidget::draw(){
	constexpr float minx = -2;
	constexpr float maxx =  2;
	constexpr float miny = -2;
	constexpr float maxy =  2;
	constexpr float scale = .1;
	
	if(_u == nullptr || _v == nullptr)
		return;
	
	QVector<float> lengths;
	lengths.resize(_height * _width);
	
	for(int i = 0;i < _height * _width;i++){
		float u = _u->at(i);
		float v = _v->at(i);
		lengths[i] = std::sqrt(u*u + v*v);
	}
	
    qglColor(Qt::gray);
	int i = 0;
	for(int yi = 0;yi < _height;yi++){
		for(int xi = 0;xi < _width;xi++){
			float x = minx + (maxx - minx) / _width  * xi;
			float y = miny + (maxy - miny) / _height * yi;
			arrow(x, y, scale * _u->at(i) / lengths[i], scale * _v->at(i) / lengths[i]);
			i++;
		}
	}
}

void VecFieldWidget::setData(int height, int width, 
	const QVector<float>& u, const QVector<float>& v){
	_height = height;
	_width = width;
	_u = &u;
	_v = &v;
}
