/**
 *  Created on: 16/10/2008
 *      Author: ben
 */

#ifndef ORGANISMTOOLS_H_
#define ORGANISMTOOLS_H_

#include <string>
#include <list>
#include "organism.h"

/**
 * Tools, including: loading.
 *
 */
class OrganismTools {
public:
	static Organism* load(const std::string& name) ;

	// Construct an organism from a TPS mesh.
	static Organism* loadMesh(std::string filename) ;

	// Test Organisms
	static Organism* loadOneTet();
	//static Organism* loadVF();
	//static Organism* loadEESpecial();
	//static Organism* loadEETest(std::string name) ;
	static std::list<std::string> testnames();
};

#endif /* ORGANISMTOOLS_H_ */
