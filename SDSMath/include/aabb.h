#ifndef AABB_H
#define AABB_H

#include "vector3.h"

class AABB
{
	public:
	static const AABB ZERO;

	public:
	AABB():mValid(false){}
	AABB(double[]);
	AABB(double x1, double y1, double z1, double x2, double y2, double z2, bool valid = true);
	AABB(Vector3d min, Vector3d max);

	inline double* pts();
	inline const double* cpts() const;

	inline double dx() const;
	inline double dy() const;
	inline double dz() const;

	inline Vector3d min() const;
	inline Vector3d max() const;
	// c: centroid, cl: center of bottom quad
	inline Vector3d c() const;
	inline Vector3d cl() const;

	inline bool valid(bool v);
	inline bool validate();
	inline bool valid() const;
	inline bool contains(Vector3d p) const;

	inline double& operator[](int i);

	void operator+=(const AABB& a);
	void operator*=(double d);

	friend AABB operator*(const AABB& a, double b);
	friend AABB operator*(double b, const AABB& a);

	void move(Vector3d);

	protected:
	double mPts[6]; // [minx,miny,minz,maxx,maxy,maxz]
	bool mValid;
};

inline std::ostream& operator<<(std::ostream& o, const AABB& aabb)
{
	const double* pts = aabb.cpts();
	o << "{" << pts[0] << " "
			 << pts[1] << " "
			 << pts[2] << " "
			 << pts[3] << " "
			 << pts[4] << " "
			 << pts[5] << "}";
	return o;
}

#include "aabb.inl"

#endif
