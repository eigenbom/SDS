#include "meshops.h"
#include "simulationio.h"
#include "segmentio.h"
#include "organism.h"
#include "organismtools.h"
#include "meshtools.h"
#include "oworld.h"
#include "cell.h"
#include "face.h"
#include <boost/foreach.hpp>

/*
 * Check if faceref and f refer to the same thing
 */
bool isEqual(MeshOps::FaceRef fr, Face* f)
{
	// first check that all three vertices are the same and in the same order
	int j,k,l;
	fr.tetra->oppositeFaceIndices(fr.index,j,k,l);
	Vertex *v0 = &fr.tetra->v(j),
			*v1 = &fr.tetra->v(k),
			*v2 = &fr.tetra->v(l);

	//std::cerr << "Testing fr(" << v0 << "," << v1 << "," << v2
	// 		<< ") against f(" << &f->v(0) << "," << &f->v(1) << "," << &f->v(2) << ")\n";

	int fv0i = f->index(v0);
	if (fv0i==-1) return false;
	if (not ((v0==&f->v(fv0i)) and
		(v1==&f->v((fv0i+1)%3)) and
		(v2==&f->v((fv0i+2)%3))))
	{
		return false;
	}

	return true;
}

bool verifyHullOfWholeOrganism(Organism* o)
{
	std::list<MeshOps::FaceRef> faces = MeshOps::getHullFromTetrahedraSet(o->mesh(),o->mesh()->tetras());
	BOOST_FOREACH(MeshOps::FaceRef fr, faces)
	{
		// cehck that there's exactly one matching surface face
		bool match = false;
		BOOST_FOREACH(Face* f, o->mesh()->faces())
		{
			if (isEqual(fr,f))
			{
				if (match)
				{
					std::cerr << "More than one face matches a faceref.\n";
					return false;
				}
				else
					match = true;
			}
		}

		if (!match)
		{
			std::cerr << "No face matched this faceref.\n";
			return false;
		}
	}

	// check that for every face, theres one matching faceref
	BOOST_FOREACH(Face* f, o->mesh()->faces())
	{
		bool match = false;
		BOOST_FOREACH(MeshOps::FaceRef fr, faces)
		{
			if (isEqual(fr,f))
			{
				if (match)
				{
					std::cerr << "More than one faceref matches a face.\n";
					return false;
				}
				else
					match = true;
			}
		}

		if (!match)
		{
			std::cerr << "No faceref matched this face.\n";
			return false;
		}
	}

	return true;
}

int main()
{
	std::cout << "Testing OneTet\n";
	Organism* o = OrganismTools::loadOneTet();
	assert(verifyHullOfWholeOrganism(o));
	std::cout << "Test Passed.\n";
	delete o;

	std::cout << "Testing ../data/lattice111.1\n";
	o = OrganismTools::load("../data/lattice111.1");
	assert(verifyHullOfWholeOrganism(o));
	std::cout << "Test Passed.\n";

	std::cout << "Testing ../data/cube2.1\n";
	o = OrganismTools::load("../data/cube2.1");
	assert(verifyHullOfWholeOrganism(o));
	std::cout << "Test Passed.\n";

	return 0;
}
