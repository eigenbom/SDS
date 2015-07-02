// Some miscellaneous Vector functions

#ifndef VMATH_H
#define VMATH_H

#include "hmath.h"
#include "vector3.h"

namespace Math
{
	template <typename T>
	Vector3<T> lerp(const Vector3<T>& from, const Vector3<T>& to, double t);

	inline Vector3<double> rotateZ(const Vector3<double>& v, double amount);

	template <typename T>
	inline Vector3<T> abs(const Vector3<T>& v);
};

#include "vmath.inl"

#endif
