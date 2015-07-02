#include "worldview.h"

#include <QtGui>
#include <QtOpenGL>
#include <QMutex>
#include <QTime>

#include "verlet.h"
#include "vector3.h"
#include "physics.h"
#include "shapes.h"
#include "organism.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>

#include <boost/foreach.hpp>

#include <GL/gl.h>
#include <GL/glu.h>

const int gPaletteSize = 5;
const Vector3d gPalette[gPaletteSize] = {Vector3d(1,1,1),
		Vector3d(188./255,200./255,230./255),
		Vector3d(85./255,112./255,158./255),
		Vector3d(34./255,52./255,97./255),
		Vector3d(11./255,20./255,57./255)
};

WorldView::WorldView(QWidget* parent)
	:QGLViewer(parent)
	,mTimer(0)

	,mMsLastFrame(0)

	,mDrawAABB(true)
	,mDrawGround(false)
	,mDrawOrigin(false)
	,mDrawMeshBounds(false)
	,mDrawNothing(false)
{
	xRot = 0;
	yRot = 0;
	zRot = 0;
	mZoom = 1;

	mDrawer = new OrganismDrawer();
	mDrawer->setColour(gPalette[2]);
}

WorldView::~WorldView()
{
	// makeCurrent();
	delete mDrawer;
}


void WorldView::init()
{
	setBackgroundColor(palette().color(QPalette::Highlight));

	glShadeModel(GL_SMOOTH);

	glLineWidth(1.0);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// light setup
	GLfloat zero[] = {0,0,0,1.0};
	GLfloat mat_spec[] = { .5, .5, .5, 1.0 };
	GLfloat mat_shine[] = { 50 };
	GLfloat lmodel_ambient[] = { 0, 0, 0, 1.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shine);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zero);

	// key light
	GLfloat light0_pos[] = { -0.3, 0.6, 1.0, 0.0 };
	GLfloat white_light[] = { .9, .9, .9, 1.0 };
	GLfloat light0_spec[] = {0.3,0.3,0.3,1};

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);

	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
}

void WorldView::draw()
{
	drawScene();
}

void WorldView::reset()
{
	if (mWorld)
	{
		setCurrentBounds();
		showEntireScene();
	}
}

void WorldView::setCurrentBounds()
{
	Organism* o = mWorld->organism();
	if (o)
	{
		// update bounding box
		const AABB& aabb = o->mesh()->aabb();
		if (aabb.valid())
		{
			Vector3d vmin = aabb.min();
			Vector3d vmax = aabb.max();
			setSceneBoundingBox(qglviewer::Vec(vmin.x(),vmin.y(),vmin.z()),qglviewer::Vec(vmax.x(),vmax.y(),vmax.z()));
		}
	}
	updateGL();
}

