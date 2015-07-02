#ifndef MATRIX4_H
#define MATRIX4_H

#include <ostream>

#include "promote.h"
#include "vector3.h"

template <typename T = double>
class Matrix4
{
	public:

	Matrix4(){}
	Matrix4(T aa, T ab, T ac, T ad,
					T ba, T bb, T bc, T bd,
					T ca, T cb, T cc, T cd,
					T da, T db, T dc, T dd);
	Matrix4(const Vector3<T>& col1, const Vector3<T>& col2, const Vector3<T>& col3);
	template <typename U>	Matrix4(const Matrix4<U>& m);

	static const Matrix4 IDENTITY;
	static const Matrix4 ZERO;
	
	// accessors
	inline T operator() (int i, int j) const; // (row,col)
	inline T& get(int i,int j);
	void set(int col, int row, T val);
		
	inline operator T*();
	inline operator const T*();

	// operators
	 
	template <typename U, typename V> friend
	Matrix4 <typename promote_trait<U,V>::T_promote>
	operator* (const Matrix4<U>& a, const Matrix4<V>& b);

	template <typename U, typename V> friend
	Matrix4 <typename promote_trait<U,V>::T_promote>
	operator+ (const Matrix4<U>& a, const Matrix4<V>& b);

	template <typename U, typename V> friend
	Matrix4 <typename promote_trait<U,V>::T_promote>
	operator- (const Matrix4<U>& a, const Matrix4<V>& b);

	template <typename U, typename V> friend
	Vector3 <typename promote_trait<U,V>::T_promote>
	operator*(const Matrix4<U>& a, const Vector3<V>& v);
		
	template <typename U, typename V> friend
	Matrix4 <typename promote_trait<U,V>::T_promote>
	operator*(U a, const Matrix4<V>& m);
		
	template <typename U, typename V> friend
	bool
	operator==(const Matrix4<U>& a, const Matrix4<V>& b);

	// streaming 
	template <typename U>	friend 
		std::ostream& 
		operator<<(std::ostream& o, const Matrix4<U>& m);

	template <typename U> friend class Matrix4;

	protected:
	T mData[16]; //store column major order
};

typedef Matrix4<> Matrix4d;
typedef Matrix4<int> Matrix4i;

#include "matrix4.inl"


#endif
