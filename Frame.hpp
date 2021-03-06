#ifndef VEC_FIELD_HPP
#define VEC_FIELD_HPP

#include <armadillo>

#include <QDataStream>
#include <QVector>

#include <boost/shared_array.hpp>

/**
 * Represents a vector field.
 */
class Frame {
	boost::shared_array<const bool> barriers;

public:
	int height;
	int width;
	arma::mat ux;
	arma::mat uy;
	arma::mat density;

	bool getBarrier(int row, int col);
    Frame getSubframe(int row, int col, int height, int width);

	Frame(int height, int width,
		  const boost::shared_array<const bool> barriers,
		  const arma::mat& ux,
		  const arma::mat& uy,
		  const arma::mat& density);
	
	// Leave the default copy constructor.
	Frame(const Frame&) = default;
	Frame& operator=(const Frame&) = default;
};

#endif // VEC_FIELD_HPP
