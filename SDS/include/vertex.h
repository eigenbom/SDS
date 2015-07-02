#ifndef VERTEX_H
#define VERTEX_H

#include "vector3.h"

#include <boost/foreach.hpp>
#include <list>

class Face;
class Mesh;
class Vertex
{
public:
	Vertex();
	Vertex(const Vector3d& v, double m=1);
	Vertex(double x, double y, double z, double m = 1);

	/// change force, velocity, position or mass
	inline void zeroF();
	inline void zeroV();
	inline void addF(const Vector3d& f);
	inline void addX(const Vector3d& x);
	inline void resetX(const Vector3d& x); // resets position and velocity
	inline void addM(double dm);
	inline void setM(double d);

	/// retrieve vertex state
	inline const Vector3d& v() const;
	inline const Vector3d& x() const;
	inline const Vector3d& f() const;
	inline const double m() const;

	/// XXX: deprecated
	inline const Vector3d& ox() const; // old x position

	/// n() returns interpolated norm on surface
	// PRE: surface()==true
	Vector3d n() const;

	inline bool surface() const;
	inline const std::list<Face*>&	surfaceFaces() const;
	inline const std::list<Vertex*>& neighbours() const;


	/// add a new neighbour of this vertex
	// PRE: v != this, and v isn't in neighbours already
	void addNeighbour(Vertex* v);
	void addFaceNeighbour(Face* f);

	void removeNeighbour(Vertex* v);
	void removeFaceNeighbour(Face* f);

	/// radius of vertex (calculated from mass)
	inline double r() const;

	/// energy of this vertex
	double kineticEnergy() const;
	double potentialEnergy(double heightOfVertexRelativeToWorld) const;

	inline void setSurface(bool s);

	inline bool isFrozen() const;
	inline void setFrozen(bool f);

	inline bool tag() const;
	inline void tag(bool t);

	// expose this for some things, generally NOT_A_GOOD_IDEA (TM)
	Vector3d mX; // position
	Vector3d mOldX; // last position (for the verlet integrator)
	Vector3d mF; // force accumulator
	double mMass;

protected:

	void computeR();

	bool mSurface; // vertex is on the surface of the mesh
	std::list<Face*> mFaceNeighbours;
	std::list<Vertex*> mNeighbours; // all neighbours

	bool mIsFrozen;

	bool mTag; // collision tag

	// derived vars
	Vector3d mV; // velocity (derived from mX and mOldX)
	double mR; // radius

	friend class MeshLoader;
	friend class Mesh;
	friend class Tetra;
	friend class Physics;
	friend class Face;
	friend class Edge;
	friend class Stepper;
	friend class MeshTester;
	friend class Collision;
};

#include "vertex.inl"

#endif
