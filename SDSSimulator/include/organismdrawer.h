/*
 * organismviewer.h
 *
 *  Created on: 16/10/2008
 *      Author: ben
 */

#ifndef ORGANISMDRAWER_H_
#define ORGANISMDRAWER_H_

#include "meshdrawer.h"
#include "cell.h"
#include "organism.h"

class OrganismDrawer: public MeshDrawer {
public:
	OrganismDrawer();
	virtual ~OrganismDrawer();

	void setOrganism(Organism*);
	// draw the organism, depending on Mode
	void draw();
	void drawCell(Cell* c);

	public:
	/// Drawing Mode
	enum OMode{DOrganism, DMesh};
	void setMode(OMode m){mOMode = m;}
	void setMode(Mode m){mOMode = DMesh; MeshDrawer::setMode(m);}

	protected:

	OMode mOMode;
	Organism* mOrganism;

	void drawOrganism();

};

#endif /* ORGANISMDRAWER_H_ */
