/*
 * meshops.h
 *
 * Mesh Operations
 *
 *  Created on: 02/03/2010
 *      Author: ben
 */

#ifndef MESHOPS_H_
#define MESHOPS_H_

#include "mesh.h"
#include "tetra.h"
#include <list>

class MeshOps
{
public:

	/**
	 * A reference to a face in a mesh.
	 */
	struct FaceRef
	{
		Tetra* tetra;
		int index;

		FaceRef(Tetra* t, int i):tetra(t),index(i){}
		bool operator==(const FaceRef& fr){return fr.tetra==tetra and fr.index==index;}
	};

	/**
	 * Builds the list of hull faces that surround a connected set of tetrahedra T.
	 *
	 * PRE: T must be connected, and should only have one hull.
	 */
	static std::list<FaceRef> getHullFromTetrahedraSet(Mesh* m, const std::list<Tetra*>& T);
};

#endif /* MESHOPS_H_ */
