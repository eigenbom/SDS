
namespace Math
{
	double step(double a, double x) //0 if a<x, else 1
	{
		return (double)(x>=a);
	}

	double pulse(double a, double b, double x) //if x<a or x>b then 0, else 1
	{
		return step(a,x) - step(b,x);
	}

	 double clamp(double a, double b, double x)
	{
		if (x<a) return a;
		else if (x>b) return b;
		else return x;
	}

	double lerp(double a, double b, double t)
	{
		return a + (b-a)*t;
	}

	/*
	double abs(double a)
	{
		return a<0?-a:a;
	}*/

	int sign(double a) // 1 if a>=0 else -1
	{
		return (a<0)?-1:1;
	}
	
	double smoothstep(double a, double b, double x) //a smooth step from a to b
	{
		if (x<a) return 0;
		else if (x>=b) return 1;

		x = (x-a)/(b-a);
		return x*x*(3-2*x);
	}

	int powi(int m, int n) // m^n
	{
		int t = 1;
		int i;
		for(i=0;i<n;i++)
			t *= m;
		return t;
	}

	double mod(double a, double b)
	{
		int n = (int)(a/b);
		a -= n*b;
		if (a<0) a+=b;
		return a;
	}

	double gammacorrect(double gamma, double x)
	{
		return pow(x,1/gamma);
	}

	double bias(double b, double x)
	{
		return pow(x, (double)(log(b)/log(0.5)));
	}

	double gain(double g, double x)
	{
		if (x<0.5)
			return bias(1-g,2*x)/2;
		else
			return 1-bias(1-g,2-2*x)/2;
	}
}

