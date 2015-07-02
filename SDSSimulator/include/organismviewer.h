/*
 * organismviewer.h
 *
 *  Created on: 21/09/2009
 *      Author: ben
 */

#ifndef ORGANISMVIEWER_H_
#define ORGANISMVIEWER_H_

#include <QGLViewer/qglviewer.h>

#include <set>

#include "vector3.h"

#include "aabb.h"
#include "oworld.h"
#include "tetra.h"

#include "organism.h"
// #include "organismdrawer.h"

#include "annotation.h"

/**
 * Organism Viewer is a QtWidget that draws an SDS organism.
 * It also handles selection of parts, drawing of embellishments, etc.
 *
 * The user sets the current organism to draw using the setOrganism() and setWorldBounds()
 * functions. Or alternatively, setWorld().
 *
 */
class OrganismViewer: public QGLViewer
{
	Q_OBJECT

	public:
	OrganismViewer(QWidget* parent);
	~OrganismViewer();

	void setWorld(OWorld* o, bool resetViewPos = true);
	void addStaticMesh(Mesh* m);
	OWorld* getWorld(){return mOWorld;}

	public:
	void setOrganism(Organism* o);
	void setWorldBounds(const AABB& aabb);

	public:


	/**
	 * Add an annotation to this view.
	 * An annotation can be a set of triangles, a text label in 3d, a highlighted vertex, etc.
	 * Some annotations are voided if the organism changes or is updated.
	 *
	 * Note: don't forget to update() after you have added all the annotations for this frame.
	 */
	void addAnnotation(Annotation* a);
	void clearAnnotations();

	public slots:
	void update(){updateGL();}

	void setDrawBounds(bool);
	void setDrawOrigin(bool);
	void setDrawGround(bool);

	void setDrawForces(bool);
	void setDrawVelocities(bool);
	void setDrawFaceNormals(bool);

	///  alpha = 0..100, alpha = 0 transparent
	void setAlpha(int alpha);

	void setGamma(int gamma);
	void setLighting(bool lighting);
	void setDrawMorphogensNormalised(bool n);
	void setDrawMorphogenLabels(bool d);

	/**
	 * Set the current drawing mode.
	 * @param mode One of "surface", "wire", "cells", "vertices", "morphogens"
	 */
	void setDrawMode(QString mode);

	/**
	 * Set the current selection mode.
	 *
	 * @param mode One of "cell", "vertex", "edge", or "face"
	 */
	void setSelectionMode(QString mode);

	// void morphogenButtonClicked(int);
	void selectMorphogen(int);
	void selectVariable(int);

	/**
	 * The signals emitted by the organismwidget include selected elements.
	 * Depending on the selection mode, different types of elements can be selected.
	 *
	 * NOTE: These signals are forwarded to organismWidget
	 */
	signals:
	void cellSelected(Cell*);
	void vertexSelected(Vertex*);
	void edgeSelected(Edge*);
	void faceSelected(Face*);

	protected:
	void draw();
	void init();
	void reset(bool resetViewPos = true);
	void setCurrentBounds();

	// primary drawing methods
	void drawOrganism();

	void drawWireframe();
	void drawSurface(Mesh* m = NULL);
	void drawVertices();
	void drawCells();

	void drawText(Vector3d pos, QString text);

	// embellishment drawing methods
	void drawAnnotations();
	void drawBounds();
	void drawGround();

	// annotation drawing methods
	void drawForces();
	void drawVelocities();
	void drawFaceNormals();

	// selection drawing methods
	void drawWithNames();
	void drawVerticesWithNames();
	void drawCellsWithNames();
	void drawEdgesWithNames();
	void drawFacesWithNames();

	QColor getContentsColour(Cell* cc);
	Vector3d getContentsColourV(Cell* cc);
	double getSelectedValue(Cell* c);

	// selecting method
	virtual void postSelection(const QPoint& p);

	protected:

	// elements of the scene to draw
	OWorld* mOWorld;
	Organism* mOrganism;
	AABB mWorldBounds;
	std::list<Annotation*> mAnnotations;
	std::list<Mesh*> mStaticMeshes;
	// std::set<int> mDisplayMorphogens;
	int mSelectedMorphogen;
	int mSelectedVariable;
	bool mDrawMorphogensNormalised;
	bool mDrawMorphogenLabels;

	// drawing state
	bool mDrawBounds;
	bool mDrawGround;
	bool mDrawOrigin;

	bool mDrawForces;
	bool mDrawVelocities;
	bool mDrawFaceNormals;

	bool mLighting;

	float mAlpha;
	float mGamma;

	QString mDrawMode;
	enum SelectionMode{SMCell,SMVertex,SMFace,SMEdge} mSelectionMode;
};

#endif /* ORGANISMVIEWER_H_ */
