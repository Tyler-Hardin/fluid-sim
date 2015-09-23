#ifndef SIMSTATE_HPP
#define SIMSTATE_HPP

#include "Frame.hpp"

#include <armadillo>

#include <boost/shared_array.hpp>

class SimState
{
	// Set once the simulation has been started (when step() is first called).
	bool started = false;

	int height;						// lattice dimensions
	int width;
	//double viscosity;				// fluid viscosity
	double omega;					// "relaxation" parameter
	double u0;						// initial and in-flow speed

	boost::shared_array<bool> barrier;

	// abbreviations for lattice-Boltzmann weight factors
	constexpr static double four9ths = 4.0/9.0;
	constexpr static double one9th   = 1.0/9.0;
	constexpr static double one36th  = 1.0/36.0;

	arma::mat n0;
	arma::mat nN;
	arma::mat nS;
	arma::mat nE;
	arma::mat nW;
	arma::mat nNE;
	arma::mat nSE;
	arma::mat nNW;
	arma::mat nSW;
	arma::mat rho;		// macroscopic density
	arma::mat _ux;		// macroscopic x velocity
	arma::mat _uy;		// macroscopic y velocity

	void stream();
	void collide();

public:
	SimState(int height, int width, double viscosity = 0.02, double u0 = 0.1);

	Frame getFrame();
	void step();

	void toggleBarrier(int row, int col);

	const arma::mat& ux();
	const arma::mat& uy();
	const arma::mat& density();

};

#endif // SIMSTATE_HPP
