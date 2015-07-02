#include "meshdrawer.h"

#include "shapes.h"
#include "vector3.h"

#include <GL/gl.h>

#include <boost/foreach.hpp>
#include <algorithm>

MeshDrawer::MeshDrawer()
:mMode(DTetra)

// drawing parameter defaults
,mDrawForces(false)
,mDrawVelocities(false)
,mDrawFaceNormals(false)
,mDrawVertexNormals(false)
,mDrawHistory(false)

,mColour(1,1,1)
,mColourAlpha(1)
,mTetShrinkAmount(0.95)
{

}

void MeshDrawer::draw(Mesh* m)
{
	switch (mMode)
	{
		case DTetra: {drawDTetra(m); break;}
		case DEdge: {drawDEdge(m); break;}
		case DFace: {drawDFace(m); break;}
		case DFaceSmooth: {drawDFaceSmooth(m); break;}
		case DVertex: {drawDVertex(m); break;}
		case DMass: {drawDMass(m); break; }
		case DFaceEdges: {drawDFaceEdges(m); break;}
		case DWire: {drawDWire(m); break; }
	}

	if (mDrawForces) drawForces(m);
	if (mDrawVelocities) drawVelocities(m);
	if (mDrawFaceNormals) drawFaceNormals(m);
	if (mDrawVertexNormals) drawVertexNormals(m);
	if (mDrawHistory) drawHistory(m);

	BOOST_FOREACH(const boost::any& el, mSelectedElements)
	{
		glColor3f(.9,.5,0);
		if (!el.empty())
		{
			if (el.type()==typeid(Vertex*))
				{
					glPointSize(10);
					glBegin(GL_POINTS);
						glVertex3dv((const double*)(boost::any_cast<Vertex*>(el)->x()));
					glEnd();
				}
			else if (el.type()==typeid(Edge*))
				{
					Edge* e = boost::any_cast<Edge*>(el);
					glPushAttrib(GL_LINE_BIT);
					glLineWidth(10);

					glBegin(GL_LINES);
							const Vector3d& v1 = e->cv(0).x();
							const Vector3d& v2 = e->cv(1).x();
							glVertex3dv((const double*)(v1));
							glVertex3dv((const double*)(v2));
					glEnd();
					glPopAttrib();
				}
			// else if ...
		}
	}
}

void MeshDrawer::selectElement(boost::any el)
{
	mSelectedElements.push_back(el);
}
/*
void MeshDrawer::deselectElement(boost::any el)
{
	mSelectedElements.remove(el);
}*/

void MeshDrawer::deselectAllElements()
{
	mSelectedElements.clear();
}

void MeshDrawer::drawDTetra(Mesh* mMesh)
{
	glEnable(GL_LIGHTING);

	glBegin(GL_TRIANGLES);

	BOOST_FOREACH(const Tetra* t, mMesh->tetras())
	{
		glColor3d(mColour.x(),mColour.y(),mColour.z());

		Vector3d c = t->center();
		Vector3d v[] = {t->cv(0).x(),t->cv(1).x(),t->cv(2).x(),t->cv(3).x()};
		Vector3d diff[] = {v[0]-c,v[1]-c,v[2]-c,v[3]-c};

		for(int i=0;i<4;++i)
			v[i] -= diff[i]*(1-mTetShrinkAmount);

		if (t->intersected())
			glColor3f(1,0,0);

		Vector3d faces[] = {v[0],v[2],v[1],v[0],v[1],v[3],v[1],v[2],v[3],v[3],v[2],v[0]};
		for(int k=0;k<4;++k)
		{
			glNormal3dv((const double*)(cross(faces[3*k+1]-faces[3*k],faces[3*k+2]-faces[3*k]).normalise()));

			for(int j=0;j<3;++j)
			{
				glVertex3dv((const double*)(faces[k*3+j]));
			}
		}
	}
	glEnd();
}