/*
void
WorldView::initializeGL()
{
	glClearColor(1,1,1,1);

	glShadeModel(GL_SMOOTH);

	glLineWidth(1.0);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// light setup
	GLfloat zero[] = {0,0,0,1.0};
	GLfloat mat_spec[] = { .5, .5, .5, 1.0 };
	GLfloat mat_shine[] = { 50 };
	GLfloat lmodel_ambient[] = { 0, 0, 0, 1.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shine);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zero);

	// key light
	GLfloat light0_pos[] = { -0.3, 0.6, 1.0, 0.0 };
	GLfloat white_light[] = { .9, .9, .9, 1.0 };
	GLfloat light0_spec[] = {0.3,0.3,0.3,1};

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);

	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
}
*/
/*

void
WorldView::resizeGL(int w, int h)
{
	int side = std::min(w,h);
	glViewport((w-side)/2,(h-side)/2,side,side);

	glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (mWorld)
		{
			double mmin = mWorld->bounds().dx()/100.0;
			if (mmin > mWorld->bounds().dy()/100.0)
				mmin = mWorld->bounds().dy()/100.0;
			if (mmin > mWorld->bounds().dz()/100.0)
				mmin = mWorld->bounds().dz()/100.0;

			double mmax = mWorld->bounds().dx()*100.0;
			if (mmax < mWorld->bounds().dy()*100.0)
				mmax = mWorld->bounds().dy()*100.0;
			if (mmax < mWorld->bounds().dz()*100.0)
				mmax = mWorld->bounds().dz()*100.0;

			gluPerspective(70.0,1.0,mmin,mmax);
		}
		else
			gluPerspective(70.0,1.0,0.01,10);
	glMatrixMode(GL_MODELVIEW);
}

void
WorldView::paintGL()
{
	QTime t; t.start();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (mWorld)
	{
		static const AABB& bounds = mWorld->bounds();
		const Vector3d c = bounds.c();
		const Vector3d cl = bounds.cl();
		const double dx = bounds.dx();
		const double dz = bounds.dz();
		const Vector3d min = bounds.min();

		const AABB& mb =mWorld->organism()->mesh()->aabb(); //mMesh->aabb();
		Vector3d mc = mb.c();

		// Vector3d lookat = mc;
		Vector3d lookat = c;
		// Vector3d from = lookat + Vector3d(0,0,3*mb.dz());
		Vector3d from = lookat + Vector3d(0,0,dz);

		gluLookAt(from.x(),from.y(),from.z(),lookat.x(),lookat.y(),lookat.z(),0,1,0);
		glTranslatef(lookat.x(),lookat.y(),lookat.z());

		glRotated(xRot/16.0,1.0,0.0,0.0);
		glRotated(yRot/16.0,0.0,1.0,0.0);
		glRotated(zRot/16.0,0.0,0.0,1.0);

		glScalef(mZoom,mZoom,mZoom);

		glTranslatef(-lookat.x(),-lookat.y(),-lookat.z());

		// draw world bounds
		if (mDrawAABB)
		{
			drawAABB();

			// XXX: this should overlap
			//drawAABB(mStepper->collision().bounds());
		}


		if (mDrawOrigin)
			drawOrigin();

		if (mDrawGround)
		{
			//glColor3f(.9,.9,.9);
			glColor3dv((const double*)gPalette[1]);
			glDisable(GL_LIGHTING);
			glBegin(GL_QUADS);
				glNormal3f(0,1,0);
				glVertex3dv((const double*)min);
				glVertex3dv((const double*)(min + Vector3d(dx,0,0)));
				glVertex3dv((const double*)(min + Vector3d(dx,0,dz)));
				glVertex3dv((const double*)(min + Vector3d(0,0,dz)));
			glEnd();
			glEnable(GL_LIGHTING);
		}

		// draw all meshes
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		drawMeshes();

		// draw annotations and temporary points

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glLineWidth(2);
		glColor4f(0,0,0,.5);
		typedef std::pair<Vector3d,Vector3d> pv;
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

		typedef boost::tuple<Vertex*,Vertex*> bivert;
		BOOST_FOREACH(bivert& bt, mTemp2Verts)
		{
			glVertex3dv(boost::get<0>(bt)->x());
			glVertex3dv(boost::get<1>(bt)->x());
		}

		typedef boost::tuple<Vertex*,Vertex*,Vertex*> trivert;
		BOOST_FOREACH(trivert& bt, mTempTriVerts)
		{
			glVertex3dv(boost::get<0>(bt)->x());
			glVertex3dv(boost::get<1>(bt)->x());

			glVertex3dv(boost::get<0>(bt)->x());
			glVertex3dv(boost::get<2>(bt)->x());

			glVertex3dv(boost::get<1>(bt)->x());
			glVertex3dv(boost::get<2>(bt)->x());
		}

		typedef boost::tuple<Vertex*,Vertex*,Vertex*,Vertex*> qvert;
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
		glColor4f(0,0,0,.5);
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

		glColor4f(0,0,0,.5);
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

		glPushAttrib(GL_POLYGON_BIT); // disable culling
		glDisable(GL_CULL_FACE);

		glBegin(GL_TRIANGLES);
		glColor4f(0.5,0.5,0.5,0.5);
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

		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

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

		glGetDoublev(GL_MODELVIEW_MATRIX,modelmtx);
		glGetDoublev(GL_PROJECTION_MATRIX,projmtx);
		glGetIntegerv(GL_VIEWPORT,viewport);


		glColor4f(0,0,0,1);
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
	}
	else
	{
		drawOrigin();
	}

	mMsLastFrame = t.elapsed();
}

*/

