#include "matrix3.h"
#include "matrix4.h"

#include <iostream>
#include <cassert>

using std::cout;

int main()
{
	cout << "testing Matrix3\n";

	Vector3i c1(-1,3,-1), c2(1,-1,3), c3(2,1,4);
	Matrix3i m(c1,c2,c3);	
	
	assert(m.det()==10);
	
	Matrix3d minv = m.inv() * m;
	Matrix3i minv_trunc = minv + 0.00001*Matrix3d::IDENTITY;

	assert(minv_trunc == Matrix3i::IDENTITY);
	assert((minv_trunc*Vector3i(1,2,3)) == Vector3i(1,2,3));
	assert(minv_trunc == minv_trunc.inv()); 

	Matrix3i 
		a(0,1,2,
			3,4,5,
			6,7,8);

	assert(a==a);
	assert(2*a==Matrix3i(0,2,4,6,8,10,12,14,16));
	assert(2*a==a*(2*Matrix3i::IDENTITY));
	assert(a*Vector3i(0,1,2)==Vector3i(5,14,23));
	assert((a*a)==
		Matrix3i(
			15,18,21,
			42,54,66,
			69,90,111));

	cout << "tests passed\n";
	cout << "testing Matrix4\n";

	Matrix4i m4(c1,c2,c3);
	assert(m4==Matrix4i(c1.x(),c2.x(),c3.x(),0,c1.y(),c2.y(),c3.y(),0,c1.z(),c2.z(),c3.z(),0,0,0,0,1));


	m4 = Matrix4i(
			0,1,2,3,
			1,2,3,4,
			2,3,4,5,
			1,1,1,2);
	assert(m4 == Matrix4i(0,1,2,3,	1,2,3,4,	2,3,4,5,	1,1,1,2));
	assert(Matrix4i::IDENTITY * Matrix4i::IDENTITY == Matrix4i::IDENTITY);
	assert(m4*m4 == Matrix4i(
				 8,11,14,20,
				12,18,24,34,
				16,25,34,48,
				 5, 8,11,16));

	assert(m4*Vector3i(1,2,3)==Vector3i(11,18,25));

	cout << "tests passed\n";
	return 1;
}
