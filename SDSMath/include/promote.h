/***********************************************************************
 * promote.h   Arithmetic type promotion trait class
 * Author: Todd Veldhuizen         (tveldhui@seurat.uwaterloo.ca)
 *
 * This program may be distributed in an unmodified form.  It may not be
 * sold or used in a commercial product.
 *
 * For more information on these template techniques, please see the
 * Blitz++ Numerical Library Project, at URL http://monet.uwaterloo.ca/blitz/
 *
 * This trait class provides a mechanism for type promotion on vectors,
 * matrices, and arrays.  For example, when you add a Vector<int> to
 * a Vector<double>, the result should be promoted to a Vector<double>.
 *
 * Sample usage:
 * template<class A, class B>
 * Vector<promote_trait<A,B>::T_promote> operator+(const Vector<A>&,
 *      const Vector<B>&);
 */

#ifndef PROMOTE_H
#define PROMOTE_H

class Warning_promote_trait_not_specialized_for_this_case { };

template<class A, class B>
class promote_trait {
public:
        typedef Warning_promote_trait_not_specialized_for_this_case T_promote;
};

template<> class promote_trait<char, char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<char, unsigned char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<char, short int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<char, short unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<char, int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<char, unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<char, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<char, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<char, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<char, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<char, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<unsigned char, char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<unsigned char, unsigned char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<unsigned char, short int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<unsigned char, short unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned char, int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<unsigned char, unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned char, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<unsigned char, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned char, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<unsigned char, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<unsigned char, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<short int, char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<short int, unsigned char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<short int, short int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<short int, short unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short int, int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<short int, unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short int, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<short int, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<short int, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<short int, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<short int, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<short unsigned int, char> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short unsigned int, unsigned char> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short unsigned int, short int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short unsigned int, short unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short unsigned int, int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short unsigned int, unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<short unsigned int, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<short unsigned int, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<short unsigned int, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<short unsigned int, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<short unsigned int, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<int, char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<int, unsigned char> {
public:
	typedef int T_promote;
};

template<> class promote_trait<int, short int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<int, short unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<int, int> {
public:
	typedef int T_promote;
};

template<> class promote_trait<int, unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<int, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<int, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<int, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<int, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<int, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<unsigned int, char> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned int, unsigned char> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned int, short int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned int, short unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned int, int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned int, unsigned int> {
public:
	typedef unsigned int T_promote;
};

template<> class promote_trait<unsigned int, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<unsigned int, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned int, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<unsigned int, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<unsigned int, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long, char> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, unsigned char> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, short int> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, short unsigned int> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, int> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, unsigned int> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, long> {
public:
	typedef long T_promote;
};

template<> class promote_trait<long, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<long, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<long, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<long, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<unsigned long, char> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, unsigned char> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, short int> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, short unsigned int> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, int> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, unsigned int> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, unsigned long> {
public:
	typedef unsigned long T_promote;
};

template<> class promote_trait<unsigned long, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<unsigned long, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<unsigned long, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<float, char> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, unsigned char> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, short int> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, short unsigned int> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, int> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, unsigned int> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, long> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, unsigned long> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, float> {
public:
	typedef float T_promote;
};

template<> class promote_trait<float, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<float, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<double, char> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, unsigned char> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, short int> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, short unsigned int> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, int> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, unsigned int> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, long> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, unsigned long> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, float> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, double> {
public:
	typedef double T_promote;
};

template<> class promote_trait<double, long double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, char> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, unsigned char> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, short int> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, short unsigned int> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, int> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, unsigned int> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, long> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, unsigned long> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, float> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, double> {
public:
	typedef long double T_promote;
};

template<> class promote_trait<long double, long double> {
public:
	typedef long double T_promote;
};

#endif
