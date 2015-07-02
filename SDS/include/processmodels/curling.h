#ifndef PROCESSMODEL_CURLING_H
#define PROCESSMODEL_CURLING_H

#include "limbbudmodel.h"

// Morphogen 1 is the limb stimulation morphogen
// Morphogen 2 is the curling edge selector (and initial d_R)

// Var 1 is the curling factor (d_R) that is propogated to all children

class Curling: public LimbGrowth
{
public:
	Curling();
	virtual void setup();

	virtual ~Curling(){}
	virtual void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "Curling";}

	virtual void writeStatic(std::ostream& o);
	virtual void readStaticParams(std::istream& i);

protected:

	// Inherited:
	// double rE, rM, drdtE, drdtM; // etc.
	double mStim; //stimulation threshold
	double mCurlingFactor;
};

#endif