void MeshDrawer::drawDFace(Mesh* mMesh)
{
	glEnable(GL_LIGHTING);

	setColour();
	glBegin(GL_TRIANGLES);
	BOOST_FOREACH(const Face* f, mMesh->outerFaces())
	{
		Vector3d faces[] = {f->cv(0).x(),f->cv(1).x(),f->cv(2).x()};
		Vector3d norm = f->n();
		for(int j=0;j<3;++j)
		{
			glNormal3dv((const double*)norm);
			glVertex3dv((const double*)faces[j]);
		}
	}
	glEnd();
}

void MeshDrawer::drawDFaceSmooth(Mesh* mMesh)
{
	glEnable(GL_LIGHTING);

	setColour();
	glBegin(GL_TRIANGLES);
	BOOST_FOREACH(const Face* f, mMesh->outerFaces())
	{
		Vector3d faces[] = {f->cv(0).x(),f->cv(1).x(),f->cv(2).x()};
		Vector3d norms[] = {f->cv(0).n(),f->cv(1).n(),f->cv(2).n()};
		for(int j=0;j<3;++j)
		{
			glNormal3dv((const double*)norms[j]);
			glVertex3dv((const double*)faces[j]);
		}
	}
	glEnd();
}

void MeshDrawer::drawDEdge(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glLineWidth(1);
	setColour();
	glBegin(GL_LINES);

	BOOST_FOREACH(const Edge* e, mMesh->edges())
	{
		const Vector3d& v1 = e->cv(0).x();
		const Vector3d& v2 = e->cv(1).x();

		// XXX: GLdouble* == double*? [unsafe]
		glVertex3dv((const double*)(v1));
		glVertex3dv((const double*)(v2));
	}
	glEnd();

	drawDVertex(mMesh);

	glPopAttrib();
}

void MeshDrawer::drawDVertex(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glPointSize(3);
	setColour();
	glBegin(GL_POINTS);
	BOOST_FOREACH(const Vertex* v, mMesh->vertices())
	{
		// XXX: GLdouble* == double*? [unsafe]
		glVertex3dv((const double*)(v->x()));
	}
	glEnd();

	glPopAttrib();
}

void MeshDrawer::drawDMass(Mesh* m)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glEnable(GL_LIGHTING);

	static GLuint dl = 0;
	if (dl == 0)
	{
		dl = glGenLists(1);
		glNewList(dl,GL_COMPILE);
			Shapes::Sphere(1,8);
		glEndList();
	}

	//double densityModifier = std::pow(1.0/Verlet::DENSITY,1.0/3.0);

	this->setColour();
	BOOST_FOREACH(const Vertex* v, m->vertices())
	{
		const Vector3d& c = v->x();
		// double r = densityModifier*v->r();
		double r = v->r();

		glPushMatrix();
			glTranslatef(c.x(),c.y(),c.z());
			glScalef(r,r,r);
			glCallList(dl);
		glPopMatrix();
	}

	glPopAttrib();
}

void MeshDrawer::drawDFaceEdges(Mesh* m)
{
	//glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glEnable(GL_LIGHTING);
	glColor3f(1,1,1);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1,1);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glBegin(GL_TRIANGLES);
	BOOST_FOREACH(const Face* f, m->outerFaces())
	{
		Vector3d faces[] = {f->cv(0).x(),f->cv(1).x(),f->cv(2).x()};
		Vector3d norm = f->n();
		for(int j=0;j<3;++j)
		{
			glNormal3dv((const double*)norm);
			glVertex3dv((const double*)faces[j]);
		}

	}
	glEnd();
	//glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glLineWidth(2);
	glColor3f(0,0,0);

	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_TRIANGLES);
		BOOST_FOREACH(const Face* f, m->outerFaces())
		{
			Vector3d faces[] = {f->cv(0).x(),f->cv(1).x(),f->cv(2).x()};
			for(int j=0;j<3;++j)
			{
				glVertex3dv((const double*)faces[j]);
			}
		}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	glPopAttrib();
}

