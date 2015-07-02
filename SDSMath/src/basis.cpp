#include "basis.h"
#include "hmath.h"

#include <iostream>

using namespace Math;
using std::string;

// Basis

std::vector<Parameter> Basis::mParameters;
bool Basis::mInit = false;

Basis::Basis()
:pContrast(1),pXScale(1),pYScale(1),pZScale(1)
{
	if (!mInit)
	{
		mParameters.push_back(Parameter("contrast",0.0f,10.0,1.f));
		mParameters.push_back(Parameter("xscale",0.0f,10.0,1.f));
		mParameters.push_back(Parameter("yscale",0.0f,10.0,1.f));
		mParameters.push_back(Parameter("zscale",0.0f,10.0,1.f));
		mInit = true;
	}
}

const Parameter* 
Basis::getParameter(string name)
{
	std::vector<Parameter>::const_iterator it = getParameters().begin();
	while (it!=mParameters.end())
	{
		if (it->name == name) return &(*it);
		it++;
	}

	return NULL;
}

void 
Basis::
setParameter(string name, double f)
{
	if (name=="contrast") pContrast = f;
	else if (name=="xscale") pXScale = f;
	else if (name=="yscale") pYScale = f;
	else if (name=="zscale") pZScale = f;
}

void 
Basis::
setParameter(string name, int i)
{
	
}

double 
Basis::
getParameterf(string name)
{
	if (name=="contrast") return pContrast;
	else if (name=="xscale") return pXScale;
	else if (name=="yscale") return pYScale;
	else if (name=="zscale") return pZScale;

	return 0;
}

int 
Basis::
getParameteri(string name)
{
	
	return 0;
}


// Checker

double
Checker::
operator()(double x, double y, double z)
{
	// determine which block I'm in
	int ix = (int)(x*xs());
	int iy = (int)(y*ys());
	int iz = (int)(z*zs());

	return applyParams((double)((((ix+iy)&1) + iz)&1));
}

// Sphere

double
Sphere::
operator()(double x, double y, double z)
{
	// determine which block I'm in	
	int ix = (int)(x*xs()); double dx = x - ix - 0.5;
	int iy = (int)(y*ys()); double dy = y - iy - 0.5;
	int iz = (int)(z*zs()); double dz = z - iz - 0.5;

	double sq_dist = sqrt(dx*dx + dy*dy + dz*dz) * 2;
	return applyParams(Math::clamp(0,1,1 - sq_dist));
}

// Noise

const int Noise::SZ = 256;
const int Noise::SZM = Noise::SZ - 1;

Noise::Noise(int repeat):mRNG(),REPEAT(repeat),REPEATM(repeat-1){_init();}
Noise::Noise(unsigned long seed,int repeat):mRNG(seed),REPEAT(repeat),REPEATM(repeat-1){_init();}
Noise::~Noise(){delete[]P;}

void 
Noise::
_init()
{	
	P = new byte[SZ*2];
	// P[] = [0..255];
	int i;
	for(i=0;i<SZ;i++)
		P[i] = i;

	// permute P
	for(i=0;i<SZ;i++){
		byte j = (byte)(256*mRNG.getFloat());
		byte tmp = P[j];
		P[j] = P[i];
		P[i] = tmp;
	}

	for (i=0;i<SZ;i++)
	{
		P[i+SZ] = P[i];
	}
}

