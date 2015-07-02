/*
 * Refer to transform.h for details concerning the implementation files.
 */

#include "transform.h"

#include <iostream>
#include "log.h"

#include "face.h"
#include "edge.h"
#include "organism.h"

#include "gmath.h"
#include "vector3.h"
#include "matrix3.h"
#include "geometry.h"


#define DEBUG_TRANSFORM

#ifndef DEBUG_TRANSFORM
	#undef LOG
	#define LOG(x) ;
#endif
// static State and Properties

Transform::State Transform::state = Transform::TRANSFORM_COMPLETED;
PropertyList Transform::properties;
std::list<PropertyList> Transform::allProperties;

std::string Transform::sErrorMessage;

double Transform::sDivideInternalAngleThreshold = .5; /* <0 disabled */ // in radians
double Transform::sDivideSurfaceAngleThreshold = .5; /* <0 disabled */

void Transform::called(std::string s)
{
	properties.add("Transformation",s);
	LOG("Transformation" << s);
}

void Transform::reset()
{
	properties.clear();
	state = TRANSFORM_COMPLETED;
}

void Transform::finaliseProperties()
{
	allProperties.push_back(properties);
}

boost::tuple<Cell*,Cell*> Transform::testQuit()
{
	state = TRANSFORM_ERROR;
	std::string errorMessage = "Test Quit";
	Transform::properties.add(errorMessage);
	setErrorMessage(errorMessage);
	return boost::make_tuple<Cell*,Cell*>(NULL,NULL);
}

boost::tuple<Cell*,Cell*> Transform::error(std::string s)
{
	state = TRANSFORM_ERROR;

	if (not s.empty())
	{
		setErrorMessage(s);
	}

	std::string errorMessage = "Transform Error: " + getErrorMessage();
	Transform::properties.add(errorMessage);

	return boost::make_tuple<Cell*,Cell*>(NULL,NULL);
}

std::string Transform::getErrorMessage()
{
	return sErrorMessage;
}

void Transform::setErrorMessage(std::string msg)
{
	sErrorMessage = msg;
}

// Helper Utilities

///  Retrieve neighbouring tetra in a certain direction
Tetra* Transform::getTetraInDirection(Cell* c, Vector3d dir, Organism* o)
{
	Mesh* m = o->mesh();
	Tetra* ti = NULL;

	const Vertex* v = c->v();
	Vector3d cx = v->x();
	std::list<Tetra*> n = m->getNeighbours(c->v());
	BOOST_FOREACH(Tetra* t, n)
	{
		// first we check to see if the opposite face is in the direction dir from c->x()
		int f[3];
		t->oppositeFaceIndices(t->vIndex(v),f[0],f[1],f[2]);
		Vertex* fv[3] = {t->pv(f[0]),t->pv(f[1]),t->pv(f[2])};
		Vector3d fvx[3] = {fv[0]->x(),fv[1]->x(),fv[2]->x()};

		// if it is, then we check for the (unique) face that intersects the ray
		// projecting from c->x() in direction dir
		Vector3d centroid = (fv[0]->x() + fv[1]->x() + fv[2]->x())/3;
		if (dot(centroid - cx, dir) > 0)
		{
			// use ray-triangle intersect test
			bool intersects;
			double intersectionPt[3];

			// ray goes through points p1 and p2
			double* p1 = (double*)cx;
			Vector3d cxplusdir = cx + dir;
			double* p2 = (double*)cxplusdir;

			double triangle_vertices[9] = {fvx[0][0],fvx[0][1],fvx[0][2],
					fvx[1][0],fvx[1][1],fvx[1][2],
					fvx[2][0],fvx[2][1],fvx[2][2]};

			triangle_contains_line_exp_3d(triangle_vertices, p1, p2, &intersects, intersectionPt);

			if (intersects)
			{
				ti = t;
				break;
			}
		}
	}

	return ti;
}

///  Retrieve neighbouring surface face in a certain direction
// PRE: c is on the boundary
Face* Transform::getFaceInDirection(Cell* c, Vector3d dir, Organism* o)
{
	// for each adjacent surface face...
	//std::cerr << c << " " << c->v()->surfaceFaces().size() << "\n";
	BOOST_FOREACH(Face* f, c->v()->surfaceFaces())
	{
		// project d onto the plane that contains f
		Vector3d fn = f->n();
		Vector3d projD = dir - fn*dot(dir,fn);

		// set up coordinate system
		int ci = f->index(c->v());
		Vertex* v[3] = {
				&f->v(ci),
				&f->v((ci+1)%3),
				&f->v((ci+2)%3)
		};

		double b1, b2;
		Math::baryTri(v[0]->x(),v[1]->x(),v[2]->x(),v[0]->x()+projD,b1,b2);
		if (b1 >= 0 and b2 >= 0)
		{
			return f;
		}
	}
	return NULL;
}

