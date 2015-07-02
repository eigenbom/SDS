#ifndef MESH_H
#define MESH_H

/* Mesh: A collection of tetrahedra, connected arbitrarily.
 * - Constructed through MeshLoader or MeshTester
 * BP 250108
 *
 * TODO: Remove internal face list. Build list only when required.
 */

#include "tetra.h"
#include "face.h"
#include "edge.h"
#include "vertex.h"
#include "aabb.h"
#include "vector3.h"
#include "bstreamable.h"

#include <list>
#include <map>

class World;
class Mesh: public BStreamable {
	public:

	Mesh(bool tc = true); // empty mesh
	virtual ~Mesh();

	/**
	 * Clear all the contents of this mesh.
	 */
	void clear();

	// modifiers
	/// move the mesh (resets velocity)
	void move(Vector3d);
	void scale(double sx = 1, double sy = 1, double sz = 1);

	/// Make a duplicate of this mesh (deeeep copy)
	Mesh* copy();

	/// Update Bounding Box
	void updateAABB();

	// accessors
	inline const std::list<Vertex*>& vertices() const;
	inline const std::list<Edge*>& edges() const;
	inline const std::list<Tetra*>& tetras() const;
	inline const std::list<Face*>& outerFaces() const;

	// XXX: Deprecated, we don't store all faces, only outer faces
	inline const std::list<Face*>& faces() const;

	/**
	 * Find an element (or all elements) that contains the specified vertices.
	 *
	 * You don't need to pass all the vertices, for example,
	 * calling getTetra(v1) will return an arbitrary tetrahedron
	 * that contains v1.
	 *
	 * @return The element, or null if doesn't exist.
	 */
	Edge* getEdge(Vertex* v1, Vertex* v2 = NULL) const;
	std::list<Edge*> getAllEdges(Vertex* v1) const;
	Face* getFace(Vertex* v1, Vertex* v2 = NULL, Vertex* v3 = NULL) const;
	Tetra* getTetra(Vertex* v1, Vertex* v2 = NULL, Vertex* v3 = NULL, Vertex* v4 = NULL) const;
	/// helper functions
	Face* getSurfaceFace(Vertex* va, Vertex* vb = NULL, Vertex* vc = NULL) const;

	// return the i'th face of tetra t if it exists
	Face* getFace(Tetra* t, int i) const;

	/// checks if t contains e, PRE:t,e not NULL
	static bool doesTetraContainEdge(Tetra* t, Edge* e);

	static void removeFaceNeighbourLinks(Face* f);
	static void addFaceNeighbourLinks(Face* f);

	/// Higher-level accessing functions
	/// Return a list of all adjacent tetrahedra
	std::list<Tetra*> getNeighbours(Vertex* v) const;

	/// Return the faces f1 and f2 that share edge e.
	/// No particular order is guaranteed.
	/// @return true if the faces were found, false if they weren't.
	bool getAdjacentFaces(Edge* e, Face* &f1, Face* &f2) const;
	/// Same as getAdjacentFaces(Edge...) but v1, v2 define the edge.
	bool getAdjacentFaces(Vertex* v1, Vertex* v2, Face* &f1, Face* &f2) const;

	/// directly change content of mesh data structures
	/// does not do things like update neighbourhood relations etc ... (should it?)
	void addVertex(Vertex*);
	void addEdge(Edge*);
	//void addFace(Face*);
	void addOuterFace(Face*);

	/// This tags the face as an outer face in the mesh data structure
	/// PRE: f is in faces list but NOT IN outer Faces list
	void setAsOuterFace(Face* f);
	void addTetra(Tetra*);
	/// remove functions
	/// WARNING: does not delete the element, only removes it from the lists
	/// DEPRECATED: better to use the delete functions below as they handle everything!
	void removeTetra(Tetra* t);
	void removeFace(Face* f); // from both outer face and face list
	void removeEdge(Edge* e);
	void removeVertex(Vertex* v);


	/// remove functions
	/// These functions remove the element, fix the topology, and delete the element.
	/// NOTE: The element is deleted, so the pointer is no longer valid!
	//void deleteTetra(Tetra* t);
	void deleteFace(Face* f);
	/** deletes the edge, removes the neighbour link between verts **/
	void deleteEdge(Edge* e);
	/** deletes the vertex and any adjacent edges **/
	//void deleteVertex(Vertex* v);

	// get information about the mesh
	/// current volume
	double volume() const;
	/// sum of vertex masses, note: not uniformly dense so mass!=volume
	double mass() const;
	/// total energy in the mesh (potential + kinetic)
	double energy() const;

	/// bounds
	inline const AABB& aabb();

	// has the mesh changed topology since last call?
	inline bool hasTopoChanged();
	inline void topoChange();
	inline void setTopoChanged(bool t);

	/**
	 * For debugging. Checks that the mesh data structure is valid.
	 * E.g., all vertex neighbour pairs are represented by an edge, etc.
	 */
	bool isSane();

	/// read/write in full form
	void bWriteFull(std::ostream& bin);
	void bReadFull(std::istream&,bool verbose=false);

	/// coming soon?
	void bWriteBare(std::ostream& bin){}
	void bReadBare(std::istream& bin, bool verbose=false){}

	/// deprecated
	void bWriteNew(std::ostream&);
	/// deprecated
	void bReadNew(std::istream& i, bool v = false){bReadFull(i,v);}

	// extra data
	void bWriteSpringMultipliers(std::ostream& bin);
	void bReadSpringMultipliers(std::istream& bin);

	void bWriteFrozenVerts(std::ostream& bin);
	void bReadFrozenVerts(std::istream& bin);

	/// serialising functions (DEPRECATED?)
	//void bWrite(std::ostream&);
	//void bRead(std::istream&);

	// retrieve the vertex->id map used when last called WriteNew
	// used by organism to serialise
	inline std::map<Vertex*,unsigned int>& vertexMapWrite();
	inline std::map<unsigned int,Vertex*>& vertexMapRead();

	protected:

	std::list<Vertex*> mVertices;
	std::list<Edge*> mEdges;
	std::list<Tetra*> mTetras;
	std::list<Face*> mOuterFaces; // surface faces (NB: all oriented ccw when viewed from outside)

	// XXX: Remove this
	std::list<Face*> mFaces;

	AABB mAABB;

	bool mTopoChanged;

	/* serialising stuff... */
	// SETUP PTR->ID MAPPING
	std::map<Vertex*, unsigned int> mVertexMapWrite;
	std::map<Face*, unsigned int> mFaceMapWrite;
	std::map<Tetra*, unsigned int> mTetraMapWrite;

	std::map<unsigned int,Vertex*> mVertexMapRead;
	//

	friend class MeshTools;
	friend class MeshTester;
	friend class Physics;
	friend class Stepper;
	friend class Collision;
	friend class MeshInfo;
};

#include "mesh.inl"

#endif
