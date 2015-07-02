/* Random: A pseudo-random number generator (Mersenne)
 * - encapsulates state (allowing for more than one rng object at a time)
 *
 * BP 31.07.05
 */

#ifndef RANDOM_H
#define RANDOM_H

class Random
{
	public:

	Random(); 									// uses the time to seed
	Random(unsigned long seed); // user provides a seed

	static Random& rand(){return *mInstance;}

	/* Random number procedures */

	/* Uniform */
	float getFloat(); // R /\ [0,1)
	double getDouble(); // R /\ [0,1)
	int getInt(int low, int high); // Z /\ [low, high)

	/* Gaussian */
	float getFloatG(); // gaussian distrib with mean 0 and variance 1
	float getFloatG(float mean, float sd);

	/* reseed */
	void seed(unsigned long s){_initGenRand(s);}

	/* accessor of the seed array */
	unsigned long seed(){return mt[0];}

	private:

	static const int N;
	static const int M;
	static const unsigned long MATRIX_A;
	static const unsigned long UPPER_MASK;
	static const unsigned long LOWER_MASK;

	unsigned long mt[624];
	int mti;

	// init functions
	void _initGenRand(unsigned long s);
	void _initByArray(unsigned long init_key[], int key_length);

	// gen functions
	unsigned long _genRandInt32();
	long _genRandInt31();
	double _genRandReal();

	static Random* mInstance;

};


#endif
