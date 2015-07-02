#include "transform.h"

#include "hmath.h"
#include "vector3.h"

#include <cfloat>

boost::tuple<Cell*,Cell*> Transform::Divide(Cell* c, Vector3d dir, Organism* o)
{
	return Transform::DivideBalanced(c,dir,o);
}

boost::tuple<Cell*,Cell*> Transform::DivideSimple(Cell* c, Vector3d dir, Organism* o)
{
	dir.normaliseInPlace();
	Mesh* m = o->mesh();

	if (c->isBoundary())
	{
		// get the neighbour cell closest to direction dir
		Cell* nc = NULL;
		double ncang = DBL_MAX;
		BOOST_FOREACH(Cell* n, o->getNeighbours(c))
		{
			if (n->isBoundary())
			{
				double ang = Math::acos(dot((n->x() - c->x()).normalise(),dir));
				if (ang<ncang)
				{
					ncang = ang;
					nc = n;
				}
			}
		}

		if (!nc) return error("no neighbour cell in direction dir");

		if (ncang < sDivideInternalAngleThreshold) // divide an edge
		{
			return Transform::DivideSurfaceEdge(c,o->edge(c,nc),o);
		}
		else // divide a face
		{
			// Find the face in the direction given
			Face* f = Transform::getFaceInDirection(c,dir,o);
			if (f!=NULL)
			{
				return Transform::DivideTetra(c,f,o);
			}

			// else, must be an internal divide, so do it!
			// TODO: What about DivideAway?
		}
	}

	// else, divide into the complex

	// get the neighbour cell closest to direction dir
	Cell* nc = NULL;
	double ncang = DBL_MAX;
	BOOST_FOREACH(Cell* n, o->getNeighbours(c))
	{
		double ang = Math::acos(dot((n->x() - c->x()).normalise(),dir));
		if (ang<ncang)
		{
			ncang = ang;
			nc = n;
		}
	}

	if (ncang < sDivideInternalAngleThreshold)
	{
		return Transform::DivideInternalEdge(c,o->edge(c,nc),o);
	}
	else // divide a tetra instead...
	{
		Tetra* ti = Transform::getTetraInDirection(c,dir,o);
		if (ti==NULL)
		{
			// this shouldn't happen, but if it does, then just choose any tetra
			ti = *m->getNeighbours(c->v()).begin();
			LOG("c.x: " << c->x() << " dir: " << dir);
		}
		// assert(ti!=NULL);

		// 2. Divide into ti
		return Transform::DivideTetra(c,ti,o);
	}
}