/// compute the volume of a tetrahedra joining four cells with radii r1...r4
double Transform::tetrahedraRestVolume(double r1, double r2, double r3, double r4)
{
	double r1r2r4 = r1 + r2 + r4, r1r3r4 = r1 + r3 + r4, r2r3r4 = r2 + r3 + r4;
	double r123 = r1*r2*r3, r12 = r1*r2, r13 = r1*r3, r23 = r2*r3;

	double foo1 = sqrt(r23*(r1r2r4)*(r1r3r4));
	double foo2 = sqrt(r13*(r1r2r4)*(r2r3r4));
	double foo3 = sqrt(r12*(r1r3r4)*(r2r3r4));

	return (1/(3*(r1 + r4)*(r2 + r4)*(r3 + r4)))*
	 sqrt((r123 + r4*foo1 +
	          r4*foo2 -
	     r4*foo3)*
	       (r123 + r4*foo1 -
	     r4*foo2 +
	          r4*foo3)*(r123 -
	     r4*foo1 +
	          r4*foo2 +
	     r4*foo3)*
	       ((-r1)*r23 + r4*foo1 +
	     r4*foo2 +
	          r4*foo3));
}

double Transform::tetrahedraRestVolume(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4, Organism* o)
{
	return tetrahedraRestVolume(o->getAssociatedCell(v1),o->getAssociatedCell(v2),o->getAssociatedCell(v3),o->getAssociatedCell(v4));
}

double Transform::tetrahedraRestVolume(Cell* c1, Cell* c2, Cell* c3, Cell* c4)
{
	return tetrahedraRestVolume(c1->r(),c2->r(),c3->r(),c4->r());
}

void Transform::addEdgeIfNone(Vertex* v1, Vertex* v2, Organism* o)
{
	if (o->mesh()->getEdge(v1,v2)==NULL)
	{
		Edge* e = new Edge(v1,v2,o->getAssociatedCell(v1)->r() + o->getAssociatedCell(v2)->r());
		v1->addNeighbour(v2);
		v2->addNeighbour(v1);
		o->mesh()->addEdge(e);
	}
}

void Transform::enumerateSurroundingTetrasAndVertices(Vertex* a, Vertex* b, Organism* o, std::vector<Tetra*>& tetras, std::vector<int>& vertexIndices)
{
	// Two cases: edge ab is (1) on the surface or (2) internal to the mesh.
	Face *f1 = NULL, *f2 = NULL;
	if (o->mesh()->getAdjacentFaces(a,b,f1,f2))
	// then we have a surface edge..
	{
		if (not ((f1!=NULL and f2!=NULL) and (f1->outer() and f2->outer())))
		{
			error("f1 or f2 is not an outer edge");
			return;
		}

		// 1. Get the vertex that falls on the right of a->b
		// this is the vertex whose index is one below b (without being = a)
		int f1b = f1->index(b), f2b = f2->index(b);

		if (not (
				(f1b>=0 and f1b<3) &&
				(f2b>=0 and f2b<3)))
		{
			error("fxb issue");
			return;
		}

		int indexbelowbf1 = (f1b + 2)%3; // same as -1 (mod 3)
		int indexbelowbf2 = (f2b + 2)%3;

		Vertex* right = NULL;
		Vertex* ibf1 = &f1->v(indexbelowbf1);
		Vertex* ibf2 = &f2->v(indexbelowbf2);

		if (not (ibf1!=NULL and ibf2!=NULL))
		{
			error("ibf issue");
			return;
		}

		if (ibf1!=a)
			right = ibf1;
		else if (ibf2!=a)
			right = ibf2;

		if (right==NULL)
		{
			error("\"right\" issue");
			return;
		}

		// Have vertex "right" that is on the surface and falls to the right of a->b
		// 2. Find tetra tr that contains a,b, right
		Tetra* tr = o->mesh()->getTetra(a,b,right);
		if(tr==NULL)
		{
			error("tr == null!");
			return;
		}
		int vir = tr->vIndex(right);

		// 3. Start tetra enumeration with tr
		tetras.push_back(tr);
		vertexIndices.push_back(vir);
		bool foundloop = findTetras(tetras,vertexIndices,tr,vir,a,b,o);
		if(foundloop)
		{
			error("foundloop should be false, is true");
			return;
		}

		// 4. Return with results
		return;

	}
	else
	// the edge is internal..
	{
		// Find a tetra that contains a and b.
		Tetra* t = o->mesh()->getTetra(a,b);
		if (t==NULL)
		{
			error("t is null");
			return;
		}

		// Find the vertex of t that isn't a or b, and is on the "left" of the other opposite vertex
		int ofi[3];
		t->oppositeFaceIndices(t->vIndex(a),ofi[0],ofi[1],ofi[2]);
		int vi = -1;
		int vb = t->vIndex(b);
		for(int i=0;i<3;i++)
		{
			if (ofi[i]==vb)
			{
				vi = ofi[(i+1)%3];
				break;
			}
		}

		if (not (vi!=-1 and vi!=t->vIndex(a) and vi!=vb))
		{
			error("not (vi!=-1 and vi!=t->vIndex(a) and vi!=vb)");
			return;
		}

		// LOG tetra info
		LOG("Transform::enumerateSurroundingTetrasAndVertices, start tetra at: ");
		for(int i=0;i<4;i++)
			LOG(t->v(i).x() << " ");
		LOG("\n");

		bool foundloop = findTetras(tetras,vertexIndices,t,vi,a,b,o);
		if(!foundloop)
		{
			error("foundloop");
			return;
		}

		tetras.push_back(t);
		vertexIndices.push_back(vi);

		return;
	}
}

