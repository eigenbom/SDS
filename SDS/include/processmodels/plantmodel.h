#ifndef PLANTMODEL_H
#define PLANTMODEL_H

#include "processmodel.h"

// morphogens
// 0: limb bud
// 1: freeze selection
// 2: budding timer
// 3: bud-activator

// vars
// 0: max cell size when budding

class PlantModel: public ProcessModel
{
public:
	PlantModel();
	virtual void setup();

	virtual ~PlantModel(){}
	virtual void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "PlantModel";}

	virtual void writeStatic(std::ostream& o);
	virtual void readStaticParams(std::istream& i);

protected:
	// stuff here...

	double rE, rM, drdtE, drdtM; // etc.
	double budGrowthFactor, budSize;
};

#endif
