/*
 * organismviewer.cpp
 *
 *  Created on: 21/09/2009
 *      Author: ben
 */

#include "organismviewer.h"

#include <boost/foreach.hpp>
#include <cfloat>

#include <QApplication>

#include "limbbudmodel.h"

#include "matrix4.h"
#include "random.h"

#include "glutils.h"

const int gPaletteSize = 5;
const Vector3d gPalette[gPaletteSize] =
{ Vector3d(1, 1, 1), Vector3d(188. / 255, 200. / 255, 230. / 255), Vector3d(85.
		/ 255, 112. / 255, 158. / 255), Vector3d(34. / 255, 52. / 255, 97.
		/ 255), Vector3d(11. / 255, 20. / 255, 57. / 255) };

OrganismViewer::OrganismViewer(QWidget* parent) :
	QGLViewer(parent)

	, mOrganism(NULL)
	, mWorldBounds()

	, mDrawBounds(false)
	, mDrawGround(false)
	, mDrawOrigin(false)

	, mDrawForces(false)
	, mDrawVelocities(false)
	, mDrawFaceNormals(false)
	, mLighting(true)

	, mAlpha(1)
	, mGamma(1)

	, mSelectedMorphogen(-1)
	, mSelectedVariable(-1)
	, mDrawMorphogensNormalised(false)
	, mDrawMorphogenLabels(false)

	, mDrawMode("surface")
	, mSelectionMode(SMCell)
{

}

OrganismViewer::~OrganismViewer()
{
}

void OrganismViewer::setOrganism(Organism* o)
{
	mOrganism = o;
	reset();
}

void OrganismViewer::setWorldBounds(const AABB& aabb)
{
	mWorldBounds = aabb;
}

void OrganismViewer::setWorld(OWorld* o, bool rvp)
{
	mOWorld = o;

	if (o==NULL)
	{
		mOrganism = NULL;
	}
	else
	{
		mOrganism = o->organism();
		mWorldBounds = o->bounds();
		reset(rvp);
	}
}

void OrganismViewer::addStaticMesh(Mesh* m)
{
	mStaticMeshes.push_back(m);
}

void OrganismViewer::addAnnotation(Annotation* a)
{
	mAnnotations.push_back(a);
}

void OrganismViewer::clearAnnotations()
{
	BOOST_FOREACH(Annotation* a, mAnnotations)
		delete a;
	mAnnotations.clear();

}

void OrganismViewer::setDrawBounds(bool t)
{
	mDrawBounds = t;
	updateGL();
}

void OrganismViewer::setDrawOrigin(bool t)
{
	mDrawOrigin = t;
	updateGL();
}

void OrganismViewer::setDrawGround(bool t)
{
	mDrawGround = t;
	updateGL();
}

void OrganismViewer::setDrawForces(bool t)
{
	mDrawForces = t;
	updateGL();
}

void OrganismViewer::setDrawVelocities(bool t)
{
	mDrawVelocities = t;
	updateGL();
}

void OrganismViewer::setDrawFaceNormals(bool t)
{
	mDrawFaceNormals = t;
	updateGL();
}

void OrganismViewer::setDrawMode(QString mode)
{
	mDrawMode = mode;
	updateGL();
}

void OrganismViewer::setAlpha(int alpha)
{
	mAlpha = alpha*0.01;
	updateGL();
}

void OrganismViewer::setGamma(int gamma)
{
	// map gamma > 50 to 1+(gamma-50)/10
	// map gamma < 50 to 1-((50-gamma)/50)
	if (gamma>=50) mGamma = 1 + (gamma-50)/10.;
	else if (gamma < 50) mGamma = 1-(50.-gamma)/50;
	updateGL();
}

void OrganismViewer::setLighting(bool lighting)
{
	mLighting = lighting;
	updateGL();
}

void OrganismViewer::setDrawMorphogensNormalised(bool n)
{
	mDrawMorphogensNormalised = n;
	updateGL();
}

void OrganismViewer::setDrawMorphogenLabels(bool d)
{
	mDrawMorphogenLabels = d;
	updateGL();
}

