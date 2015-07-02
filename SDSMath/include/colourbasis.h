#ifndef COLOUR_BASIS_H
#define COLOUR_BASIS_H

#include "basis.h"
#include "colour.h"
#include "colourspline.h"

#include <vector>
#include <string>

class ColourBasis: public Parameterised
{
	public:

	virtual ~ColourBasis(){}
	virtual Colour operator()(double x, double y, double z) = 0;
	
	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	virtual const Parameter* getParameter(std::string name);
	
	virtual void setParameter(std::string name, double f){};
	virtual void setParameter(std::string name, int i){};

	virtual double getParameterf(std::string name){return 0;}
	virtual int getParameteri(std::string name){return 0;}

	/* interface for filters and operators */

	virtual void setBasis(Basis* b, int index){};
	virtual void setCBasis(ColourBasis* b, int index){};

	virtual int numBasis(){return 0;} // the number of bases required
	virtual int numCBasis(){return 0;} // the number of colour bases required
	virtual bool isReady(){return true;} // does this operator have all its required bases?

	virtual bool hasSpline(){return false;}
	virtual ColourSpline* getSpline(){return 0;}

	public:

	static ParamList mParameters;

};

class ColourMap: public ColourBasis
{
	public:

	ColourMap();
	~ColourMap();
	virtual Colour operator()(double x, double y, double z);
	
	virtual void setBasis(Basis* b, int index){if (index==0) mBasis = b;}
	virtual void setCBasis(ColourBasis* b, int index){}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual int numCBasis(){return 0;} // the number of colour bases required
	virtual bool isReady(){return mBasis!=0;} // does this operator have all its required bases?

	virtual bool hasSpline(){return true;}
	virtual ColourSpline* getSpline(){return mSpline;}

	protected:

	Basis* mBasis;
	ColourSpline* mSpline;
};

class Marble: public ColourMap
{
	public:

	Marble():ColourMap(){};
	virtual Colour operator()(double x, double y, double z);


};

class Flagstone: public ColourBasis
{
	public:

	Flagstone();
	virtual ~Flagstone();
	virtual Colour operator()(double x, double y, double z);

	private:

	Voronoi mBasis;
	Noise mNoise;
	bool mOwn;
};


// CMixer: lerps between two basis functions
class CMixer: public ColourBasis
{
	public:
	
	CMixer(); // ColourBasis* one, ColourBasis* two, Basis* mix):mOne(one),mTwo(two),mAmount(mix){}
	
	virtual ~CMixer(){}; // {delete mOne; delete mTwo; delete mAmount;}
	virtual Colour operator()(double x, double y, double z); // returns a single double in the range [0,1]

	virtual void setBasis(Basis* b, int index)
		{if (index==0) mBasis = b;}
	virtual void setCBasis(ColourBasis* b, int index)
		{if (index==0) mOne = b; else if (index==1) mTwo = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual int numCBasis(){return 2;} // the number of colour bases required
	virtual bool isReady(){return mBasis!=0 && mOne!=0 && mTwo!=0;} // does this operator have all its required bases?

	private:

	Basis* mBasis;
	ColourBasis *mOne, *mTwo;
};


class CDarken: public ColourBasis
{
	public:
	
	CDarken(); // ColourBasis* background, Basis* mask, double amount = 1):mOne(background),mTwo(mask),mAmount(amount){}
	
	virtual ~CDarken(){}
	virtual Colour operator()(double x, double y, double z); 

	virtual void setBasis(Basis* b, int index)
		{if (index==0) mBasis = b;}
	virtual void setCBasis(ColourBasis* b, int index)
		{if (index==0) mOne = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual int numCBasis(){return 1;} // the number of colour bases required
	virtual bool isReady(){return mBasis!=0 && mOne!=0;} // does this operator have all its required bases?

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	protected:

	static ParamList mParameters;

	ColourBasis *mOne;
	Basis *mBasis;
	double mAmount;

};

#endif
