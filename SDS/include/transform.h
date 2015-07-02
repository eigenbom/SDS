/** Implements the different topological transformation operations of SDS
 *
 * Note that the function definitions are split amongst multiple source files
 * all prefixed with "transform". This is due to the size of the code base, and
 * different implementations of functions (e.g., a debug and release version.)
 * */

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include <boost/tuple/tuple.hpp>
#include <map>

#include "organism.h"
#include "vertex.h"
#include "tetra.h"
#include "cell.h"
#include "vector3.h"
#include "propertylist.h"
#include "log.h"

class Transform {
public:
	/// For debugging purposes, ome transformations may perform cumulative transformation over time.
	// The state of these transformations is Transform::Transforming
	// The transformation is not complete until the state is Completed.
	// Repeatedly call the transform function to compute the next partial transformation.
	// properties contains information related to the very last transformation performed
	// it is cleared automatically
	// allProperties stores all properties until it is reset - it is used to have a list of all transformations performed in a single step
	enum State{TRANSFORM_ERROR,TRANSFORM_COMPLETED,TRANSFORM_TRANSFORMING};
	static State state;
	static PropertyList properties;
	static std::list<PropertyList> allProperties;
	static void clearAllProperties(){allProperties.clear();}

	static std::string getErrorMessage();
	static void setErrorMessage(std::string msg);
	static bool hasError(){return state==TRANSFORM_ERROR;}

	/// reset the propertylist, state, etc..
	static void reset();

private:
	static std::string sErrorMessage;

public:

	/// HIGH LEVEL CELL DIVISION
	/**
	 * Implements a division operation, dividing cell c in direction dir.
	 * Note that Cell* c can no longer be guaranteed to exist.
	 *
	 * @param c The cell that is dividing.
	 * @param dir The direction to divide.
	 * @param o The organism this cell is part of.
	 * @return The two daughter cells that result from this operation. A (NULL,NULL) tuple is returned if an error occurs.
	 */
	static boost::tuple<Cell*,Cell*> Divide(Cell* c, Vector3d dir, Organism* o);

	/**
	 * A simple and quick division operator.
	 */
	static boost::tuple<Cell*,Cell*> DivideSimple(Cell* c, Vector3d dir, Organism* o);

	/**
	 * Implements a more "balanced", but slower cell division.
	 */
	static boost::tuple<Cell*,Cell*> DivideBalanced(Cell* c, Vector3d dir, Organism* o);

	/**
	 * Add new cells and retetrahedralise the hull of T.
	 *
	 * POST: the tetra in T are deleted
	 */
	static bool Complexify(std::list<Cell*> C, std::list<Tetra*> T, Organism* o);

	/// LOW LEVEL CELL DIVISION TRANSFORMATIONS

	/// Subdivide a tetrahedra in a mesh from the direction of vertex
	// PRE: v \in t, m is valid, m is uniformly dense
	// POST: t doesn't exist anymore, it is deleted
	// NOTE: New cells have half the mass of the old cell
	static boost::tuple<Cell*,Cell*> DivideTetra(Cell* c, Tetra* t, Organism* o);

	/// Subdivide the surface face f
	// PRE: f \in m->outerFaces()
	// PRE: c->v \in f
	static boost::tuple<Cell*,Cell*> DivideTetra(Cell* c, Face* f, Organism* o);

	/// Division away from boundary
	// PRE: f \in m->outerFaces()
	// PRE: c->v \in f
	static boost::tuple<Cell*, Cell*> DivideAway(Cell* c, Face* f, Organism* o);

	/// Division into an adjacent edge
	// PRE: e is internal
	// PRE: c->v \in e
	static boost::tuple<Cell*, Cell*> DivideInternalEdge(Cell* c, Edge* e, Organism* o);

	/// Division into an adjacent edge
	// PRE: e is on the surface
	// PRE: c->v \in e
	static boost::tuple<Cell*, Cell*> DivideSurfaceEdge(Cell* c, Edge* e, Organism* o);

	// Division in a specific direction
	/// The threshold of which to subdivide an edge or tetra
	static double sDivideInternalAngleThreshold;
	/// Division in a specific direction
	// PRE: Direction points INTO the mesh. (i.e., c is internal, or c is on the surface but dir points into the mesh)
	static boost::tuple<Cell*, Cell*> DivideInternal(Cell* c, Vector3d dir, Organism* o);

