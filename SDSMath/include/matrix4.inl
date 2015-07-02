template <typename T>
const Matrix4<T> Matrix4<T>::IDENTITY = Matrix4<T>(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);

template <typename T>
const Matrix4<T> Matrix4<T>::ZERO = Matrix4<T>(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);

template <typename T>
Matrix4<T>::Matrix4(
		T aa, T ab, T ac, T ad,
		T ba, T bb, T bc, T bd,
		T ca, T cb, T cc, T cd,
		T da, T db, T dc, T dd)
{
	mData[0] = aa; mData[4] = ab; mData[8] = ac; mData[12] = ad;
	mData[1] = ba; mData[5] = bb; mData[9] = bc; mData[13] = bd;
	mData[2] = ca; mData[6] = cb; mData[10] = cc; mData[14] = cd;
	mData[3] = da; mData[7] = db; mData[11] = dc; mData[15] = dd;
}

template <typename T>
Matrix4<T>::Matrix4(const Vector3<T>& col1, const Vector3<T>& col2, const Vector3<T>& col3)
{
	mData[0] = col1.x();
	mData[1] = col1.y();
	mData[2] = col1.z();
	mData[3] = 0;

	mData[4] = col2.x();
	mData[5] = col2.y();
	mData[6] = col2.z();	
	mData[7] = 0;
	
	mData[8] = col3.x();
	mData[9] = col3.y();
	mData[10] = col3.z();
	mData[11] = 0;

	mData[12] = 0;
	mData[13] = 0;
	mData[14] = 0;
	mData[15] = 1;
}

template <typename T>
template <typename U>	
Matrix4<T>::Matrix4(const Matrix4<U>& m)
{
	for(int i=0;i<16;++i)
		mData[i] = static_cast<T>(m.mData[i]);
}

// accessors

template <typename T>
Matrix4<T>::operator T*()
{return mData;}	

template <typename T>
Matrix4<T>::operator const T*()
{return mData;}	

template <typename T>
T 
Matrix4<T>::operator() (int i, int j) const 
{return mData[j*4+i];} //row i, col j

template <typename T>
T& 
Matrix4<T>::get(int i,int j)
{return mData[j*4+i];}

template <typename T>
void 
Matrix4<T>::set(int col, int row, T val)
{
	get(row,col)=val;
}	

// operators

template <typename U, typename V> 
Matrix4 <typename promote_trait<U,V>::T_promote>
operator* (const Matrix4<U>& a, const Matrix4<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix4<R> c;
	for(int n=0;n<4;n++)
		for(int m=0;m<4;m++)
			c.get(n,m) = a(n,0)*b(0,m) + a(n,1)*b(1,m) + a(n,2)*b(2,m) + a(n,3)*b(3,m);
	return c;
}
	
template <typename U, typename V>
Matrix4 <typename promote_trait<U,V>::T_promote>
operator+ (const Matrix4<U>& a, const Matrix4<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix4<R> ret;
	for(int i=0;i<16;++i)
		ret.mData[i] = a.mData[i] + b.mData[i];
	return ret;
}

template <typename U, typename V>
Matrix4 <typename promote_trait<U,V>::T_promote>
operator- (const Matrix4<U>& a, const Matrix4<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix4<R> ret;
	for(int i=0;i<16;++i)
		ret.mData[i] = a.mData[i] - b.mData[i];
	return ret;
}






template <typename U, typename V> 
Vector3 <typename promote_trait<U,V>::T_promote>
operator*(const Matrix4<U>& a, const Vector3<V>& b)
{
	Vector3 <typename promote_trait<U,V>::T_promote> c;
	int n;
	for(n=0;n<3;n++)
		c[n] = a(n,0)*b(0) + a(n,1)*b(1) + a(n,2)*b(2) + a(n,3);
	return c;
}

template <typename U, typename V>
Matrix4 <typename promote_trait<U,V>::T_promote>
operator*(U a, const Matrix4<V>& m)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix4<R> ret;
	for(int i=0;i<16;++i)
		ret.mData[i] = m.mData[i] * a;
	return ret;
}

template <typename U, typename V> 
bool
operator==(const Matrix4<U>& a, const Matrix4<V>& b)
{
	for(int i=0;i<16;++i)
		if (a.mData[i]!=b.mData[i]) return false;
	return true;
}


// streaming
template <typename U>
	std::ostream& 
	operator<<(std::ostream& o, const Matrix4<U>& m)
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
			o << m(i,j) << " ";
		o << "\n";
	}

	return o;
}

