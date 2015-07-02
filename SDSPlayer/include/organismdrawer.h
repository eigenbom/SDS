/*
 * organismviewer.h
 *
 *  Created on: 16/10/2008
 *      Author: ben
 */

#ifndef ORGANISMVIEWER_H_
#define ORGANISMVIEWER_H_

#include "meshdrawer.h"
#include "cell.h"
#include "organism.h"

#include <QGLWidget>

class OrganismDrawer: public MeshDrawer {
public:
	OrganismDrawer(QGLWidget* qglw);
	virtual ~OrganismDrawer();

	void setOrganism(Organism*);
	// draw the organism, depending on Mode
	void draw();
	void drawCell(Cell* c);

	/// draw the vertices with names for selection
	void drawVertsWithNames();
	void drawEdgesWithNames();
	void drawTetsWithNames();

	public:
	/// Drawing Mode
	enum OMode{DOrganism, DMesh};
	void setMode(OMode m){mOMode = m;}
	void setMode(Mode m){mOMode = DMesh; MeshDrawer::setMode(m);}

	protected:

	OMode mOMode;
	QGLWidget* mQGLW;
	Organism* mOrganism;

	void drawOrganism();

	// std::list<boost::any> mSelectedElements;
};

#endif /* ORGANISMVIEWER_H_ */