	/// The threshold of which to subdivide an edge or tetra
	static double sDivideSurfaceAngleThreshold;
	/// Division along surface in a specific direction (external cell)
	// PRE: c is a boundary cell
	static boost::tuple<Cell*, Cell*> DivideAlong(Cell* c, Vector3d dir, Organism* o);

	/// CELL MOVEMENT TRANSFORMATIONS
	static void MoveVertexThroughOppositeFace(int vIndex, Tetra* tetra, Organism* o);
	static void MoveEdgeThroughEdge(int ea, int eb, Tetra* t, Organism* o);

private:
	// vertex-through-face internal and boundary cases
	static void vfInternalCase(int vi, int ai, int bi, int ci, Tetra* t, Organism* o);
	static void vfBoundaryCase(int vi, int ai, int bi, int ci, Tetra* t, Face* f0, Organism* o);

public:

	/// returns a <b>Clockwise</b> ordered (from "a" looking down to "b") list of tetrahedra adjacent to the edge (a,b)
	/// and return the vertex indices corresponding to the surrounding verts
	/// If edge ab is on the surface, then the tetras do not form a closed loop and the starting tetra is on the right of edge ab
	static void enumerateSurroundingTetrasAndVertices(Vertex* a, Vertex* b, Organism* o, std::vector<Tetra*>& tetras, std::vector<int>& vertexIndices);

	/// traces the path of tetras joined to the edge e1,e2
	/// @return true if a loop was found, false if not
	static bool findTetras(std::vector<Tetra*>& X, std::vector<int>& V, Tetra* t, int vi, Vertex* e1, Vertex* e2, Organism* o);
	static void projectPoints(std::vector<Vertex*>& points, Vertex* u1, Vertex* u2, Vertex* v1, Vertex* v2, std::vector<Vector3d>& projected);
	static void projectTransformedPoints(std::vector<Vector3d>& points, Vertex* u1, Vertex* u2, Vertex* v1, Vertex* v2, std::vector<Vector3d>& projected);

	/// find a valid common triangulation of points when perspectively projected from both u1->u2 and u2->u1
	// returns false if no valid triangulation
	static bool findTriangulation(std::vector<Vertex*>& points, Vertex* u1, Vertex* u2, Vertex* v1, Vertex* v2, std::vector<Vector3i>& triangulation);

	static void addEdgeIfNone(Vertex* v1, Vertex* v2, Organism* o);

	/// compute the volume of a tetrahedra joining four cells with radii r1...r4
	// XXX: notes: 4 sqrts involved, not too fast, probably numerically unstable...
	static double tetrahedraRestVolume(double r1, double r2, double r3, double r4);

	/// Overloaded helpers
	static double tetrahedraRestVolume(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4, Organism* o);
	static double tetrahedraRestVolume(Cell* c1, Cell* c2, Cell* c3, Cell* c4);

public:
	/// HELPER FUNCTIONS

	///  Retrieve neighbouring tetra in a certain direction
	// PRE: c is internal
	static Tetra* getTetraInDirection(Cell* c, Vector3d dir, Organism* o);

	///  Retrieve neighbouring surface face in a certain direction
	// PRE: c is on the boundary
	static Face* getFaceInDirection(Cell* c, Vector3d dir, Organism* o);

	// used in transform_division_balanced
	static long MAPFACEID(Vertex* v1, Vertex* v2, Vertex* v3, std::map<Vertex*,int>& index);
	static long MAPFACEID_I(int iv1, int iv2, int iv3, std::map<Vertex*,int>& index);
	static long MAPEDGEID_I(int iv1, int iv2, std::map<Vertex*,int>& index);

	// sanity checks
	static bool checkTetraNeighbour(Tetra* t, int index);
private:

	// adds the property ("Transformation",s) to properties, each transformation should add its own name to the property list
	static void called(std::string s);


	/**
	 * return this when an error occurs
	 * it sets the simulation state to ERROR
	 * it prints out a message
	 * and it returns null
	 */
	static boost::tuple<Cell*,Cell*> error(std::string s = "");

	// adds the current properties to allPropertiesList
	// should be called before returning
	static void finaliseProperties();

	/**
	 * return this when you want to halt
	 * it sets the simulation state to STOP
	 * it prints out a message
	 * and it returns null
	 */
	static boost::tuple<Cell*,Cell*> testQuit();

};

#endif /* TRANSFORM_H_ */