void OrganismViewer::setSelectionMode(QString mode)
{
	if (mode=="cell")
		mSelectionMode = SMCell;
	else if (mode=="edge")
		mSelectionMode = SMEdge;
	else if (mode=="vertex")
		mSelectionMode = SMVertex;
	else if (mode=="face")
		mSelectionMode = SMFace;

	updateGL();
}

void OrganismViewer::selectMorphogen(int m)
{
	mSelectedMorphogen = m;
	mSelectedVariable = -1;
	updateGL();
}

void OrganismViewer::selectVariable(int m)
{
	mSelectedVariable = m;
	mSelectedMorphogen = -1;
	updateGL();
}

void OrganismViewer::init()
{
	setBackgroundColor(palette().color(QPalette::Highlight));

	glShadeModel(GL_SMOOTH);

	glLineWidth(1.0);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// light setup
	GLfloat zero[] =
	{ 0, 0, 0, 1.0 };
	GLfloat mat_spec[] =
	{ .5, .5, .5, 1.0 };
	GLfloat mat_shine[] =
	{ 50 };
	GLfloat lmodel_ambient[] =
	{ 0, 0, 0, 1.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shine);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zero);

	// key light
	GLfloat light0_pos[] =
	{ -0.3, 0.6, 1.0, 0.0 };
	GLfloat white_light[] =
	{ .9, .9, .9, 1.0 };
	GLfloat light0_spec[] =
	{ 0.3, 0.3, 0.3, 1 };

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
}

void OrganismViewer::draw()
{
	if (mOrganism)
	{
		if (mDrawBounds) drawBounds();
		if (mDrawOrigin) drawAxis();
		if (mDrawGround) drawGround();

		BOOST_FOREACH(Mesh* m, mStaticMeshes)
		{
			this->drawSurface(m);
		}

		drawOrganism();
		drawAnnotations();
	}
	else
	{
		drawAxis();
	}
}

void OrganismViewer::reset(bool rvp)
{
	if (mOrganism != NULL)
	{
		setCurrentBounds();
		if (rvp)
			showEntireScene();
	}
}

void OrganismViewer::setCurrentBounds()
{
	if (mOrganism != NULL)
	{
		// update bounding box
		//const AABB& aabb = mOrganism->mesh()->aabb();
		const AABB& aabb = mWorldBounds;
		if (aabb.valid())
		{
			Vector3d vmin = aabb.min();
			Vector3d vmax = aabb.max();
			setSceneBoundingBox(qglviewer::Vec(vmin.x(), vmin.y(), vmin.z()),
					qglviewer::Vec(vmax.x(), vmax.y(), vmax.z()));
		}
	}
	updateGL();
}

void OrganismViewer::drawBounds()
{
	glPushAttrib(GL_LIGHTING_BIT);
	glPushAttrib(GL_LINE_BIT);

	glDisable(GL_LIGHTING);
	glLineWidth(1);
	glColor3dv((const double*) (gPalette[1]));
	drawAABB(mWorldBounds);

	glPopAttrib();
	glPopAttrib();
}

void OrganismViewer::drawOrganism()
{
	if (mDrawMode=="surface")
	{
		drawSurface();
	}
	else if (mDrawMode=="wire")
	{
		drawWireframe();
	}
	else if (mDrawMode=="cells")
	{
		drawCells();
	}
	else if (mDrawMode=="vertices")
	{
		drawVertices();
	}
}

