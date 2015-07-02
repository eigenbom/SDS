/** MeshDrawer :: draws a mesh
 * Can select between tetrahedras, edges, and vertices.
 * Can optionally embellish with any of the following:
 * - forces
 * - velocities
 * - vertex masses
 * - stresses, etc.
 *
 * Draw modes: Tetrahedra, OuterFace, Edge, Vertex, and..
 * - Stress mode (Tetrahedra)
 * --- colours tetrahedra based on compression
 * - Stress mode (Edge)
 * --- colours edges based on compression
 * - Force mode (Vertex)
 * --- colours vertices based on forces
 *
 * and much much more!
 *
 * BP010208
 */

#ifndef MESHDRAWER_H
#define MESHDRAWER_H

#include "mesh.h"
#include "vector3.h"
#include "tetra.h"

#include <list>
#include <boost/any.hpp>

class MeshDrawer
{
	public: // interface

	MeshDrawer();
	void draw(Mesh* m);

	public:
	/// Drawing Mode
	enum Mode{DWire, DVertex, DEdge, DFace, DFaceSmooth, DTetra, DMass, DFaceEdges};
	void setMode(Mode m){mMode = m;}

	void toggleDrawForces(){mDrawForces = !mDrawForces;}
	void toggleDrawVelocities(){mDrawVelocities = !mDrawVelocities;}
	void toggleDrawFaceNormals(){mDrawFaceNormals = !mDrawFaceNormals;}
	void toggleDrawVertexNormals(){mDrawVertexNormals = !mDrawVertexNormals;}
	void toggleDrawHistory(){mDrawHistory = !mDrawHistory;}

	void setDrawForces(bool t){mDrawForces = t;}
	void setDrawVelocities(bool t){mDrawVelocities = t;}
	void setDrawFaceNormals(bool t){mDrawFaceNormals = t;}
	void setDrawVertexNormals(bool t){mDrawVertexNormals = t;}
	void setDrawHistory(bool t){mDrawHistory = t;}

	// select an element to show
	void selectElement(boost::any el);
	void deselectAllElements();

	void setColour(Vector3d col){mColour = col;}

	protected:

  // drawing state
	Mode mMode;

	/// drawing annotations
	bool mDrawForces,
		mDrawVelocities,
		mDrawFaceNormals,
		mDrawVertexNormals,
		mDrawHistory;

	Vector3d mColour;
	double mColourAlpha;
	double mTetShrinkAmount;

	// helper functions
	protected:

	void drawDTetra(Mesh* m);
	void drawDFace(Mesh* m);
	void drawDFaceSmooth(Mesh* m);
	void drawDEdge(Mesh* m);
	void drawDVertex(Mesh* m);
	void drawDMass(Mesh* m);
	void drawDFaceEdges(Mesh* m);
	void drawDWire(Mesh* m);

	void drawFaceNormals(Mesh*);
	void drawVertexNormals(Mesh*);

	void drawForces(Mesh*);
	void drawVelocities(Mesh*);
	void drawHistory(Mesh*);

	void setColour();
	void drawVector(const Vector3d& v,
			Vector3d colfrom = Vector3d(1,0,0),
			Vector3d colto = Vector3d(1,1,1));

	std::list<boost::any> mSelectedElements;
};

#endif
