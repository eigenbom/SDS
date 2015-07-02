/*
 * annotation.cpp
 *
 *  Created on: 23/09/2009
 *      Author: ben
 */
#include "annotation.h"

#include "glutils.h"

#include <GL/gl.h>

void Annotation::setHighlightColour()
{
	glColor4f(1,1,.6,.8);
}

void VertexAnnotation::draw()
{
	glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glPointSize(10);
	setHighlightColour();

	Vertex* v = data();
	glBegin(GL_POINTS);
	glVertex3dv(v->x());
	glEnd();

	glPopAttrib();
}

void EdgeAnnotation::draw()
{
	glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(5);
	setHighlightColour();

	Edge* e = data();
	// data = Edge*
	Vector3d from = e->v(0)->x();
	Vector3d to = e->v(1)->x();
	glBegin(GL_LINES);
	glVertex3dv(from);
	glVertex3dv(to);
	glEnd();

	glPopAttrib();
}

void FaceAnnotation::draw()
{
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	setHighlightColour();

	switch(mMode)
	{
		case 0:
		{
			Face* f = boost::any_cast<Face*>(mData);
			drawTri(f->v(0).x(),f->v(1).x(),f->v(2).x(),true);
			break;
		}
		case 1:
		{
			boost::tuple<Vector3d,Vector3d,Vector3d> tup = boost::any_cast<boost::tuple<Vector3d,Vector3d,Vector3d> >(mData);
			Vector3d v0 = boost::get<0>(tup), v1 = boost::get<1>(tup), v2 = boost::get<2>(tup);
			drawTri(v0,v1,v2,true);
			break;
		}
		case 2:
		{
			boost::tuple<Vertex*,Vertex*,Vertex*> tup = boost::any_cast<boost::tuple<Vertex*,Vertex*,Vertex*> >(mData);
			Vector3d v0 = boost::get<0>(tup)->x(), v1 = boost::get<1>(tup)->x(), v2 = boost::get<2>(tup)->x();
			drawTri(v0,v1,v2,true);
		}
	}

	glPopAttrib();
}

void CellAnnotation::draw()
{
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	setHighlightColour();

	Cell* c = data();

	const Vector3d& x = c->v()->x();
	double r = c->r();
	glPushMatrix();
	glTranslatef(x.x(),x.y(),x.z());
	glScalef(r,r,r);
	drawSphere();
	glPopMatrix();

	glPopAttrib();
}

void TetraAnnotation::draw()
{
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	setHighlightColour();

	switch (mMode)
	{
		case 0:
		{
			drawTetra(boost::any_cast<Tetra*>(mData),.9);
			break;
		}
	}
	glPopAttrib();

}
