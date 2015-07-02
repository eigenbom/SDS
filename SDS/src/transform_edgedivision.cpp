#include "transform.h"

#include <boost/foreach.hpp>

#define DEBUG_DIVIDEINTERNALEDGE

#ifndef DEBUG_DIVIDEINTERNALEDGE
	#undef LOG
	#define LOG(x) ;
#endif


boost::tuple<Cell*, Cell*> Transform::DivideInternalEdge(Cell* ca, Edge* e, Organism* o)
{
	reset();
	called("DivideInternalEdge(Cell,Edge)");

	Mesh* m = o->mesh();

	Vertex* a = ca->v();
	Vertex* b = e->v(0)==a?e->v(1):e->v(0);
	Cell* cb = o->getAssociatedCell(b);

	double newMass = a->m()/2;
	ca->setR(Math::radiusOfSphereGivenVolume(newMass));

	// enumerate vertices and tetrahedra clockwise when looking from a toward b
	std::vector<Tetra*> tetras;
	std::vector<int> vertexIndices;
	std::vector<Vertex*> vertices;
	Transform::enumerateSurroundingTetrasAndVertices(a,b,o,  tetras,vertexIndices);

	if (hasError()) return error();

	LOG("Transform::DivideInternalEdge: " <<
			"tetras.size()=" << tetras.size() << "\n" <<
			"vertexIndices.size()=" << vertexIndices.size() << "\n"
			);

	for(int i=0;i<tetras.size();++i)
		vertices.push_back(&tetras[i]->v(vertexIndices[i]));

	// enumerate tetra neighbours
	// tetra[i] is joined to tetra tia via a, to tib via b
	std::vector<Tetra*> tia;
	std::vector<Tetra*> tib;

	BOOST_FOREACH(Tetra* t, tetras)
	{
		tia.push_back(t->neighbour(a));
		tib.push_back(t->neighbour(b));
	}

	// delete edges and tetras
	// only need to delete edge e, keep the rest for later...
	m->deleteEdge(e);
	e = NULL;

	// add new cell at pos a+b/2
	Cell* cc = new Cell((a->x()+b->x())/2,newMass);
	Vertex* c = cc->v();
	m->addVertex(c);
	o->addCell(cc);

	// add edges ac, bc
	o->connectCells(ca,cc);
	o->connectCells(cb,cc);

	// add edges cv_i for all v_i
	BOOST_FOREACH(Vertex* vi, vertices)
		o->connectCells(cc,o->getAssociatedCell(vi));

	// add tetras
	std::vector<Tetra*> newTetrasA, newTetrasB;
	for(int i=0;i<vertices.size();++i)
	{
		Vertex *vi = vertices[i], *vip1 = vertices[(i+1)%vertices.size()];

		// vi,vi+1,a,c
		// vi,vi+1,c,b
		Tetra* ta = new Tetra(vi,vip1,a,c,Transform::tetrahedraRestVolume(vi,vip1,a,c,o));
		Tetra* tb = new Tetra(vi,vip1,c,b,Transform::tetrahedraRestVolume(vi,vip1,c,b,o));
		// add tetras to mesh
		m->addTetra(ta);
		m->addTetra(tb);

		// set up outside neighbours, if any...
		Tetra* tna = tia[i];
		Tetra* tnb = tib[i];
		Tetra* t = tetras[i];

		// print out tna,tnb, and t to see if they have verts in common,
		// which they definitely should
		LOG("ta: " << &ta->v(0) << " " << &ta->v(1) << " "<< &ta->v(2) << " "<< &ta->v(3) << "\n");
		LOG("tb: " << &tb->v(0) << " " << &tb->v(1) << " "<< &tb->v(2) << " "<< &tb->v(3) << "\n");
		LOG("t: " << &t->v(0) << " " << &t->v(1) << " "<< &t->v(2) << " "<< &t->v(3) << "\n");

		ta->setNeighbour(c,tnb);
		tb->setNeighbour(c,tna);
		if (tna!=NULL) tna->setNeighbour(tna->getNeighbourIndex(t),tb);
		if (tnb!=NULL) tnb->setNeighbour(tnb->getNeighbourIndex(t),ta);

		// set up pairwise neighbourhoods
		ta->setNeighbour(a,tb);
		tb->setNeighbour(b,ta);

		newTetrasA.push_back(ta);
		newTetrasB.push_back(tb);

		LOG("Added tets " << ta << " and " << tb << " (of vert" << i << ")\n");
		LOG("Outside neighbourhood:\n");
		if (tnb)
		{
			LOG("\ttnb\n\t");
			LOG(ta << " --" << &ta->v(ta->getNeighbourIndex(tnb)) << "--> " << tnb << "\n\t");
			LOG(tnb << " --" << &tnb->v(tnb->getNeighbourIndex(ta)) << "--> " << ta << "\n");
		}
		if (tna)
		{
			LOG("\ttna\n\t");
			LOG(tb << " --" << &tb->v(tb->getNeighbourIndex(tna)) << "--> " << tna << "\n\t");
			LOG(tna << " --" << &tna->v(tna->getNeighbourIndex(tb)) << "--> " << tb << "\n");
		}
	}

	// set forward-back neighbours
	for(int i=0;i<vertices.size();i++)
	{
		Vertex *vi = vertices[i], *vip1 = vertices[(i+1)%vertices.size()];

		Tetra *tai = newTetrasA[i], *taip1 = newTetrasA[(i+1)%newTetrasA.size()], *taim1 = newTetrasA[(i+newTetrasA.size()-1)%newTetrasA.size()];
		Tetra *tbi = newTetrasB[i], *tbip1 = newTetrasB[(i+1)%newTetrasB.size()], *tbim1 = newTetrasB[(i+newTetrasB.size()-1)%newTetrasB.size()];

		tai->setNeighbour(vi,taip1);
		tai->setNeighbour(vip1,taim1);
		tbi->setNeighbour(vi,tbip1);
		tbi->setNeighbour(vip1,tbim1);

		LOG("\ntai " << i << "\n");
		LOG("Connecting " << tai << " to " << taip1 << " via " << vi);
		LOG("\nConnecting " << tai << " to " << taim1 << " via " << vip1);
		LOG("\ntbi " << i << "\n");
		LOG("Connecting " << tbi << " to " << tbip1 << " via " << vi);
		LOG("\nConnecting " << tbi << " to " << tbim1 << " via " << vip1);

	}

	// remove old tetras
	BOOST_FOREACH(Tetra* t, tetras)
	{
		m->removeTetra(t);
		delete t;
	}
	tetras.clear();

	// IS THAT IT?

	// debug check
	#ifdef DEBUG
		if (not m->isSane())
		{
			return error("Mesh not sane after dividing internal edge.");
		}
	#endif

	finaliseProperties();
	return boost::make_tuple(ca,cc);
}

