#ifndef MESHVIEW_H
#define MESHVIEW_H

#include <QGLViewer/qglviewer.h>
#include <QTimer>
#include <QMutex>

#include "vector3.h"

#include "oworld.h"
#include "tetra.h"
#include "edge.h"
#include "organismdrawer.h"

#include <boost/tuple/tuple.hpp>

class Window;
class WorldView: public QGLViewer
{
	Q_OBJECT

	OWorld* mWorld; // world to view
	OrganismDrawer* mDrawer;

	public:
	WorldView(QWidget* parent);
	~WorldView();

	/// Number of milliseconds it took to draw the last scene
	int msLastFrame(){return mMsLastFrame;}

	void setWorld(OWorld* w){mWorld = w; if (w) mDrawer->setOrganism(w->organism());}

	// XXX: these are now specified in the UI editor
	// QSize minimumSizeHint() const {return QSize(400,400);}
	// QSize sizeHint() const {return QSize(800,800);}

	void drawTempPoint(Vector3d pos);
	void drawTempVec(Vector3d from, Vector3d dir);
	void drawTempAABB(AABB aabb);

	void drawTemp(Tetra* t){mTempTetras.push_back(t);}
	void drawTemp(Vertex* v){mTempVertices.push_back(v);}
	void drawTemp(Edge* e){mTempEdges.push_back(e);}
	void drawTemp(Face* f){mTempFaces.push_back(f);}

	void drawTemp(boost::tuple<Vertex*,Vertex*> bt){mTemp2Verts.push_back(bt);}
	void drawTemp(boost::tuple<Vertex*,Vertex*,Vertex*> bt){mTempTriVerts.push_back(bt);}
	void drawTemp(boost::tuple<Vertex*,Vertex*,Vertex*,Vertex*> bt){mTemp4Verts.push_back(bt);}

	void clearTemps();

public slots:
	void setXRotation(int angle);
	void setYRotation(int angle);
	void setZRotation(int angle);
	void setZoom(double zoom);

	void toggleDrawForces();
	void toggleDrawVelocities();
	void toggleDrawFaceNormals();
	void toggleDrawVertexNormals();
	void toggleDrawHistory();
	void toggleDrawMeshBounds();


	void toggleDrawBounds();
	void toggleDrawAxis();
	void toggleDrawGround();

	void setDrawBounds(bool);
	void setDrawAxis(bool);
	void setDrawGround(bool);

	void drawModeOrganism();
	void drawModeTetrahedra();
	void drawModeSurface();
	void drawModeSurfaceSmooth();
	void drawModeEdge();
	void drawModeVertex();
	void drawModeMass();
	void drawModeFaceEdges();
	void drawModeNothing();

	void timer();

protected:
	void draw();
	void init();
	void reset();

	//void initializeGL();
	// void paintGL();
	//void resizeGL(int w, int h);

	void drawScene();
	void setCurrentBounds();

	/*
	void mousePressEvent(QMouseEvent* ev);
	void mouseMoveEvent(QMouseEvent* ev);
	*/

	void setMutex(QMutex* m){mMutex = m;}
	void drawVector(const Vector3d& v);

	void drawMeshes(); // calls one of the following...based on viewMode

	void drawOrigin();
	void drawAABB();
	void drawTetAABBs();
	void drawAABB(const AABB&);
	void drawTetra(const Tetra* t);

	private:
	int xRot, yRot, zRot;
	double mZoom;
	QPoint lastPos;
	QTimer* mTimer;
	QMutex* mMutex;

	int mMsLastFrame;

	bool mDrawAABB;
	bool mDrawGround;
	bool mDrawOrigin;
	bool mDrawMeshBounds;
	bool mDrawNothing;

	std::list<Vector3d> mTempPoints;
	std::list<std::pair<Vector3d,Vector3d> > mTempVecs;
	std::list<AABB> mTempAABBs;
	std::list<Tetra*> mTempTetras;
	std::list<Vertex*> mTempVertices;
	std::list<Edge*> mTempEdges;
	std::list<Face*> mTempFaces;
	std::list<boost::tuple<Vertex*,Vertex*> > mTemp2Verts;
	std::list<boost::tuple<Vertex*,Vertex*,Vertex*> > mTempTriVerts;
	std::list<boost::tuple<Vertex*,Vertex*,Vertex*,Vertex*> > mTemp4Verts;

	static const double mDT = 0.02;
	friend class Stepper;
	friend class Window;
};

#endif


