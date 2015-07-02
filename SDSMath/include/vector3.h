/* A 3d Vector class
 * BP
 *
 * 07.11.07
 */

#ifndef VECTOR3_H
#define VECTOR3_H

#include <ostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "promote.h"

template <typename T = double>
class Vector3
{
	public:
	typedef T type;

	/* Constructors */
	Vector3(){}
	Vector3(T x, T y, T z);
	template <typename U>	Vector3(const Vector3<U>& vec);
	template <typename U> Vector3(U*);

	static const Vector3 ZERO;
	static const Vector3 X;
	static const Vector3 Y;
	static const Vector3 Z;

	/* Accessors */

	// element access
	T x() const;
	T y() const;
	T z() const;
	T x(T x);
	T y(T y);
	T z(T z);

	T& operator[](int i);
	T operator()(int i) const;

	// array access
	operator T*();
	operator const T*() const;
	void setData(T x, T y, T z);

	/* Operations */
	Vector3 operator-() const;

	// element-wise operations
	// these are type smart and will cast
	// to the more general type

	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
	 	operator+(const Vector3<U>& v) const;

	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
	 	operator-(const Vector3<U>& v) const;

	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
		operator*(U d) const;

	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
		operator/(U d) const;

	template <typename U>
		Vector3&
		operator*=(U d);

	template <typename U>
		Vector3&
		operator/=(U d);

	template <typename U, typename V> friend
		Vector3<typename promote_trait<U,V>::T_promote>
		operator*(U, const Vector3<V>& v);

	template <typename U>
		Vector3&
		operator+=(const Vector3<U>& v);

	template <typename U>
		Vector3&
		operator-=(const Vector3<U>& v);

	/* Comparison */
	template <typename U, typename V> friend
		bool
		operator==(const Vector3<U>& a, const Vector3<V>& b);

	template <typename U, typename V> friend
		bool
		operator!=(const Vector3<U>& a, const Vector3<V>& b);

	/* Procedures */
	Vector3& normaliseInPlace();
	Vector3<typename promote_trait<double,T>::T_promote> normalise();

	typename promote_trait<double,T>::T_promote length() const;
	T sqlength() const; // square length
	T mlength() const; // manhattan length

	// globals
	// lin alg.
	template <typename U, typename W> friend
	Vector3<typename promote_trait<U,W>::T_promote>
		cross(const Vector3<U>& a, const Vector3<W>& b);

	template <typename U, typename W>	friend
		typename promote_trait<U,W>::T_promote
		dot(const Vector3<U>& v1, const Vector3<W>& v2);

	template <typename U, typename W>	friend
		typename promote_trait<U,W>::T_promote
		dist(const Vector3<U>& v1, const Vector3<W>& v2);

	template <typename U, typename W>	friend
		typename promote_trait<U,W>::T_promote
		sqdist(const Vector3<U>& v1, const Vector3<W>& v2);


	/* streaming */

	template <typename U>	friend
	std::ostream& operator<<(std::ostream&,const Vector3<U>& v);

	protected:
	T mData[3];
};

typedef Vector3<> Vector3d;
typedef Vector3<int> Vector3i;

#include "vector3.inl"

#endif
