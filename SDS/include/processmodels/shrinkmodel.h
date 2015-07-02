/*
 * shrinkmodel.h
 *
 *  Created on: 22/12/2009
 *      Author: ben
 */

#ifndef SHRINKMODEL_H_
#define SHRINKMODEL_H_

#include "processmodel.h"
#include "organism.h"
#include "bstreamable.h"
#include "random.h"
#include "limbbudmodel.h"

#include <ostream>
#include "libconfig.h++"

/**
 * Another model of limb growth.
 */
/*
class ShrinkModel: public ProcessModel
{
public:
	// the CellDataType includes a cell type enumerator, and the morphogen information
	class CD: public CellContents
	{
	public:
		CD();
		virtual void write(std::ostream& f);
		virtual void read(std::istream& f);

		virtual void setMorphogen(int morphogenIndex, double value);
		virtual double getMorphogen(int morphogenIndex);
		virtual void setType(int type);
		virtual int getType();
		virtual int numMorphogens();

		MorphogenArray<3> morphogenInfo; // NB: we keep three morphogens to maintain compatibility with LimbBudModel, but we only use one
		int type; // >0 active tip
	};
	virtual CellContents* newCellContents(){return new CD();}
	virtual std::string name(){return "ShrinkModel";}

	virtual void saveToSetting(libconfig::Setting& setting);

	ShrinkModel();

	void step(Organism* o, double dt); // simulate diffusion and activate rules
	void updateDiffusionParams();

	double diffusion, decay, rM, rE, drdtM, drdtE, shrinkRate, minTipRadius;


	//double diffusionFgf8, diffusionFgf10, decayFgf8, decayFgf10,
	//		aerS, aerR, pzSE, pzSM, pzR, rE, rM, drdtE, drdtM;


	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

protected:
	virtual void loadParamsFromSetting(libconfig::Setting& s);

	// derived from the above parameters at initialisation
	double diffusionRates[1];
	double decayRates[1];
};
*/


#endif /* SHRINKMODEL_H_ */