void OrganismViewer::drawWireframe()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);

	glDisable(GL_LIGHTING);
	glLineWidth(1);
	glPointSize(5);

	glColor3f(0,0,0);
	glBegin(GL_LINES);

	BOOST_FOREACH(Edge* e, mOrganism->mesh()->edges())
	{
		Vertex* v0 = e->v(0), *v1 = e->v(1);
		Cell* c0 = mOrganism->getAssociatedCell(v0), *c1 = mOrganism->getAssociatedCell(v1);

		Vector3d col0 = getContentsColourV(c0), col1 = getContentsColourV(c1);
		double surfaceAlpha = ((e->cv(0).surface() and e->cv(1).surface()))?1:.5;

		/*
		if (e->cv(0).surface() and e->cv(1).surface())
			glColor4f(0,0,0,1);
		else
			glColor4f(0.5,0.5,0.5,1);
		*/

		glColor4d(col0[0],col0[1],col0[2],surfaceAlpha);
		glVertex3dv((const double*)(v0));
		glColor4d(col1[0],col1[1],col1[2],surfaceAlpha);
		glVertex3dv((const double*)(v1));
	}

	glEnd();

	// draw rest length too
	glLineWidth(2.0);
	glColor4f(1,.5,.5,.3);
	glBegin(GL_LINES);
	BOOST_FOREACH(const Edge* e, mOrganism->mesh()->edges())
	{
		const Vector3d& v1 = e->cv(0).x();
		const Vector3d& v2 = e->cv(1).x();

		Vector3d diff = v2-v1;
		double length = diff.length();
		double rest = e->rest();
		double phi = (length-rest)/2.;

		Vector3d p1 = v1 + phi*(diff/length);
		Vector3d p2 = v2 - phi*(diff/length);

		glVertex3dv((const double*)(p1));
		glVertex3dv((const double*)(p2));
	}
	glEnd();

	glBegin(GL_POINTS);
	BOOST_FOREACH(const Vertex* v, mOrganism->mesh()->vertices())
	{
		if (v->isFrozen())
			glColor4f(.5,.5,.5,0.4f);
		else
			glColor4f(0,0,0,0.4f);

		glVertex3dv((const double*)(v->x()));
	}
	glEnd();

	glPopAttrib();
}

void OrganismViewer::drawSurface(Mesh* m)
{
	bool justMesh = true;
	if (m==NULL)
	{
		justMesh = false;
		m = mOrganism->mesh();
	}

	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	if (mLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1,1);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glBegin(GL_TRIANGLES);
	BOOST_FOREACH(Face* f, m->outerFaces())
	{
		Vector3d faces[] = {f->cv(0).x(),f->cv(1).x(),f->cv(2).x()};
		Vector3d norm = f->n();
		for(int j=0;j<3;++j)
		{
			glNormal3dv((const double*)norm);
			if (justMesh)
			{
				glColor4f(.5,.5,.5,.4);
			}
			else
			{
				Vector3d c = getContentsColourV(mOrganism->getAssociatedCell(&f->v(j)));
				glColor4f(c[0],c[1],c[2],1);
			}
			glVertex3dv((const double*)faces[j]);
		}
	}
	glEnd();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_LIGHTING);

	glLineWidth(1);
	glColor4f(0,0,0,1);

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
	glPopAttrib();
}

void OrganismViewer::drawVertices()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
	glDisable(GL_LIGHTING);
	glPointSize(3);

	glBegin(GL_POINTS);
	BOOST_FOREACH(const Vertex* v, mOrganism->mesh()->vertices())
	{
		if (v->isFrozen())
			glColor4f(.5,.5,.5,mAlpha);
		else
			glColor4f(0,0,0,mAlpha);
		glVertex3dv((const double*)(v->x()));
	}
	glEnd();

	glPopAttrib();
}

/**
 * DepthSorter is used to sort the cells by their depth for drawing.
 * We can't use the depth buffer because we use spheres and don't want the
 * nasty intersections.
 */
class DepthSorter
{
public:
	DepthSorter(Matrix4d& transform, double* _minZ, double* _maxZ):mTransform(transform),minZ(_minZ),maxZ(_maxZ){}

	// returns true if a is further than b (i.e., z value is more negative)
	bool operator()(Cell* a, Cell* b)
	{
		double az = (mTransform * a->v()->x()).z();
		double bz = (mTransform * b->v()->x()).z();

		*minZ = std::min(bz,std::min(az,*minZ));
		*maxZ = std::max(bz,std::max(az,*maxZ));

		return az<=bz;
	}

