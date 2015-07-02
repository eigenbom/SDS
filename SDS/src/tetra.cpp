#include "tetra.h"
#include "matrix3.h"
#include "face.h"
#include "physics.h"

#include <cmath>
#include <cfloat>

Tetra::Tetra()
:mRestMultiplier(1)
{
	mSpringCoefficient = Physics::kV;
	setNoNeighbours();
}

Tetra::Tetra(Vertex* a, Vertex* b, Vertex* c, Vertex* d)
	:mIntersected(false)
	,mOuter(false)
	,mRestMultiplier(1)
{
	mV[0] = a;
	mV[1] = b;
	mV[2] = c;
	mV[3] = d;
	setNoNeighbours();

	mRestVolume = volume(); //1/6. * dot(b->x()-a->x(),cross(c->x()-a->x(),d->x()-a->x()));
	mSpringCoefficient = Physics::kV;
}

Tetra::Tetra(Vertex* a, Vertex* b, Vertex* c, Vertex* d, double restvol)
:mIntersected(false)
,mOuter(false)
,mRestVolume(restvol)
,mRestMultiplier(1)
{
	mV[0] = a;
	mV[1] = b;
	mV[2] = c;
	mV[3] = d;
	setNoNeighbours();
	mSpringCoefficient = Physics::kV;
}

int Tetra::getNeighbourIndex(Tetra* t) const
{
	for(int i=0;i<4;i++)
		if (mN[i]==t) return i;
	return -1;
}

Vector3d Tetra::centerOfMass() const
{
	return
	(mV[0]->m()*mV[0]->x() + mV[1]->m()*mV[1]->x() + mV[2]->m()*mV[2]->x() + mV[3]->m()*mV[3]->x()) /
	(mV[0]->m() + mV[1]->m() + mV[2]->m() + mV[3]->m());
}


// XXX: Brute force
const AABB& Tetra::updateAABB()
{
	double min[3] = {DBL_MAX,DBL_MAX,DBL_MAX}, max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX};

	for(int i=0;i<4;++i)
	{
		Vector3d p = mV[i]->x();
		double x = p.x(), y = p.y(), z = p.z();

		if (x < min[0]) min[0] = x;
		if (x > max[0]) max[0] = x;

		if (y < min[1]) min[1] = y;
		if (y > max[1]) max[1] = y;

		if (z < min[2]) min[2] = z;
		if (z > max[2]) max[2] = z;
	}

	mAABB[0] = min[0];
	mAABB[1] = min[1];
	mAABB[2] = min[2];
	mAABB[3] = max[0];
	mAABB[4] = max[1];
	mAABB[5] = max[2];
	mAABB.valid(true);

	return mAABB;
}

bool Tetra::isVertex(const Vertex* v) const // returns true if v==mV[i] for some i
{
	for(int i=0;i<4;i++)
		if (v==mV[i]) return true;
	return false;
}

bool Tetra::contains(const Vertex* v) const { return isVertex(v); }
bool Tetra::contains(const Vertex* v1, const Vertex* v2) const {return isVertex(v1) and isVertex(v2);} // same as isVertex()
bool Tetra::contains(const Vertex* v1, const Vertex* v2, const Vertex* v3) const {return isVertex(v1) and isVertex(v2) and isVertex(v3);} // same as isVertex()
bool Tetra::contains(const Vertex* v1, const Vertex* v2, const Vertex* v3, const Vertex* v4) const {return isVertex(v1) and isVertex(v2) and isVertex(v3) and isVertex(v4);} // same as isVertex()

// bary(): returns the barycentric coordinates of p w.r.t, this tetra
Vector3d Tetra::bary(const Vector3d& p) const
{
	// let {x0,x1,x2,x3} be the coordinates of this tetra
	// let A = [x1 - x0, x2 - x0, x3 - x0] define a new coordinate system,
	// where p = x0 + A*b, and b is the barycentric coordinates

	// i.e., b = inv(A) * (p-x0)

	const Vector3d &x0 = mV[0]->x(), &x1 = mV[1]->x(), &x2 = mV[2]->x(), &x3 = mV[3]->x();
	Matrix3d	A(x1-x0,x2-x0,x3-x0);
	return A.inv()*(p-x0);
}

Vector3d Tetra::getFaceCentroid(int i) const
{
	int j,k,l;
	this->oppositeFaceIndices(i,j,k,l);
	Vector3d vj = mV[j]->x(), vk = mV[k]->x(), vl = mV[l]->x();
	return (vj+vk+vl)/3;
}

Face* Tetra::createFace(int i)
{
	switch (i)
	{
		case 3: return new Face(mV[0],mV[2],mV[1]); break;
		case 2: return new Face(mV[0],mV[1],mV[3]); break;
		case 0: return new Face(mV[1],mV[2],mV[3]); break;
		case 1: return new Face(mV[3],mV[2],mV[0]); break;
		default: return NULL;
	}
}

void Tetra::setNoNeighbours()
{
	mN[0] = mN[1] = mN[2] = mN[3] = NULL;
}

void Tetra::oppositeFaceIndices(int i, int& j, int& k, int& l) const
{
	if (i==0) {j = 1; k = 2; l = 3;}
	else if (i==1) {j = 3; k = 2; l = 0;}
	else if (i==2) {j = 0; k = 1; l = 3;}
	else if (i==3) {j = 0; k = 2; l = 1;}
}

boost::tuple<Vertex*,Vertex*,Vertex*> Tetra::oppositeFaceVertices(int i) const
{
	int j,k,l;
	oppositeFaceIndices(i,j,k,l);
	return boost::make_tuple(mV[j],mV[k],mV[l]);
}

double Tetra::rest() const
{
	return mRestVolume*mRestMultiplier;
}

double Tetra::restMultiplier() const
{
	return mRestMultiplier;
}

void Tetra::setRest(double r){mRestVolume = r;}

void Tetra::setRestMultiplier(double rm){mRestMultiplier = rm;}

double Tetra::energy() const
{
	double diff = (volume() - mRestVolume) / mRestVolume;
	return .5*Physics::kV*diff*diff;
}

std::list<Vertex*> Tetra::vertices() const
{
	return std::list<Vertex*>(mV,mV+4);
}

void Tetra::setFrozen(bool f)
{
	for(int i=0;i<4;i++) mV[i]->setFrozen(f);
}

bool Tetra::isFrozen()
{
	return
		mV[0]->isFrozen() and
		mV[1]->isFrozen() and
		mV[2]->isFrozen() and
		mV[3]->isFrozen();
}
