/*
 * glutils.cpp
 *
 *  Created on: 23/09/2009
 *      Author: ben
 */

#include "glutils.h"
#include <QGLViewer/qglviewer.h>

// helpers
void drawAABB(const AABB& a)
{
	double w[3] =
	{ a.dx(), a.dy(), a.dz() };
	Vector3d min = a.min();

	glBegin(GL_LINES);
	for (int i = 0; i < 3; i++)
	{
		// draw 4 lines along axis i
		Vector3d v1(Vector3d::ZERO), v2(Vector3d::ZERO), d(Vector3d::ZERO);

		d[i] = w[i];
		v1[(i + 2) % 3] = w[(i + 2) % 3];
		v2[(i + 1) % 3] = w[(i + 1) % 3];

		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
			{
				// draw a line between quads at from->to

				// from_pt = min + j*<..,other1,..> + k*<..other2..>
				// to_pt = max - (1-j)*<other1> - (1-k)*<other2>
				Vector3d from = min + ((j == 1) ? v1 : Vector3d::ZERO) + ((k
						== 1) ? v2 : Vector3d::ZERO);
				Vector3d to = from + d;

				glVertex3dv((double*) from);
				glVertex3dv((double*) to);
			}
	}
	glEnd();
}

void drawTetra(const Tetra* t, double scale)
{

	Vector3d c = t->center();
	Vector3d v[] = {t->cv(0).x()-c,t->cv(1).x()-c,t->cv(2).x()-c,t->cv(3).x()-c};
	Vector3d faces[] = {v[0],v[2],v[1],v[0],v[1],v[3],v[1],v[2],v[3],v[3],v[2],v[0]};
	glPushMatrix();
	glTranslatef(c.x(),c.y(),c.z());
	glScalef(scale,scale,scale);
	glBegin(GL_TRIANGLES);
	for(int k=0;k<4;++k)
	{
		glNormal3dv((const double*)(cross(faces[3*k+1]-faces[3*k],faces[3*k+2]-faces[3*k]).normalise()));

		for(int j=0;j<3;++j)
		{
			glVertex3dv((const double*)(faces[k*3+j]));
		}
	}
	glEnd();
	glPopMatrix();
}

void drawTri(const Vector3d& a, const Vector3d& b, const Vector3d& c, bool outline)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	glEnable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1,1);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glBegin(GL_TRIANGLES);
		drawTriInline(a,b,c);
	glEnd();

	if (outline)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_LIGHTING);

		glLineWidth(1);
		glColor3f(0,0,0);

		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glBegin(GL_TRIANGLES);
		drawTriInline(a,b,c,false);
		glEnd();
	}

	glPopAttrib();
}

void drawTriInline(const Vector3d& v0, const Vector3d& v1, const Vector3d& v2, bool withNormal)
{
	if (withNormal)
		glNormal3dv(cross(v1-v0,v2-v0));
	glVertex3dv(v0);
	glVertex3dv(v1);
	glVertex3dv(v2);
}

void drawSphere()
{
	static GLuint dl = 0;
	if (dl == 0)
	{
		dl = glGenLists(1);
		glNewList(dl,GL_COMPILE);
			GLUquadric* quadric = gluNewQuadric();
			gluSphere(quadric,1,16,16);
			gluDeleteQuadric(quadric);
		glEndList();
	}
	glCallList(dl);
}

void drawVector(const Vector3d& v, Vector3d fc, Vector3d tc)
{
	glBegin(GL_LINES);
		glColor3f(fc.x(),fc.y(),fc.z());
		glVertex3f(0,0,0);
		glColor3f(tc.x(),tc.y(),tc.z());
		glVertex3dv((const double*)v);
	glEnd();
}
