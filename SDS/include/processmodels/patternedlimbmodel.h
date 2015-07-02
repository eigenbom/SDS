/*
 * patternedlimbmodel.h
 *
 *  Created on: 10/02/2010
 *      Author: ben
 */

#ifndef PATTERNEDLIMBMODEL_H_
#define PATTERNEDLIMBMODEL_H_

#include "limbbudmodel.h"
#include "organism.h"
#include "bstreamable.h"
#include "random.h"

#include <ostream>
#include "libconfig.h++"

/**
 * The Striped Limb acts just like regular limb growth but additionally deposits stripes along the limb.
 *
 * The system uses three morphogens..
 *	 0 - limb growth morphogen
 *   1 - growing tip timer
 *   2 - stripe indicator
 */
class StripedLimb: public LimbGrowth
{
public:
	StripedLimb();
	virtual ~StripedLimb(){}
	virtual void setup();

	virtual void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "StripedLimb";}

	virtual void writeStatic(std::ostream& o);
	virtual void readStaticParams(std::istream& i);

protected:

	double stripeDecay;
	double mPassiveCellGrowthRate;
	double mStripedCellGrowthRate;
};

#endif /* PATTERNEDLIMBMODEL_H_ */
