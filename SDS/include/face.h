#ifndef FACE_H
#define FACE_H

#include "vertex.h"
#include "aabb.h"
#include <iostream>

/** A triangular face in a 3d tetrahedral complex
 *
 */
class Face
{
public:

	Face();
	Face(Vertex* a, Vertex* b, Vertex* c);

	/// center: the center point of the face
	inline Vector3d center() const;
	/// q: a point _somewhere_ on the face
	inline const Vector3d& q() const;
	/// n: a vector normal to the face
	inline Vector3d n() const;

	inline double rest() const;
	inline double rest(double r);

	inline const Vertex& cv(int i) const;
	inline Vertex& v(int i) const;
	inline void setVertices(Vertex* a, Vertex* b, Vertex* c);

	// vertex and edge composition tests
	inline bool contains(Vertex* v) const;
	bool contains(Vertex* a, Vertex* b) const;

	inline int index(Vertex* v) const;

	// outer(): is this triangle on the surface?
	inline bool outer() const;
	inline void setOuter(bool o);



	/// bounding box of this face
	AABB aabb();

	/// DEPRECATED: note this returns some false positives
	// for those triangles that are touching the surface
	// but not part of the surface
	inline bool surface() const;

protected:

	Vertex* mV[3];
	double mRestArea;
	bool mOuter; // is this face an outer face?

	friend class Physics;
	friend class Mesh;
};

#include "face.inl"

#endif
