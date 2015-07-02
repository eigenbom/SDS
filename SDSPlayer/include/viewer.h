/*
 * Viewer.h
 *
 *  Created on: 26/03/2009
 *      Author: ben
 */

#ifndef VIEWER_H_
#define VIEWER_H_

#include <QGLViewer/qglviewer.h>
#include <boost/any.hpp>

#include "organismdrawer.h"
#include "organism.h"

/**
 * Viewer provides a 3d view on a mesh or organism.
 * It supports interaction / selection / multiple draw modes / annotations / etc.
 *
 */
class Viewer: public QGLViewer
{
	Q_OBJECT
public:
	Viewer(QWidget* parent = NULL);
	~Viewer();

	void setOrganism(Organism*);
	void reset();

protected:
	void draw();
	void drawWithNames();
	void init();
	virtual void postSelection(const QPoint& p);

public slots:
	void setViewMode(QString mode);
	void setSelectMode(QString mode);

signals:
	void itemSelected(boost::any&);

protected:
	QString mViewMode; // one of "wireframe","surface","cells"
	Organism* mOrganism;
	OrganismDrawer* mDrawer;

	QString mSelectMode;
	boost::any mSelectedItem;
};

#endif /* VIEWER_H_ */
