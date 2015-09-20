#include "SimState.hpp"

/**
 * Implements numpy.array.roll().
 */
template<typename T>
static arma::Mat<T> roll(arma::Mat<T> in, int shift, int axis) {
	arma::Mat<T> out(in.n_rows, in.n_cols);

	shift = -1*shift;

	if(shift < 0) {
		if(axis == 0) {
			while(shift < 0) {
				shift += in.n_rows;
			}
		}
		else if(axis == 1) {
			while(shift < 0) {
				shift += in.n_cols;
			}
		}
	}

	for(arma::uword row = 0;row < in.n_rows;row++) {
		for(arma::uword col = 0;col < in.n_cols;col++) {
			if(axis == 0) {
				out(row, col) = in((row + shift) % in.n_rows, col);
			} else if(axis == 1) {
				out(row, col) = in(row, (col + shift) % in.n_cols);
			}
		}
	}
	return out;
}

SimState::SimState(int height, int width, double viscosity, double u0) : height(height),
	width(width), viscosity(viscosity), omega(1 / (3*viscosity + 0.5)), u0(u0) {}

/**
 * @brief Implement collide step of LBM.
 */
void SimState::collide() {
	rho = n0 + nN + nS + nE + nW + nNE + nSE + nNW + nSW;
	ux = (nE + nNE + nSE - nW - nNW - nSW) / rho;
	uy = (nN + nNE + nNW - nS - nSE - nSW) / rho;
	arma::mat ux2 = ux % ux;				// pre-compute terms used repeatedly...
	arma::mat uy2 = uy % uy;
	arma::mat u2 = ux2 + uy2;
	arma::mat omu215 = 1 - 1.5*u2;		// "one minus u2 times 1.5"
	arma::mat uxuy = ux % uy;

	n0 = (1-omega)*n0 + omega * four9ths * rho % omu215;
	nN = (1-omega)*nN + omega * one9th * rho % (omu215 + 3*uy + 4.5*uy2);
	nS = (1-omega)*nS + omega * one9th * rho % (omu215 - 3*uy + 4.5*uy2);
	nE = (1-omega)*nE + omega * one9th * rho % (omu215 + 3*ux + 4.5*ux2);
	nW = (1-omega)*nW + omega * one9th * rho % (omu215 - 3*ux + 4.5*ux2);
	nNE = (1-omega)*nNE + omega * one36th * rho % (omu215 + 3*(ux+uy) + 4.5*(u2+2*uxuy));
	nNW = (1-omega)*nNW + omega * one36th * rho % (omu215 + 3*(-ux+uy) + 4.5*(u2-2*uxuy));
	nSE = (1-omega)*nSE + omega * one36th * rho % (omu215 + 3*(ux-uy) + 4.5*(u2-2*uxuy));
	nSW = (1-omega)*nSW + omega * one36th * rho % (omu215 + 3*(-ux-uy) + 4.5*(u2+2*uxuy));


	// Force steady rightward flow at ends (no need to set 0, N, and S components):
	int rows = height;
	for(int row = 0;row < rows;row++) {
		nE(row,0) = one9th * (1 + 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
		nW(row,0) = one9th * (1 - 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
		nNE(row,0) = one36th * (1 + 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
		nSE(row,0) = one36th * (1 + 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
		nNW(row,0) = one36th * (1 - 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
		nSW(row,0) = one36th * (1 - 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	}
}

/**
 * @brief Implement stream step of LBM.
 */
void SimState::stream() {
	nN  = roll(nN,   1, 0);			// axis 0 is north-south; + direction is north
	nNE = roll(nNE,  1, 0);
	nNW = roll(nNW,  1, 0);
	nS  = roll(nS,  -1, 0);
	nSE = roll(nSE, -1, 0);
	nSW = roll(nSW, -1, 0);
	nE  = roll(nE,   1, 1);			// axis 1 is east-west; + direction is east
	nNE = roll(nNE,  1, 1);
	nSE = roll(nSE,  1, 1);
	nW  = roll(nW,  -1, 1);
	nNW = roll(nNW, -1, 1);
	nSW = roll(nSW, -1, 1);

	// Use tricky boolean arrays to handle barrier collisions (bounce-back):
//	nN[barrierN] = nS[barrier]
//	nS[barrierS] = nN[barrier]
//	nE[barrierE] = nW[barrier]
//	nW[barrierW] = nE[barrier]
//	nNE[barrierNE] = nSW[barrier]
//	nNW[barrierNW] = nSE[barrier]
//	nSE[barrierSE] = nNW[barrier]
//	nSW[barrierSW] = nNE[barrier]

	int rows = height;
	int cols = width;
	for(int row = 0;row < rows;row++) {
		for(int col = 0;col < cols;col++) {
			if(barrier[row * cols + col]) {
				if(row > 0) {
					nS(row - 1, col) = nN(row, col);

					if(col > 0) {
						nSW(row - 1, col - 1) = nNE(row, col);
					}
					if(col < cols - 1) {
						nSE(row - 1, col + 1) = nNW(row, col);
					}
				}
				if(row < rows - 1) {
					nN(row + 1, col) = nS(row, col);

					if(col > 0) {
						nNW(row + 1, col - 1) = nSE(row, col);
					}
					if(col < cols - 1) {
						nNE(row + 1, col + 1) = nSW(row, col);
					}
				}
				if(col > 0) {
					nW(row, col - 1) = nE(row, col);
				}
				if(col < cols - 1) {
					nE(row, col + 1) = nW(row, col);
				}
			}
		}
	}
}
