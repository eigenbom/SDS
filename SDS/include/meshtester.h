#ifndef MESHTESTER_H
#define MESHTESTER_H

/* MeshTester:
 * Provides simple test meshes
 *
 *
 * BP 15.11.07
 */

#include "mesh.h"
#include "vector3.h"
#include <string>

class MeshTester
{
	public:

	// create test mesh by name
	static Mesh* CreateTestMesh(std::string name);
	static Mesh* TestStreamer(std::string name, std::string binfile);

	// simple test meshes
	static Mesh* CreateEdgeTestMesh();
	static Mesh* CreateTriTestMesh();
	static Mesh* CreateDampingTestMesh();
	static Mesh* CreateDampingTestMeshB();

	static Mesh* CreateCubeColTest();
	static Mesh* CreateCubeColTest2();
	static Mesh* CreateCubeColTest3();
	static Mesh* CreateCubeColTest4();
	static Mesh* CreateCubeColTest5();
	static Mesh* CreateCubeColTest6();

	/// Create a tetrahedra with unit length edges
	static Mesh* CreateUnitEdgeTetra();

	static Mesh* cube(int xw, int yw, int zw, double w);

	private:
	// helpers

	static void addSingleTetra(Vector3d a, Vector3d b, Vector3d c, Vector3d d, Mesh* m);
	static void addYReflectedTetra(Vector3d a, Vector3d b, Vector3d c, Vector3d d, Mesh* m);

	// PRE: mesh is setup with appropriate attribs
	// which are added to ... e.g., aabb, volume, ...
	static void addcube(Vector3i dimensions, Vector3d origin, double subcubewidth, Mesh* m);
};

#endif
