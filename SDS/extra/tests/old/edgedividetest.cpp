#include "organism.h"
#include "transform.h"

#include "mesh.h"
#include "vertex.h"
#include "meshtester.h"
#include "meshloader.h"
#include "test.h"

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <list>
#include <algorithm>
#include <iterator>

void edgeDivideTest()
{
	std::cout << "Test: SDS::Transform::DivideInternalEdge" << std::endl;

	Mesh* m = MeshLoader::Load("sdstests/lattice222.1"); //note CWD
	sdstest(m!=NULL,"Mesh didn't load correctly!");
	sdstest(m->isSane(),"Mesh isn't sane!");

	Organism* o = new Organism(m);
	BOOST_FOREACH(Vertex* v,m->vertices())
		o->addCell(new Cell(v));

	// an internal edge connects node 12 to node 13
	std::list<Vertex*>::const_iterator iter = m->vertices().begin();
	std::advance(iter,12);
	Vertex *v0 = *iter;
	std::advance(iter,1);
	Vertex *v1 = *iter;
	assert(v0);
	assert(v1);

	//std::cout << "Selected verts at " << v0->x() << " and " << v1->x() << "\n";

	Edge* internalEdge = m->getEdge(v0,v1);
	assert(internalEdge);

	Transform::DivideInternalEdge(o->getAssociatedCell(v0),internalEdge,o);
	sdstest(m->isSane(),"Mesh isn't sane!");

	std::cout << "Test complete.\n";
}
