#ifndef TESTWORLDS_H
#define TESTWORLDS_H

#include "world.h"
#include <string>

/** A set of sample worlds to use for testing etc.
 *
 */
class TestWorlds
{
public:

	static World* loadWorld(std::string);

protected:
	static World* smallSpheres();
	static World* lotsOfSmallSpheres();
	static World* fewCubes();
};

#endif
