/*
 * Viewer.cpp
 *
 *  Created on: 26/03/2009
 *      Author: ben
 */

#include "viewer.h"

#include <QtOpenGL>
#include <QPalette>

#include <GL/gl.h>
#include <GL/glu.h>

#include <iterator>
#include <iostream>

#include "vector3.h"

#include "aabb.h"

Viewer::Viewer(QWidget* parent)
:QGLViewer(parent),mViewMode("surface"),mOrganism(NULL),mDrawer(NULL),mSelectMode("cells"),mSelectedItem()
{
	mDrawer = new OrganismDrawer(this);
	setViewMode(mViewMode);


}

Viewer::~Viewer(){}

void Viewer::setViewMode(QString mode)
{
	mViewMode = mode;
	if (mViewMode=="wireframe")
	{
		mDrawer->setColour(Vector3d(0,0,0));
		mDrawer->setMode(MeshDrawer::DWire);
	}
	else
		mDrawer->setColour(Vector3d(1,1,1));
	if (mViewMode=="surface")
		mDrawer->setMode(MeshDrawer::DFaceEdges);
	else if (mViewMode=="smooth")
		mDrawer->setMode(MeshDrawer::DFaceSmooth);
	else if (mViewMode=="cells")
		mDrawer->setMode(OrganismDrawer::DOrganism);
	updateGL();
}

void Viewer::setSelectMode(QString str)
{
	mSelectMode = str;

	if (str=="cells")
	{

	}
	else if (str=="edges")
	{

	}
	else if (str=="tetrahedra")
	{

	}
}

void Viewer::init()
{
	setBackgroundColor(palette().color(QPalette::Highlight));
	//setBackgroundColor(palette().color(QPalette::Background));
}

template <typename T>
T unsafeGet(const std::list<T>& list, int n)
{
	typename std::list<T>::const_iterator it = list.begin();
	std::advance(it,n);
	return *it;
}

void Viewer::postSelection(const QPoint& point)
{
	bool found;
	qglviewer::Vec selectedPoint = camera()->pointUnderPixel(point, found);
	if (found)
	{
		if (mSelectMode=="cells")
			mSelectedItem = unsafeGet(mOrganism->mesh()->vertices(),selectedName());
		else if (mSelectMode=="edges")
			mSelectedItem = unsafeGet(mOrganism->mesh()->edges(),selectedName());

		emit itemSelected(mSelectedItem);

		mDrawer->deselectAllElements();
		mDrawer->selectElement(mSelectedItem);

		// locate vertex
		//QMessageBox::information(this, "Selected",QString::number(selectedName()));
	}
	else
		;
		//QMessageBox::information(this, "Nothing selected!", "!");
}

void Viewer::drawWithNames()
{
	if (mSelectMode=="cells")
	{
		mDrawer->drawVertsWithNames();
	}
	else if (mSelectMode=="edges")
	{
		mDrawer->drawEdgesWithNames();
	}
	else if (mSelectMode=="tetrahedra")
	{
		mDrawer->drawTetsWithNames();
	}
}

void Viewer::draw()
{
	mDrawer->draw();
}

void Viewer::reset()
{
	if (mOrganism)
		showEntireScene();
}

void Viewer::setOrganism(Organism* o)
{
	// cleanup old mesh?

	// set new mesh
	mOrganism = o;
	mDrawer->setOrganism(mOrganism);

	if (mOrganism)
	{
		// update bounding box
		const AABB& aabb = mOrganism->mesh()->aabb();
		if (aabb.valid())
		{
			Vector3d vmin = aabb.min();
			Vector3d vmax = aabb.max();
			setSceneBoundingBox(qglviewer::Vec(vmin.x(),vmin.y(),vmin.z()),qglviewer::Vec(vmax.x(),vmax.y(),vmax.z()));
		}
	}

	updateGL();
}
