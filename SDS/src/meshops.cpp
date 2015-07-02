/*
 * meshops.cpp
 *
 *  Created on: 02/03/2010
 *      Author: ben
 */

#include "meshops.h"

#include <algorithm>
#include <boost/foreach.hpp>

std::list<MeshOps::FaceRef> MeshOps::getHullFromTetrahedraSet(Mesh* m, const std::list<Tetra*>& T)
{
	std::list<FaceRef> hull;

	BOOST_FOREACH(Tetra* t, T)
	{
		for(int i=0;i<4;i++)
		{
			Tetra* tn = t->neighbour(i);
			if (tn==NULL)
			{
				// face is a boundary face
				hull.push_back(FaceRef(t,i));
			}
			else if (std::find(T.begin(),T.end(),tn) != T.end())
			{
				// tn is in T
				// pass
			}
			else
			{
				// tn is not in T
				hull.push_back(FaceRef(t,i));
			}
		}
	}

	return hull;
}