	Matrix4d& mTransform;
	double *minZ, *maxZ;
};
void OrganismViewer::drawCells()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT);
	if (mLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glDisable(GL_DEPTH_TEST);

	// sort the cells back to front
	Matrix4d m4d;
	glGetDoublev(GL_MODELVIEW_MATRIX,(double*)m4d);
	std::list<Cell*> cells = mOrganism->cells();
	double minZ = 100000, maxZ = -100000;
	cells.sort<DepthSorter>(DepthSorter(m4d,&minZ,&maxZ));

	BOOST_FOREACH(Cell* c, cells)
	{
		double zp = ((m4d*c->v()->x()).z() - minZ)/(maxZ-minZ);

		Vector3d depthColour = Vector3d(zp,zp,zp);
		Vector3d morphogenColour = getContentsColourV(c);
		double lerpFactor = 0.5;
		Vector3d cc = depthColour*lerpFactor + morphogenColour*(1-lerpFactor);

		glColor4d(cc[0],cc[1],cc[2],mAlpha);

		// draw the cell
		const Vector3d& x = c->v()->x();
		double r = c->r();
		glPushMatrix();
		glTranslatef(x.x(),x.y(),x.z());
		glScalef(r,r,r);
		drawSphere();
		glPopMatrix();

		// draw the label
		if (mDrawMorphogenLabels)
			if (not (mSelectedMorphogen==-1 and mSelectedVariable==-1)){
				drawText(x,QString("%1f").arg(getSelectedValue(c),2,'e',2));
			}
	}

	/*
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glLineWidth(1);
	glBegin(GL_LINES);
	BOOST_FOREACH(const Edge* e, mOrganism->mesh()->edges())
	{
		const Vector3d& v1 = e->cv(0).x();
		const Vector3d& v2 = e->cv(1).x();

		// approximate depth
		double zp = ((m4d*((v2+v1)/2)).z() - minZ)/(maxZ-minZ);
		glColor4d(0,0,0,1-zp);

		glVertex3dv((const double*)(v1));
		glVertex3dv((const double*)(v2));
	}
	glEnd();
	*/

	glPopAttrib();
}

void OrganismViewer::drawText(Vector3d x, QString text)
{
	GLdouble modelmtx[16], projmtx[16];
	GLint viewport[4];

	glGetDoublev(GL_MODELVIEW_MATRIX,modelmtx);
	glGetDoublev(GL_PROJECTION_MATRIX,projmtx);
	glGetIntegerv(GL_VIEWPORT,viewport);
	GLdouble win[3];
	gluProject(x.x(),x.y(),x.z(),modelmtx,projmtx,viewport,&win[0],&win[1],&win[2]);

	// transform the "gl window" coordinates to "qt window" coordinates then display
	glColor4f(0,0,0,1);
	// mQGLW->renderText(win[0],mQGLW->height()-win[1],QString::number(1-m0));
	this->renderText(win[0],height()-win[1],text);
}

