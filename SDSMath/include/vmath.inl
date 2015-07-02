namespace Math
{
	template <typename T>
	Vector3<T> lerp(const Vector3<T>& from, const Vector3<T>& to, double t)
	{
		return (1-t)*from + t*to;
	}

	Vector3<double> rotateZ(const Vector3<double>& v, double amount)
	{	
		double ox, oy;
		double ca = cos(amount), sa = sin(amount);
		ox = v.x()*ca - v.y()*sa;
		oy = v.x()*sa + v.y()*ca;
		return Vector3<double>(ox,oy,v.z());
	}

	template <typename T>
	Vector3<T> abs(const Vector3<T>& v)
	{
		return Vector3<T>(abs(v.x()),abs(v.y()),abs(v.z()));
	}
};
