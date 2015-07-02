#include "edge.h"
#include "physics.h"

Edge::Edge()
:mRestMultiplier(1)
{
	mSpringCoefficient = Physics::kD;
}

Edge::Edge(Vertex* a, Vertex* b)
:mRestMultiplier(1)
{
	mV[0]=a;
	mV[1]=b;
	mRestLength = (b->x()-a->x()).length();
	mSpringCoefficient = Physics::kD;
}

Edge::Edge(Vertex* a, Vertex* b, double rest)
:mRestMultiplier(1)
{
	mV[0]=a;
	mV[1]=b;
	mRestLength = rest;
	mSpringCoefficient = Physics::kD;
}

//Vertex* v(int i);
Vertex* Edge::v(int i)
{
	return mV[i];
}

const Vertex& Edge::cv(int i) const
{
	return *mV[i];
}

double Edge::length() const
{
	return (mV[1]->x()-mV[0]->x()).length();
}

double Edge::springCoefficient() const
{
	return mSpringCoefficient;
}

void Edge::setRest(double r)
{
	mRestLength = r;
}

void Edge::setSpringCoefficient(double k)
{
	mSpringCoefficient = k;
}

double Edge::rest() const
{
	return mRestLength*mRestMultiplier;
}

double Edge::restMultiplier() const
{
	return mRestMultiplier;
}

void Edge::setRestMultiplier(double rest_multiplier)
{
	mRestMultiplier = rest_multiplier;
}

/// potential energy
double Edge::energy() const
{
	return .5 * Physics::kD * (mV[0]->x()-mV[1]->x()).sqlength();
}
