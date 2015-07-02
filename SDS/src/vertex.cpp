#include "vertex.h"
#include "face.h"
#include "physics.h"

#include <cmath>

Vertex::Vertex()
:mMass(0)
,mSurface(false)
,mIsFrozen(false)
,mTag(false)
,mX(Vector3d::ZERO)
,mOldX(Vector3d::ZERO)
,mF(Vector3d::ZERO)
,mV(Vector3d::ZERO)
{}

Vertex::Vertex(const Vector3d& v, double m)
:mX(v),mOldX(v),mF(Vector3d::ZERO),mMass(m)
,mSurface(false)
,mIsFrozen(false)
,mTag(false),mV(Vector3d::ZERO)
{
	computeR();
}

Vertex::Vertex(double x, double y, double z, double m)
:mX(x,y,z),mOldX(x,y,z),mF(Vector3d::ZERO),mMass(m)
,mSurface(false)
,mIsFrozen(false)
,mTag(false),mV(Vector3d::ZERO)
{
	computeR();
}

double Vertex::kineticEnergy() const
{
	return mMass * .5 * v().sqlength();
}

double Vertex::potentialEnergy(double heightOfVertexRelativeToWorld) const
{
	return mMass * Physics::GRAVITY * heightOfVertexRelativeToWorld;
}

Vector3d Vertex::n() const
{
	Vector3d v = Vector3d::ZERO;
	BOOST_FOREACH(const Face* f, mFaceNeighbours)
	{
		v += f->n();
	}

	return v.normalise();
}

void Vertex::addNeighbour(Vertex* v)
{
	mNeighbours.push_back(v);
}

void Vertex::addFaceNeighbour(Face* f)
{
	mFaceNeighbours.push_back(f);
}

void Vertex::removeNeighbour(Vertex* v)
{
	mNeighbours.remove(v);
}

void Vertex::removeFaceNeighbour(Face* f)
{
	mFaceNeighbours.remove(f);
}

void Vertex::computeR()
{
	mR = std::pow(.75*mMass/M_PI,1/3.0);
}
