#include "colourspline.h"

ColourSpline::ColourSpline(double* cs, int n)
{
	//cerr << "--colourspline--\n";
	int i;
	for(i=0;i<n;i++)
	{
		double* p = &cs[i*5];
		double f = *p;
		Colour c(*(p+1),*(p+2),*(p+3),*(p+4));
		
		mColours.push_back(Knot(f,c));
		//cerr << f << " <" << c.r << " " << c.g << " " << c.b << ">\n";
	}
	//cerr << "----------------\n";
}

void
ColourSpline::set(double* cs, int n)
{
	mColours.clear();
	
	int i;
	for(i=0;i<n;i++)
	{
		double* p = &cs[i*5];
		double f = *p;
		Colour c(*(p+1),*(p+2),*(p+3),*(p+4));
		
		mColours.push_back(Knot(f,c));
	}
}


Colour ColourSpline::operator()(double val)
{
	return Colour::catromSpline(val,mColours);
}

void ColourSpline::setColour(int index, Colour col)
{
	mColours[index].second = col;
}