double Noise::snoise(double x, double y, double z)
{
	// compute which cell we are in
	int i = (int)floor(x); int ii = i&REPEATM; int ii1 = (i+1)&REPEATM;
	int j = (int)floor(y); int jj = j&REPEATM; int jj1 = (j+1)&REPEATM;
	int k = (int)floor(z); int kk = k&REPEATM; int kk1 = (k+1)&REPEATM;
	
	fvec rel = {x-i,y-j,z-k};
	
	double u = smoothstep(rel.x);
	double v = smoothstep(rel.y);
	double w = smoothstep(rel.z);

	double x1 = grad(ii,jj,kk,rel);
	rel.x -= 1;
	double x2 = grad(ii1,jj,kk,rel);
	double y1 = lerp(x1,x2,u);

	rel.x = x - i;
	rel.y = y - j - 1;
	x1 = grad(ii,jj1,kk,rel);
	rel.x -= 1;
	x2 = grad(ii1,jj1,kk,rel);
	double y2 = lerp(x1,x2,u);
	double z1 = lerp(y1,y2,v);

	rel.x = x - i;
	rel.y = y - j;
	rel.z = z - k - 1;
	x1 = grad(ii,jj,kk1,rel);
	rel.x -= 1;
	x2 = grad(ii1,jj,kk1,rel);
	y1 = lerp(x1,x2,u);

	rel.x = x - i;
	rel.y = y - j - 1;
	x1 = grad(ii,jj1,kk1,rel);
	rel.x -= 1;
	x2 = grad(ii1,jj1,kk1,rel);
	y2 = lerp(x1,x2,u);
	double z2 = lerp(y1,y2,v);
	return lerp(z1,z2,w);
}

double 
Noise::
operator()(double x, double y, double z)
{
	return applyParams((1+snoise(x*xs(),y*ys(),z*zs()))/2);
}

inline int Noise::fold(int i, int j, int k)
{
	return P[ P[ P[i]+j ]+k ];
}

inline int Noise::fold(vec& v)
{
	return P[ P[ P[v[0]]+v[1] ]+v[2] ];
}

inline double Noise::grad(int i, int j, int k, fvec& r)
{
	/* adapted from Perlin's improved noise java implementation
	http://mrl.nyu.edu/~perlin/noise/
	*/
	int h = fold(i,j,k) & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	double u = h<8 ? r.x : r.y;                 // INTO 12 GRADIENT DIRECTIONS.
	double v = h<4 ? r.y : h==12||h==14 ? r.x : r.z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

inline double Noise::smoothstep (double t)
{
	return t*t*t*(10-t*(15-6*t)); //a smoother C^2 interpolation function
}

inline double Noise::dot(fvec& a, fvec& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

// Turbulence

const int Turbulence::OCTAVES = 8;

std::vector<Parameter> Turbulence::mParameters;

Turbulence::Turbulence(int numOctaves)
:mNoise(),mOctaves(numOctaves),mLacunarity(2.0)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("octaves",1,16,OCTAVES));
		mParameters.push_back(Parameter("lacunarity",0.01,4.0,2.0f));
	}
}

Turbulence::Turbulence(unsigned long seed, int numOctaves)
:mNoise(seed),mOctaves(numOctaves),mLacunarity(2.0)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("octaves",1,16,OCTAVES));
		mParameters.push_back(Parameter("lacunarity",0.01,4.0,2.0f));
	}
}


double
Turbulence::
operator()(double x, double y, double z)
{
	x*=xs();
	y*=ys();
	z*=zs();
	
	// 1/f fractal (using impr. noise)
	double val = 0;
	
	int i;
	double f = 1;
	for(i=0;i<mOctaves;i++)
	{		
		val += fabs(mNoise.snoise(x,y,z))*f;
		x *= mLacunarity;		y *= mLacunarity;		z *= mLacunarity;
		f /= mLacunarity;
	}
	return applyParams(val);	
}

void Turbulence::setParameter(string name, double f)
{
	if (name=="lacunarity") mLacunarity = f;
	else	Basis::setParameter(name,f);
}

void Turbulence::setParameter(string name, int i)
{
	if (name=="octaves") mOctaves = i;
	else
		Basis::setParameter(name,i);
}

double Turbulence::getParameterf(string name)
{
	if (name=="lacunarity") return mLacunarity;
	else return Basis::getParameterf(name);
}

int Turbulence::getParameteri(string name)
{
	if (name=="octaves") return mOctaves;
	else 
		return Basis::getParameteri(name);
}

// FBM

const int FBM::OCTAVES = 8;
std::vector<Parameter> FBM::mParameters;

