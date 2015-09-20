#ifndef SIMSTATE_H
#define SIMSTATE_H

#include <armadillo>

class SimState
{
	int height;						// lattice dimensions
	int width;
	double viscosity;				// fluid viscosity
	double omega;					// "relaxation" parameter
	double u0;						// initial and in-flow speed

	bool* barrier;

	constexpr static double four9ths = 4.0/9.0;	// abbreviations for lattice-Boltzmann weight factors
	constexpr static double one9th   = 1.0/9.0;
	constexpr static double one36th  = 1.0/36.0;

	arma::mat n0 = four9ths * (arma::mat(height, width, arma::fill::ones)  - 1.5*u0*u0);
	arma::mat nN = one9th * (arma::mat(height, width, arma::fill::ones)  - 1.5*u0*u0);
	arma::mat nS = one9th * (arma::mat(height, width, arma::fill::ones)  - 1.5*u0*u0);
	arma::mat nE = one9th * (arma::mat(height, width, arma::fill::ones) + 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	arma::mat nW = one9th * (arma::mat(height, width, arma::fill::ones) - 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	arma::mat nNE = one36th * (arma::mat(height, width, arma::fill::ones) + 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	arma::mat nSE = one36th * (arma::mat(height, width, arma::fill::ones) + 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	arma::mat nNW = one36th * (arma::mat(height, width, arma::fill::ones) - 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	arma::mat nSW = one36th * (arma::mat(height, width, arma::fill::ones) - 3*u0 + 4.5*u0*u0 - 1.5*u0*u0);
	arma::mat rho = n0 + nN + nS + nE + nW + nNE + nSE + nNW + nSW;	// macroscopic density
	arma::mat ux = (nE + nNE + nSE - nW - nNW - nSW) / rho;			// macroscopic x velocity
	arma::mat uy = (nN + nNE + nNW - nS - nSE - nSW) / rho;			// macroscopic y velocity

	void stream();
	void collide();

public:
	SimState(int height, int width, double viscosity = 0.02, double u0 = 0.1);
};

#endif // SIMSTATE_H