/// Division into an adjacent edge
// PRE: e is on the surface
// PRE: c->v \in e
boost::tuple<Cell*, Cell*> Transform::DivideSurfaceEdge(Cell* ca, Edge* e, Organism* o)
{
	reset();
	called("DivideSurfaceEdge(Cell,Edge)");

	// init etc
	Mesh* m = o->mesh();
	Vertex* a = ca->v();
	Vertex* b = e->v(0)==a?e->v(1):e->v(0);
	Cell* cb = o->getAssociatedCell(b);
	double newMass = a->m()/2;
	ca->setR(Math::radiusOfSphereGivenVolume(newMass));

	// enumerate tetras
	std::vector<Tetra*> tetras;
	std::vector<int> vertexIndices;
	Transform::enumerateSurroundingTetrasAndVertices(a,b,o,tetras,vertexIndices);

	if (hasError()) return error();

	LOG("Transform::DivideSurfaceEdge: " <<
			"tetras.size()=" << tetras.size() << "\n" <<
			"vertexIndices.size()=" << vertexIndices.size() << "\n"
			);

	// build list of all the vertices
	// NOTE: |vertices| = |tetras|+1
	std::vector<Vertex*> vertices;
	for(int i=0;i<tetras.size();++i)
		vertices.push_back(&tetras[i]->v(vertexIndices[i]));

	// add last vertex ... (block because of temp vars)
	{
		Tetra* _lt = tetras[tetras.size()-1];
		int _lv = 6 - (_lt->vIndex(a) + _lt->vIndex(b) + vertexIndices[tetras.size()-1]);
		vertices.push_back(&_lt->v(_lv));
	}

	// enumerate tetra neighbours
	// tetra[i] is joined to tetra tia via a, to tib via b
	std::vector<Tetra*> tia;
	std::vector<Tetra*> tib;

	BOOST_FOREACH(Tetra* t, tetras)
	{
		tia.push_back(t->neighbour(a));
		tib.push_back(t->neighbour(b));
	}

	LOG("Transform::DivideSurfaceEdge: deleting edges and surface faces\n");

	// delete edge e
	m->deleteEdge(e);
	e = NULL;

	// delete surface faces
	Face *f1 = m->getFace(a,b,vertices[0]), *f2 = m->getFace(a,b,vertices[vertices.size()-1]);
	if (not (f1!=NULL and f2!=NULL))
	{
		return error("f1==NULL || f2==NULL");
	}

		// delete f1
		m->removeFace(f1);
		a->removeFaceNeighbour(f1);
		b->removeFaceNeighbour(f1);
		vertices[0]->removeFaceNeighbour(f1);
		delete f1;

		// delete f2
		m->removeFace(f2);
		a->removeFaceNeighbour(f2);
		b->removeFaceNeighbour(f2);
		vertices[vertices.size()-1]->removeFaceNeighbour(f2);
		delete f2;


	LOG("Transform::DivideSurfaceEdge: adding new cell.\n");

	// add new cell at pos a+b/2
	Cell* cc = new Cell((a->x()+b->x())/2,newMass);
	Vertex* c = cc->v();
	c->setSurface(true);
	m->addVertex(c);
	o->addCell(cc);

	// add edges ac, bc
	o->connectCells(ca,cc);
	o->connectCells(cb,cc);

	// add edges cv_i for all v_i
	BOOST_FOREACH(Vertex* vi, vertices)
		o->connectCells(cc,o->getAssociatedCell(vi));

	LOG("Transform::DivideSurfaceEdge: adding new tetras\n");

	// add new tetras
	std::vector<Tetra*> newTetrasA, newTetrasB;
	for(int i=0;i<vertices.size()-1;++i)
	{
		Vertex *vi = vertices[i], *vip1 = vertices[i+1];

		// vi,vi+1,a,c
		// vi,vi+1,c,b
		Tetra* ta = new Tetra(vi,vip1,a,c,Transform::tetrahedraRestVolume(vi,vip1,a,c,o));
		Tetra* tb = new Tetra(vi,vip1,c,b,Transform::tetrahedraRestVolume(vi,vip1,c,b,o));

		// add end pieces (surface faces) if we are at the end
		if (i==0)
		{
			Face *f1 = new Face(a,vi,c), *f2 = new Face(c,vi,b);
			m->addOuterFace(f1);
			m->addOuterFace(f2);

			// add surface face neighbours
			a->addFaceNeighbour(f1);
			vi->addFaceNeighbour(f1);
			c->addFaceNeighbour(f1);

			b->addFaceNeighbour(f2);
			vi->addFaceNeighbour(f2);
			c->addFaceNeighbour(f2);
		}
		else if (i==vertices.size()-2)
		{
			Face *f1 = new Face(a,c,vip1), *f2 = new Face(c,b,vip1);
			m->addOuterFace(f1);
			m->addOuterFace(f2);

			// add surface face neighbours
			a->addFaceNeighbour(f1);
			vip1->addFaceNeighbour(f1);
			c->addFaceNeighbour(f1);

			b->addFaceNeighbour(f2);
			vip1->addFaceNeighbour(f2);
			c->addFaceNeighbour(f2);
		}

		LOG("Transform::DivideSurfaceEdge: adding tet " << i << " \n");

		// add tetras to mesh
		m->addTetra(ta);
		m->addTetra(tb);

		// set up outside neighbours, if any...
		Tetra* tna = tia[i];
		Tetra* tnb = tib[i];
		Tetra* t = tetras[i];

		// print out tna,tnb, and t to see if they have verts in common,
		// which they definitely should
		LOG("ta: " << &ta->v(0) << " " << &ta->v(1) << " "<< &ta->v(2) << " "<< &ta->v(3) << "\n");
		LOG("tb: " << &tb->v(0) << " " << &tb->v(1) << " "<< &tb->v(2) << " "<< &tb->v(3) << "\n");
		LOG("t: " << &t->v(0) << " " << &t->v(1) << " "<< &t->v(2) << " "<< &t->v(3) << "\n");

		ta->setNeighbour(c,tnb);
		tb->setNeighbour(c,tna);
		if (tna!=NULL) tna->setNeighbour(tna->getNeighbourIndex(t),tb);
		if (tnb!=NULL) tnb->setNeighbour(tnb->getNeighbourIndex(t),ta);

		// set up pairwise neighbourhoods
		ta->setNeighbour(a,tb);
		tb->setNeighbour(b,ta);

		newTetrasA.push_back(ta);
		newTetrasB.push_back(tb);

		LOG("Added tets " << ta << " and " << tb << " (of vert" << i << ")\n");
		LOG("Outside neighbourhood:\n");
		if (tnb)
		{
			LOG("\ttnb\n\t");
			LOG(ta << " --" << &ta->v(ta->getNeighbourIndex(tnb)) << "--> " << tnb << "\n\t");
			LOG(tnb << " --" << &tnb->v(tnb->getNeighbourIndex(ta)) << "--> " << ta << "\n");
		}
		if (tna)
		{
			LOG("\ttna\n\t");
			LOG(tb << " --" << &tb->v(tb->getNeighbourIndex(tna)) << "--> " << tna << "\n\t");
			LOG(tna << " --" << &tna->v(tna->getNeighbourIndex(tb)) << "--> " << tb << "\n");
		}
	}

	LOG("Transform::DivideSurfaceEdge: setting tetra neighbours\n");

	// set forward-back neighbours
	for(int i=0;i<vertices.size()-1;i++)
	{
		Vertex *vi = vertices[i], *vip1 = vertices[i+1];

		Tetra *tai = newTetrasA[i];
		Tetra *tbi = newTetrasB[i];

		Tetra *taip1 = (i==vertices.size()-2)?NULL:newTetrasA[i+1];
		Tetra *tbip1 = (i==vertices.size()-2)?NULL:newTetrasB[i+1];

		Tetra *taim1 = (i==0)?NULL:newTetrasA[i-1];
		Tetra *tbim1 = (i==0)?NULL:newTetrasB[i-1];

		tai->setNeighbour(vi,taip1);
		tai->setNeighbour(vip1,taim1);
		tbi->setNeighbour(vi,tbip1);
		tbi->setNeighbour(vip1,tbim1);

		LOG("\ntai " << i << "\n");
		LOG("Connecting " << tai << " to " << taip1 << " via " << vi);
		LOG("\nConnecting " << tai << " to " << taim1 << " via " << vip1);
		LOG("\ntbi " << i << "\n");
		LOG("Connecting " << tbi << " to " << tbip1 << " via " << vi);
		LOG("\nConnecting " << tbi << " to " << tbim1 << " via " << vip1);
	}

	// remove old tetras
	BOOST_FOREACH(Tetra* t, tetras)
	{
		m->removeTetra(t);
		delete t;
	}

	// debug check
	#ifdef DEBUG
		if (not (m->isSane()))
		{
			return error("Mesh not sane after dividing surface edge");
		}
	#endif

	finaliseProperties();

	LOG("Done!\n");
	return boost::make_tuple(ca,cc);
}