FBM::FBM()
:mBasis(0),mLacunarity(2),mOctaves(OCTAVES)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("octaves",1,16,OCTAVES));
		mParameters.push_back(Parameter("lacunarity",0.01,4.0,2.0f));
	}
}


FBM::~FBM()
{
}

double 
FBM::
operator()(double x, double y, double z)
{x*=xs();
	y*=ys();
	z*=zs();

	double val = 0;
	
	int i;
	double f = 1;
	for(i=0;i<mOctaves;i++)
	{		
		val += (*mBasis)(x,y,z)*f;
		x *= mLacunarity;		y *= mLacunarity;		z *= mLacunarity;
		f /= mLacunarity;
	}

	return applyParams(val/2);	
}

void FBM::setParameter(string name, double f)
{
	if (name=="lacunarity") mLacunarity = f;
	else	Basis::setParameter(name,f);
}

void FBM::setParameter(string name, int i)
{
	if (name=="octaves") mOctaves = i;
	else
		Basis::setParameter(name,i);
}

double FBM::getParameterf(string name)
{
	if (name=="lacunarity") return mLacunarity;
	else return Basis::getParameterf(name);
}

int FBM::getParameteri(string name)
{
	if (name=="octaves") return mOctaves;
	else 
		return Basis::getParameteri(name);
}


// Voronoi

Voronoi::Voronoi(unsigned int repeat, double lambda)
:mRandom(),mLambda(lambda),mRepeat(repeat),mRepeatM(repeat-1)
{
	_init();
}

Voronoi::Voronoi(unsigned long seed, unsigned int repeat, double lambda)
:mRandom(seed),mLambda(lambda),mRepeat(repeat),mRepeatM(repeat-1)
{
	_init();
}

void Voronoi::_init()
{
	// setup a table of the cdf up to m = 5
	// P(lambda,0) = e^-lambda
	// P(lambda,m) = lambda/m * P(lambda,m-1)
	static double P[5];
	P[0] = exp(-mLambda);
	int i;
	for(i=1;i<5;i++)
	{
		P[i] = (1 + (mLambda/i)) * P[i-1];
	}

	// initialise num pts table
	for(i=0;i<256;i++)
	{
		double p = mRandom.getFloat(); // a num in [0,1]
		// find out where p exists in the cdf 

		int j;
		for(j=0;j<5;j++)
		{
			if (p < P[j]) break;
		}
			
		// j pts

		mTable[i] = j;		
	}
}

double 
Voronoi::
operator()(double x, double y, double z)
{x*=xs();
	y*=ys();
	z*=zs();

	// Based on Worley's implementation in T&M.
	
	// i = floor x, j = floor y, k = floor z
	int i = (int)floor(x); double fx = x-i;
	int j = (int)floor(y); double fy = y-j;
	int k = (int)floor(z); double fz = z-k;

	double closest = 999999;
	double tmp = closestInCube(i,j,k,x,y,z);
	if (tmp<closest) closest = tmp;

	// sm = smallest n'th dist
	// figure out if any neighbouring cubes are closer than sm (by comparing squared distances)
	double xx = fx*fx; double xx1 = 1 + xx - 2*fx;
	double yy = fy*fy; double yy1 = 1 + yy - 2*fy;
	double zz = fz*fz; double zz1 = 1 + zz - 2*fz;
	
	int ii,jj,kk;
	for(ii=-1;ii<=1;ii++)
		for(jj=-1;jj<=1;jj++)
			for(kk=-1;kk<=1;kk++)
			{
				if (ii==0 && jj==0 && kk==0) continue;
				
				double dist = closest;
				bool t = false;

				if (ii<0 && xx<dist) t = true;
				else if (ii>0 && xx1<dist) t = true;				
				
				if (jj<0 && yy<dist) t = true; 
				else if (jj>0 && yy1<dist) t = true;

				if (kk<0 && zz<dist) t = true;
				else if (kk>0 && zz1<dist) t = true;				

				if (t) // then we have a possible cuboid
				{
					// else the cube is closer than the closest point so we have to check its points
					double tmp = closestInCube(ii+i,jj+j,kk+k,x,y,z);	

					if (tmp<closest) closest = tmp;		
				}
			}

	return applyParams(sqrt(closest));
}