void OrganismViewer::drawAnnotations()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(1);

	if (mDrawForces) drawForces();
	if (mDrawVelocities) drawVelocities();
	if (mDrawFaceNormals) drawFaceNormals();

	glPopAttrib();

	BOOST_FOREACH(Annotation* a, mAnnotations)
	{
		a->draw();
	}

	/*

	glDisable(GL_DEPTH_TEST);

	glLineWidth(1);
	glColor4f(0, 0, 0, .5);
	typedef std::pair<Vector3d, Vector3d> pv;
	BOOST_FOREACH(const pv& p, mTempVecs)
	{
		const Vector3d& from = p.first;
		const Vector3d& to = p.second;

		glTranslatef(from.x(),from.y(),from.z());
		drawVector(to);
		glTranslatef(-from.x(),-from.y(),-from.z());
	}

	BOOST_FOREACH(Edge* e, mTempEdges)
	{
		Vector3d from = e->v(0)->x();
		Vector3d to = e->v(1)->x() - from;
		glTranslatef(from.x(),from.y(),from.z());
		drawVector(to);
		glTranslatef(-from.x(),-from.y(),-from.z());
	}

	// draw vertex and face neighbours of verts
	BOOST_FOREACH(const Vertex* v, mTempVertices)
	{
		Vector3d from = v->x();

		BOOST_FOREACH(const Vertex* vn, v->neighbours())
		{
			Vector3d to = vn->x() - from;
			glTranslatef(from.x(),from.y(),from.z());
			drawVector(to);
			glTranslatef(-from.x(),-from.y(),-from.z());
		}

		BOOST_FOREACH(const Face* f, v->surfaceFaces())
		{
			Vector3d to = f->center() - from;
			glTranslatef(from.x(),from.y(),from.z());
			drawVector(to);
			glTranslatef(-from.x(),-from.y(),-from.z());
		}
	}

	glBegin(GL_LINES);

	typedef boost::tuple<Vertex*, Vertex*> bivert;
	BOOST_FOREACH(bivert& bt, mTemp2Verts)
	{
		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<1>(bt)->x());
	}

	typedef boost::tuple<Vertex*, Vertex*, Vertex*> trivert;
	BOOST_FOREACH(trivert& bt, mTempTriVerts)
	{
		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<1>(bt)->x());

		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<2>(bt)->x());

		glVertex3dv(boost::get<1>(bt)->x());
		glVertex3dv(boost::get<2>(bt)->x());
	}

	typedef boost::tuple<Vertex*, Vertex*, Vertex*, Vertex*> qvert;
	BOOST_FOREACH(qvert& bt, mTemp4Verts)
	{
		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<1>(bt)->x());

		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<2>(bt)->x());

		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<3>(bt)->x());

		glVertex3dv(boost::get<1>(bt)->x());
		glVertex3dv(boost::get<2>(bt)->x());

		glVertex3dv(boost::get<1>(bt)->x());
		glVertex3dv(boost::get<3>(bt)->x());

		glVertex3dv(boost::get<2>(bt)->x());
		glVertex3dv(boost::get<3>(bt)->x());
	}

	glEnd();

	glPointSize(4);
	glColor4f(0, 0, 0, .5);
	glBegin(GL_POINTS);
	BOOST_FOREACH(Vector3d& v, mTempPoints)
	{
		glVertex3f(v.x(),v.y(),v.z());
	}

	BOOST_FOREACH(const Vertex* v, mTempVertices)
	{
		if (v->surface())
		glColor4f(1,0,0,1);
		else
		glColor4f(0,0,0,.5);
		glVertex3dv((const double*)v->x());
	}

	glColor4f(0, 0, 0, .5);
	BOOST_FOREACH(const Face* f, mTempFaces)
	{
		Vector3d c = f->center();
		glVertex3dv((double*)c);
	}
	BOOST_FOREACH(trivert& bt, mTempTriVerts)
	{
		Vector3d c = (boost::get<0>(bt)->x() + boost::get<1>(bt)->x() + boost::get<2>(bt)->x())/3.;
		glVertex3dv((double*)c);
	}
	glEnd();

	BOOST_FOREACH(const AABB& aabb, mTempAABBs)
	{
		drawAABB(aabb);
	}

	glEnable(GL_DEPTH_TEST);

	glPushAttrib(GL_POLYGON_BIT);
	glDisable(GL_CULL_FACE);

	glBegin(GL_TRIANGLES);
	glColor4f(0.5, 0.5, 0.5, 0.5);
	BOOST_FOREACH(const Face* f, mTempFaces)
	{
		glVertex3dv((const double*)(f->v(0).x()));
		glVertex3dv((const double*)(f->v(1).x()));
		glVertex3dv((const double*)(f->v(2).x()));
	}
	BOOST_FOREACH(trivert& bt, mTempTriVerts)
	{
		glVertex3dv(boost::get<0>(bt)->x());
		glVertex3dv(boost::get<1>(bt)->x());
		glVertex3dv(boost::get<2>(bt)->x());
	}
	glEnd();

	glPopAttrib();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	BOOST_FOREACH(const Tetra* t, mTempTetras)
	{
		assert(t);

		glColor4f(0.5,0.5,0.5,0.2);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		drawTetra(t);

		glColor4f(0,0,0,.5);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		drawTetra(t);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// draw neighbour relation
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	BOOST_FOREACH(Tetra* t, mTempTetras)
	{
		assert(t);
		Vector3d from = t->center();

		for(int i=0;i<4;i++)
		{
			Tetra* tn = t->neighbour(i);
			if (tn)
			{
				Vector3d to = tn->center() - from;

				glTranslatef(from.x(),from.y(),from.z());
				drawVector(to);
				glTranslatef(-from.x(),-from.y(),-from.z());
			}
		}
	}

	// annotate vertices of tetras
	// set up manual projection
	GLdouble modelmtx[16], projmtx[16];
	GLint viewport[4];

	glGetDoublev(GL_MODELVIEW_MATRIX, modelmtx);
	glGetDoublev(GL_PROJECTION_MATRIX, projmtx);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glColor4f(0, 0, 0, 1);
	BOOST_FOREACH(Tetra* t, mTempTetras)
	{
		for(int i=0;i<4;i++)
		{
			Vector3d p = t->v(i).x();
			GLdouble win[3];
			gluProject(p.x(),p.y(),p.z(),modelmtx,projmtx,viewport,&win[0],&win[1],&win[2]);

			// transform the "gl window" coordinates to "qt window" coordinates then display
			this->renderText(win[0],height()-win[1],QString::number(i));
			// this->renderText(win[0],height()-win[1],QString("%1f,%2f").arg(win[0],2).arg(win[1],2));
		}

		for(int i=0;i<4;i++)
		{
			Tetra* tn = t->neighbour(i);
			if (tn)
			{
				Vector3d p = tn->center();
				GLdouble win[3];
				gluProject(p.x(),p.y(),p.z(),modelmtx,projmtx,viewport,&win[0],&win[1],&win[2]);
				// transform the "gl window" coordinates to "qt window" coordinates then display
				this->renderText(win[0],height()-win[1],QString::number(i));
				// this->renderText(win[0],height()-win[1],QString("%1f,%2f").arg(win[0],2).arg(win[1],2));
			}
		}

	}

	glPopAttrib();

	*/
}

