#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <string>
#include <cstdlib>

inline int exitwith(int line, std::string fail)
{
	std::cout << "fail at " << line << ": " << fail << std::endl;
	std::exit(0);
	return 0;
}

#define sdstest(condition,fail) \
	(((condition) == false)?exitwith( __LINE__,(fail)):0)

void transformtests();
void divideTetraTest();
void edgeDivideTest();

void sdsLoaderTest();

#endif
