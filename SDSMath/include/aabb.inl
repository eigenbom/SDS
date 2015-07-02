double* AABB::pts(){return mPts;}
const double* AABB::cpts() const {return mPts;}

double AABB::dx() const {return mPts[3]-mPts[0];}
double AABB::dy() const {return mPts[4]-mPts[1];}
double AABB::dz() const {return mPts[5]-mPts[2];}

Vector3d AABB::min() const {return Vector3d(mPts[0],mPts[1],mPts[2]);}
Vector3d AABB::max() const {return Vector3d(mPts[3],mPts[4],mPts[5]);}
// c: centroid, cl: center of bottom quad
Vector3d AABB::c() const {return Vector3d(mPts[0] + dx()/2,mPts[1] + dy()/2,mPts[2]+dz()/2);}
Vector3d AABB::cl() const {return Vector3d(mPts[0] + dx()/2, mPts[1], mPts[2]+dz()/2);}

bool AABB::valid(bool v){return mValid=v;}
bool AABB::validate(){return mValid = (mPts[0] < mPts[3] and mPts[1] < mPts[4] and mPts[2] < mPts[5]);}
bool AABB::valid() const {return mValid;}
bool AABB::contains(Vector3d p) const
{
	return p.x() >= mPts[0] and
		p.x() <= mPts[3] and
		p.y() >= mPts[1] and
		p.y() <= mPts[4] and
		p.z() >= mPts[2] and
		p.z() <= mPts[5];
}

double& AABB::operator[](int i)
{
	return mPts[i];
}

inline AABB operator*(const AABB& a, double b)
{
	double dx = a.dx(), dy = a.dy(), dz = a.dz();
	double newdx = dx*b, newdy = dy*b, newdz = dz*b;
	Vector3d c = a.c();

	return AABB(c.x()-newdx/2,c.y()-newdy/2,c.z()-newdz/2,c.x()+newdx/2,c.y()+newdy/2,c.z()+newdz/2);
}

inline AABB operator*(double b, const AABB& a)
{
	double dx = a.dx(), dy = a.dy(), dz = a.dz();
	double newdx = dx*b, newdy = dy*b, newdz = dz*b;
	Vector3d c = a.c();

	return AABB(c.x()-newdx/2,c.y()-newdy/2,c.z()-newdz/2,c.x()+newdx/2,c.y()+newdy/2,c.z()+newdz/2);
}
