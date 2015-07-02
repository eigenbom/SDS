#include "aabb.h"
#include <algorithm>

const AABB AABB::ZERO = AABB(0,0,0,0,0,0,false);

AABB::AABB(double p[])
{
	std::copy(p,p+6,mPts);
	validate();
}

AABB::AABB(double x1, double y1, double z1, double x2, double y2, double z2, bool valid)
{
	mPts[0] = x1;
	mPts[1] = y1;
	mPts[2] = z1;
	mPts[3] = x2;
	mPts[4] = y2;
	mPts[5] = z2;

	if (valid)
		validate();
}

AABB::AABB(Vector3d min, Vector3d max)
{
	mPts[0] = min.x();
	mPts[1] = min.y();
	mPts[2] = min.z();
	mPts[3] = max.x();
	mPts[4] = max.y();
	mPts[5] = max.z();
	validate();
}

void AABB::operator+=(const AABB& b)
{
	double* apts = pts();
	const double* bpts = b.cpts();

	for(int i=0;i<3;++i)
		apts[i] = std::min(apts[i],bpts[i]);

	for(int i=0;i<3;++i)
		apts[3+i] = std::max(apts[3+i],bpts[3+i]);

	validate();
}

void AABB::operator*=(double d)
{
	if (d < 0) d = -d;

	Vector3d center = this->c();

	for(int i=0; i<3; i++)
	{
		(*this)[i] = center[i] + d * ((*this)[i] - center[i]);
		(*this)[3+i] = center[i] + d * ((*this)[3+i] - center[i]);
	}

	validate();
}

void AABB::move(Vector3d v)
{
	mPts[0]+=v.x(); mPts[3]+=v.x();
	mPts[1]+=v.y(); mPts[4]+=v.y();
	mPts[2]+=v.z(); mPts[5]+=v.z();
}
