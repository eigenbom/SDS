#include "sdsutil.h"

#include "mesh.h"
#include "vertex.h"

#include "tetgen.h"
#include "vector3.h"
#include "hmath.h"

#include <cfloat>
#include <cmath>
#include <cassert>
#include <set>
#include <boost/foreach.hpp>

std::map<Tetra*, std::list<Tetra*> > SDSUtil::getTetTopoGraph(Cell* c, Organism* o)
{
	std::map<Tetra*, std::list<Tetra*> > adjList;

	std::list<Tetra*> itlist = o->mesh()->getNeighbours(c->v());
	std::set<Tetra*> incidentTets = std::set<Tetra*>(itlist.begin(),itlist.end());

	// for each tetrahedra in the list,
	// mark whether it has a neighbour also in the list
	BOOST_FOREACH(Tetra* t, incidentTets)
	{
		adjList[t] = std::list<Tetra*>();

		for(int i=0;i<4;i++)
		{
			Tetra* tin = t->neighbour(i);

			if (incidentTets.count(tin)>0)
			{
				// mark the edge between t to ti
				adjList[t].push_back(tin);
			}
		}
	}

	return adjList;
}

double SDSUtil::doVertexPlaneIntersect(Tetra* tet, int ai, int bi, int ci, int di)
{
	Vertex *qv = &tet->v(ai), *bv = &tet->v(bi), *cv = &tet->v(ci), *dv = &tet->v(di);

	Vector3d b1 = bv->x(), b0 = bv->mOldX,
	c1 = cv->x(), c0 = cv->mOldX,
	d1 = dv->x(), d0 = dv->mOldX,
	q1 = qv->x(), q0 = qv->mOldX;

	/*
	std::cerr << "vpi\n"
		<< b0 << "\n"
		<< b1 << "\n"
		<< c0 << "\n"
		<< c1 << "\n"
		<< d0 << "\n"
		<< d1 << "\n"
		<< q0 << "\n"
		<< q1 << "\n";
		*/

	// find minimal t in solutions to cubic

	double b0xc0yd0z = b0.x()*c0.y()*d0.z();
	double b0xc0yd1z = b0.x()*c0.y()*d1.z();
	double b0xc1yd0z = b0.x()*c1.y()*d0.z();
	double b0xc1yd1z = b0.x()*c1.y()*d1.z();
	double b1xc0yd0z = b1.x()*c0.y()*d0.z();
	double b1xc0yd1z = b1.x()*c0.y()*d1.z();
	double b1xc1yd0z = b1.x()*c1.y()*d0.z();
	double b1xc1yd1z = b1.x()*c1.y()*d1.z();
	double b0xc0zd0y = b0.x()*c0.z()*d0.y();
	double b0xc0zd1y = b0.x()*c0.z()*d1.y();
	double b0xc1zd0y = b0.x()*c1.z()*d0.y();
	double b0xc1zd1y = b0.x()*c1.z()*d1.y();
	double b1xc0zd0y = b1.x()*c0.z()*d0.y();
	double b1xc0zd1y = b1.x()*c0.z()*d1.y();
	double b1xc1zd0y = b1.x()*c1.z()*d0.y();
	double b1xc1zd1y = b1.x()*c1.z()*d1.y();
	double b0yc0xd0z = b0.y()*c0.x()*d0.z();
	double b0yc0xd1z = b0.y()*c0.x()*d1.z();
	double b0yc1xd0z = b0.y()*c1.x()*d0.z();
	double b0yc1xd1z = b0.y()*c1.x()*d1.z();
	double b1yc0xd0z = b1.y()*c0.x()*d0.z();
	double b1yc0xd1z = b1.y()*c0.x()*d1.z();
	double b1yc1xd0z = b1.y()*c1.x()*d0.z();
	double b1yc1xd1z = b1.y()*c1.x()*d1.z();
	double b0yc0zd0x = b0.y()*c0.z()*d0.x();
	double b0yc0zd1x = b0.y()*c0.z()*d1.x();
	double b0yc1zd0x = b0.y()*c1.z()*d0.x();
	double b0yc1zd1x = b0.y()*c1.z()*d1.x();
	double b1yc0zd0x = b1.y()*c0.z()*d0.x();
	double b1yc0zd1x = b1.y()*c0.z()*d1.x();
	double b1yc1zd0x = b1.y()*c1.z()*d0.x();
	double b1yc1zd1x = b1.y()*c1.z()*d1.x();
	double b0zc0xd0y = b0.z()*c0.x()*d0.y();
	double b0zc0xd1y = b0.z()*c0.x()*d1.y();
	double b0zc1xd0y = b0.z()*c1.x()*d0.y();
	double b0zc1xd1y = b0.z()*c1.x()*d1.y();
	double b1zc0xd0y = b1.z()*c0.x()*d0.y();
	double b1zc0xd1y = b1.z()*c0.x()*d1.y();
	double b1zc1xd0y = b1.z()*c1.x()*d0.y();
	double b1zc1xd1y = b1.z()*c1.x()*d1.y();
	double b0zc0yd0x = b0.z()*c0.y()*d0.x();
	double b0zc0yd1x = b0.z()*c0.y()*d1.x();
	double b0zc1yd0x = b0.z()*c1.y()*d0.x();
	double b0zc1yd1x = b0.z()*c1.y()*d1.x();
	double b1zc0yd0x = b1.z()*c0.y()*d0.x();
	double b1zc0yd1x = b1.z()*c0.y()*d1.x();
	double b1zc1yd0x = b1.z()*c1.y()*d0.x();
	double b1zc1yd1x = b1.z()*c1.y()*d1.x();
	double b0xc0yq0z = b0.x()*c0.y()*q0.z();
	double b0xc0yq1z = b0.x()*c0.y()*q1.z();
	double b0xc1yq0z = b0.x()*c1.y()*q0.z();
	double b0xc1yq1z = b0.x()*c1.y()*q1.z();
	double b1xc0yq0z = b1.x()*c0.y()*q0.z();
	double b1xc0yq1z = b1.x()*c0.y()*q1.z();
	double b1xc1yq0z = b1.x()*c1.y()*q0.z();
	double b1xc1yq1z = b1.x()*c1.y()*q1.z();
	double b0xc0zq0y = b0.x()*c0.z()*q0.y();
	double b0xc0zq1y = b0.x()*c0.z()*q1.y();
	double b0xc1zq0y = b0.x()*c1.z()*q0.y();
	double b0xc1zq1y = b0.x()*c1.z()*q1.y();
	double b1xc0zq0y = b1.x()*c0.z()*q0.y();
	double b1xc0zq1y = b1.x()*c0.z()*q1.y();
	double b1xc1zq0y = b1.x()*c1.z()*q0.y();
	double b1xc1zq1y = b1.x()*c1.z()*q1.y();
	double b0yc0xq0z = b0.y()*c0.x()*q0.z();
	double b0yc0xq1z = b0.y()*c0.x()*q1.z();
	double b0yc1xq0z = b0.y()*c1.x()*q0.z();
	double b0yc1xq1z = b0.y()*c1.x()*q1.z();
	double b1yc0xq0z = b1.y()*c0.x()*q0.z();
	double b1yc0xq1z = b1.y()*c0.x()*q1.z();
	double b1yc1xq0z = b1.y()*c1.x()*q0.z();
	double b1yc1xq1z = b1.y()*c1.x()*q1.z();
	double b0yc0zq0x = b0.y()*c0.z()*q0.x();
	double b0yc0zq1x = b0.y()*c0.z()*q1.x();
	double b0yc1zq0x = b0.y()*c1.z()*q0.x();
	double b0yc1zq1x = b0.y()*c1.z()*q1.x();
	double b1yc0zq0x = b1.y()*c0.z()*q0.x();
	double b1yc0zq1x = b1.y()*c0.z()*q1.x();
	double b1yc1zq0x = b1.y()*c1.z()*q0.x();
	double b1yc1zq1x = b1.y()*c1.z()*q1.x();
	double b0zc0xq0y = b0.z()*c0.x()*q0.y();
	double b0zc0xq1y = b0.z()*c0.x()*q1.y();
	double b0zc1xq0y = b0.z()*c1.x()*q0.y();
	double b0zc1xq1y = b0.z()*c1.x()*q1.y();
	double b1zc0xq0y = b1.z()*c0.x()*q0.y();
	double b1zc0xq1y = b1.z()*c0.x()*q1.y();
	double b1zc1xq0y = b1.z()*c1.x()*q0.y();
	double b1zc1xq1y = b1.z()*c1.x()*q1.y();
	double b0zc0yq0x = b0.z()*c0.y()*q0.x();
	double b0zc0yq1x = b0.z()*c0.y()*q1.x();
	double b0zc1yq0x = b0.z()*c1.y()*q0.x();
	double b0zc1yq1x = b0.z()*c1.y()*q1.x();
	double b1zc0yq0x = b1.z()*c0.y()*q0.x();
	double b1zc0yq1x = b1.z()*c0.y()*q1.x();
	double b1zc1yq0x = b1.z()*c1.y()*q0.x();
	double b1zc1yq1x = b1.z()*c1.y()*q1.x();
	double b0xd0yq0z = b0.x()*d0.y()*q0.z();
	double b0xd0yq1z = b0.x()*d0.y()*q1.z();
	double b0xd1yq0z = b0.x()*d1.y()*q0.z();
	double b0xd1yq1z = b0.x()*d1.y()*q1.z();
	double b1xd0yq0z = b1.x()*d0.y()*q0.z();
	double b1xd0yq1z = b1.x()*d0.y()*q1.z();
	double b1xd1yq0z = b1.x()*d1.y()*q0.z();
	double b1xd1yq1z = b1.x()*d1.y()*q1.z();
	double b0xd0zq0y = b0.x()*d0.z()*q0.y();
	double b0xd0zq1y = b0.x()*d0.z()*q1.y();
	double b0xd1zq0y = b0.x()*d1.z()*q0.y();
	double b0xd1zq1y = b0.x()*d1.z()*q1.y();
	double b1xd0zq0y = b1.x()*d0.z()*q0.y();
	double b1xd0zq1y = b1.x()*d0.z()*q1.y();
	double b1xd1zq0y = b1.x()*d1.z()*q0.y();
	double b1xd1zq1y = b1.x()*d1.z()*q1.y();
	double b0yd0xq0z = b0.y()*d0.x()*q0.z();
	double b0yd0xq1z = b0.y()*d0.x()*q1.z();
	double b0yd1xq0z = b0.y()*d1.x()*q0.z();
	double b0yd1xq1z = b0.y()*d1.x()*q1.z();
	double b1yd0xq0z = b1.y()*d0.x()*q0.z();
	double b1yd0xq1z = b1.y()*d0.x()*q1.z();
	double b1yd1xq0z = b1.y()*d1.x()*q0.z();
	double b1yd1xq1z = b1.y()*d1.x()*q1.z();
	double b0yd0zq0x = b0.y()*d0.z()*q0.x();
	double b0yd0zq1x = b0.y()*d0.z()*q1.x();
	double b0yd1zq0x = b0.y()*d1.z()*q0.x();
	double b0yd1zq1x = b0.y()*d1.z()*q1.x();
	double b1yd0zq0x = b1.y()*d0.z()*q0.x();
	double b1yd0zq1x = b1.y()*d0.z()*q1.x();
	double b1yd1zq0x = b1.y()*d1.z()*q0.x();
	double b1yd1zq1x = b1.y()*d1.z()*q1.x();
	double b0zd0xq0y = b0.z()*d0.x()*q0.y();
	double b0zd0xq1y = b0.z()*d0.x()*q1.y();
	double b0zd1xq0y = b0.z()*d1.x()*q0.y();
	double b0zd1xq1y = b0.z()*d1.x()*q1.y();
	double b1zd0xq0y = b1.z()*d0.x()*q0.y();
	double b1zd0xq1y = b1.z()*d0.x()*q1.y();
	double b1zd1xq0y = b1.z()*d1.x()*q0.y();
	double b1zd1xq1y = b1.z()*d1.x()*q1.y();
	double b0zd0yq0x = b0.z()*d0.y()*q0.x();
	double b0zd0yq1x = b0.z()*d0.y()*q1.x();
	double b0zd1yq0x = b0.z()*d1.y()*q0.x();
	double b0zd1yq1x = b0.z()*d1.y()*q1.x();
	double b1zd0yq0x = b1.z()*d0.y()*q0.x();
	double b1zd0yq1x = b1.z()*d0.y()*q1.x();
	double b1zd1yq0x = b1.z()*d1.y()*q0.x();
	double b1zd1yq1x = b1.z()*d1.y()*q1.x();
	double c0xd0yq0z = c0.x()*d0.y()*q0.z();
	double c0xd0yq1z = c0.x()*d0.y()*q1.z();
	double c0xd1yq0z = c0.x()*d1.y()*q0.z();
	double c0xd1yq1z = c0.x()*d1.y()*q1.z();
	double c1xd0yq0z = c1.x()*d0.y()*q0.z();
	double c1xd0yq1z = c1.x()*d0.y()*q1.z();
	double c1xd1yq0z = c1.x()*d1.y()*q0.z();
	double c1xd1yq1z = c1.x()*d1.y()*q1.z();
	double c0xd0zq0y = c0.x()*d0.z()*q0.y();
	double c0xd0zq1y = c0.x()*d0.z()*q1.y();
	double c0xd1zq0y = c0.x()*d1.z()*q0.y();
	double c0xd1zq1y = c0.x()*d1.z()*q1.y();
	double c1xd0zq0y = c1.x()*d0.z()*q0.y();
	double c1xd0zq1y = c1.x()*d0.z()*q1.y();
	double c1xd1zq0y = c1.x()*d1.z()*q0.y();
	double c1xd1zq1y = c1.x()*d1.z()*q1.y();
	double c0yd0xq0z = c0.y()*d0.x()*q0.z();
	double c0yd0xq1z = c0.y()*d0.x()*q1.z();
	double c0yd1xq0z = c0.y()*d1.x()*q0.z();
	double c0yd1xq1z = c0.y()*d1.x()*q1.z();
	double c1yd0xq0z = c1.y()*d0.x()*q0.z();
	double c1yd0xq1z = c1.y()*d0.x()*q1.z();
	double c1yd1xq0z = c1.y()*d1.x()*q0.z();
	double c1yd1xq1z = c1.y()*d1.x()*q1.z();
	double c0yd0zq0x = c0.y()*d0.z()*q0.x();
	double c0yd0zq1x = c0.y()*d0.z()*q1.x();
	double c0yd1zq0x = c0.y()*d1.z()*q0.x();
	double c0yd1zq1x = c0.y()*d1.z()*q1.x();
	double c1yd0zq0x = c1.y()*d0.z()*q0.x();
	double c1yd0zq1x = c1.y()*d0.z()*q1.x();
	double c1yd1zq0x = c1.y()*d1.z()*q0.x();
	double c1yd1zq1x = c1.y()*d1.z()*q1.x();
	double c0zd0xq0y = c0.z()*d0.x()*q0.y();
	double c0zd0xq1y = c0.z()*d0.x()*q1.y();
	double c0zd1xq0y = c0.z()*d1.x()*q0.y();
	double c0zd1xq1y = c0.z()*d1.x()*q1.y();
	double c1zd0xq0y = c1.z()*d0.x()*q0.y();
	double c1zd0xq1y = c1.z()*d0.x()*q1.y();
	double c1zd1xq0y = c1.z()*d1.x()*q0.y();
	double c1zd1xq1y = c1.z()*d1.x()*q1.y();
	double c0zd0yq0x = c0.z()*d0.y()*q0.x();
	double c0zd0yq1x = c0.z()*d0.y()*q1.x();
	double c0zd1yq0x = c0.z()*d1.y()*q0.x();
	double c0zd1yq1x = c0.z()*d1.y()*q1.x();
	double c1zd0yq0x = c1.z()*d0.y()*q0.x();
	double c1zd0yq1x = c1.z()*d0.y()*q1.x();
	double c1zd1yq0x = c1.z()*d1.y()*q0.x();
	double c1zd1yq1x = c1.z()*d1.y()*q1.x();
	// coefficients calculated using mathematica

	// constant
	double constant = b0zc0yd0x - b0yc0zd0x - b0zc0xd0y + b0xc0zd0y + b0yc0xd0z -
	 b0xc0yd0z - b0zc0yq0x + b0yc0zq0x + b0zd0yq0x - c0zd0yq0x -
	  b0yd0zq0x + c0yd0zq0x + b0zc0xq0y - b0xc0zq0y -
	 b0zd0xq0y + c0zd0xq0y + b0xd0zq0y - c0xd0zq0y - b0yc0xq0z +
	  b0xc0yq0z + b0yd0xq0z - c0yd0xq0z - b0xd0yq0z +
	 c0xd0yq0z;

	 // t term
	 double t = (-3*b0zc0yd0x + b1zc0yd0x + 3*b0yc0zd0x -
	    b1yc0zd0x + b0zc1yd0x - b0yc1zd0x + 3*b0zc0xd0y -
	    b1zc0xd0y - 3*b0xc0zd0y + b1xc0zd0y - b0zc1xd0y +
	    b0xc1zd0y - 3*b0yc0xd0z + b1yc0xd0z + 3*b0xc0yd0z -
	    b1xc0yd0z + b0yc1xd0z - b0xc1yd0z + b0zc0yd1x -
	    b0yc0zd1x - b0zc0xd1y + b0xc0zd1y + b0yc0xd1z -
	    b0xc0yd1z + 3*b0zc0yq0x - b1zc0yq0x - 3*b0yc0zq0x +
	    b1yc0zq0x - b0zc1yq0x + b0yc1zq0x - 3*b0zd0yq0x +
	    b1zd0yq0x + 3*c0zd0yq0x - c1zd0yq0x + 3*b0yd0zq0x -
	    b1yd0zq0x - 3*c0yd0zq0x + c1yd0zq0x + b0zd1yq0x -
	    c0zd1yq0x - b0yd1zq0x + c0yd1zq0x - 3*b0zc0xq0y +
	    b1zc0xq0y + 3*b0xc0zq0y - b1xc0zq0y + b0zc1xq0y -
	    b0xc1zq0y + 3*b0zd0xq0y - b1zd0xq0y - 3*c0zd0xq0y +
	    c1zd0xq0y - 3*b0xd0zq0y + b1xd0zq0y + 3*c0xd0zq0y -
	    c1xd0zq0y - b0zd1xq0y + c0zd1xq0y + b0xd1zq0y -
	    c0xd1zq0y + 3*b0yc0xq0z - b1yc0xq0z - 3*b0xc0yq0z +
	    b1xc0yq0z - b0yc1xq0z + b0xc1yq0z - 3*b0yd0xq0z +
	    b1yd0xq0z + 3*c0yd0xq0z - c1yd0xq0z + 3*b0xd0yq0z -
	    b1xd0yq0z - 3*c0xd0yq0z + c1xd0yq0z + b0yd1xq0z -
	    c0yd1xq0z - b0xd1yq0z + c0xd1yq0z - b0zc0yq1x +
	    b0yc0zq1x + b0zd0yq1x - c0zd0yq1x - b0yd0zq1x +
	    c0yd0zq1x + b0zc0xq1y - b0xc0zq1y - b0zd0xq1y +
	    c0zd0xq1y + b0xd0zq1y - c0xd0zq1y - b0yc0xq1z +
	    b0xc0yq1z + b0yd0xq1z - c0yd0xq1z - b0xd0yq1z +
	    c0xd0yq1z);

	    // t^2 term
	    double t2 = (3*b0zc0yd0x - 2*b1zc0yd0x - 3*b0yc0zd0x +
	    2*b1yc0zd0x - 2*b0zc1yd0x + b1zc1yd0x + 2*b0yc1zd0x -
	    b1yc1zd0x - 3*b0zc0xd0y + 2*b1zc0xd0y + 3*b0xc0zd0y -
	    2*b1xc0zd0y + 2*b0zc1xd0y - b1zc1xd0y - 2*b0xc1zd0y +
	    b1xc1zd0y + 3*b0yc0xd0z - 2*b1yc0xd0z - 3*b0xc0yd0z +
	    2*b1xc0yd0z - 2*b0yc1xd0z + b1yc1xd0z + 2*b0xc1yd0z -
	    b1xc1yd0z - 2*b0zc0yd1x + b1zc0yd1x + 2*b0yc0zd1x -
	    b1yc0zd1x + b0zc1yd1x - b0yc1zd1x + 2*b0zc0xd1y -
	    b1zc0xd1y - 2*b0xc0zd1y + b1xc0zd1y - b0zc1xd1y +
	    b0xc1zd1y - 2*b0yc0xd1z + b1yc0xd1z + 2*b0xc0yd1z -
	    b1xc0yd1z + b0yc1xd1z - b0xc1yd1z - 3*b0zc0yq0x +
	    2*b1zc0yq0x + 3*b0yc0zq0x - 2*b1yc0zq0x + 2*b0zc1yq0x -
	    b1zc1yq0x - 2*b0yc1zq0x + b1yc1zq0x + 3*b0zd0yq0x -
	    2*b1zd0yq0x - 3*c0zd0yq0x + 2*c1zd0yq0x - 3*b0yd0zq0x +
	    2*b1yd0zq0x + 3*c0yd0zq0x - 2*c1yd0zq0x - 2*b0zd1yq0x +
	    b1zd1yq0x + 2*c0zd1yq0x - c1zd1yq0x + 2*b0yd1zq0x -
	    b1yd1zq0x - 2*c0yd1zq0x + c1yd1zq0x + 3*b0zc0xq0y -
	    2*b1zc0xq0y - 3*b0xc0zq0y + 2*b1xc0zq0y - 2*b0zc1xq0y +
	    b1zc1xq0y + 2*b0xc1zq0y - b1xc1zq0y - 3*b0zd0xq0y +
	    2*b1zd0xq0y + 3*c0zd0xq0y - 2*c1zd0xq0y + 3*b0xd0zq0y -
	    2*b1xd0zq0y - 3*c0xd0zq0y + 2*c1xd0zq0y + 2*b0zd1xq0y -
	    b1zd1xq0y - 2*c0zd1xq0y + c1zd1xq0y - 2*b0xd1zq0y +
	    b1xd1zq0y + 2*c0xd1zq0y - c1xd1zq0y - 3*b0yc0xq0z +
	    2*b1yc0xq0z + 3*b0xc0yq0z - 2*b1xc0yq0z + 2*b0yc1xq0z -
	    b1yc1xq0z - 2*b0xc1yq0z + b1xc1yq0z + 3*b0yd0xq0z -
	    2*b1yd0xq0z - 3*c0yd0xq0z + 2*c1yd0xq0z - 3*b0xd0yq0z +
	    2*b1xd0yq0z + 3*c0xd0yq0z - 2*c1xd0yq0z - 2*b0yd1xq0z +
	    b1yd1xq0z + 2*c0yd1xq0z - c1yd1xq0z + 2*b0xd1yq0z -
	    b1xd1yq0z - 2*c0xd1yq0z + c1xd1yq0z + 2*b0zc0yq1x -
	    b1zc0yq1x - 2*b0yc0zq1x + b1yc0zq1x - b0zc1yq1x +
	    b0yc1zq1x - 2*b0zd0yq1x + b1zd0yq1x + 2*c0zd0yq1x -
	    c1zd0yq1x + 2*b0yd0zq1x - b1yd0zq1x - 2*c0yd0zq1x +
	    c1yd0zq1x + b0zd1yq1x - c0zd1yq1x - b0yd1zq1x +
	    c0yd1zq1x - 2*b0zc0xq1y + b1zc0xq1y + 2*b0xc0zq1y -
	    b1xc0zq1y + b0zc1xq1y - b0xc1zq1y + 2*b0zd0xq1y -
	    b1zd0xq1y - 2*c0zd0xq1y + c1zd0xq1y - 2*b0xd0zq1y +
	    b1xd0zq1y + 2*c0xd0zq1y - c1xd0zq1y - b0zd1xq1y +
	    c0zd1xq1y + b0xd1zq1y - c0xd1zq1y + 2*b0yc0xq1z -
	    b1yc0xq1z - 2*b0xc0yq1z + b1xc0yq1z - b0yc1xq1z +
	    b0xc1yq1z - 2*b0yd0xq1z + b1yd0xq1z + 2*c0yd0xq1z -
	    c1yd0xq1z + 2*b0xd0yq1z - b1xd0yq1z - 2*c0xd0yq1z +
	    c1xd0yq1z + b0yd1xq1z - c0yd1xq1z - b0xd1yq1z +
	    c0xd1yq1z);

	    //t^3 term
	    double t3 = (-b0zc0yd0x + b1zc0yd0x + b0yc0zd0x -
	    b1yc0zd0x + b0zc1yd0x - b1zc1yd0x - b0yc1zd0x +
	    b1yc1zd0x + b0zc0xd0y - b1zc0xd0y - b0xc0zd0y +
	    b1xc0zd0y - b0zc1xd0y + b1zc1xd0y + b0xc1zd0y -
	    b1xc1zd0y - b0yc0xd0z + b1yc0xd0z + b0xc0yd0z -
	    b1xc0yd0z + b0yc1xd0z - b1yc1xd0z - b0xc1yd0z +
	    b1xc1yd0z + b0zc0yd1x - b1zc0yd1x - b0yc0zd1x +
	    b1yc0zd1x - b0zc1yd1x + b1zc1yd1x + b0yc1zd1x -
	    b1yc1zd1x - b0zc0xd1y + b1zc0xd1y + b0xc0zd1y -
	    b1xc0zd1y + b0zc1xd1y - b1zc1xd1y - b0xc1zd1y +
	    b1xc1zd1y + b0yc0xd1z - b1yc0xd1z - b0xc0yd1z +
	    b1xc0yd1z - b0yc1xd1z + b1yc1xd1z + b0xc1yd1z -
	    b1xc1yd1z + b0zc0yq0x - b1zc0yq0x - b0yc0zq0x +
	    b1yc0zq0x - b0zc1yq0x + b1zc1yq0x + b0yc1zq0x -
	    b1yc1zq0x - b0zd0yq0x + b1zd0yq0x + c0zd0yq0x -
	    c1zd0yq0x + b0yd0zq0x - b1yd0zq0x - c0yd0zq0x +
	    c1yd0zq0x + b0zd1yq0x - b1zd1yq0x - c0zd1yq0x +
	    c1zd1yq0x - b0yd1zq0x + b1yd1zq0x + c0yd1zq0x -
	    c1yd1zq0x - b0zc0xq0y + b1zc0xq0y + b0xc0zq0y -
	    b1xc0zq0y + b0zc1xq0y - b1zc1xq0y - b0xc1zq0y +
	    b1xc1zq0y + b0zd0xq0y - b1zd0xq0y - c0zd0xq0y +
	    c1zd0xq0y - b0xd0zq0y + b1xd0zq0y + c0xd0zq0y -
	    c1xd0zq0y - b0zd1xq0y + b1zd1xq0y + c0zd1xq0y -
	    c1zd1xq0y + b0xd1zq0y - b1xd1zq0y - c0xd1zq0y +
	    c1xd1zq0y + b0yc0xq0z - b1yc0xq0z - b0xc0yq0z +
	    b1xc0yq0z - b0yc1xq0z + b1yc1xq0z + b0xc1yq0z -
	    b1xc1yq0z - b0yd0xq0z + b1yd0xq0z + c0yd0xq0z -
	    c1yd0xq0z + b0xd0yq0z - b1xd0yq0z - c0xd0yq0z +
	    c1xd0yq0z + b0yd1xq0z - b1yd1xq0z - c0yd1xq0z +
	    c1yd1xq0z - b0xd1yq0z + b1xd1yq0z + c0xd1yq0z -
	    c1xd1yq0z - b0zc0yq1x + b1zc0yq1x + b0yc0zq1x -
	    b1yc0zq1x + b0zc1yq1x - b1zc1yq1x - b0yc1zq1x +
	    b1yc1zq1x + b0zd0yq1x - b1zd0yq1x - c0zd0yq1x +
	    c1zd0yq1x - b0yd0zq1x + b1yd0zq1x + c0yd0zq1x -
	    c1yd0zq1x - b0zd1yq1x + b1zd1yq1x + c0zd1yq1x -
	    c1zd1yq1x + b0yd1zq1x - b1yd1zq1x - c0yd1zq1x +
	    c1yd1zq1x + b0zc0xq1y - b1zc0xq1y - b0xc0zq1y +
	    b1xc0zq1y - b0zc1xq1y + b1zc1xq1y + b0xc1zq1y -
	    b1xc1zq1y - b0zd0xq1y + b1zd0xq1y + c0zd0xq1y -
	    c1zd0xq1y + b0xd0zq1y - b1xd0zq1y - c0xd0zq1y +
	    c1xd0zq1y + b0zd1xq1y - b1zd1xq1y - c0zd1xq1y +
	    c1zd1xq1y - b0xd1zq1y + b1xd1zq1y + c0xd1zq1y -
	    c1xd1zq1y - b0yc0xq1z + b1yc0xq1z + b0xc0yq1z -
	    b1xc0yq1z + b0yc1xq1z - b1yc1xq1z - b0xc1yq1z +
	    b1xc1yq1z + b0yd0xq1z - b1yd0xq1z - c0yd0xq1z +
	    c1yd0xq1z - b0xd0yq1z + b1xd0yq1z + c0xd0yq1z -
	    c1xd0yq1z - b0yd1xq1z + b1yd1xq1z + c0yd1xq1z -
	    c1yd1xq1z + b0xd1yq1z - b1xd1yq1z - c0xd1yq1z +
	    c1xd1yq1z);

	// print cubic
	// std::cerr << "Solving Equation: " << t3 << "t^3 + " << t2 << "t^2 + " << t << "t + " << constant << std::endl;

	// find least solution in (0,1)
	double x1, x2, x3;
	int numberOfSolutions;

	if (fabs(t3) < DBL_EPSILON) // some small number
	{
		numberOfSolutions = Math::solveQuadratic(t2,t,constant,x1,x2);
	}
	else
	{
		// solve cubic
		numberOfSolutions = Math::solveCubic(t2/t3,t/t3,constant/t3,x1,x2,x3);
	}

	// get the smallest in (0,1) of these solutions
	double smallestSolution = 0;
	bool hasValidSolution = false;

	// num
	if (numberOfSolutions<=0)
	{
		std::cerr << "Error! SDSUtil::doVertexPlaneIntersect::numberOfSolutions <= 0.\n Exitting.";
		return -1;
	}
	else if (numberOfSolutions>=1 and x1 >= 0 and x1 <= 1)
	{
		hasValidSolution = true;
		smallestSolution = x1;
	}
	// else x1 < 0 (or x1 > 1, which implies x2,x3 > 1)
	else if (numberOfSolutions>=2 and x2 >= 0 and x2 <= 1)
	{
		hasValidSolution = true;
		smallestSolution = x2;
	}
	else if (numberOfSolutions==3 and x3 >= 0 and x3 <= 1)
	{
		smallestSolution = x3;
	}

	if (not hasValidSolution)
		return -1;

	return smallestSolution;
}