void OrganismViewer::drawGround()
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
	glDisable(GL_LIGHTING);

	const double dx = mWorldBounds.dx();
	const double dz = mWorldBounds.dz();
	const Vector3d min = mWorldBounds.min();

	glColor3dv((const double*) gPalette[1]);

	glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);
		glVertex3dv((const double*) min);
		glVertex3dv((const double*) (min + Vector3d(dx, 0, 0)));
		glVertex3dv((const double*) (min + Vector3d(dx, 0, dz)));
		glVertex3dv((const double*) (min + Vector3d(0, 0, dz)));
	glEnd();

	glPopAttrib();
}

/** annotation drawing methods */
void OrganismViewer::drawForces()
{
	BOOST_FOREACH(const Vertex* v, mOrganism->mesh()->vertices())
	{
		const Vector3d& f = v->f(), &x = v->x();
		glTranslatef(x.x(),x.y(),x.z());
		drawVector(f);
		glTranslatef(-x.x(),-x.y(),-x.z());
	}
}

void OrganismViewer::drawVelocities()
{
	BOOST_FOREACH(const Vertex* vert, mOrganism->mesh()->vertices())
	{
		const Vector3d& v = vert->v(), &x = vert->x();
		glTranslatef(x.x(),x.y(),x.z());
		drawVector(v);
		glTranslatef(-x.x(),-x.y(),-x.z());
	}
}

void OrganismViewer::drawFaceNormals()
{
	BOOST_FOREACH(Face* f, mOrganism->mesh()->outerFaces())
	{
		Vector3d from = f->center();
		Vector3d n = f->n();
		glTranslatef(from.x(),from.y(),from.z());
		drawVector(n);
		glTranslatef(-from.x(),-from.y(),-from.z());
	}
}

// selection drawing methods
void OrganismViewer::drawWithNames()
{
	switch (mSelectionMode)
	{
		case SMCell: drawCellsWithNames(); break;
		case SMVertex: drawVerticesWithNames(); break;
		case SMFace: drawFacesWithNames(); break;
		case SMEdge: drawEdgesWithNames(); break;
	}
}

