#ifndef MATRIX3_H
#define MATRIX3_H

#include <ostream>

#include "promote.h"
#include "vector3.h"

template <typename T = double>
class Matrix3
{
	public:

	Matrix3(){}
	Matrix3(T aa, T ab, T ac, 
					T ba, T bb, T bc,
					T ca, T cb, T cc);
	Matrix3(const Vector3<T>& col1, const Vector3<T>& col2, const Vector3<T>& col3);
	template <typename U>	Matrix3(const Matrix3<U>& m);

	static const Matrix3 IDENTITY;
	static const Matrix3 ZERO;
	
	// accessors
	inline T operator() (int i, int j) const; // (row,col)
	inline T& get(int i,int j);
	void set(int col, int row, T val);
		
	inline operator T*();
	inline operator const T*();

	// operators
	 
	template <typename U, typename V> friend
	Matrix3 <typename promote_trait<U,V>::T_promote>
	operator* (const Matrix3<U>& a, const Matrix3<V>& b);

	template <typename U, typename V> friend
	Matrix3 <typename promote_trait<U,V>::T_promote>
	operator+ (const Matrix3<U>& a, const Matrix3<V>& b);

	template <typename U, typename V> friend
	Matrix3 <typename promote_trait<U,V>::T_promote>
	operator- (const Matrix3<U>& a, const Matrix3<V>& b);

	template <typename U, typename V> friend
	Vector3 <typename promote_trait<U,V>::T_promote>
	operator*(const Matrix3<U>& a, const Vector3<V>& v);
		
	template <typename U, typename V> friend
	Matrix3 <typename promote_trait<U,V>::T_promote>
	operator*(U a, const Matrix3<V>& m);
		
	template <typename U, typename V> friend
	bool
	operator==(const Matrix3<U>& a, const Matrix3<V>& b);

	// special functions
	
	Matrix3<typename promote_trait<T,double>::T_promote> inv(); // matrix inverse
	T det(); // determinant

	// streaming 
	template <typename U>	friend 
		std::ostream& 
		operator<<(std::ostream& o, const Matrix3<U>& m);

	template <typename U> friend class Matrix3;

	protected:
	T mData[9]; //store column major order

	// getters...a_ij (row i, col j), from 1,1
	T a11(){return get(0,0);}
	T a12(){return get(0,1);}
	T a13(){return get(0,2);}
	T a21(){return get(1,0);}
	T a22(){return get(1,1);}
	T a23(){return get(1,2);}
	T a31(){return get(2,0);}
	T a32(){return get(2,1);}
	T a33(){return get(2,2);}
};

// 2x2 determinant
template <typename T>
T det2x2(T a11, T a12, 
				 T a21, T a22);

typedef Matrix3<> Matrix3d;
typedef Matrix3<int> Matrix3i;

#include "matrix3.inl"


#endif
