#include "colour.h"
#include "hmath.h"

#include <iostream>

Colour::Colour(){}

Colour::Colour(double _r, double _g, double _b, double _a)
:r(_r),g(_g),b(_b),a(_a){}

Colour Colour::operator*(double f) const
{
	return Colour(r*f,g*f,b*f,a*f);
}

Colour Colour::operator+(const Colour& c) const
{	
	return Colour(r+c.r,g+c.g,b+c.b,a+c.a);
}

Colour Colour::operator-(const Colour& c) const
{	
	return Colour(r-c.r,g-c.g,b-c.b,a-c.a);
}

void Colour::clamp()
{
	r = (double)Math::clamp(0,1,r);
	g = (double)Math::clamp(0,1,g);
	b = (double)Math::clamp(0,1,b);
	a = (double)Math::clamp(0,1,a);
}

Colour Colour::mix(Colour a, Colour b, double f)
{
	return a*(1-f) + b*f;
}

const Colour Colour::black(0,0,0,1);
const Colour Colour::white(1,1,1,1);
const Colour Colour::blue(0,0,1,1);
const Colour Colour::red(1,0,0,1);
const Colour Colour::green(0,1,0,1);
const Colour Colour::yellow(1,1,0,1);
const Colour Colour::cyan(0,1,1,1);
const Colour Colour::magenta(1,0,1,1);

// catromSpline: Performs a catmull-rom spline interpolation at val of the double,Colour pair array
Colour Colour::catromSpline(double val, std::vector<std::pair<double,Colour> >& colours)
{
	typedef std::pair<double,Colour> sPair;
	typedef std::vector<sPair>				sList;
	
	sList::iterator it = colours.begin();
	while (it!=colours.end())
	{
		if (val<it->first) break;
		it++;
	}

	if (it!=colours.begin()) it--;
	if (it==colours.end()) it--;

	sList::iterator itm1 = it;
	if (itm1!=colours.begin()) itm1--;
	
	sList::iterator itp1 = it; itp1++;
	if (itp1==colours.end()) itp1--;
	
	sList::iterator itp2 = itp1; itp2++;
	if (itp2==colours.end()) itp2--;
	
	/* val is somewhere between p1 and p2. */
	sPair& pp0 = *itm1;
	sPair& pp1 = *it;
	sPair& pp2 = *itp1;
	sPair& pp3 = *itp2;


	/* Now interpolate! */
	double dp02 = pp2.first - pp0.first;
	double dp13 = pp3.first - pp1.first;
	double t = (val - pp1.first)/(pp2.first - pp1.first);
	if (t<0) t = 0;
	else if (t>1) t = 1;

	Colour m1 = (pp2.second - pp0.second) * (1/dp02);
	Colour m2 = (pp3.second - pp1.second) * (1/dp13);
	
	Colour& p1 = pp1.second;	
	Colour& p2 = pp2.second;	
	
	double H0 = (2*t-3)*t*t + 1;
	double H1 = ((t - 2)*t + 1)*t;
	double H2 = ((-2*t+3)*t*t);
	double H3 = (t - 1)*t*t;

	return p1*H0 + m1*H1 + p2*H2 + m2*H3;
}

/* The following code is adapted from Texturing&Modelling */

/* Coefficients of basis matrix. */
#define CR00     -0.5
#define CR01      1.5
#define CR02     -1.5
#define CR03      0.5
#define CR10      1.0
#define CR11     -2.5
#define CR12      2.0
#define CR13     -0.5
#define CR20     -0.5
#define CR21      0.0
#define CR22      0.5
#define CR23      0.0
#define CR30      0.0
#define CR31      1.0
#define CR32      0.0
#define CR33      0.0
	
Colour
Colour::spline(double val, std::vector<Colour>& knot)
{
			int nknots = knot.size();

			int span;
			int nspans = nknots - 3;
			Colour c0, c1, c2, c3;	/* coefficients of the cubic.*/

			if (nspans < 1) {  /* illegal */
					return Colour::black;
			}

			/* Find the appropriate 4-point span of the spline. */
			double x = Math::clamp(0, 1, val) * nspans;
			span = (int) x;
			if (span >= nknots - 3)
					span = nknots - 3;
			x -= span;

			/* Evaluate the span cubic at x using Horner's rule. */
			c3 = knot[0+span]*CR00 + knot[1+span]*CR01
				 + knot[2+span]*CR02 + knot[3+span]*CR03;
			c2 = knot[0+span]*CR10 + knot[1+span]*CR11
				 + knot[2+span]*CR12 + knot[3+span]*CR13;
			c1 = knot[0+span]*CR20 + knot[1+span]*CR21
				 + knot[2+span]*CR22 + knot[3+span]*CR23;
			c0 = knot[0+span]*CR30 + knot[1+span]*CR31
				 + knot[2+span]*CR32 + knot[3+span]*CR33;

			Colour result = ((c3*x + c2)*x + c1)*x + c0;
			result.clamp();
			return result;

}
