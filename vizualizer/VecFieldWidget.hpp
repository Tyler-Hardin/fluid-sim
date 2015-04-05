#ifndef VEC_FIELD_WIDGET_HPP
#define VEC_FIELD_WIDGET_HPP

#include <QGLWidget>

class VecFieldWidget : public QGLWidget {
	Q_OBJECT

	int elapsed;
	
	int _height = 0, _width = 0;
	const QVector<float>* _u = nullptr;
	const QVector<float>* _v = nullptr;
    
public:
	VecFieldWidget(QWidget* parent = nullptr);
	
	QSize sizeHint() const;
	
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void draw();
	
	void setData(int height, int width, const QVector<float>& u, const QVector<float>& v);
};

#endif // VEC_FIELD_WIDGET_HPP
