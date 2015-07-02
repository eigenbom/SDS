#include "gmath.h"
#include "vector3.h"

#include <boost/tuple/tuple.hpp>

#include <iostream>
#include <vector>

#define TEST(x,msg) {std::cout << "TEST: " << msg; if (!x) {std::cout << " failed\n"; return -1;} else {std::cout << " passed\n";}}

int main()
{
	// INPUT
	std::vector<Vector3d> cube;
	for(int i=0;i<2;i++)
		for(int j=0;j<2;j++)
			for(int k=0;k<2;k++)
				cube.push_back(Vector3d(i,j,k));
	std::vector<boost::tuple<int,int,int> > noTriangles;

	// OUTPUT
	std::vector<boost::tuple<int,int,int,int> > tetrahedra;
	std::vector<boost::tuple<int,int,int,int> > tetrahedraNeighbours;
	std::vector<boost::tuple<int,int> > edges;

	// TETRA
	Math::tetrahedralise(cube, noTriangles, tetrahedra, tetrahedraNeighbours, edges);

	// TESTS
	TEST(tetrahedra.size()>0, "tetrahedra");

	return 0;
}