boost::tuple<Cell*,Cell*> Transform::DivideTetra(Cell* c, Tetra* t, Organism* o)
{
	reset();
	called("DivideTetra(Cell,Tetra)");

	Mesh* m = o->mesh();
	Vertex* v = c->v();
	Vertex *v0 = &t->v(0), *v1 = &t->v(1), *v2 = &t->v(2), *v3 = &t->v(3);
	Cell* cs[4] = {o->getAssociatedCell(v0),
		o->getAssociatedCell(v1),
		o->getAssociatedCell(v2),
		o->getAssociatedCell(v3)};

	// NOTE: n0..n3 could be NULL
	Tetra *n0 = t->neighbour(0),
		*n1 = t->neighbour(1),
		*n2 = t->neighbour(2),
		*n3 = t->neighbour(3);

	// compute new mass and radius of cells
	double oldMass = c->m();
	double newMass = oldMass/2;
	double newRadius = Math::radiusOfSphereGivenVolume(newMass);

	c->setR(newRadius);
	c->setM(newMass);

	// Create the new Cell at the center of mass of the tetra
	Vector3d com = t->centerOfMass();
	Vertex* x = new Vertex(com,newMass);
	x->setSurface(false);

	Cell* newcell = new Cell(x,newRadius);

	// add a new cell to the organism, and the vertex to the mesh
	// XXX: should bind these two calls together
	o->addCell(newcell);
	m->addVertex(x);

	x->addNeighbour(v0);
	x->addNeighbour(v1);
	x->addNeighbour(v2);
	x->addNeighbour(v3);

	v0->addNeighbour(x);
	v1->addNeighbour(x);
	v2->addNeighbour(x);
	v3->addNeighbour(x);

	//v->addNeighbour(x);

	// Create the 4 new tetrahedra
	Tetra *t0 = new Tetra(x,v1,v2,v3,tetrahedraRestVolume(newcell,cs[1],cs[2],cs[3])),
			*t1 = new Tetra(v0,x,v2,v3,tetrahedraRestVolume(cs[0],newcell,cs[2],cs[3])),
			*t2 = new Tetra(v0,v1,x,v3,tetrahedraRestVolume(cs[0],cs[1],newcell,cs[3])),
			*t3 = new Tetra(v0,v1,v2,x,tetrahedraRestVolume(cs[0],cs[1],cs[2],newcell));

	t0->setNeighbours(n0,t1,t2,t3);
	t1->setNeighbours(t0,n1,t2,t3);
	t2->setNeighbours(t0,t1,n2,t3);
	t3->setNeighbours(t0,t1,t2,n3);

	if (n0)	n0->setNeighbour(n0->getNeighbourIndex(t),t0);
	if (n1)	n1->setNeighbour(n1->getNeighbourIndex(t),t1);
	if (n2)	n2->setNeighbour(n2->getNeighbourIndex(t),t2);
	if (n3)	n3->setNeighbour(n3->getNeighbourIndex(t),t3);

	m->removeTetra(t);
	delete t;

	m->addTetra(t0);
	m->addTetra(t1);
	m->addTetra(t2);
	m->addTetra(t3);

	// create the faces
	// XXX: wasting cycles setting the rest area in the Face constructor
	/*
	 * NO INTERNAL FACES NECESSARY
	Face *f0 = new Face(v0,x,v3),
		*f1 = new Face(v1,x,v3),
		*f2 = new Face(v2,x,v3),
		*f3 = new Face(v0,x,v2),
		*f4 = new Face(v0,v1,x),
		*f5 = new Face(v1,v2,x);
	f0->setOuter(false);
	f1->setOuter(false);
	f2->setOuter(false);
	f3->setOuter(false);
	f4->setOuter(false);
	f5->setOuter(false);

	m->addFace(f0);
	m->addFace(f1);
	m->addFace(f2);
	m->addFace(f3);
	m->addFace(f4);
	m->addFace(f5);
	*/

	// create the edges
	Edge *e0 = new Edge(v0,x,cs[0]->r()+x->r()),
		*e1 = new Edge(v1,x,cs[1]->r()+x->r()),
		*e2 = new Edge(v2,x,cs[2]->r()+x->r()),
		*e3 = new Edge(v3,x,cs[3]->r()+x->r());

	m->addEdge(e0);
	m->addEdge(e1);
	m->addEdge(e2);
	m->addEdge(e3);

	// adjust the rest lengths of all edges connected to c
	BOOST_FOREACH(Vertex* vn, v->neighbours())
	{
		Cell* neighbourCell = o->getAssociatedCell(vn);
		if (!neighbourCell) return error("neighbour vertex has no corresponding cell");

		Edge* e = o->edge(c,neighbourCell);
		if (!e) return error("no edge between vertex and neighbour");
		e->setRest(c->r() + neighbourCell->r());
	}

	finaliseProperties();
	return boost::tuple<Cell*,Cell*>(c,newcell);
}