void
WorldView::drawScene()
{
	if (mWorld)
	{
		static const AABB& bounds = mWorld->bounds();
		const Vector3d c = bounds.c();
		const Vector3d cl = bounds.cl();
		const double dx = bounds.dx();
		const double dz = bounds.dz();
		const Vector3d min = bounds.min();

		const AABB& mb =mWorld->organism()->mesh()->aabb(); //mMesh->aabb();
		Vector3d mc = mb.c();

		// Vector3d lookat = mc;
		Vector3d lookat = c;
		// Vector3d from = lookat + Vector3d(0,0,3*mb.dz());
		Vector3d from = lookat + Vector3d(0,0,dz);

		/*
		gluLookAt(from.x(),from.y(),from.z(),lookat.x(),lookat.y(),lookat.z(),0,1,0);
		glTranslatef(lookat.x(),lookat.y(),lookat.z());

		glRotated(xRot/16.0,1.0,0.0,0.0);
		glRotated(yRot/16.0,0.0,1.0,0.0);
		glRotated(zRot/16.0,0.0,0.0,1.0);

		glScalef(mZoom,mZoom,mZoom);

		glTranslatef(-lookat.x(),-lookat.y(),-lookat.z());
		*/

		// draw world bounds
		if (mDrawAABB)
		{
			drawAABB();

			// XXX: this should overlap
			//drawAABB(mStepper->collision().bounds());
		}


		if (mDrawOrigin)
			drawOrigin();

		if (mDrawGround)
		{
			//glColor3f(.9,.9,.9);
			glColor3dv((const double*)gPalette[1]);
			glDisable(GL_LIGHTING);
			glBegin(GL_QUADS);
				glNormal3f(0,1,0);
				glVertex3dv((const double*)min);
				glVertex3dv((const double*)(min + Vector3d(dx,0,0)));
				glVertex3dv((const double*)(min + Vector3d(dx,0,dz)));
				glVertex3dv((const double*)(min + Vector3d(0,0,dz)));
			glEnd();
			glEnable(GL_LIGHTING);
		}

		// draw all meshes
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		drawMeshes();

		// draw annotations and temporary points

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glLineWidth(2);
		glColor4f(0,0,0,.5);
		typedef std::pair<Vector3d,Vector3d> pv;
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

		typedef boost::tuple<Vertex*,Vertex*> bivert;
		BOOST_FOREACH(bivert& bt, mTemp2Verts)
		{
			glVertex3dv(boost::get<0>(bt)->x());
			glVertex3dv(boost::get<1>(bt)->x());
		}

		typedef boost::tuple<Vertex*,Vertex*,Vertex*> trivert;
		BOOST_FOREACH(trivert& bt, mTempTriVerts)
		{
			glVertex3dv(boost::get<0>(bt)->x());
			glVertex3dv(boost::get<1>(bt)->x());

			glVertex3dv(boost::get<0>(bt)->x());
			glVertex3dv(boost::get<2>(bt)->x());

			glVertex3dv(boost::get<1>(bt)->x());
			glVertex3dv(boost::get<2>(bt)->x());
		}

		typedef boost::tuple<Vertex*,Vertex*,Vertex*,Vertex*> qvert;
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
		glColor4f(0,0,0,.5);
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

		glColor4f(0,0,0,.5);
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

		glPushAttrib(GL_POLYGON_BIT); // disable culling
		glDisable(GL_CULL_FACE);

		glBegin(GL_TRIANGLES);
		glColor4f(0.5,0.5,0.5,0.5);
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

		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

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

		glGetDoublev(GL_MODELVIEW_MATRIX,modelmtx);
		glGetDoublev(GL_PROJECTION_MATRIX,projmtx);
		glGetIntegerv(GL_VIEWPORT,viewport);


		glColor4f(0,0,0,1);
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
	}
	else
	{
		drawOrigin();
	}

	// mMsLastFrame = t.elapsed();
}

