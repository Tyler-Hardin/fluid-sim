#ifndef VEC_FIELD_HPP
#define VEC_FIELD_HPP

#include <QDataStream>
#include <QVector>

/**
 * Represents a vector field.
 */
class VecField {

	VecField(int height, int width, 
		const QVector<float>& u, const QVector<float>& v) : 
		height(height), width(width), u(u), v(v) {}
public:
	const int height;
	const int width;
	const QVector<float> u;
	const QVector<float> v;
	
	// Leave the normal copy constructor.
	VecField(const VecField&) = default;
	
	/**
	 * Read a new vector field from input stream.
	 *
	 * @param in		the input stream
	 * @param height	the height of the vector field
	 * @param width		the width of the vector field
	 */
	static VecField read(QDataStream& in, int height, int width);
};

#endif // VEC_FIELD_HPP
