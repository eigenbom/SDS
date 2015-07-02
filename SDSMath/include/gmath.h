/* Some useful geometry routines
 * BP 09.11.07
 */

#ifndef GMATH_H
#define GMATH_H

#include "vector3.h"
#include "hmath.h"
#include <vector>
#include <list>
#include <boost/tuple/tuple.hpp>

namespace Math
{
	// baryTri: computes the barycentric coordinates (b1,b2) of p w.r.t the triangle efg
	// PRE: p is co-planar with efg
	// POST: e + (f-e)*b1 + (g-e)*b2 = p

	template <typename T>
	void baryTri(const Vector3<T>& e, const Vector3<T>& f, const Vector3<T>& g, const Vector3<T>& p, T& b1, T& b2);

	inline double volumeOfSphereGivenRadius(double radius);
	inline double radiusOfSphereGivenVolume(double vol);

	/// triangulate: a fast general simple polygon triangulation routine
	// PRE: points specify a polygon in an anti-clockwise order (in the x-y plane)
	// PRE: only first two dimensions of 3d points are used
	// PRE: points.size() is not big, e.g., usually < 10
	// POST: return a set of triangles (specified by indexes into original list)
	std::vector<Vector3i> triangulate(std::vector<Vector3d> points);

	/// finds a common triangulation that satisfies both polygons
	// PRE: points in anti-clockwise order, points form a simple polygon
	// PRE: only first two dimensions of points are used
	// POST: false if no common triangulation exists
	// this code is based on Geometric Tools for Computer Graphics, Chapter 13, Eberly/Schneider

	bool commonTriangulation(std::vector<int>& indexList, std::vector<Vector3d>& vlist1, std::vector<Vector3d>& vlist2, std::vector<Vector3i>& triangleList);
	bool commonTriangulation(std::vector<Vector3d>& vlist1, std::vector<Vector3d>& vlist2, std::vector<Vector3i>& triangleList);

	/// tetrahedralisation routine
	// uses tetgen internally
	// note that tetrahedralise MAY ADD NEW POINTS, but hull will remain unchanged
	void tetrahedralise(std::vector<Vector3d>& points,
						std::vector<boost::tuple<int,int,int> >& trianglesOnHull,
						std::vector<boost::tuple<int,int,int,int> >& tetrahedra);

	void tetrahedralise(std::vector<Vector3d>& points,
						std::vector<boost::tuple<int,int,int> >& trianglesOnHull,
						std::vector<boost::tuple<int,int,int,int> >& tetrahedra,
						std::vector<boost::tuple<int,int,int,int> >& tetrahedraNeighbours,
						std::vector<boost::tuple<int,int> >& edges);
};

#include "gmath.inl"

#endif
