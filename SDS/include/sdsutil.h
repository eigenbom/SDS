#ifndef SDSUTIL_H
#define SDSUTIL_H

#include "tetra.h"
#include "cell.h"
#include "organism.h"

#include <map>
#include <list>

/**
 * Miscellaneous methods.
 */
class SDSUtil
{
public:

	/**
	 * Generate the tet-topology graph of all tetrahedra connected to cell c.
	 *
	 * @param c Cell
	 * @param o Organism
	 * @return an adjacency list.
	 */
	static std::map<Tetra*, std::list<Tetra*> > getTetTopoGraph(Cell* c, Organism* o);

	/// find the (non-world) time (0,1) when a intersects the moving plane defined by b,c,d
	/// returns <0 if no intersection in that time period occurs
	static double doVertexPlaneIntersect(Tetra* t, int ai, int bi, int ci, int di);

	/// rewind all vertices positions back by dh seconds, given current position occurs dt seconds after xOld
	/// keeps xOld the same
	static void rewindAllVertices(Mesh* m, double dh, double dt);

	// a wrapper around the standalong tetgen program
	static bool runTetgenCommandLine(int argc, char** argv);
};

#endif