boost::tuple<Cell*,Cell*> Transform::DivideTetra(Cell* c, Face* f, Organism* o)
{
	reset();
	called("DivideTetra(Cell,Face)");

	Mesh* m = o->mesh();
	Vertex* v = c->v();
	int iv0 = f->index(v);

	if (iv0<0 or iv0>=3)
	{
		LOG("DivideTetra: error");

		state = TRANSFORM_ERROR;
		Transform::properties.add("error vertex v not in face f");
		Transform::properties.add("v",v);
		Transform::properties.add("f",f);
		return boost::make_tuple<Cell*,Cell*>(NULL,NULL);
	}

	int iv1 = (iv0+1)%3;
	int iv2 = (iv1+1)%3;

	Vertex *v0 = &f->v(iv0), *v1 = &f->v(iv1), *v2 = &f->v(iv2);

	// Find the tetrahedra connected to this face
	Tetra* t = m->getTetra(v0,v1,v2);
	if (!t) return error("No tetrahedra connected to opposite face of vertex");

	Vertex* v3 = &t->v(6 - (t->vIndex(v0)+t->vIndex(v1)+t->vIndex(v2)));

	Tetra* tn[3] = {t->neighbour(v0),t->neighbour(v1),t->neighbour(v2)};

	// compute center of mass of f
	Cell *c0 = o->getAssociatedCell(v0),
		*c1 = o->getAssociatedCell(v1),
		*c2 = o->getAssociatedCell(v2),
		*c3 = o->getAssociatedCell(v3);
	Vector3d com = (v0->x()*v0->m() + v1->x()*v1->m() + v2->x()*v2->m()) / (v0->m() + v1->m() + v2->m());

	// compute new mass and radius of cells
	double oldMass = c->m();
	double newMass = oldMass/2;
	double newRadius = Math::radiusOfSphereGivenVolume(newMass);

	c->setR(newRadius);
	c->setM(newMass);

	Vertex* vdash = new Vertex(com,newMass);
	Cell* cdash = new Cell(vdash,newRadius);

	m->addVertex(vdash);
	vdash->setSurface(true);
	o->addCell(cdash);

	// add edges
	Edge *e0 = new Edge(vdash,v0,cdash->r() + c0->r()),
		*e1 = new Edge(vdash,v1,cdash->r() + c1->r()),
		*e2 = new Edge(vdash,v2,cdash->r() + c2->r()),
		*e3 = new Edge(vdash,v3,cdash->r() + c3->r());

	m->addEdge(e0);
	m->addEdge(e1);
	m->addEdge(e2);
	m->addEdge(e3);

	vdash->addNeighbour(v0);
	vdash->addNeighbour(v1);
	vdash->addNeighbour(v2);
	vdash->addNeighbour(v3);

	v0->addNeighbour(vdash);
	v1->addNeighbour(vdash);
	v2->addNeighbour(vdash);
	v3->addNeighbour(vdash);

	// add new surface faces
	Face *f0 = new Face(v1,v2,vdash),
		*f1 = new Face(v2,v0,vdash),
		*f2 = new Face(v0,v1,vdash);

	m->addOuterFace(f0);
	m->addOuterFace(f1);
	m->addOuterFace(f2);

	// fix topologies

	v0->addFaceNeighbour(f1);
	v0->addFaceNeighbour(f2);

	v1->addFaceNeighbour(f0);
	v1->addFaceNeighbour(f2);

	v2->addFaceNeighbour(f0);
	v2->addFaceNeighbour(f1);

	vdash->addFaceNeighbour(f0);
	vdash->addFaceNeighbour(f1);
	vdash->addFaceNeighbour(f2);

	// remove and delete old face

	m->removeFace(f);
	v0->removeFaceNeighbour(f);
	v1->removeFaceNeighbour(f);
	v2->removeFaceNeighbour(f);
	delete f;

	// add new tetra

	Tetra* t0 = new Tetra(v3,v1,v2,vdash,Transform::tetrahedraRestVolume(c3,c1,c2,cdash));
	Tetra* t1 = new Tetra(v3,v0,vdash,v2,Transform::tetrahedraRestVolume(c3,c0,cdash,c2));
	Tetra* t2 = new Tetra(v3,v0,v1,vdash,Transform::tetrahedraRestVolume(c3,c0,c1,cdash));

	m->addTetra(t0);
	m->addTetra(t1);
	m->addTetra(t2);

	t0->setNeighbour(1,t1);
	t0->setNeighbour(2,t2);
	t0->setNeighbour(3,tn[0]); if (tn[0]) tn[0]->replaceNeighbour(t,t0);

	t1->setNeighbour(1,t0);
	t1->setNeighbour(2,tn[1]); if (tn[1]) tn[1]->replaceNeighbour(t,t1);
	t1->setNeighbour(3,t2);

	t2->setNeighbour(1,t0);
	t2->setNeighbour(2,t1);
	t2->setNeighbour(3,tn[2]); if (tn[2]) tn[2]->replaceNeighbour(t,t2);

	// delete old tetra
	m->removeTetra(t);
	delete t;

	finaliseProperties();
	return boost::make_tuple(c,cdash);
}


