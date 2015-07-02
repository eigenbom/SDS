namespace Math
{
	template <typename T>
	void baryTri(const Vector3<T>& e, const Vector3<T>& f, const Vector3<T>& g, const Vector3<T>& p, T& b1, T& b2)
	{
		// BP: See my work notes for workings

		Vector3<T> a1 = f-e, a2 = g-e;

		static const double SOME_SMALL_NUMBER = 0.0001;

		T b2denom = a2.y()*a1.x() - a1.y()*a2.x();
		if (abs(b2denom) < SOME_SMALL_NUMBER)
		{
			// calc b2 another way
			b2denom = a2.y()*a1.z() - a1.y()*a2.z();
			if (abs(b2denom) < SOME_SMALL_NUMBER)
			{
				b2denom = a2.x()*a1.z() - a1.x()*a2.z();
				b2 = ((p.x()-e.x())*a1.z()-(p.z()-e.z())*a1.x())/b2denom;
			}
			else
			{
				b2 = ((p.y()-e.y())*a1.z()-(p.z()-e.z())*a1.y())/b2denom;
			}
		}
		else
		{
			b2 = ((p.y()-e.y())*a1.x() - (p.x()-e.x())*a1.y())/b2denom;
		}

		if (abs(a1.x()) < SOME_SMALL_NUMBER)
		{
			if (abs(a1.y()) >= SOME_SMALL_NUMBER)
			{
				b1 = (p.y()-e.y()-a2.y()*b2)/a1.y();
			}
			else
			{
				b1 = (p.z()-e.z()-a2.z()*b2)/a1.z();
			}
		}
		else
		{
			b1 = (p.x()-e.x()-a2.x()*b2)/a1.x();
		}
	}


	double volumeOfSphereGivenRadius(double radius)
	{
		return (4.0/3)*PI*radius*radius*radius;
	}

	double radiusOfSphereGivenVolume(double vol)
	{
		return Math::pow((.75/PI)*vol,1/3.0);
	}
}
