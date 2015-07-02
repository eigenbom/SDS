// implementation of Vector3 inlined functions

// Constructors

template <typename T>
Vector3<T>::Vector3(T x, T y, T z)
{
	mData[0] = x;
	mData[1] = y;
	mData[2] = z;
}

template <typename T>
template <typename U>
Vector3<T>::Vector3(U* arr)
{
	mData[0] = arr[0];
	mData[1] = arr[1];
	mData[2] = arr[2];
}

template <typename T>
template <typename U>
Vector3<T>::Vector3(const Vector3<U>& vec)
{
	mData[0] = (T)vec.x();
	mData[1] = (T)vec.y();
	mData[2] = (T)vec.z();
}

/* Accessors */

template <typename T>
T Vector3<T>::x() const {return mData[0];}
template <typename T>
T Vector3<T>::y() const {return mData[1];}
template <typename T>
T Vector3<T>::z() const {return mData[2];}

template <typename T>
	T& Vector3<T>::operator[](int i){return mData[i];}
template <typename T>
	T Vector3<T>::operator()(int i) const {return mData[i];}

template <typename T>
	T Vector3<T>::x(T x){return mData[0] = x;}
template <typename T>
	T Vector3<T>::y(T y){return mData[1] = y;}
template <typename T>
	T Vector3<T>::z(T z){return mData[2] = z;}

template <typename T>
	Vector3<T>::operator T*(){return mData;}
template <typename T>
	Vector3<T>::operator const T*() const{return mData;}

template <typename T>
	void Vector3<T>::setData(T x, T y, T z)
{
	mData[0] = x;
	mData[1] = y;
	mData[2] = z;
}

// operators

	/* Operations */
template <typename T>
Vector3<T> Vector3<T>::operator-() const
{
	return Vector3<T>(-mData[0],-mData[1],-mData[2]);
}

// element-wise operations
// these are type smart and will cast
// to the more general type

template <typename T>
	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
	 	Vector3<T>::operator+(const Vector3<U>& v) const
{
	typedef Vector3<typename promote_trait<U,T>::T_promote> VR;
	return VR(mData[0]+v.x(),mData[1]+v.y(),mData[2]+v.z());
}

template <typename T>
	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
	 	Vector3<T>::operator-(const Vector3<U>& v) const
{
	typedef Vector3<typename promote_trait<U,T>::T_promote> VR;
	return VR(mData[0]-v.x(),mData[1]-v.y(),mData[2]-v.z());
}

template <typename T>
	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
		Vector3<T>::operator*(U d) const
{
	typedef typename promote_trait<U,T>::T_promote R;
	return Vector3<R>(mData[0]*d,mData[1]*d,mData[2]*d);
}

template <typename T>
	template <typename U>
		Vector3<typename promote_trait<U,T>::T_promote>
		Vector3<T>::operator/(U d) const
{
	typedef typename promote_trait<U,T>::T_promote R;
	return Vector3<R>(mData[0]/d,mData[1]/d,mData[2]/d);
}

template <typename T>
template <typename U>
Vector3<T>&
Vector3<T>::operator*=(U d)
{
	mData[0] *= d;
	mData[1] *= d;
	mData[2] *= d;
	return *this;
}

template <typename T>
template <typename U>
Vector3<T>&
Vector3<T>::operator/=(U d)
{
	mData[0] /= d;
	mData[1] /= d;
	mData[2] /= d;
	return *this;
}

template <typename T>
	template <typename U>
		Vector3<T>&
		Vector3<T>::operator+=(const Vector3<U>& v)
{
	mData[0] += v.x();
	mData[1] += v.y();
	mData[2] += v.z();
	return *this;
}

template <typename T>
	template <typename U>
		Vector3<T>&
		Vector3<T>::operator-=(const Vector3<U>& v)
{
	mData[0] -= v.x();
	mData[1] -= v.y();
	mData[2] -= v.z();
	return *this;
}

