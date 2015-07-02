/* Various basis functions for procedural texture synthesis
 * BP 18.08.05
 *
 * Basis Operators: Some of these basis functions take other basis functions as
 *   parameters. In this way they can be thought of as operators/filters/etc.
 *   **No deletion** of bases is done by any basis operator.
 *  Furthermore, the operators have a "ready" flag that is set to true when
 *    it has all its required bases.
 *
 * Interface: Each basis has a set of tweakable parameters.
 *  Each parameter has a type (int or double), a default, a range, and a name.
 *    
 *
 */

#ifndef BASIS_H
#define BASIS_H
   
#include "random.h"
#include "hmath.h"

#include <string>
#include <vector>
 
// Parameter: An abstraction of a basis parameter
 
struct Parameter
{
	enum Type {Int,Float};
	
	std::string name;
	Type type;
	union
	{
		int ilow;
		double flow;
	};	

	union
	{
		int ihigh;
		double fhigh;
	};

	union
	{
		int idef;
		double fdef;
	};

	Parameter(std::string n, int low, int high, int def):name(n),type(Int),ilow(low),ihigh(high),idef(def){}
	Parameter(std::string n, double low, double high, double def):name(n),type(Float),flow(low),fhigh(high),fdef(def){}
};

typedef std::vector<Parameter> ParamList;

class Parameterised
{
	public:
	
	virtual const std::vector<Parameter>& getParameters() = 0;
	virtual const Parameter* getParameter(std::string name) = 0;
	
	virtual void setParameter(std::string name, double f) = 0;
	virtual void setParameter(std::string name, int i) = 0;

	virtual double getParameterf(std::string name) = 0;
	virtual int getParameteri(std::string name) = 0;
};

// Basis: The interface for all basis functions.
class Basis: public Parameterised
{
	/* General parameters:
    double scale[3]; // scale in all directions
		double brightness; // a brightness parameter
	*/
	protected:	
	
	// common vector
	static std::vector<Parameter> mParameters;
	static bool mInit;

	// params
	double pContrast;
	
	double pXScale;
	double pYScale;
	double pZScale;
	
	virtual double applyParams(double f){return Math::clamp(0.01,0.99,pContrast*(f-0.5)+0.5);}
		
	public:

	Basis();	
	virtual ~Basis(){}
	virtual double operator()(double x, double y, double z) = 0; // returns a single double in the range [0,1]

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	virtual const Parameter* getParameter(std::string name);
	
	virtual void setParameter(std::string name, double f);
	virtual void setParameter(std::string name, int i);

	virtual double getParameterf(std::string name);
	virtual int getParameteri(std::string name);

	/* scaling parameters */
	double xs(){return pXScale;}
	double ys(){return pYScale;}
	double zs(){return pZScale;}
	
	/* interface for filters and operators */

	virtual void setBasis(Basis* b, int index){};
	virtual int numBasis(){return 0;} // the number of bases required
	virtual bool isReady(){return true;} // does this operator have all its required bases?
};

// Checker: A 3d checkerboard pattern (with blocks of size 1.0x1.0x1.0)
class Checker: public Basis
{	
	public:

	virtual double operator()(double x, double y, double z);
};

// Sphere: A sphere in a 1x1x1 cube (with 0 at the edge and 1 at the center)
class Sphere: public Basis
{	
	public:

	virtual double operator()(double x, double y, double z);
};


// Noise: A 3d seedable Perlin noise function
class Noise: public Basis
{
	public:

	Noise(int repeat = SZ);
	Noise(unsigned long seed, int repeat = SZ);
	virtual ~Noise();

	virtual double operator()(double x, double y, double z);
	double snoise(double x, double y, double z); // a signed version (-1,-1)
	
	private: // argh don't look at this mess ...
		Random mRNG;

		static const int SZ;
		static const int SZM;

		const int REPEAT; // repeat size <= SZ
		const int REPEATM; // repeat size mask

		typedef unsigned char byte;
		typedef int vec[3];
		struct fvec {double x,y,z;};

		void _init();	
		int fold(int i, int j, int k);
		int fold(vec& v);
		double grad(int i, int j, int k, fvec& r);
		double smoothstep (double t);
		double dot(fvec& a, fvec& b);

		byte* P;	
};