void SDSUtil::rewindAllVertices(Mesh* m, double dh, double dt)
{
	double d = (dt-dh)/dt;
	BOOST_FOREACH(Vertex* v, m->vertices() )
	{
		v->mX = v->mOldX + (v->mX - v->mOldX)*d;
	}
}

bool SDSUtil::runTetgenCommandLine(int argc, char** argv)
{
  tetgenbehavior b;
  tetgenio in, addin, bgmin;

  if (!b.parse_commandline(argc, argv)) {
	return false;
  }
  if (b.refine) {
	if (!in.load_tetmesh(b.infilename)) {
	  return false;
	}
  } else {
	if (!in.load_plc(b.infilename, (int) b.object)) {
	  return false;
	}
  }
  if (b.insertaddpoints) {
	if (!addin.load_node(b.addinfilename)) {
	  addin.numberofpoints = 0l;
	}
  }
  if (b.metric) {
	if (!bgmin.load_tetmesh(b.bgmeshfilename)) {
	  bgmin.numberoftetrahedra = 0l;
	}
  }

  if (bgmin.numberoftetrahedra > 0l) {
	tetrahedralize(&b, &in, NULL, &addin, &bgmin);
  } else {
	tetrahedralize(&b, &in, NULL, &addin, NULL);
  }

  return true;
}