double
Voronoi::
operator()(double x, double y, double z, int n) // specify n
{x*=xs();
	y*=ys();
	z*=zs();

	// this returns the distance to the n'th closest point (for n=1 use the other version)
	// Based on Worley's implementation in T&M.
	
	// i = floor x, j = floor y, k = floor z
	int i = (int)floor(x); double fx = x-i;
	int j = (int)floor(y); double fy = y-j;
	int k = (int)floor(z); double fz = z-k;

	double* closest = new double[n];
	int it;
	for(it=0;it<n;it++) closest[it] = 999999; //some large number
	
	closestNthInCube(i,j,k,x,y,z,n,closest);

	// sm = smallest n'th dist
	// figure out if any neighbouring cubes are closer than sm (by comparing squared distances)
	double xx = fx*fx; double xx1 = 1 + xx - 2*fx;
	double yy = fy*fy; double yy1 = 1 + yy - 2*fy;
	double zz = fz*fz; double zz1 = 1 + zz - 2*fz;
	
	int ii,jj,kk;
	for(ii=-1;ii<=1;ii++)
		for(jj=-1;jj<=1;jj++)
			for(kk=-1;kk<=1;kk++)
			{
				if (ii==0 && jj==0 && kk==0) continue;
				
				double dist = closest[n-1];
				bool t = false;

				if (ii<0 && xx<dist) t = true;
				else if (ii>0 && xx1<dist) t = true;				
				
				if (jj<0 && yy<dist) t = true; 
				else if (jj>0 && yy1<dist) t = true;

				if (kk<0 && zz<dist) t = true;
				else if (kk>0 && zz1<dist) t = true;				

				if (t) // then we have a possible cuboid
				{
					// else the cube is closer than the closest point so we have to check its points
					closestNthInCube(ii+i,jj+j,kk+k,x,y,z,n,closest);	
				}
			}

	double nth = closest[n-1];
	delete[]closest;
	return sqrt(nth);

}

double
Voronoi::
n2n1(double x, double y, double z) // specify n
{
	// this returns n2 - n1
	// Based on Worley's implementation in T&M.
	
	// i = floor x, j = floor y, k = floor z
	int i = (int)floor(x); double fx = x-i;
	int j = (int)floor(y); double fy = y-j;
	int k = (int)floor(z); double fz = z-k;

	double* closest = new double[2];
	int it;
	for(it=0;it<2;it++) closest[it] = 999999; //some large number
	
	closestNthInCube(i,j,k,x,y,z,2,closest);

	// sm = smallest n'th dist
	// figure out if any neighbouring cubes are closer than sm (by comparing squared distances)
	double xx = fx*fx; double xx1 = 1 + xx - 2*fx;
	double yy = fy*fy; double yy1 = 1 + yy - 2*fy;
	double zz = fz*fz; double zz1 = 1 + zz - 2*fz;
	
	int ii,jj,kk;
	for(ii=-1;ii<=1;ii++)
		for(jj=-1;jj<=1;jj++)
			for(kk=-1;kk<=1;kk++)
			{
				if (ii==0 && jj==0 && kk==0) continue;
				
				double dist = closest[1];
				bool t = false;

				if (ii<0 && xx<dist) t = true;
				else if (ii>0 && xx1<dist) t = true;				
				
				if (jj<0 && yy<dist) t = true; 
				else if (jj>0 && yy1<dist) t = true;

				if (kk<0 && zz<dist) t = true;
				else if (kk>0 && zz1<dist) t = true;				

				if (t) // then we have a possible cuboid
				{
					// else the cube is closer than the closest point so we have to check its points
					closestNthInCube(ii+i,jj+j,kk+k,x,y,z,2,closest);	
				}
			}

	double sum = sqrt(closest[1]) - sqrt(closest[0]);
	delete[]closest;
	return sum;

}