// Turbulence: Perlin turbulence
class Turbulence: public Basis
{
	protected:

	static std::vector<Parameter> mParameters;
	
	public:

	Turbulence(int numOctaves = OCTAVES);
	Turbulence(unsigned long seed, int numOctaves = OCTAVES);

	virtual double operator()(double x, double y, double z);
	virtual const std::vector<Parameter>& getParameters(){return mParameters;}

	virtual void setParameter(std::string name, double f);
	virtual void setParameter(std::string name, int i);

	virtual double getParameterf(std::string name);
	virtual int getParameteri(std::string name);

	private:

	Noise mNoise;
	int mOctaves;
	double mLacunarity;
	
	static const int OCTAVES;
};

// FBM: Fractal Brownian Motion (i.e. Fractal Sum) 
//      Takes a pointer to a Basis Object to act upon 
class FBM: public Basis
{
	public:

	FBM(); // Basis* basis, int numOctaves = OCTAVES);
	virtual ~FBM();

	virtual double operator()(double x, double y, double z);
	
	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	
	virtual void setParameter(std::string name, double f);
	virtual void setParameter(std::string name, int i);

	virtual double getParameterf(std::string name);
	virtual int getParameteri(std::string name);

	/* interface for filters and operators */

	virtual void setBasis(Basis* b, int index){if (index==0) mBasis = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual bool isReady(){return (mBasis!=0);} // does this operator have all its required bases?

	private:
	
	Basis* mBasis;
	double mLacunarity;
	int mOctaves;

	static const int OCTAVES;	

	static std::vector<Parameter> mParameters;
};

// Voronoi: A Voronoi basis (based on Worley) of degree 1
/*
	Notes: choose a power of 2 for repeat
					lambda dictates how many bubbles per 1x1x1 cube their are
*/

class Voronoi: public Basis
{
	public:
	
	Voronoi(unsigned int repeat = 256, double lambda = 1.5);
	Voronoi(unsigned long seed, unsigned int repeat = 256, double lambda = 1.5);
	
	virtual double operator()(double x, double y, double z); // n = 1
	virtual double operator()(double x, double y, double z, int n); // specify n
	virtual unsigned long getCell(double x, double y, double z); // returns the hash of the closest cell in F1
	virtual double n2n1(double x, double y, double z); // returns n2 - n1

	protected:

	unsigned long hash(long i, long j, long k);
	double closestInCube(int i, int j, int k, double x, double y, double z);
	double closestInCube(int i, int j, int k, double x, double y, double z, unsigned long &ref);
	void closestNthInCube(int i, int j, int k, double x, double y, double z, int n, double* closest);
	void _init();
	void insert(double el, double* arr, int sz); // insertion sort

	Random mRandom;
	double mLambda; // mean density of pts per cube
	
	unsigned int mRepeat;	
	unsigned int mRepeatM; //mask
	
	unsigned char mTable[256]; // pt table
};

// n2-n1 wrapper class
class Voronoi21: public Voronoi
{
	public:
	
	Voronoi21(unsigned int repeat = 256, double lambda = 1.5):Voronoi(repeat,lambda){}
	Voronoi21(unsigned long seed, unsigned int repeat = 256, double lambda = 1.5):Voronoi(seed,repeat,lambda){}

	virtual double operator()(double x, double y, double z)
	{
		x*=xs();
		y*=ys();
		z*=zs();

					return applyParams(n2n1(x,y,z));
	} // n = 1
};


// Basis Filters

// Perturber: Perturbs the texture coord lookup by the specified amounts
// Params: "x scale", "y scale", "z scale"
// Basis: 0: Lookup 1: Perturb
class BPerturber: public Basis
{
	public:

	BPerturber(); // :mBasis(b),mVar(var){}
	virtual ~BPerturber(){}
	virtual double operator()(double x, double y, double z);

	virtual void setBasis(Basis* b, int index){if (index==0) mBasis = b; else if (index==1) mPerturb = b;}
	virtual int numBasis(){return 2;} // the number of bases required
	virtual bool isReady(){return (mBasis!=0)&&(mPerturb!=0);} // does this operator have all its required bases?

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	protected:

	Basis *mBasis, *mPerturb;
	double mX, mY, mZ;

	static std::vector<Parameter> mParameters;
};



class BStepper: public Basis
{
	public:

