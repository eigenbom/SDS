#include "vector3.h"
#include <cassert>
#include <iostream>
#include <cmath>

using std::cout;

bool approximately(double a, double b);

int main()
{
	Vector3d a(0,1,2);	

	int dat[] = {0, 1, 2, 3, 4, 5, 6, 8, 10};
	int datlen = sizeof(dat)/sizeof(int);

	Vector3i u(dat), v(dat+3), w(dat+6), x(dat);
	Vector3i y(x);

	assert(u==x);
	assert(u==y);
	assert(2*v==w);
	assert(u+Vector3i(1,1,1)*3==v);

	const double ulen = std::sqrt(5.0);
	const double EPS = 0.0000001;
	double ulen2 = u.length();
	assert(ulen2 > (ulen-EPS) and ulen2 < (ulen+EPS));
	assert(v.sqlength()==(3*3+4*4+5*5) and v.sqlength()==dot(v,v));
	assert(v.mlength()==(3+4+5) and v.mlength()==dot(v,Vector3i(1,1,1)));
	assert(dot(v,x)==(0+v.y()+2*v.z()));

	u.x(1);
	assert(u.x()==1);
	u[2]=1;
	assert(u.z()==1);
	u = Vector3d(3.5,4.2,5.99);
	assert(u==v);

	for(int i=0;i<3;++i)
		assert(((const int*)u)[i]==v(i));
	u.setData(0,0,1);
	assert(u==Vector3i::Z);
	assert(u==Vector3<char>::Z);

	assert(Vector3i(1,2,3)*2==Vector3i(2,4,6));

	assert(Vector3i(1,2,3)/2==Vector3i(0,1,1));
	assert(Vector3i(1,2,3)/2.!=Vector3i(0,1,1));

	assert(cross(Vector3i(1,2,3),Vector3i(0,2,3))==Vector3<int>(0,-3,2));
	
	for(int i=0;i<datlen;++i)
	for(int j=0;j<datlen;++j)
	for(int k=0;k<datlen;++k)
		assert(cross(Vector3i(i,j,k),Vector3i(1,2,3))==Vector3i(j*3-k*2,k*1-i*3,i*2-j*1));

	// normalisation tests
	double coords[] = {1.0,2.0,3.0};
	double length = std::sqrt(coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2]);
	Vector3d vec(coords);

	assert(approximately(vec.length(),length));
	assert(approximately(vec.normalise().length(),1));

	vec.normaliseInPlace();

	assert(approximately(vec.sqlength(),1));
	assert(approximately(vec.length(),1));

	vec = Vector3d(coords);
	vec = -vec;

	assert(approximately(vec.length(),length));
	assert(approximately(vec.normalise().length(),1));
	
	vec.normaliseInPlace();

	assert(approximately(vec.length(),1));

	cout << "Tests passed.\n";
	return 0;
}

bool approximately(double a, double b)
{
	return std::abs(b-a) < 0.00001;
}