unsigned long 
Voronoi::
getCell(double x, double y, double z)
{
	// Based on Worley's implementation in T&M.
	
	// i = floor x, j = floor y, k = floor z
	int i = (int)floor(x); double fx = x-i;
	int j = (int)floor(y); double fy = y-j;
	int k = (int)floor(z); double fz = z-k;

	double closest = 999999;
	unsigned long ref = 0;
	double tmp = closestInCube(i,j,k,x,y,z,ref);
	if (tmp<closest) closest = tmp;

	// sm = smallest n'th dist
	// figure out if any neighbouring cubes are closer than sm (by comparing squared distances)
	double xx = fx*fx; double xx1 = 1 + xx - 2*fx;
	double yy = fy*fy; double yy1 = 1 + yy - 2*fy;
	double zz = fz*fz; double zz1 = 1 + zz - 2*fz;
	
	int ii,jj,kk;
	for(ii=-1;ii<=1;ii++)
		for(jj=-1;jj<=1;jj++)
			for(kk=-1;kk<=1;kk++)
			{
				if (ii==0 && jj==0 && kk==0) continue;
				
				double dist = closest;
				bool t = false;

				if (ii<0 && xx<dist) t = true;
				else if (ii>0 && xx1<dist) t = true;				
				
				if (jj<0 && yy<dist) t = true; 
				else if (jj>0 && yy1<dist) t = true;

				if (kk<0 && zz<dist) t = true;
				else if (kk>0 && zz1<dist) t = true;				

				if (t) // then we have a possible cuboid
				{
					// else the cube is closer than the closest point so we have to check its points
					unsigned long tmpRef;
					double tmp = closestInCube(ii+i,jj+j,kk+k,x,y,z,tmpRef);	

					if (tmp<closest)
					{
						closest = tmp;		
						ref = tmpRef;
					}
				}
			}

	return ref;
}

inline
void 
Voronoi::
insert(double el, double* arr, int sz) // insertion sort
{
	int i = sz - 1;	
	if (el>=arr[i]) return;
		
	for(;el<arr[i-1] && i>0;i--)
	{
		arr[i] = arr[i-1];
	}
	
	arr[i] = el;
}
																																														    
inline
void 
Voronoi::
closestNthInCube(int i, int j, int k, double x, double y, double z, int n, double* closest)
{
	// seed = hash i,j,k
	unsigned long seed = hash(i&mRepeatM,j&mRepeatM,k&mRepeatM);	

	// use seed to hash into table of 256 precomputed values for the number of pts in the cell
	// use upper 8 bits
	int numPts = mTable[seed >> 24];

	// use seed to seed rng and get the x,y,z coords of each pt (relative to i,j,k)
	// as we generate each pt we keep a sorted list of the nearest n points (i.e. insertion sort)
	// for voronoi (n=1) just store the closest distance^2
	//seed = 1402024253*seed + 586950981;

	mRandom.seed(seed);
	int ii;
	for(ii=0;ii<numPts;ii++)
	{	
		double x0,y0,z0;	

		/*
		x0 = (i + (seed+0.5)*(1.0/42944967296.0)) - x; // was mRandom.getFloat()
		seed = 1402024253*seed + 586950981;
		y0 = (j + (seed+0.5)*(1.0/42944967296.0)) - y;
		seed = 1402024253*seed + 586950981;
		z0 = (k + (seed+0.5)*(1.0/42944967296.0)) - z;
		seed = 1402024253*seed + 586950981;
		*/
	
		x0 = (i + mRandom.getFloat()) - x;
		y0 = (j + mRandom.getFloat()) - y;
		z0 = (k + mRandom.getFloat()) - z;

		double tmp = x0*x0 + y0*y0 + z0*z0;
		insert(tmp,closest,n);
	}
}

