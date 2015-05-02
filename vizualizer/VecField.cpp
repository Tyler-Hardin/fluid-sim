#include "VecField.hpp"

VecField VecField::read(QDataStream& in, int height, int width){
	QVector<float> u(height * width);
	QVector<float> v(height * width);
	
	for(int i = 0;i < height * width && !in.atEnd();i++){
		in >> u[i];
		in >> v[i];
	}
	
	return VecField(height, width, u, v);
}
