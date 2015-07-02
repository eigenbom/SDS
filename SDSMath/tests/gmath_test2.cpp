#include "gmath.h"
#include "hmath.h"
#include "vmath.h"
#include "random.h"
#include "vector3.h"

#include <iostream>
#include <cassert>
#include <iomanip>
#include <vector>
#include <cfloat>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/timer.hpp>

#include "random.h"

Random r;

typedef Vector3d Vec;

using namespace boost;

int main()
{
	std::cout << "Testing GMath::tetrahedralise\n**********************\n";

	// basic test, construct a single tetrahedra hull and expect a single tetrahedra back
	// test its orientation, etc...
	Vec _points[] = {Vec(0,1,0),Vec(1,0,0),Vec(-1,0,1),Vec(-1,0,-1)};
	std::vector<Vec> points(_points,_points+4);

	tuple<int,int,int> _faces[] = {make_tuple(0,1,2),make_tuple(0,2,3),make_tuple(0,3,1),make_tuple(1,3,2)};
	std::vector<tuple<int,int,int> > faces(_faces,_faces+4);

	std::vector<tuple<int, int, int, int> > tetras;
	Math::tetrahedralise(points,faces,tetras);

	assert(tetras.size()==1);

	return 0;
}