void WorldView::clearTemps()
{
	mTempPoints.clear();
	mTempVecs.clear();
	mTempAABBs.clear();
	mTempTetras.clear();
	mTempFaces.clear();
	mTempVertices.clear();
	mTempEdges.clear();
	mTempTriVerts.clear();
	mTemp2Verts.clear();
	mTemp4Verts.clear();
}

/*
void WorldView::mousePressEvent(QMouseEvent* e)
{
	lastPos = e->pos();
}

void WorldView::mouseMoveEvent(QMouseEvent* e)
{
	int dx = e->x() - lastPos.x();
	int dy = e->y() - lastPos.y();

	if ((e->buttons() == (Qt::LeftButton | Qt::RightButton))  || e->buttons() & Qt::MidButton)
	{
		setZoom(mZoom + ((double)dy/height()));
	}
	else if (e->buttons() & Qt::LeftButton)
	{
		setXRotation(xRot + 8*dy);
		setYRotation(yRot + 8*dx);
	}
	else if (e->buttons() & Qt::RightButton)
	{
		setXRotation(xRot + 8*dy);
		setYRotation(yRot + 8*dx);
	}
	lastPos = e->pos();
}
*/


void
WorldView::setXRotation(int angle)
{
	if (angle!=xRot)
	{
		xRot = angle;
		updateGL();
	}
}

void
WorldView::setYRotation(int angle)
{
	if (angle!=yRot)
	{
		yRot = angle;
		updateGL();
	}
}

void
WorldView::setZRotation(int angle)
{
	if (angle!=zRot)
	{
		zRot = angle;
		updateGL();
	}
}

void
WorldView::setZoom(double z)
{
	mZoom = z;
	updateGL();
}

void WorldView::toggleDrawForces()
{
	mDrawer->toggleDrawForces();
}

void WorldView::toggleDrawVelocities()
{
	mDrawer->toggleDrawVelocities();
}

void WorldView::toggleDrawFaceNormals()
{
	mDrawer->toggleDrawFaceNormals();
}

void WorldView::toggleDrawVertexNormals()
{
	mDrawer->toggleDrawVertexNormals();
}

void WorldView::toggleDrawHistory()
{
	mDrawer->toggleDrawHistory();
}

void WorldView::toggleDrawMeshBounds()
{
	mDrawMeshBounds = !mDrawMeshBounds;
}

void WorldView::toggleDrawBounds(){
	mDrawAABB = !mDrawAABB;
}
void WorldView::toggleDrawAxis()
{
	mDrawOrigin = !mDrawOrigin;
}
void WorldView::toggleDrawGround()
{
	mDrawGround = !mDrawGround;
}

void WorldView::setDrawBounds(bool b){mDrawAABB = b;}
void WorldView::setDrawAxis(bool b){mDrawOrigin = b;}
void WorldView::setDrawGround(bool b){mDrawGround = b;}

void WorldView::drawModeOrganism()
{
	mDrawer->setMode(OrganismDrawer::DOrganism);
	mDrawNothing = false;
}

void WorldView::drawModeTetrahedra()
{
	mDrawer->setMode(MeshDrawer::DTetra);
	mDrawNothing = false;
}

void WorldView::drawModeSurface()
{
	mDrawer->setMode(MeshDrawer::DFace);
	mDrawNothing = false;
}

void WorldView::drawModeSurfaceSmooth()
{
	mDrawer->setMode(MeshDrawer::DFaceSmooth);
	mDrawNothing = false;
}

void WorldView::drawModeEdge()
{
	mDrawer->setMode(MeshDrawer::DEdge);
	mDrawNothing = false;
}

void WorldView::drawModeVertex()
{
	mDrawer->setMode(MeshDrawer::DVertex);
	mDrawNothing = false;
}

