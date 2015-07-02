#include "face.h"
#include "vector3.h"

#include <cfloat>

Face::Face(){}

Face::Face(Vertex* a, Vertex* b, Vertex* c)
{
	mV[0] = a; mV[1] = b;	mV[2] = c;
	Vector3d n = cross(b->x()-a->x(),c->x()-a->x());
	mRestArea = n.length() / 2;
}

AABB Face::aabb()
{
	double min[3] = {DBL_MAX,DBL_MAX,DBL_MAX}, max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX};
	const Vector3d* p[] = {&mV[0]->x(),&mV[1]->x(),&mV[2]->x()};
	for(int i=0;i<3;++i)
	{
		if (p[i]->x() < min[0]) min[0] = p[i]->x();
		if (p[i]->y() < min[1]) min[1] = p[i]->y();
		if (p[i]->z() < min[2]) min[2] = p[i]->z();

		if (p[i]->x() > max[0]) max[0] = p[i]->x();
		if (p[i]->y() > max[1]) max[1] = p[i]->y();
		if (p[i]->z() > max[2]) max[2] = p[i]->z();
	}

	return AABB(min[0],min[1],min[2],max[0],max[1],max[2]);
}

bool Face::contains(Vertex* a, Vertex* b) const
{
	for(int i=0;i<3;++i)
		for(int j=1;j<3;++j)
			if (a==mV[i] and b==mV[(i+j)%3]) return true;

	return false;
}