inline
double 
Voronoi::
closestInCube(int i, int j, int k, double x, double y, double z)
{
	// seed = hash i,j,k
	unsigned long seed = hash(i&mRepeatM,j&mRepeatM,k&mRepeatM);	

	// use seed to hash into table of 256 precomputed values for the number of pts in the cell
	// use upper 8 bits
	int numPts = mTable[seed >> 24];

	// use seed to seed rng and get the x,y,z coords of each pt (relative to i,j,k)
	// as we generate each pt we keep a sorted list of the nearest n points (i.e. insertion sort)
	// for voronoi (n=1) just store the closest distance^2
	//seed = 1402024253*seed + 586950981;
	mRandom.seed(seed); //<- wow this is slow!!! use a simple lcm method instead 

	double closest = 999999;	
	
	int ii;
	for(ii=0;ii<numPts;ii++)
	{	
		double x0,y0,z0;

		/*
		x0 = (i + seed*(1.0/42944967296.0)) - x; // was mRandom.getFloat()
		seed = 1402024253*seed + 586950981;
		y0 = (j + seed*(1.0/42944967296.0)) - y;
		seed = 1402024253*seed + 586950981;
		z0 = (k + seed*(1.0/42944967296.0)) - z;
		seed = 1402024253*seed + 586950981;
		seed = 1402024253*seed + 586950981;
		*/

		x0 = (i + mRandom.getFloat()) - x;
		y0 = (j + mRandom.getFloat()) - y;
		z0 = (k + mRandom.getFloat()) - z;

		double tmp = x0*x0 + y0*y0 + z0*z0;
		if (tmp < closest) closest = tmp;
	}

	return closest;
}

inline
double 
Voronoi::
closestInCube(int i, int j, int k, double x, double y, double z, unsigned long& ref)
{
	// seed = hash i,j,k
	unsigned long seed = hash(i&mRepeatM,j&mRepeatM,k&mRepeatM);		

	// use seed to hash into table of 256 precomputed values for the number of pts in the cell
	// use upper 8 bits
	int numPts = mTable[seed >> 24];

	// use seed to seed rng and get the x,y,z coords of each pt (relative to i,j,k)
	// as we generate each pt we keep a sorted list of the nearest n points (i.e. insertion sort)
	// for voronoi (n=1) just store the closest distance^2
	//seed = 1402024253*seed + 586950981;
	mRandom.seed(seed);

	double closest = 999999;	
	
	int ii;
	for(ii=0;ii<numPts;ii++)
	{	
		unsigned long currentRef = mRandom.seed();
	
		double x0,y0,z0;
		
		/*
		x0 = (i + (seed+0.5)*(1.0/42944967296.0)) - x; // was mRandom.getFloat()
		seed = 1402024253*seed + 586950981;
		y0 = (j + (seed+0.5)*(1.0/42944967296.0)) - y;
		seed = 1402024253*seed + 586950981;
		z0 = (k + (seed+0.5)*(1.0/42944967296.0)) - z;
		seed = 1402024253*seed + 586950981;
		*/
		
		x0 = (i + mRandom.getFloat()) - x;
		y0 = (j + mRandom.getFloat()) - y;
		z0 = (k + mRandom.getFloat()) - z;

		currentRef += mRandom.seed();

		double tmp = x0*x0 + y0*y0 + z0*z0;
		if (tmp < closest)
		{
			closest = tmp;
			ref = currentRef;
		}
	}

	return closest;
}

inline 
unsigned long 
Voronoi::
hash(long i, long j, long k)
{
	// from Worley T&M pg.143
	return (702395077*i + 915488749*j + 2120969693*k);
}
// BPerturber 

std::vector<Parameter> BPerturber::mParameters;

BPerturber::BPerturber() // :mBasis(b),mVar(var){}
:mBasis(0),mPerturb(0),mX(0.1),mY(0.1),mZ(0.1)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("xperturb",-10.0f,10.0f,0.1f));
		mParameters.push_back(Parameter("yperturb",-10.0f,10.0f,0.1f));
		mParameters.push_back(Parameter("zperturb",-10.0f,10.0f,0.1f));
	}
}

void 
BPerturber::
setParameter(string name, double f)
{
	if (name=="xperturb") mX = f;
	else if (name=="yperturb") mY = f;
	else if (name=="zperturb") mZ = f;
	else Basis::setParameter(name,f);
}