void OrganismViewer::drawVerticesWithNames()
{
	if (mOrganism==NULL) return;

	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
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

void OrganismViewer::drawCellsWithNames()
{
	if (mOrganism==NULL) return;

	glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	int i = 0;
	BOOST_FOREACH(Cell* c, mOrganism->cells())
	{
		const Vector3d& x = c->v()->x();
		double r = c->r();
		glPushMatrix();
		glTranslatef(x.x(),x.y(),x.z());
		glScalef(r,r,r);
		glPushName(i);
		drawSphere();
		glPopName();
		glPopMatrix();
		i++;
	}

	glPopAttrib();
}

void OrganismViewer::drawEdgesWithNames()
{
	if (mOrganism==NULL) return;

	glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(5);

	int i = 0;
	BOOST_FOREACH(const Edge* e, mOrganism->mesh()->edges())
	{
		glPushName(i);
		glBegin(GL_LINES);
			glVertex3dv((const double*)(e->cv(0).x()));
			glVertex3dv((const double*)(e->cv(1).x()));
		glEnd();
		glPopName();
		i++;
	}

	glPopAttrib();
}

void OrganismViewer::drawFacesWithNames()
{
	if (mOrganism==NULL) return;
	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	int i = 0;
	BOOST_FOREACH(const Face* f, mOrganism->mesh()->outerFaces())
	{
		glPushName(i);
		glBegin(GL_TRIANGLES);
			drawTriInline(f->v(0).x(),f->v(1).x(),f->v(2).x(),false);
		glEnd();
		glPopName();
		i++;
	}
	glPopAttrib();
}

QColor OrganismViewer::getContentsColour(Cell* c)
{
	// add contributions from each active morphogen
	Vector3d result = getContentsColourV(c);
	return QColor::fromRgbF(result[0],result[1],result[2],1.f);
}

Vector3d OrganismViewer::getContentsColourV(Cell* c)
{
	// add contributions from each active morphogen
	if (mSelectedMorphogen==-1 and mSelectedVariable==-1) return Vector3d(1,1,1);
	else if (mSelectedMorphogen>=0)
	{
		CellContents* cc = c->getCellContents();
		Vector3d white(.8,.8,.8);
		if (cc!=NULL)
		{
			double m = cc->getMorphogen(mSelectedMorphogen);
			if (mDrawMorphogensNormalised)
				m /= c->vol();
			Vector3d black(.2,.2,.2); // don't start at completely black

			Vector3d result = black + white*m;
			result = Vector3d(Math::gammacorrect(mGamma,result[0]),Math::gammacorrect(mGamma,result[1]),Math::gammacorrect(mGamma,result[2]));

			return result;
		}
		else
			return white;
	}
	else if (mSelectedVariable>=0)
	{
		CellContents* cc = c->getCellContents();
		Vector3d white(.8,.8,.8);
		if (cc!=NULL)
		{
			double m = cc->getVar(mSelectedVariable);
			if (mDrawMorphogensNormalised)
				m /= c->vol();

			Vector3d black(.2,.2,.2); // don't start at completely black

			Vector3d result = black + white*m;
			result = Vector3d(Math::gammacorrect(mGamma,result[0]),Math::gammacorrect(mGamma,result[1]),Math::gammacorrect(mGamma,result[2]));
			return result;
		}
		else
			return white;
	}
}

double OrganismViewer::getSelectedValue(Cell* c)
{
	if (!c) return 0;
	CellContents* cc = c->getCellContents();
	if (mSelectedMorphogen>=0){
		return cc->getMorphogen(mSelectedMorphogen);
	}
	else if (mSelectedVariable>=0){
		return cc->getVar(mSelectedVariable);
	}
	else return 0;
}

template <typename T>
T unsafeGet(const std::list<T>& list, int n)
{
	typename std::list<T>::const_iterator it = list.begin();
	std::advance(it,n);
	return *it;
}

void OrganismViewer::postSelection(const QPoint& point)
{
	//bool found;
	//qglviewer::Vec selectedPoint = camera()->pointUnderPixel(point, found);
	if (selectedName()>=0)
	{
		switch (mSelectionMode)
		{
			case SMCell:
			{
				emit cellSelected(unsafeGet(mOrganism->cells(),selectedName()));
				break;
			}
			case SMVertex:
			{
				emit vertexSelected(unsafeGet(mOrganism->mesh()->vertices(),selectedName()));
				break;
			}
			case SMEdge:
			{
				emit edgeSelected(unsafeGet(mOrganism->mesh()->edges(),selectedName()));
				break;
			}
			case SMFace:
			{
				emit faceSelected(unsafeGet(mOrganism->mesh()->outerFaces(),selectedName()));
				break;
			}
		}
	}
	else
	{
		QApplication::beep();
	}
}
