//  Copyright (c) 2019 AUTHORS
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "octotiger/test_problems/rotating_star.hpp"

#include "octotiger/defs.hpp"
#include "octotiger/options.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

class rotating_star_analytic {
private:
	std::vector<std::vector<double>> rho_;
	std::vector<std::vector<double>> ene_;
	double dr_, dz_, omega_;
	int nr_, nz_;
	static constexpr real coeff[16][16] = { { 0.00390625000000, -0.0351562500000, -0.0351562500000, 0.00390625000000,
			-0.0351562500000, 0.316406250000, 0.316406250000, -0.0351562500000, -0.0351562500000, 0.316406250000,
			0.316406250000, -0.0351562500000, 0.00390625000000, -0.0351562500000, -0.0351562500000, 0.00390625000000 }, {
			-0.00130208333333, 0.0117187500000, 0.0117187500000, -0.00130208333333, 0.0351562500000, -0.316406250000,
			-0.316406250000, 0.0351562500000, -0.0351562500000, 0.316406250000, 0.316406250000, -0.0351562500000,
			0.00130208333333, -0.0117187500000, -0.0117187500000, 0.00130208333333 }, { -0.00390625000000, 0.0351562500000,
			0.0351562500000, -0.00390625000000, 0.00390625000000, -0.0351562500000, -0.0351562500000, 0.00390625000000,
			0.00390625000000, -0.0351562500000, -0.0351562500000, 0.00390625000000, -0.00390625000000, 0.0351562500000,
			0.0351562500000, -0.00390625000000 }, { 0.00130208333333, -0.0117187500000, -0.0117187500000, 0.00130208333333,
			-0.00390625000000, 0.0351562500000, 0.0351562500000, -0.00390625000000, 0.00390625000000, -0.0351562500000,
			-0.0351562500000, 0.00390625000000, -0.00130208333333, 0.0117187500000, 0.0117187500000, -0.00130208333333 }, {
			-0.00130208333333, 0.0351562500000, -0.0351562500000, 0.00130208333333, 0.0117187500000, -0.316406250000,
			0.316406250000, -0.0117187500000, 0.0117187500000, -0.316406250000, 0.316406250000, -0.0117187500000,
			-0.00130208333333, 0.0351562500000, -0.0351562500000, 0.00130208333333 }, { 0.000434027777778, -0.0117187500000,
			0.0117187500000, -0.000434027777778, -0.0117187500000, 0.316406250000, -0.316406250000, 0.0117187500000,
			0.0117187500000, -0.316406250000, 0.316406250000, -0.0117187500000, -0.000434027777778, 0.0117187500000,
			-0.0117187500000, 0.000434027777778 }, { 0.00130208333333, -0.0351562500000, 0.0351562500000, -0.00130208333333,
			-0.00130208333333, 0.0351562500000, -0.0351562500000, 0.00130208333333, -0.00130208333333, 0.0351562500000,
			-0.0351562500000, 0.00130208333333, 0.00130208333333, -0.0351562500000, 0.0351562500000, -0.00130208333333 }, {
			-0.000434027777778, 0.0117187500000, -0.0117187500000, 0.000434027777778, 0.00130208333333, -0.0351562500000,
			0.0351562500000, -0.00130208333333, -0.00130208333333, 0.0351562500000, -0.0351562500000, 0.00130208333333,
			0.000434027777778, -0.0117187500000, 0.0117187500000, -0.000434027777778 }, { -0.00390625000000, 0.00390625000000,
			0.00390625000000, -0.00390625000000, 0.0351562500000, -0.0351562500000, -0.0351562500000, 0.0351562500000,
			0.0351562500000, -0.0351562500000, -0.0351562500000, 0.0351562500000, -0.00390625000000, 0.00390625000000,
			0.00390625000000, -0.00390625000000 }, { 0.00130208333333, -0.00130208333333, -0.00130208333333, 0.00130208333333,
			-0.0351562500000, 0.0351562500000, 0.0351562500000, -0.0351562500000, 0.0351562500000, -0.0351562500000,
			-0.0351562500000, 0.0351562500000, -0.00130208333333, 0.00130208333333, 0.00130208333333, -0.00130208333333 }, {
			0.00390625000000, -0.00390625000000, -0.00390625000000, 0.00390625000000, -0.00390625000000, 0.00390625000000,
			0.00390625000000, -0.00390625000000, -0.00390625000000, 0.00390625000000, 0.00390625000000, -0.00390625000000,
			0.00390625000000, -0.00390625000000, -0.00390625000000, 0.00390625000000 }, { -0.00130208333333, 0.00130208333333,
			0.00130208333333, -0.00130208333333, 0.00390625000000, -0.00390625000000, -0.00390625000000, 0.00390625000000,
			-0.00390625000000, 0.00390625000000, 0.00390625000000, -0.00390625000000, 0.00130208333333, -0.00130208333333,
			-0.00130208333333, 0.00130208333333 }, { 0.00130208333333, -0.00390625000000, 0.00390625000000, -0.00130208333333,
			-0.0117187500000, 0.0351562500000, -0.0351562500000, 0.0117187500000, -0.0117187500000, 0.0351562500000,
			-0.0351562500000, 0.0117187500000, 0.00130208333333, -0.00390625000000, 0.00390625000000, -0.00130208333333 }, {
			-0.000434027777778, 0.00130208333333, -0.00130208333333, 0.000434027777778, 0.0117187500000, -0.0351562500000,
			0.0351562500000, -0.0117187500000, -0.0117187500000, 0.0351562500000, -0.0351562500000, 0.0117187500000,
			0.000434027777778, -0.00130208333333, 0.00130208333333, -0.000434027777778 }, { -0.00130208333333, 0.00390625000000,
			-0.00390625000000, 0.00130208333333, 0.00130208333333, -0.00390625000000, 0.00390625000000, -0.00130208333333,
			0.00130208333333, -0.00390625000000, 0.00390625000000, -0.00130208333333, -0.00130208333333, 0.00390625000000,
			-0.00390625000000, 0.00130208333333 }, { 0.000434027777778, -0.00130208333333, 0.00130208333333, -0.000434027777778,
			-0.00130208333333, 0.00390625000000, -0.00390625000000, 0.00130208333333, 0.00130208333333, -0.00390625000000,
			0.00390625000000, -0.00130208333333, -0.000434027777778, 0.00130208333333, -0.00130208333333, 0.000434027777778 } };
public:
	double interpolate(const std::vector<std::vector<double>>& f, double R, double z) const {
		R = std::abs(R);
		z = std::abs(z);
		int i = int(R / dr_) + nr_ / 2 - 1;
		int k = int(z / dz_) + nz_ / 2 - 1;
		real rc = 0.0;
		if (i >= 0 && i < nr_ - 3 && k >= 0 && k < nz_ - 3) {
			for (int i0 = 0; i0 < 4; i0++) {
				for (int k0 = 0; k0 < 4; k0++) {
					double x = (2.0 * (R / dr_ - int(R / dr_)) - 1.0);
					double y = (2.0 * (z / dz_ - int(z / dz_)) - 1.0);
					for (int i1 = 0; i1 < 4; i1++) {
						for (int k1 = 0; k1 < 4; k1++) {
							rc += coeff[i1 + 4 * k1][i0 + 4 * k0] * f[i + i0][k + k0] * std::pow(x, k1) * std::pow(y, i1);
						}
					}
				}
			}
			return std::max(rc, 0.0);
		} else {
			return 0.0;
		}

	}
	rotating_star_analytic() {
		std::ifstream fp("rotating_star.bin", std::ios::in | std::ios::binary);
		if (fp.fail()) {
			std::cout << "Could not open rotating_star.bin, aborting\n";
			throw;
		}
		std::cout << "Reading rotating_star.bin\n";

		fp.read(reinterpret_cast<char*>(&nr_), sizeof(decltype(nr_)));
		fp.read(reinterpret_cast<char*>(&nz_), sizeof(decltype(nz_)));
		dr_ = 1.0 / nr_;
		dz_ = 1.0 / nz_;
		nr_ *= 2;
		nz_ *= 2;
		fp.read(reinterpret_cast<char*>(&omega_), sizeof(double));
		rho_.resize(nr_, std::vector<double>(nz_));
		ene_.resize(nr_, std::vector<double>(nz_));
		for (int i = 0; i < nr_; i++) {
			for (int k = 0; k < nz_; k++) {
				fp.read(reinterpret_cast<char*>(&(rho_[i][k])), sizeof(double));
				fp.read(reinterpret_cast<char*>(&(ene_[i][k])), sizeof(double));
			}
		}
		std::cout << "Done reading rotating_star.bin\n";
	}
	void state_at(double& rho, double& ene, double& sx, double& sy, double x, double y, double z) const {
		const double R = std::sqrt(x * x + y * y);
		rho = interpolate(rho_, R, z);
		ene = interpolate(ene_, R, z);
		sx = -y * rho * omega_;
		sy = +x * rho * omega_;
	}
	double get_omega() {
		return omega_;
	}

};