double 
BPerturber::
getParameterf(string name)
{
	if (name=="xperturb") return mX;
	else if (name=="yperturb") return mY;
	else if (name=="zperturb") return mZ;
	else return Basis::getParameterf(name);
}

double
BPerturber::
operator()(double x, double y, double z)
{x*=xs();
	y*=ys();
	z*=zs();

	double am = (*mPerturb)(x,y,z);
	return applyParams((*mBasis)(x+am*mX,y+am*mY,z+am*mZ));
}

// BStepper 

std::vector<Parameter> BStepper::mParameters;

BStepper::BStepper() // :mBasis(b),mVar(var){}
:mBasis(0),mVar(0.5)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("step",0.0f,1.0f,0.5f));
	}
}

void 
BStepper::
setParameter(string name, double f)
{
	if (name=="step") mVar = f;
	else Basis::setParameter(name,f);
}

double 
BStepper::
getParameterf(string name)
{
	if (name=="step") return mVar;
	else return Basis::getParameterf(name);
}

// BGainer

std::vector<Parameter> BGainer::mParameters;

BGainer::BGainer() // :mBasis(b),mVar(var){}
:mBasis(0),mVar(0.5)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("gain",0.0f,1.0f,0.5f));
	}
}

void 
BGainer::
setParameter(string name, double f)
{
	if (name=="gain") mVar = f;
	else Basis::setParameter(name,f);
}

double 
BGainer::
getParameterf(string name)
{
	if (name=="gain") return mVar;
	else return Basis::getParameterf(name);
}


// BLerper

ParamList BLerper::mParameters;

BLerper::BLerper() //Basis* one, Basis* two, double amount)
:mOne(0),mTwo(0),mAmount(0.5)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("amount",0.0,1.0,0.5f));
	}
}

double 
BLerper::
operator()(double x, double y, double z) // returns a single double in the range [0,1]
{x*=xs();
	y*=ys();
	z*=zs();

	return applyParams(Math::lerp((*mOne)(x,y,z),(*mTwo)(x,y,z),mAmount));
}
	
void BLerper::setParameter(string name, double f)
{
	if (name=="amount") mAmount = f;
	else
			Basis::setParameter(name,f);
}

double BLerper::getParameterf(string name)
{
	if (name=="amount") return mAmount;
	else 
			return Basis::getParameterf(name);
}

// BScaler

std::vector<Parameter> BScaler::mParameters;

BScaler::BScaler() // :mBasis(b),mVar(var){}
:mBasis(0),mX(1),mY(1),mZ(1)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("x",-5.0f,5.0f,1.f));
		mParameters.push_back(Parameter("y",-5.0f,5.0f,1.f));
		mParameters.push_back(Parameter("z",-5.0f,5.0f,1.f));
	}
}

void 
BScaler::
setParameter(string name, double f)
{
	if (name=="x") mX = f;
	else if (name=="y") mY = f;
	else if (name=="z") mZ = f;
	else Basis::setParameter(name,f);
}

double 
BScaler::
getParameterf(string name)
{
	if (name=="x") return mX;
	else if (name=="y") return mY;
	else if (name=="z") return mZ;
	else return Basis::getParameterf(name);
}



double 
BDarken::
operator()(double x, double y, double z) // returns a single double in the range [0,1]
{x*=xs();
	y*=ys();
	z*=zs();

	return applyParams((*mOne)(x,y,z) * Math::lerp(1,(*mTwo)(x,y,z),mAmount));
}

// BDarken

ParamList BDarken::mParameters;

BDarken::BDarken() //Basis* one, Basis* two, double amount)
:mOne(0),mTwo(0),mAmount(0.5)
{
	if (mParameters.size()==0)
	{
		mParameters = Basis::mParameters;
		mParameters.push_back(Parameter("amount",0.0,1.0,0.5f));
	}
}

void BDarken::setParameter(string name, double f)
{
	if (name=="amount") mAmount = f;
	else
			Basis::setParameter(name,f);
}

double BDarken::getParameterf(string name)
{
	if (name=="amount") return mAmount;
	else 
			return Basis::getParameterf(name);
}


