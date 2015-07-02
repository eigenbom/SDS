#ifndef HMATH_H
#define HMATH_H

#include <cmath>
#include <algorithm>

namespace Math
{
	// const double M_PI = 3.14159265;
	// constants
	const double PI = M_PI;
	const double TWO_PI = 2*PI;

	// other funcs
	inline double step(double a, double x); //0 if a<x, else 1
	inline double pulse(double a, double b, double x); //if x<a or x>b then 0, else 1
	inline double clamp(double a, double b, double x); //clamps x to the range [a,b]

	// double abs(double a);
	inline int sign(double a); // 1 if a>=0 else -1
	inline double lerp(double a, double b, double t);
	inline double smoothstep(double a, double b, double x); //a smooth step from a to b
	inline int powi(int m, int n); // m^n

	using std::abs;
	using std::sin;
	using std::cos;
	using std::asin;
	using std::acos;
	using std::floor;
	using std::ceil;
	using std::pow;
	using std::log;
	using std::sqrt;
	using std::min;
	using std::max;

	inline double mod(double a, double b);
	inline double gammacorrect(double gamma, double x);
	inline double bias(double b, double x);
	inline double gain(double g, double x);

	/**
	 * Solves a x^2 + b x + c = 0
	 *
	 * Returns the number of unique real roots.
	 * Results are in ascending order.
	 */
	int solveQuadratic(double a, double b, double c, double &root1, double &root2);

	/**
	 * Solves x^3 + a x^2 + b x + c = 0
	 *
	 * Returns the number of unique real roots.
	 * Results are in ascending order.
	 */
	int solveCubic(double a, double b, double c, double &root1, double &root2, double &root3);
};

#include "hmath.inl"

#endif