void MeshDrawer::drawDWire(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glLineWidth(1);
	glBegin(GL_LINES);

	BOOST_FOREACH(const Edge* e, mMesh->edges())
	{
		if (e->cv(0).surface() and e->cv(1).surface())
			glColor4f(mColour[0],mColour[1],mColour[2],1);
		else
			glColor4f(mColour[0],mColour[1],mColour[2],0.25);
		const Vector3d& v1 = e->cv(0).x();
		const Vector3d& v2 = e->cv(1).x();

		// XXX: GLdouble* == double*? [unsafe]
		glVertex3dv((const double*)(v1));
		glVertex3dv((const double*)(v2));
	}
	glEnd();

	drawDVertex(mMesh);

	glPopAttrib();

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glPointSize(3);
	setColour();
	glBegin(GL_POINTS);
	BOOST_FOREACH(const Vertex* v, mMesh->vertices())
	{
		// XXX: GLdouble* == double*? [unsafe]
		glVertex3dv((const double*)(v->x()));
	}
	glEnd();

	glPopAttrib();
}

void MeshDrawer::drawFaceNormals(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	BOOST_FOREACH(Face* f, mMesh->outerFaces())
	{
		Vector3d from = f->center();
		Vector3d n = f->n();
		glTranslatef(from.x(),from.y(),from.z());
			drawVector(n);
		glTranslatef(-from.x(),-from.y(),-from.z());
	}

	glPopAttrib();
}

void MeshDrawer::drawVertexNormals(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	BOOST_FOREACH(const Vertex* v, mMesh->vertices())
	{
		if (v->surface())
		{
			Vector3d from = v->x();
			Vector3d n = v->n();
			glTranslatef(from.x(),from.y(),from.z());
			drawVector(n);
			glTranslatef(-from.x(),-from.y(),-from.z());
		}
	}

	glPopAttrib();
}

void MeshDrawer::drawHistory(Mesh* m)
{
	glEnable(GL_LIGHTING);

	glColor4d(mColour.x(),mColour.y(),mColour.z(),.5);
	glBegin(GL_TRIANGLES);
	BOOST_FOREACH(const Face* f, m->outerFaces())
	{
		Vector3d faces[] = {f->cv(0).ox(),f->cv(1).ox(),f->cv(2).ox()};
		Vector3d norm = f->n();
		for(int j=0;j<3;++j)
		{
			glNormal3dv((const double*)norm);
			glVertex3dv((const double*)faces[j]);
		}
	}
	glEnd();

	glDisable(GL_LIGHTING);

	// draw some faint lines from old to new vertices

	glColor4d(0,0,0,1);
	glBegin(GL_LINES);
	BOOST_FOREACH(const Vertex* v, m->vertices())
	{
		glVertex3dv((const double*)v->ox());
		glVertex3dv((const double*)v->x());
	}
	glEnd();

}




void MeshDrawer::setColour()
{
	glColor3d(mColour.x(),mColour.y(),mColour.z());
}

void MeshDrawer::drawVector(const Vector3d& v, Vector3d fc, Vector3d tc)
{
	glBegin(GL_LINES);
		glColor3f(fc.x(),fc.y(),fc.z());
		glVertex3f(0,0,0);
		glColor3f(tc.x(),tc.y(),tc.z());
		glVertex3dv((const double*)v);
	glEnd();
}

void MeshDrawer::drawForces(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	BOOST_FOREACH(const Vertex* v, mMesh->vertices())
	{
		const Vector3d& f = v->f(), &x = v->x();
		glTranslatef(x.x(),x.y(),x.z());
		drawVector(f);
		glTranslatef(-x.x(),-x.y(),-x.z());
	}

	glPopAttrib();
}

void MeshDrawer::drawVelocities(Mesh* mMesh)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	BOOST_FOREACH(const Vertex* vert, mMesh->vertices())
	{
		const Vector3d& v = vert->v(), &x = vert->x();
		glTranslatef(x.x(),x.y(),x.z());
		drawVector(v);
		glTranslatef(-x.x(),-x.y(),-x.z());
	}

	glPopAttrib();
}
