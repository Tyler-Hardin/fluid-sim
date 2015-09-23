#include "Frame.hpp"

Frame::Frame(int height, int width,
			 const boost::shared_array<const bool> barriers,
			 const arma::mat& ux,
			 const arma::mat& uy,
			 const arma::mat& density) :
	height(height), width(width), barriers(barriers), ux(ux), uy(uy), density(density) {
	Q_ASSERT(ux.n_rows == (arma::uword)height && ux.n_cols == (arma::uword)width);
	Q_ASSERT(uy.n_rows == (arma::uword)height && uy.n_cols == (arma::uword)width);
	Q_ASSERT(density.n_rows == (arma::uword)height && density.n_cols == (arma::uword)width);
}
