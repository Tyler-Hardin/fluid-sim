#include "Frame.hpp"

#include <qdebug.h>

Frame::Frame(int height, int width,
			 const boost::shared_array<const bool> barriers,
			 const arma::mat& ux,
			 const arma::mat& uy,
			 const arma::mat& density) :
    barriers(barriers), height(height), width(width), ux(ux), uy(uy), density(density) {
	Q_ASSERT(ux.n_rows == (arma::uword)height && ux.n_cols == (arma::uword)width);
	Q_ASSERT(uy.n_rows == (arma::uword)height && uy.n_cols == (arma::uword)width);
	Q_ASSERT(density.n_rows == (arma::uword)height && density.n_cols == (arma::uword)width);
}

bool Frame::getBarrier(int row, int col) {
	return barriers[row * width + col];
}

Frame Frame::getSubframe(int row, int col, int height, int width) {
    Q_ASSERT(row + height < this->height);
    Q_ASSERT(col + width < this->width);

    boost::shared_array<bool> new_barriers(new bool[height * width]);

    for(int i = 0;i < height;i++) {
        for(int j = 0;j < width;j++) {
            new_barriers[i * width + j] = getBarrier(row + i, col + j);
        }
    }

    arma::mat newUx = ux.submat(row, col, row + height - 1, col + width - 1);
    qDebug() << "ux row: " << newUx.n_rows;
    return Frame(height, width, new_barriers,
                 newUx,
                 uy.submat(row, col, row + height - 1, col + width - 1),
                 density.submat(row, col, row + height - 1, col + width - 1));
}