bool Transform::findTetras(std::vector<Tetra*>& X, std::vector<int>& V, Tetra* t, int vi, Vertex* e1, Vertex* e2, Organism* o)
{
	Edge* e = o->mesh()->getEdge(e1,e2);
	if (e==NULL)
	{
		error("e == NULL");
		return false;
	}

	// insert first element into X and V
	// X.push_back(t);
	// V.push_back(i);
	Tetra* LX = t;
	int LV = vi;

	if(not Mesh::doesTetraContainEdge(t,e))
	{
		error("tetra doesn't contain edge");
		return false;
	}

	while(true)
	{
		LOG("Transform::findTetras (LX,LV) = (" << LX << "," << LV << ") \n");

		if (LV < 0 or LV > 3)
		{
			state = TRANSFORM_ERROR;
			properties.add("LX",LX);
			properties.add(LV);
			return false;
		}

		Tetra* NX = LX->neighbour(LV);

		LOG("Transform::findTetras NX = " << NX << "\n");

		if (NX == NULL)
			return false; // out of while
		else if (NX == t)
			return true; // out of while
		else
		{
			if (not Mesh::doesTetraContainEdge(NX,e))
			{
				error("tetra doesn't contain edge");
				return false;
			}

			// find v
			Vertex* vjp1 = NULL;
			for(int k=0;k<3;k++)
			{
				Vertex* vk = &LX->v((LV+k+1)%4);
				if (not (vk==e1 or vk==e2))
				{
					vjp1 = vk;
					break;
				}
			}

			if (vjp1==NULL)
			{
				error("vjp1==NULL");
				return false;
			}

			LV = NX->vIndex(vjp1);
			LX = NX;

			if (std::find(X.begin(),X.end(),LX)!=X.end())
			{
				error("not std::find(X.begin(),X.end(),LX)==X.end()");
				return false;
			}

			V.push_back(LV);
			X.push_back(LX);
		}
	}

	return true;
}

void Transform::projectPoints(std::vector<Vertex*>& points, Vertex* u1, Vertex* u2, Vertex* v1, Vertex* v2, std::vector<Vector3d>& projected)
{

	// we first do a perspective projection from the viewpoint of u1 down to a plane at u2
	Vector3d u2mu1 = u2->x() - u1->x();
	double u2mu1dot = dot(u2mu1,u2mu1);

	BOOST_FOREACH(Vertex* v, points)
	{
		std::ostringstream oss;
		oss << "lv " << v->x();
		properties.add(oss.str(), v);

		Vector3d vmu1 = v->x() - u1->x();
		double t = u2mu1dot / dot(vmu1,u2mu1);
		if (t <= 0)
		{
			error("projectPoints: t <= 0");
			return;
		}
		projected.push_back(u1->x() + vmu1 * t);
	}

	// then we remove the redundant dimension and convert to cartesian basis
	// we remove the u dimension
	// XXX: we flip u because of the orientation of X1
	Vector3d uaxis = (u1->x() - u2->x()).normalise();
	Vector3d vdaxis = v1->x() - v2->x();
	Vector3d waxis = cross(uaxis,vdaxis).normalise();
	Vector3d vaxis = cross(uaxis,waxis);
	Matrix3d uvw(uaxis,vaxis,waxis);
	Matrix3d uvwi = uvw.inv();

	BOOST_FOREACH(Vector3d& v, projected)
	{
		Vector3d lvproj = uvwi * v;
		v = Vector3d(lvproj[1],lvproj[2],0); //swizzle coords

		std::ostringstream oss;
		oss << "lvproj " << v;
		properties.add(oss.str());
	}

}