std::vector<real> rotating_star(real x, real y, real z, real dx) {
	std::vector<real> u(opts().n_fields, real(0));

	x -= opts().rotating_star_x;

	static rotating_star_analytic rs;
	const real fgamma = 5.0 / 3.0;
	rs.state_at(u[rho_i], u[egas_i], u[sx_i], u[sy_i], x, y, z);
	u[rho_i] = std::max(u[rho_i], 1.0e-10);
	u[egas_i] = std::max(u[egas_i], 1.0e-10);
	u[tau_i] = std::pow(u[egas_i], 1.0 / fgamma);
	u[egas_i] += 0.5 * (std::pow(u[sx_i], 2) + std::pow(u[sy_i], 2)) / u[rho_i];
//	if( u[rho_i] < 0.5 ) {
		u[spc_i + 1] = u[rho_i];
		u[spc_i + 0] = 0;
//	} else {
//		u[spc_i + 0] = u[rho_i];
//		u[spc_i + 1] = 0;
//	}
//	u[zz_i] = rs.get_omega() * dx * dx / 6.0 * u[rho_i];
	for (int s = 2; s < opts().n_species; s++) {
		u[spc_i + s] = 0.0;
	}
	return u;
}

std::vector<real> rotating_star_a(real x, real y, real z, real) {
	return rotating_star(x, y, z, 0);
}

constexpr real rotating_star_analytic::coeff[16][16];