boost::tuple<Cell*, Cell*> Transform::DivideAway(Cell* c, Face* f, Organism* o)
{
	reset();
	called("DivideAway(Cell,Face)");

	Mesh* m = o->mesh();
	Vertex* v = c->v();
	int iv0 = f->index(v);

	if (iv0<0 or iv0>=3)
	{
		state = TRANSFORM_ERROR;
		Transform::properties.add("error vertex v not in face f");
		Transform::properties.add("v",v);
		Transform::properties.add("f",f);
		return boost::make_tuple<Cell*,Cell*>(NULL,NULL);
	}
	int iv1 = (iv0+1)%3;
	int iv2 = (iv1+1)%3;

	Vertex *v0 = &f->v(iv0), *v1 = &f->v(iv1), *v2 = &f->v(iv2);
	Cell *c0 = o->getAssociatedCell(v0),
		*c1 = o->getAssociatedCell(v1),
		*c2 = o->getAssociatedCell(v2);

	// add a new cell
	double newMass = c->m()/2;
	double newRadius = Math::radiusOfSphereGivenVolume(newMass);
	c->setM(newMass);
	c->setR(newRadius);

	Vertex* vnew = new Vertex(f->center() + f->n().normalise() * c->r(),newMass);
	vnew->setSurface(true);
	m->addVertex(vnew);

	Cell* cnew = new Cell(vnew,newRadius);
	o->addCell(cnew);

	// add 3 new surface faces
	m->removeFace(f);
	v0->removeFaceNeighbour(f);
	v1->removeFaceNeighbour(f);
	v2->removeFaceNeighbour(f);
	delete f;

	Face *f2 = new Face(v0,v1,vnew), *f1 = new Face(v2,v0,vnew), *f0 = new Face(v1,v2,vnew);
	m->addOuterFace(f0);
	m->addOuterFace(f1);
	m->addOuterFace(f2);

	for(int i=0;i<3;i++)
	{
		f0->v(i).addFaceNeighbour(f0);
		f1->v(i).addFaceNeighbour(f1);
		f2->v(i).addFaceNeighbour(f2);
	}

	// add new Tetrahedra

	Tetra* tnew = new Tetra(vnew,v2,v1,v0,tetrahedraRestVolume(cnew,c0,c1,c2));
	m->addTetra(tnew);

	// Find the tetrahedra connected to face f
	Tetra* t = NULL;
	BOOST_FOREACH(Tetra* tet, m->tetras())
	{
		if (tet->contains(v0) and tet->contains(v1) and tet->contains(v2))
		{
			t = tet;
			break;
		}
	}
	if (!t) return error("No tetrahedron connected to a face. I was expecting one.");

	t->setNeighbour(6 - (t->vIndex(v0)+t->vIndex(v1)+t->vIndex(v2)),tnew);
	tnew->setNeighbour(0,t);

	// add new edges
	Edge *e0 = new Edge(vnew,v0,cnew->r() + c0->r());
	Edge *e1 = new Edge(vnew,v1,cnew->r() + c1->r());
	Edge *e2 = new Edge(vnew,v2,cnew->r() + c2->r());

	e0->v(0)->addNeighbour(e0->v(1)); e0->v(1)->addNeighbour(e0->v(0));
	e1->v(0)->addNeighbour(e1->v(1)); e1->v(1)->addNeighbour(e1->v(0));
	e2->v(0)->addNeighbour(e2->v(1)); e2->v(1)->addNeighbour(e2->v(0));

	m->addEdge(e0);
	m->addEdge(e1);
	m->addEdge(e2);

	finaliseProperties();
	return boost::make_tuple(c,cnew);

}

/// Division in a specific direction
/// This algorithm either divides an edge or a tetra,
/// depending on whether there is a neighbouring cell in direction dir..
boost::tuple<Cell*, Cell*> Transform::DivideInternal(Cell* c, Vector3d dir, Organism* o)
{
	dir.normaliseInPlace();
	Mesh* m = o->mesh();

	// get the neighbour cell closest to direction dir
	Cell* nc = NULL;
	double ncang = DBL_MAX;
	BOOST_FOREACH(Cell* n, o->getNeighbours(c))
	{
		double ang = Math::acos(dot((n->x() - c->x()).normalise(),dir));
		if (ang<ncang)
		{
			ncang = ang;
			nc = n;
		}
	}

	if (ncang < sDivideInternalAngleThreshold)
	{
		return Transform::DivideInternalEdge(c,o->edge(c,nc),o);
	}
	else // divide a tetra instead...
	{
		Tetra* ti = Transform::getTetraInDirection(c,dir,o);
		if (ti==NULL)
		{
			// this shouldn't happen, but if it does, then just choose any tetra
			ti = *m->getNeighbours(c->v()).begin();
			LOG("c.x: " << c->x() << " dir: " << dir);
		}
		// assert(ti!=NULL);

		// 2. Divide into ti
		return Transform::DivideTetra(c,ti,o);
	}
}

/// Division along surface in a specific direction (external cell)
// PRE: c is a boundary cell
boost::tuple<Cell*, Cell*> Transform::DivideAlong(Cell* c, Vector3d dir, Organism* o)
{
	called("DivideAlong()");

	if (not c->isBoundary()) return error("cell is not on surface, but wants to divide along surface");

	// get the neighbour cell closest to direction dir
	Cell* nc = NULL;
	double ncang = DBL_MAX;
	BOOST_FOREACH(Cell* n, o->getNeighbours(c))
	{
		if (n->isBoundary())
		{
			double ang = Math::acos(dot((n->x() - c->x()).normalise(),dir));
			if (ang<ncang)
			{
				ncang = ang;
				nc = n;
			}
		}
	}

	if (!nc) return error("can't find neighbour cell on surface in chosen direction");

	if (ncang < sDivideInternalAngleThreshold) // divide an edge
	{
		return Transform::DivideSurfaceEdge(c,o->edge(c,nc),o);
	}
	else // divide a face
	{
		// Find the face in the direction given
		Face* f = Transform::getFaceInDirection(c,dir,o);
		if (f==NULL)
		{
			// log an error and halt...
			LOG("DivideAlong() error");
			properties.add("DivideAlong() dir incorrect");
			properties.add("cell",c);
			properties.add("dir",boost::make_tuple(c->x(),c->x()+dir));
			return error("Cannot find face in specified direction");
		}

		return Transform::DivideTetra(c,f,o);
	}
}
