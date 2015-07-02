#ifndef TETRA_H
#define TETRA_H

#include "vertex.h"
#include "aabb.h"
#include <boost/tuple/tuple.hpp>

class Tetra
{
	public:

	Tetra();
	Tetra(Vertex* a, Vertex* b, Vertex* c, Vertex* d);
	Tetra(Vertex* a, Vertex* b, Vertex* c, Vertex* d, double restvol);

	// NULL -- no neighbours ... is an outer tetra ...
	inline void setNeighbours(Tetra* a, Tetra* b, Tetra* c, Tetra* d);
	/// PRE: i = 0..3, POST: t==NULL => isOuter=True
	inline void setNeighbour(int i, Tetra* t);
	/// PRE: v is in t
	inline void setNeighbour(Vertex* v, Tetra* t);

	/// replace a neighbour with another
	/// PRE: tobereplaced is a neighbour
	inline void replaceNeighbour(Tetra* tobereplaced, Tetra* toreplace);

	/// return the index of neighbour t, -1 if not a neighbour
	int getNeighbourIndex(Tetra* t) const;

	// pre 0<=int<4
	inline Tetra* neighbour(int);
	// NULL if no neighbour or v \notin this
	inline Tetra* neighbour(Vertex* v);


	inline bool isOuter() const;
	inline Vector3d center() const;

	double rest() const;
	double restMultiplier() const;
	void setRest(double r);
	void setRestMultiplier(double);

	// xxx: deprecated, use setRest
	//inline void rest(double r);

	inline double springCoefficient() const;
	inline void setSpringCoefficient(double ka);

	inline void setIntersected(bool b);
	inline bool intersected() const;

	void setFrozen(bool f);
	bool isFrozen();

	inline double volume() const;
	double energy() const;

	/// center of mass
	Vector3d centerOfMass() const;

	/// vertex, neighbour retrieval
	bool isVertex(const Vertex* v) const; // returns true if v==mV[i] for some i
	bool contains(const Vertex* v) const; // same as isVertex()
	bool contains(const Vertex* v1, const Vertex* v2) const; // same as isVertex()
	bool contains(const Vertex* v1, const Vertex* v2, const Vertex* v3) const; // same as isVertex()
	bool contains(const Vertex* v1, const Vertex* v2, const Vertex* v3, const Vertex* v4) const; // same as isVertex()


	// deprecated: use cv(int)
	inline const Vertex& vertex(int i) const;

	inline const Vertex& cv(int i) const;
	inline Vertex& v(int i);
	inline Vertex* pv(int i);

	// return all four vertices of this tetra
	std::list<Vertex*> vertices() const;

	// deprecated: use neighbour, and check for null
	inline Tetra& n(int i);

	/// return the index of vertex v in the tetra (or -1 if not part of this tetra)
	inline int vIndex(const Vertex* v) const;

	/// returns the three ordered indices of the vertices making up the opposite face to v(i)
	void oppositeFaceIndices(int i, int& j, int& k, int& l) const;
	boost::tuple<Vertex*,Vertex*,Vertex*> oppositeFaceVertices(int i) const;

	/// returns the point in the center of face i
	Vector3d getFaceCentroid(int i) const;

	// bary(): returns the barycentric coordinates of p w.r.t, this tetra
	Vector3d bary(const Vector3d& p) const;

	const AABB& updateAABB();
	inline const AABB& bounds() const;
	inline const AABB& aabb() const;

	// DEPRECATED: creates a new face on face i of this tetra
	Face* createFace(int i);

	protected:

	void setNoNeighbours();

	/** When loaded from the tetgen format, the vertices are ordered s.t the faces
	 * are pointing outwards (counter-clockwise = gl-style), where
	 * bottom-face = 0,2,1
	 * left-face = 0,1,3
	 * right-face = 1,2,3
	 * back-face = 3,2,0
	 */

	Vertex* mV[4];
	Tetra* mN[4]; // neighbours (mN[i] opposite mV[i]) -- NULL for no neighbour on that face

	bool mIntersected;
	bool mOuter;
	double mRestVolume; //volume at rest
	double mRestMultiplier;
	AABB mAABB;

	double mSpringCoefficient;

	friend class MeshLoader;
	friend class Mesh;
};

#include "tetra.inl"

#endif
