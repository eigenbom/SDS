/*
 * hotspotmodel.h
 *
 *  Created on: 12/02/2010
 *      Author: ben
 */

#ifndef HOTSPOTMODEL_H_
#define HOTSPOTMODEL_H_

#include "processmodel.h"

/**
 * A simple model that allows growth in regions painted with a morphogen.
 * The model accepts parameters of the form ... only up to 3 colours at the moment.
 *
 * processModel: {
 * 	type = "hotspotmodel";
 *  r0: .5;
 *  r1: -.2;
 *  r2: 0;
 *  diffusion0: .5;
 *  decay0: .2;
 *  // ...
 * };
 *
 * Where morphogen 0 invokes growth at a rate of .5
 *                 1 invokes shrinkage at a rate of .2
 */
class HotSpotModel: public ProcessModel
{
public:
	HotSpotModel();
	virtual ~HotSpotModel(){}

	virtual void setup();
	virtual void step(Organism* o, double dt);

	virtual void writeStatic(std::ostream& binaryOStream);

	virtual std::string name(){return "HotSpotModel";}

	// parameter interface, all parameters are doubles..
	/// returns a list of all parameter names
	virtual std::list<std::string> parameters();

	/// set a parameter
	virtual void set(std::string,double val);
	virtual double get(std::string);
protected:
	/// all subclasses with static parameters must implement this
	virtual void readStaticParams(std::istream&);

	double mMorphogenGrowthRates[3];

};

#endif /* HOTSPOTMODEL_H_ */
