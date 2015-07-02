#ifndef COLOUR_SPLINE_H
#define COLOUR_SPLINE_H

#include "colour.h"
#include <vector>

/* ColourSpline: A 1-d spline of colours...
 * A catmull rom spline is fitted through the surrounding colours
 * BP 26.08.05
 */
class ColourSpline
{
	public:

	typedef std::pair<double,Colour> Knot;
	typedef std::vector<Knot> List;

	public:

	ColourSpline(){} 
	
	//pass an array in the form val0,r0,g0,b0,a0,val1,r1,g1,b1,a1,...,valn,rn,gn,bn,an
	//where val_i <= val_i+1, val0 = 0, valn = 1 (otherwise the behaviour is undefined...)
	ColourSpline(double* cs, int n);

	Colour operator()(double val); 
	
	void set(double* cs, int n); //same as constructor (overrides any previous colour assignments)
	
	void setColour(int index, Colour col);
	Colour getColour(int index){return mColours[index].second;}
	const List& getColours(){return mColours;}
	
	protected:
	
	List mColours; // the colours
};

#endif