void WorldView::drawModeMass()
{
	mDrawer->setMode(MeshDrawer::DMass);
	mDrawNothing = false;
}

void WorldView::drawModeFaceEdges()
{
	mDrawer->setMode(MeshDrawer::DFaceEdges);
	mDrawNothing = false;
}

void WorldView::drawModeNothing()
{
	mDrawNothing = true;
}

void WorldView::timer()
{
	if (!mTimer)
	{
		mTimer = new QTimer(this);
		connect(mTimer,SIGNAL(timeout()),this,SLOT(repaint()));
	}

	if (mTimer->isActive())
		mTimer->stop();
	else
		mTimer->start((int)(mDT*1000.0)); // mspf
}

// XXX: only draws one mesh, the organism
void WorldView::drawMeshes()
{
	if (not mDrawNothing)
	{
		mMutex->lock();
		//BOOST_FOREACH(Mesh* m, mWorld->meshes())
			mDrawer->draw();
			if (mDrawMeshBounds)
				drawAABB(mWorld->organism()->mesh()->aabb());
		mMutex->unlock();
	}
}

void WorldView::drawVector(const Vector3d& v)
{
	glBegin(GL_LINES);
		glColor4f(0.5,0.5,0.5,1);
		glVertex3f(0,0,0);
		glColor4f(0.5,0.5,0.5,0);
		glVertex3dv((const double*)v);
	glEnd();
}

void WorldView::drawOrigin()
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glLineWidth(1.0);
	glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(1,0,0);

		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);

		glColor3f(0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1);
	glEnd();

	glPopAttrib();
}

void WorldView::drawAABB()
{
	glPushAttrib(GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1);
		glColor3dv((const double*)(gPalette[1]));
		//glColor3f(1,0,0);
		drawAABB(mWorld->bounds());
	glPopAttrib();
}

void WorldView::drawAABB(const AABB& a)
{
	double w[3] = {a.dx(),a.dy(),a.dz()};
	Vector3d min = a.min();

	glBegin(GL_LINES);
		for(int i=0;i<3;i++)
		{
			// draw 4 lines along axis i
			Vector3d v1(Vector3d::ZERO), v2(Vector3d::ZERO), d(Vector3d::ZERO);

			d[i] = w[i];
			v1[(i+2)%3] = w[(i+2)%3];
			v2[(i+1)%3] = w[(i+1)%3];

			for(int j=0;j<2;j++)
			for(int k=0;k<2;k++)
			{
				// draw a line between quads at from->to

				// from_pt = min + j*<..,other1,..> + k*<..other2..>
				// to_pt = max - (1-j)*<other1> - (1-k)*<other2>
				Vector3d from = min + ((j==1)?v1:Vector3d::ZERO) + ((k==1)?v2:Vector3d::ZERO);
				Vector3d to = from + d;

				glVertex3dv((double*)from);
				glVertex3dv((double*)to);
			}
		}
	glEnd();
}

void WorldView::drawTetra(const Tetra* t)
{
	glBegin(GL_TRIANGLES);

	Vector3d c = t->center();
	Vector3d v[] = {t->cv(0).x(),t->cv(1).x(),t->cv(2).x(),t->cv(3).x()};
	Vector3d faces[] = {v[0],v[2],v[1],v[0],v[1],v[3],v[1],v[2],v[3],v[3],v[2],v[0]};

	for(int k=0;k<4;++k)
	{
		glNormal3dv((const double*)(cross(faces[3*k+1]-faces[3*k],faces[3*k+2]-faces[3*k]).normalise()));

		for(int j=0;j<3;++j)
		{
			glVertex3dv((const double*)(faces[k*3+j]));
		}
	}
	glEnd();
}

void WorldView::drawTempPoint(Vector3d pos)
{
	mTempPoints.push_back(pos);
}

void WorldView::drawTempVec(Vector3d from, Vector3d dir)
{
	mTempVecs.push_back(std::pair<Vector3d,Vector3d>(from,dir));
}

void WorldView::drawTempAABB(AABB aabb)
{
	mTempAABBs.push_back(aabb);
}