void Transform::projectTransformedPoints(std::vector<Vector3d>& points, Vertex* u1, Vertex* u2, Vertex* v1, Vertex* v2, std::vector<Vector3d>& projected)
{
	// we first do a perspective projection from the viewpoint of u1 down to a plane at u2
	Vector3d u2mu1 = u2->x() - u1->x();
	double u2mu1dot = dot(u2mu1,u2mu1);

	std::ostringstream oss;
	//oss << "u1 " << u1->x() << " u2 " << u2->x();
	//properties.add(oss.str());

	BOOST_FOREACH(Vector3d& x, points)
	{
		oss.str("");
		oss << "lv " << x;
		properties.add(oss.str(),x);

		Vector3d vmu1 = x - u1->x();
		double t = u2mu1dot / dot(vmu1,u2mu1);
		//properties.add("t",t);
		if (t <= 0)
		{
			error("projectTransformedPoints: t <= 0");
			return;
		}
		Vector3d proj = u1->x() + vmu1 * t;

		oss << "proj";
		properties.add(oss.str(),proj);
		projected.push_back(proj);
	}

	// then we remove the redundant dimension and convert to cartesian basis
	// we remove the u dimension
	// XXX: we flip u because of the orientation of X1
	Vector3d uaxis = (u1->x() - u2->x()).normalise();
	Vector3d vdaxis = v1->x() - v2->x();
	Vector3d waxis = cross(uaxis,vdaxis).normalise();
	Vector3d vaxis = cross(uaxis,waxis);
	Matrix3d uvw(uaxis,vaxis,waxis);
	Matrix3d uvwi = uvw.inv();

	BOOST_FOREACH(Vector3d& v, projected)
	{
		Vector3d lvproj = uvwi * v;
		v = Vector3d(lvproj[1],lvproj[2],0); //swizzle coords

		std::ostringstream oss;
		oss << "lvproj " << v;
		properties.add(oss.str());
	}
}

bool Transform::findTriangulation(std::vector<Vertex*>& points, Vertex* u1, Vertex* u2, Vertex* v1, Vertex* v2, std::vector<Vector3i>& triangulation)
{
	if (triangulation.size()!=0)
	{
		error("triangulation size > 0");
		return false;
	}

	// project points from u1 to u2
	std::vector<Vector3d> u1u2proj, u2u1proj;

	// but first transform the points such that they all lie between u1 and u2
	std::vector<Vector3d> tpoints;
	Vector3d c = (u2->x() + u1->x())/2;
	double dist = std::min((u2->x() - c).length(),(u1->x()-c).length());
	double amountToScale = 1;

	// calculate amount to scale
	BOOST_FOREACH(Vertex* v, points)
	{
		double cv = (v->x() - c).length();
		if (cv > dist)
		{
			amountToScale = std::min(amountToScale,0.5 * dist/cv);
		}
	}

	BOOST_FOREACH(Vertex* v, points)
	{
		tpoints.push_back((v->x()-c)*amountToScale + c);
	}

	projectTransformedPoints(tpoints,u1,u2,v1,v2,u1u2proj);
	if (hasError()) return false;

	projectTransformedPoints(tpoints,u2,u1,v1,v2,u2u1proj);
	if (hasError()) return false;

	// flip the u2u1proj points about the x axis
	BOOST_FOREACH(Vector3d& p, u2u1proj) p.y(-p.y());


	// triangulate
	if (!Math::commonTriangulation(u1u2proj,u2u1proj,triangulation))
	{
		//assert(false);
		return false;
	}

	// print out triangles

	for(unsigned int index=0; index<triangulation.size(); index++)
	{
		std::ostringstream oss;
		oss << "triangle " << index;
		const Vector3i& face = triangulation[index];
		Vertex* verts[3] = {points[face[0]],points[face[1]],points[face[2]]};
		properties.add(oss.str(),boost::make_tuple(verts[0],verts[1],verts[2]));
	}

	return true;
}










