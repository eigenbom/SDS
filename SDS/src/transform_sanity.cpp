/*
 * transform_sanity.cpp
 *
 *  Created on: 24/09/2009
 *      Author: ben
 */

#include "transform.h"

#include "tetra.h"

bool Transform::checkTetraNeighbour(Tetra* t, int index)
{
	Tetra* tn = t->neighbour(index);
	Vertex* v[4] = {&t->v(0), &t->v(1), &t->v(2), &t->v(3)};

	if (tn!=NULL)
	{
		// then tn and t must share all the other verts
		for(int j=1;j<4;j++)
		{
			int k = (index + j)%4;
			if (not (tn->contains(v[k]))) return false;
		}
	}
	return true;
}