template <typename U, typename V>
bool operator==(const Vector3<U>& a, const Vector3<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote T;
	return static_cast<T>(a(0))==static_cast<T>(b(0))
		and static_cast<T>(a(1))==static_cast<T>(b(1))
		and static_cast<T>(a(2))==static_cast<T>(b(2));
}

template <typename U, typename V>
bool operator!=(const Vector3<U>& a, const Vector3<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote T;
	return static_cast<T>(a(0))!=static_cast<T>(b(0))
		or static_cast<T>(a(1))!=static_cast<T>(b(1))
		or static_cast<T>(a(2))!=static_cast<T>(b(2));
}

// procedures
template <typename T>
Vector3<T>&
Vector3<T>::normaliseInPlace()
{
	typename promote_trait<double,T>::T_promote l = length();
	mData[0] /= l;
	mData[1] /= l;
	mData[2] /= l;
	return *this;
}

template <typename T>
Vector3<typename promote_trait<double,T>::T_promote>
Vector3<T>::normalise()
{
	typedef typename promote_trait<double,T>::T_promote U;
	U l = length();
	return Vector3<U>(mData[0]/l,mData[1]/l,mData[2]/l);
}

// norms

template <typename T>
typename promote_trait<double,T>::T_promote
Vector3<T>::length() const
{
	typedef typename promote_trait<double,T>::T_promote U;
	return std::sqrt(static_cast<U>(sqlength()));
}

template <typename T>
T
Vector3<T>::sqlength() const // square length
{
	return mData[0]*mData[0] + mData[1]*mData[1] + mData[2]*mData[2];
}

template <typename T>
T
Vector3<T>::mlength() const // manhattan length
{
	return std::abs(mData[0]) + std::abs(mData[1]) + std::abs(mData[2]);
}

// global functions

// linear algebra

template <typename U, typename W>
Vector3<typename promote_trait<U,W>::T_promote>
cross(const Vector3<U>& a, const Vector3<W>& b)
{
	typedef typename promote_trait<U,W>::T_promote T;
	return Vector3<T>(a(1)*b(2)-a(2)*b(1),a(2)*b(0)-a(0)*b(2),a(0)*b(1)-a(1)*b(0));
}

template <typename U, typename W>
typename promote_trait<U,W>::T_promote
dot(const Vector3<U>& v1, const Vector3<W>& v2)
{
	typedef typename promote_trait<U,W>::T_promote T;
	return static_cast<T>(v1(0)*v2(0)+v1(1)*v2(1)+v1(2)*v2(2));
}

template <typename U, typename W>
typename promote_trait<U,W>::T_promote
dist(const Vector3<U>& v1, const Vector3<W>& v2)
{
	typedef typename promote_trait<U,W>::T_promote T;
	T dx = v1(0)-v2(0), dy = v1(1)-v2(1), dz = v1(2)-v2(2);
	return static_cast<T>(sqrt(dx*dx+dy*dy+dz*dz));
}

template <typename U, typename W>
typename promote_trait<U,W>::T_promote
sqdist(const Vector3<U>& v1, const Vector3<W>& v2)
{
	typedef typename promote_trait<U,W>::T_promote T;
	T dx = v1(0)-v2(0), dy = v1(1)-v2(1), dz = v1(2)-v2(2);
	return static_cast<T>(dx*dx+dy*dy+dz*dz);
}

// streaming
template <typename U>
std::ostream&
operator<<(std::ostream& o,const Vector3<U>& v)
{
	return o << v.x() << " " << v.y() << " " << v.z() << "\n";
}









template <typename U, typename V>
Vector3<typename promote_trait<U,V>::T_promote>
operator*(U r, const Vector3<V>& v)
{
	return v*r;
}

// constants
template<typename T>
const Vector3<T> Vector3<T>::ZERO(0,0,0);

template<typename T>
const Vector3<T> Vector3<T>::X(1,0,0);

template<typename T>
const Vector3<T> Vector3<T>::Y(0,1,0);

template<typename T>
const Vector3<T> Vector3<T>::Z(0,0,1);
