#include "organismdrawer.h"

#include "cell.h"
#include "organism.h"
#include "processmodel.h"

#include "vector3.h"
#include "matrix4.h"
#include "random.h"

#include "palette.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <boost/foreach.hpp>

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

void OrganismDrawer::drawCell(Cell* c)
{
	LimbBudModel::CD* cd;
	if (cd = dynamic_cast<LimbBudModel::CD*>(c->processInformation()))
	{
		// color amount based on morphogen concentration
		double m0 = 1 - cd->morphogenInfo.array[0].value/c->vol();

		glColor4f(m0,m0,m0,0.5);
		const Vector3d& x = c->v()->x();
		double r = c->r();
		glPushMatrix();
		glTranslatef(x.x(),x.y(),x.z());
		glScalef(r,r,r);
		drawSphere();
		glPopMatrix();

		GLdouble modelmtx[16], projmtx[16];
		GLint viewport[4];

		glGetDoublev(GL_MODELVIEW_MATRIX,modelmtx);
		glGetDoublev(GL_PROJECTION_MATRIX,projmtx);
		glGetIntegerv(GL_VIEWPORT,viewport);
		GLdouble win[3];
		gluProject(x.x(),x.y(),x.z(),modelmtx,projmtx,viewport,&win[0],&win[1],&win[2]);

		// transform the "gl window" coordinates to "qt window" coordinates then display
		glColor4f(0,0,0,1);
		//mQGLW->renderText(win[0],mQGLW->height()-win[1],QString::number(1-m0));
		// this->renderText(win[0],height()-win[1],QString("%1f,%2f").arg(win[0],2).arg(win[1],2));
	}
	else
	{
		const Vector3d& x = c->v()->x();
		double r = c->r();
		glPushMatrix();
		glTranslatef(x.x(),x.y(),x.z());
		glScalef(r,r,r);
		drawSphere();
		glPopMatrix();
	}
}

OrganismDrawer::OrganismDrawer(QGLWidget* qglw)
:MeshDrawer()
,mOMode(DOrganism)
,mOrganism(NULL)
{
	// TODO Auto-generated constructor stub

}

OrganismDrawer::~OrganismDrawer() {
	// TODO Auto-generated destructor stub
}

void OrganismDrawer::setOrganism(Organism* o)
{
	mOrganism = o;
}

// draw the organism, depending on Mode
void OrganismDrawer::draw()
{
	if (mOrganism==NULL) return;
	// draw moving elements

	if (mOMode==DOrganism)
	{
		drawOrganism();

		if (mDrawForces) drawForces(mOrganism->mesh());
		if (mDrawVelocities) drawVelocities(mOrganism->mesh());
		if (mDrawFaceNormals) drawFaceNormals(mOrganism->mesh());
		if (mDrawVertexNormals) drawVertexNormals(mOrganism->mesh());
		if (mDrawHistory) drawHistory(mOrganism->mesh());
	}
	else if (mOMode==DMesh)
	{
		MeshDrawer::draw(mOrganism->mesh());
	}
}

void OrganismDrawer::drawVertsWithNames()
{
	if (mOrganism==NULL) return;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glPointSize(5.0);

	int i = 0;
	BOOST_FOREACH(const Vertex* v, mOrganism->mesh()->vertices())
	{
		glPushName(i);
		glBegin(GL_POINTS);
		glVertex3dv((const double*)(v->x()));
		glEnd();
		glPopName();
		i++;
	}

	glPopAttrib();
}

void OrganismDrawer::drawEdgesWithNames(){}
void OrganismDrawer::drawTetsWithNames(){}

class DepthSorter
{
public:
	DepthSorter(Matrix4d& transform):mTransform(transform){}

	// returns true if a is further than b (i.e., z value is more negative)
	bool operator()(Cell* a, Cell* b)
	{
		if ((mTransform * a->v()->x()).z() <= (mTransform * b->v()->x()).z())
			return true;
		else
			return false;
	}

	Matrix4d& mTransform;
};


// draw the cells of the organism, with edges
void OrganismDrawer::drawOrganism()
{
	glPushAttrib(GL_LIGHTING_BIT);
	glPushAttrib(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// sort the cells back to front
	Matrix4d m4d;
	glGetDoublev(GL_MODELVIEW_MATRIX,(double*)m4d);
	std::list<Cell*> cells = mOrganism->cells();
	cells.sort<DepthSorter>(DepthSorter(m4d));

	static std::map<Cell*,Vector3d> cellColours;
	static Random cellColourRandom;

	BOOST_FOREACH(Cell* c, cells)
	{
		if (cellColours.find(c)==cellColours.end())
		{
			int i = cellColourRandom.getInt(0,palette_size);
			cellColours[c] = Vector3d(organism_palette[i%palette_size][0]/255.0,organism_palette[i%palette_size][1]/255.0,organism_palette[i%palette_size][2]/255.0);
		}
		Vector3d col = cellColours[c];
		glColor4f(col[0],col[1],col[2],.5);

		drawCell(c);
		/*
		const Vector3d& x = c->v()->x();
		double r = c->r();
		glPushMatrix();
			glTranslatef(x.x(),x.y(),x.z());
			glScalef(r,r,r);
			drawSphere();
		glPopMatrix();
		*/
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glColor4f(0,0,0,.1);

	glLineWidth(1);
	glBegin(GL_LINES);

	BOOST_FOREACH(const Edge* e, mOrganism->mesh()->edges())
	{
		const Vector3d& v1 = e->cv(0).x();
		const Vector3d& v2 = e->cv(1).x();
		glVertex3dv((const double*)(v1));
		glVertex3dv((const double*)(v2));
	}
	glEnd();

	glEnable(GL_DEPTH_TEST);

	glPopAttrib();
	glPopAttrib();
}


