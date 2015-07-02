#ifndef EDGE_H
#define EDGE_H

#include "vertex.h"

/**
 * An edge.
 *
 * A rest multiplier affects the computed rest length of an edge. By default it is = 1, but
 * in some cases it will not be 1. By calling setRest(r) you are setting the rest length to r*multiplier,
 * and this is what is retrieved via rest().
 *
 * Rest multipliers are used to preserve characteristics of the initial shape of a mesh.
 *
 */
class Edge
{
	public:

	/// empty edge
	Edge();
	/// Calculate rest length from current positions of a and b
	Edge(Vertex* a, Vertex* b);
	/// Give explicit rest length
	Edge(Vertex* a, Vertex* b, double rest);

	/// accessors
	Vertex* v(int i);
	const Vertex& cv(int i) const;
	double rest() const;
	double restMultiplier() const;
	double length() const;
	double springCoefficient() const;

	void setRest(double);
	void setSpringCoefficient(double k);
	void setRestMultiplier(double rest_multiplier);

	// xxx: deprecated, use setRest
	//void rest(double r);

	/// potential energy
	double energy() const;

	protected:
	Vertex* mV[2];

	double mRestLength;
	double mRestMultiplier;

	double mSpringCoefficient;

	friend class Mesh;
};


#endif
