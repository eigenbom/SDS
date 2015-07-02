#ifndef COLOUR_H
#define COLOUR_H

#include <vector>

struct Colour
{
	double r;
	double g;
	double b;
	double a;
	
	//c'tors
	Colour();
	Colour(double r, double g, double b, double a = 1);

	//ops
	Colour operator*(double amount) const;
	Colour operator+(const Colour& c) const;
	Colour operator-(const Colour& c) const;
	void clamp(); // clamp the colour back into an acceptable range (0,1)

	// utils
	static Colour mix(Colour a, Colour b, double amount); //return a lerped colour mix
	static Colour spline(double val, std::vector<Colour>& colours);
	static Colour catromSpline(double val, std::vector<std::pair<double,Colour> >& colours); //PRE; colours is in order

	//useful constants
	static const Colour black;
	static const Colour white;
	static const Colour blue;
	static const Colour red;
	static const Colour green;
	static const Colour yellow;
	static const Colour cyan;
	static const Colour magenta;
};

#endif
