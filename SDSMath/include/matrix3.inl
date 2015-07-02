template <typename T>
const Matrix3<T> Matrix3<T>::IDENTITY = Matrix3<T>(1,0,0, 0,1,0, 0,0,1);

template <typename T>
const Matrix3<T> Matrix3<T>::ZERO = Matrix3<T>(0,0,0, 0,0,0, 0,0,0);

template <typename T>
Matrix3<T>::Matrix3(
		T aa, T ab, T ac, 
		T ba, T bb, T bc,
		T ca, T cb, T cc)
{
	mData[0] = aa; mData[3] = ab; mData[6] = ac;
	mData[1] = ba; mData[4] = bb; mData[7] = bc;
	mData[2] = ca; mData[5] = cb; mData[8] = cc;
}

template <typename T>
Matrix3<T>::Matrix3(const Vector3<T>& col1, const Vector3<T>& col2, const Vector3<T>& col3)
{
	mData[0] = col1.x();
	mData[1] = col1.y();
	mData[2] = col1.z();

	mData[3] = col2.x();
	mData[4] = col2.y();
	mData[5] = col2.z();	
	
	mData[6] = col3.x();
	mData[7] = col3.y();
	mData[8] = col3.z();
}

template <typename T>
template <typename U>	
Matrix3<T>::Matrix3(const Matrix3<U>& m)
{
	for(int i=0;i<9;++i)
		mData[i] = static_cast<T>(m.mData[i]);
}

// accessors

template <typename T>
Matrix3<T>::operator T*()
{return mData;}	

template <typename T>
Matrix3<T>::operator const T*()
{return mData;}	

template <typename T>
T 
Matrix3<T>::operator() (int i, int j) const 
{return mData[j*3+i];} //row i, col j

template <typename T>
T& 
Matrix3<T>::get(int i,int j)
{return mData[j*3+i];}

template <typename T>
void 
Matrix3<T>::set(int col, int row, T val)
{
	get(row,col)=val;
}	



template <typename T>
T Matrix3<T>::det()
{
	return 
			a11()*det2x2(a22(),a23(),a32(),a33()) 
		- a21()*det2x2(a12(),a13(),a32(),a33()) 
		+ a31()*det2x2(a12(),a13(),a22(),a23());
}

template <typename T>
T det2x2(T a11, T a12, T a21, T a22) // det of 2x2
{
	return a11*a22 - a12*a21;
}

template <typename T>
Matrix3<typename promote_trait<T,double>::T_promote> Matrix3<T>::inv()
{
	typedef typename promote_trait<T,double>::T_promote R;
	R de = det();

	Matrix3<R> inv;
	for(int i=0;i<3;i++) // row
		for(int j=0;j<3;j++) // col
		{
			int l = (j==0)?1:(j==1)?0:0;
			int r = (j==0)?2:(j==1)?2:1;
			int u = (i==0)?1:(i==1)?0:0;
			int d = (i==0)?2:(i==1)?2:1;

			inv.get(j,i) = (((i+j)%2==0)?1:-1)*det2x2(get(u,l),get(u,r),get(d,l),get(d,r)) / de;
		}
	return inv;
}

template <typename U, typename V> 
Matrix3 <typename promote_trait<U,V>::T_promote>
operator* (const Matrix3<U>& a, const Matrix3<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix3<R> c;
	for(int n=0;n<3;n++)
		for(int m=0;m<3;m++)
			c.get(n,m) = a(n,0)*b(0,m) + a(n,1)*b(1,m) + a(n,2)*b(2,m);
	return c;
}
	
template <typename U, typename V>
Matrix3 <typename promote_trait<U,V>::T_promote>
operator+ (const Matrix3<U>& a, const Matrix3<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix3<R> ret;
	for(int i=0;i<9;++i)
		ret.mData[i] = a.mData[i] + b.mData[i];
	return ret;
}

template <typename U, typename V>
Matrix3 <typename promote_trait<U,V>::T_promote>
operator- (const Matrix3<U>& a, const Matrix3<V>& b)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix3<R> ret;
	for(int i=0;i<9;++i)
		ret.mData[i] = a.mData[i] - b.mData[i];
	return ret;
}






template <typename U, typename V> 
Vector3 <typename promote_trait<U,V>::T_promote>
operator*(const Matrix3<U>& a, const Vector3<V>& b)
{
	Vector3 <typename promote_trait<U,V>::T_promote> c;
	int n;
	for(n=0;n<3;n++)
		c[n] = a(n,0)*b(0) + a(n,1)*b(1) + a(n,2)*b(2);
	return c;
}

template <typename U, typename V>
Matrix3 <typename promote_trait<U,V>::T_promote>
operator*(U a, const Matrix3<V>& m)
{
	typedef typename promote_trait<U,V>::T_promote R;
	Matrix3<R> ret;
	for(int i=0;i<9;++i)
		ret.mData[i] = m.mData[i] * a;
	return ret;
}

template <typename U, typename V> 
bool
operator==(const Matrix3<U>& a, const Matrix3<V>& b)
{
	for(int i=0;i<9;++i)
		if (a.mData[i]!=b.mData[i]) return false;
	return true;
}


// streaming
template <typename U>
	std::ostream& 
	operator<<(std::ostream& o, const Matrix3<U>& m)
{
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
			o << m(i,j) << " ";
		o << "\n";
	}

	return o;
}

