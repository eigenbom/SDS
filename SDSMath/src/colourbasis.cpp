#include "colourbasis.h"
#include "hmath.h"

using namespace Math;
using std::string;

ParamList ColourBasis::mParameters;

const Parameter* 
ColourBasis::getParameter(string name)
{
	std::vector<Parameter>::const_iterator it = getParameters().begin();
	while (it!=mParameters.end())
	{
		if (it->name == name) return &(*it);
		it++;
	}
	return NULL;
}

ColourMap::ColourMap() 
:mBasis(0),mSpline(0)
{
	mSpline = new ColourSpline();	
}

ColourMap::~ColourMap()
{
	delete mSpline;	
}

Colour 
ColourMap::
operator()(double x, double y, double z)
{
	double val = (*mBasis)(x,y,z);
	return (*mSpline)(val); 
}


Colour 
Marble::
operator()(double x, double y, double z)
{
	x += (*mBasis)(x,y,z);
	if (x > 1) x -= floor(x);
	return (*mSpline)(x);		
}


Flagstone::Flagstone()
{}

Flagstone::~Flagstone()
{}

Colour 
Flagstone::
operator()(double x, double y, double z)
{
	unsigned long ref = mBasis.getCell(x,y,z);
	double val = mBasis.n2n1(x,y,z);
	
	static Colour mortar = Colour(250/255.,240/255.,215/255.);
	static Colour table[4] = {Colour(236./255,168./255,134./255), 
														Colour(233./255,220./255,191./255), 
														Colour(238./255,204./255,122./255), 
														Colour(199./255,162./255,103./255)};
	
	// get upper 2 bits
	ref >>= 30;
	return Colour::mix(table[ref]*(0.7+0.5*val)*(1+0.1*mNoise(5*x,5*y,5*z)),mortar*(1+0.05*mNoise(7.7*x,7.7*y,7.7*z)),1-Math::step(0.1,val));
}

// CMixer
CMixer::CMixer():mBasis(0),mOne(0),mTwo(0){}

Colour
CMixer::
operator()(double x, double y, double z)
{
	return Colour::mix((*mOne)(x,y,z),(*mTwo)(x,y,z),(*mBasis)(x,y,z));
}

//CDarken

ParamList CDarken::mParameters;

CDarken::CDarken()
:mOne(0),mBasis(0),mAmount(0.5)
{
	if (mParameters.size()==0)
	{
		mParameters = ColourBasis::mParameters;
		mParameters.push_back(Parameter("darken",0.0f,1.0f,0.5f));
	}
}

void CDarken::setParameter(string name, double f)
{
	if (name=="darken")
		mAmount = f;
	else ColourBasis::setParameter(name,f);
}

double CDarken::getParameterf(string name)
{
	if (name=="darken")
		return mAmount;
	else return ColourBasis::getParameterf(name);

}

Colour 
CDarken::
operator()(double x, double y, double z) // returns a single double in the range [0,1]
{
	return (*mOne)(x,y,z) * Math::lerp(1,(*mBasis)(x,y,z),mAmount);
}


