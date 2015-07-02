/*
 * hmath.cpp
 *
 *  Created on: 22/02/2010
 *      Author: ben
 */

//#include <gsl/gsl_poly.h>

/**
 * SolveQuadric, SolveCubic defined in Roots3And4.c
 */
extern "C" int SolveQuadric(double c[3], double s[2]);
extern "C" int SolveCubic(double c[4], double s[3]);

// put the smallest of s into smallest, and the biggest into biggest
void order2(double s[2], double& smallest, double& biggest)
{
	int smallestIndex = s[0]<s[1]?0:1;
	smallest = s[smallestIndex];
	biggest = s[1-smallestIndex];
}

// put the smallest of s into smallest, and the biggest into biggest
void order3(double s[3], double& a, double& b, double& c)
{
	bool s01 = s[0]<=s[1];
	bool s02 = s[0]<=s[2];
	bool s12 = s[1]<=s[2];

	if (s01)
	{
		if (s12)
		{
			// 0 < 1 < 2
			a = s[0];
			b = s[1];
			c = s[2];
		}
		else if (s02)
		{
			// 0 < 2 < 1
			a = s[0];
			b = s[2];
			c = s[1];
		}
		else
		{
			// 2 < 0 < 1
			a = s[2];
			b = s[0];
			c = s[1];
		}
	}
	else // 1 < 0
	{
		if (s02)
		{
			// 1 < 0 < 2
			a = s[1];
			b = s[0];
			c = s[2];
		}
		else // 2 < 0
		{
			if (s12)
			{
				// 1 < 2 < 0
				a = s[1];
				b = s[2];
				c = s[0];
			}
			else // 2 < 1
			{
				// 2 < 1 < 0
				a = s[2];
				b = s[1];
				c = s[0];
			}
		}
	}

}


namespace Math
{
	int solveQuadratic(double a, double b, double c, double &root1, double &root2)
	{
		double co[3] = {c,b,a};
		double s[2];

		int n = SolveQuadric(co,s);

		if (n<=0) return 0;
		else if (n==1)
		{
			root1 = s[0];
			return 1;
		}
		else if (n==2)
		{
			order2(s,root1,root2);
			return 2;
		}
		else return -1;
	}

	int solveCubic(double a, double b, double c, double &root1, double &root2, double &root3)
	{
		double co[4] = {c,b,a,1};
		double s[3];

		int n = SolveCubic(co,s);

		if (n<=0) return 0;
		else if (n==1) { root1 = s[0]; return 1; }
		else if (n==2)
		{
			order2(s,root1,root2);
			return 2;
		}
		else if (n==3)
		{
			order3(s,root1,root2,root3);
			return 3;
		}

	}
}