	BStepper(); // :mBasis(b),mVar(var){}
	virtual ~BStepper(){}
	virtual double operator()(double x, double y, double z)
	{
		x*=xs();
		y*=ys();
		z*=zs();
		return applyParams(Math::step(mVar,(*mBasis)(x,y,z)));
	}

	virtual void setBasis(Basis* b, int index){if (index==0) mBasis = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual bool isReady(){return (mBasis!=0);} // does this operator have all its required bases?

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	protected:

	Basis* mBasis;
	double mVar;

	static std::vector<Parameter> mParameters;
};

class BGainer: public Basis
{
	public:

	BGainer(); // Basis* b, double var):mBasis(b),mVar(var){}
	virtual ~BGainer(){}
	virtual double operator()(double x, double y, double z)
	{
		x*=xs();
		y*=ys();
		z*=zs();
		return applyParams(Math::gain(mVar,(*mBasis)(x,y,z)));
	}	

	virtual void setBasis(Basis* b, int index){if (index==0) mBasis = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual bool isReady(){return (mBasis!=0);} // does this operator have all its required bases?

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	private:

	Basis* mBasis;
	double mVar;
	
	static std::vector<Parameter> mParameters;
};

// Scales the basis function
class BScaler: public Basis
{
	public:

	BScaler(); // Basis* b, double xs, double ys, double zs=1):mBasis(b),mX(xs),mY(ys),mZ(zs){};
	virtual ~BScaler(){}
	virtual double operator()(double x, double y, double z)
	{	x*=xs();
		y*=ys();
		z*=zs();
		return applyParams((*mBasis)(mX*x,mY*y,mZ*z));
	}

	virtual void setBasis(Basis* b, int index){if (index==0) mBasis = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual bool isReady(){return (mBasis!=0);} // does this operator have all its required bases?

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	private:

	Basis* mBasis;
	double mX,mY,mZ;

	static std::vector<Parameter> mParameters;
};

// BLerper: lerps between two basis functions
class BLerper: public Basis
{
	protected:

	static ParamList mParameters;
	
	public:
	
	BLerper(); // Basis* one, Basis* two, double amount=0.5);
	
	virtual ~BLerper(){}
	virtual double operator()(double x, double y, double z); // returns a single double in the range [0,1]

	virtual const std::vector<Parameter>& getParameters(){return mParameters;}
	
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	/* interface for filters and operators */

	virtual void setBasis(Basis* b, int index){if (index==0) mOne = b; else if (index==1) mTwo = b;}
	virtual int numBasis(){return 2;} // the number of bases required
	virtual bool isReady(){return (mOne!=0)&&(mTwo!=0);} // does this operator have all its required bases?

	private:

	Basis *mOne, *mTwo;
	double mAmount;
};

//BInverter: inverts a basis
class BInverter: public Basis
{
	public:

	BInverter():mOne(0){}
	virtual ~BInverter(){}
	virtual double operator()(double x, double y, double z)
	{	
		x*=xs();
		y*=ys();
		z*=zs();
		return applyParams(1 - (*mOne)(x,y,z));
	}
	
	virtual void setBasis(Basis* b, int index){if (index==0) mOne = b;}
	virtual int numBasis(){return 1;} // the number of bases required
	virtual bool isReady(){return (mOne!=0);} // does this operator have all its required bases?

	private:

	Basis* mOne;
};

//Darkens one basis using another
class BDarken: public Basis
{
	public:
	
	BDarken(); //Basis* background, Basis* mask, double amount = 1):mOne(background),mTwo(mask),mAmount(amount){}
	
	virtual ~BDarken(){}
	virtual double operator()(double x, double y, double z); // returns a single double in the range [0,1]
	
	virtual void setParameter(std::string name, double f);
	virtual double getParameterf(std::string name);

	/* interface for filters and operators */

	virtual void setBasis(Basis* b, int index){if (index==0) mOne = b; else if (index==1) mTwo = b;}
	virtual int numBasis(){return 2;} // the number of bases required
	virtual bool isReady(){return (mOne!=0)&&(mTwo!=0);} // does this operator have all its required bases?

	private:

	Basis *mOne, *mTwo;
	double mAmount;

	static ParamList mParameters;
};

#endif
