/*
 * GMath.cpp
 *
 *  Created on: 28/10/2008
 *      Author: ben
 */

#include "gmath.h"

extern "C"
{
	#include "triangulate/interface.h"
}

#include "tetgen.h"
#include <iostream>
#include <algorithm>
#include <cassert>

namespace Math
{
	// NOTE: LICENSE: Non commercial use only
	std::vector<Vector3i> triangulate(std::vector<Vector3d> points)
	{
		int numpoints = points.size();

		// put data into the right format
		// XXX: TODO: could statically allocate these arrays in accordance with the implementation
		int (*result)[3] = new int[numpoints-2][3];

		double (*pointsarr)[2] = new double[numpoints+1][2]; // note (+ 1)
		for(int i=1;i<numpoints+1;i++)
		{
			pointsarr[i][0] = points[i-1].x();
			pointsarr[i][1] = points[i-1].y();
		}

		// int triangulate_polygon(int, int *, double (*)[2], int (*)[3]);
		// NOTE: LICENSE: Non commercial use only
		// XXX: We can probably easily change this if possible commercialisation
		triangulate_polygon(1,&numpoints,pointsarr,result);

		std::vector<Vector3i> resultVector;
		for(int i=0;i<numpoints-2;i++)
			resultVector.push_back(Vector3i(result[i][0]-1,result[i][1]-1,result[i][2]-1));

		delete[]result;
		delete[]pointsarr;

		return resultVector;
	}

	bool isDiagonal(std::vector<Vector3d>& vl, int i, int j);
	bool segmentInCone(Vector3d& v0, Vector3d& v1, Vector3d& vm, Vector3d& vp);
	double Kross(Vector3d& u, Vector3d& v);
	bool segmentsIntersect(Vector3d& p1, Vector3d& p2, Vector3d& q1, Vector3d& q2);
	void split(std::vector<int>& L, int li, int lj, std::vector<int>& L0, std::vector<int>& L1);
	bool commonTriangulation(std::vector<int>& indexList, std::vector<Vector3d>& vlist1, std::vector<Vector3d>& vlist2, std::vector<Vector3i>& triangleList)
	{
		int n = indexList.size();
		assert(n>=3);
		if (n==3)
		{
			triangleList.push_back(Vector3i(indexList[0],indexList[1],indexList[2]));
			return true;
		}

		// iterate through all pairs of vertices until we find a diagonal...
		// nore this will probably end up ear clipping anyway...oops
		for(int i = 0; i < n; i++)
		{
			for(int j = i + 2; j < n; j++)
			{
				if ((i-j+n)%n < 2) continue;

				//std::cout << "i" << i << "j" << j << std::endl;

				// if is a diagonal for both polygons, then it's an acceptable cut for our triangulation
				int ili = indexList[i], ilj = indexList[j];

				if (isDiagonal(vlist1,ili,ilj) and isDiagonal(vlist2,ili,ilj))
				{
					std::vector<int> subList0, subList1;
					split(indexList,i,j,subList0,subList1);

					return commonTriangulation(subList0,vlist1,vlist2,triangleList)
						and	commonTriangulation(subList1,vlist1,vlist2,triangleList);
				}
			}
		}

		//std::cerr << "no triangulation found";
		return false;
	}

	bool commonTriangulation(std::vector<Vector3d>& vlist1, std::vector<Vector3d>& vlist2, std::vector<Vector3i>& triangleList)
	{
		std::vector<int> il;
		for(unsigned int i=0;i<vlist1.size();i++)
			il.push_back(i);
		return commonTriangulation(il,vlist1,vlist2,triangleList);
	}

	// have i < j
	void split(std::vector<int>& L, int li, int lj, std::vector<int>& L0, std::vector<int>& L1)
	{
		// split L into two parts L1 and L2

		for(int i = li; i <= lj; i++)
		{
			L0.push_back(L[i]);
		}

		for(unsigned int i = 0; i <= (L.size() - (lj-li)); i++)
		{
			unsigned int j = (lj + i)%L.size();
			L1.push_back(L[j]);
		}
	}

	bool isDiagonal(std::vector<Vector3d>& vl, int i, int j)
	{
		Vector3d vli = vl[i];
		Vector3d vlj = vl[j];
		int n  = vl.size();

		assert(((i - j) + n)%n >= 2);
		// first test to see if it is inside the cone of its neighbour points
		int iM = ((i - 1) + n) % n;
		int iP = (i + 1) % n;

		//std::cerr << "segmentInCone( " << vli << "," << vlj << "," << vl[iM] << "," << vl[iP] << " )\n";

		if (not segmentInCone(vli,vlj,vl[iM],vl[iP]))
			return false;

		//std::cerr << "in cone" << std::endl;

		// else test all other edges to see if it intersects
		for(int j0 = 0; j0 < n; j0++)
		{
			int j1 = (j0 + 1)%n;

			if (j0!=i and j0!=j and j1!=i and j1!=j)
				if (segmentsIntersect(vli,vlj,vl[j0],vl[j1]))
					return false;
		}

		return true;
	}

	// XXX: this differs from the book in that we are specifying our vertices counter-clockwise
	bool segmentInCone(Vector3d& v0, Vector3d& v1, Vector3d& vm, Vector3d& vp)
	{
		// assert that v0,vm,vp are not colinear

		Vector3d diff = v1 - v0, edgeL = vm - v0, edgeR = vp - v0;

		double kr = Kross(edgeR,edgeL);
		double krdr = Kross(diff,edgeR);
		double krdl = Kross(diff,edgeL);

		//std::cerr << "kr " << kr << " krdr " << krdr << " krdl" << krdl << "\n";


		if (kr < 0)
		{
			if (krdr < 0) return true;
			if (krdr > 0 and krdl > 0) return true;
		}
		else if (kr > 0)
		{
			if (krdr < 0 and krdl > 0) return true;
		}
		else
			assert(false);
		return false;

		/*
		// counter-clockwise
		if (kr < 0)
			return (krdr>0 and krdl<0);
		else if (kr > 0)
			return (krdr<0 and krdl>0);
		else assert(false);
		*/

		/*
		// clockwise
		if (kr > 0)
			return (Kross(diff,edgeR)>0 and Kross(diff,edgeL)<0);
		else if (kr < 0)
			return (Kross(diff,edgeR)<0 and Kross(diff,edgeL)>0);
		else assert(false);
		*/
	}

	double Kross(Vector3d& u, Vector3d& v)
	{
		return u.x()*v.y() - u.y()*v.x();
	}

	bool segmentsIntersect(Vector3d& pa, Vector3d& pb, Vector3d& qa, Vector3d& qb)
	{
		static double sqrEpsilon = 0.00001;

		//based on findIntersection() (GeometricTools pg245)

		Vector3d p0 = pa, d0 = pb - pa, p1 = qa, d1 = qb - qa;

		Vector3d e = p1 - p0;
		double kross = Kross(d0,d1);
		double sqrKross = kross*kross;
		double sqrLen0 = d0.x()*d0.x() + d0.y()*d0.y();
		double sqrLen1 = d1.x()*d1.x() + d1.y()*d1.y();

		if (sqrKross > sqrEpsilon*sqrLen0*sqrLen1)
		{
			// segments not parallel
			double s = (e.x()*d1.y() - e.y()*d1.x()) / kross;
			if (s < 0 or s > 1)
				return false;
			double t = (e.x()*d0.y() - e.y()*d0.x()) / kross;
			if (t < 0 or t > 1)
				return false;

			// otherwise they must intersect
			return true;
		}

		// else lines are parallel
		double sqrLenE = e.x()*e.x() + e.y()*e.y();
		kross = Kross(e,d0);
		sqrKross = kross*kross;
		if (sqrKross > sqrEpsilon*sqrLen0*sqrLenE)
		{
			// lines are different
			return false;
		}

		// else lines are overlapping, this should never happen for our cases
		assert(false);
	}

	void tetrahedralise(std::vector<Vector3d>& points, std::vector<boost::tuple<int,int,int> >& trianglesOnHull, std::vector<boost::tuple<int,int,int,int> >& tetrahedra)
	{
		tetgenio in,out;
		tetgenbehavior tb;
		if (trianglesOnHull.size()==0)
			tb.parse_commandline("YQnqz"); // no 'p'
		else
			tb.parse_commandline("pYQnqz");

		// *****************************************
		// format the input to tetgen

		tetgenio::facet *f;
		tetgenio::polygon* p;
		in.firstnumber = 0;
		in.numberofpoints = points.size();
		in.pointlist = new REAL[in.numberofpoints * 3];
		for(int i=0;i<in.numberofpoints;i++)
		for(int j=0;j<3;j++)
			in.pointlist[(i*3)+j] = points[i][j];

		in.numberoffacets = trianglesOnHull.size();
		if (in.numberoffacets==0)
		{
			in.facetlist = NULL;
			in.facetmarkerlist = NULL;
		}
		else
		{
			in.facetlist = new tetgenio::facet[in.numberoffacets];
			//in.facetmarkerlist = new int[in.numberoffacets];
			in.facetmarkerlist = NULL;

			for(int i=0;i<in.numberoffacets;i++)
			{
				f = &in.facetlist[i];
				f->numberofpolygons = 1;
				f->polygonlist = p = new tetgenio::polygon;
				f->numberofholes = 0;
				f->holelist = NULL;
				p->numberofvertices = 3;
				p->vertexlist = new int[3];
				p->vertexlist[0] = trianglesOnHull[i].get<0>();
				p->vertexlist[1] = trianglesOnHull[i].get<1>();
				p->vertexlist[2] = trianglesOnHull[i].get<2>();

				//in.facetmarkerlist[i] = -1;
			}
		}

		// *****************************************
		// the main tetrahedralise call
		tetrahedralize(&tb,&in,&out);

		// *****************************************
		// format the output data appropriately

		// any extra points are appended to the points data structure
		// XXX: we are assuming that extra points are appended to out.pointlist
		// i think this should always be the case
		for(int i=in.numberofpoints;i<out.numberofpoints;i++)
		{
			REAL* p = &out.pointlist[i*3];
			points.push_back(Vector3d(p[0],p[1],p[2]));
		}

		for(int i=0;i<out.numberoftetrahedra;i++)
		{
			int* tet = &out.tetrahedronlist[i*4];
			tetrahedra.push_back(boost::make_tuple(tet[0],tet[1],tet[3],tet[2]));
		}

		// NOTE: the structures within in and out are automatically freed upon deletion

	}

	void tetrahedralise(std::vector<Vector3d>& points, std::vector<boost::tuple<int,int,int> >& trianglesOnHull, std::vector<boost::tuple<int,int,int,int> >& tetrahedra, std::vector<boost::tuple<int,int,int,int> >& tetrahedraNeighbours, std::vector<boost::tuple<int,int> >& edges)
	{
		tetgenio in,out;
		tetgenbehavior tb;
		if (trianglesOnHull.size()==0)
			tb.parse_commandline("YQnqz"); // no 'p'
		else
			tb.parse_commandline("pYQnqz");



		// *****************************************
		// format the input to tetgen

		tetgenio::facet *f;
		tetgenio::polygon* p;
		in.firstnumber = 0;
		in.numberofpoints = points.size();
		in.pointlist = new REAL[in.numberofpoints * 3];
		for(int i=0;i<in.numberofpoints;i++)
		for(int j=0;j<3;j++)
			in.pointlist[(i*3)+j] = points[i][j];

		in.numberoffacets = trianglesOnHull.size();

		if (in.numberoffacets==0)
		{
			in.facetlist = NULL;
			in.facetmarkerlist = NULL;
		}
		else
		{
			in.facetlist = new tetgenio::facet[in.numberoffacets];
			//in.facetmarkerlist = new int[in.numberoffacets];
			in.facetmarkerlist = NULL;

			for(int i=0;i<in.numberoffacets;i++)
			{
				f = &in.facetlist[i];
				f->numberofpolygons = 1;
				f->polygonlist = p = new tetgenio::polygon;
				f->numberofholes = 0;
				f->holelist = NULL;
				p->numberofvertices = 3;
				p->vertexlist = new int[3];
				p->vertexlist[0] = trianglesOnHull[i].get<0>();
				p->vertexlist[1] = trianglesOnHull[i].get<1>();
				p->vertexlist[2] = trianglesOnHull[i].get<2>();

				//in.facetmarkerlist[i] = -1;
			}
		}

		// *****************************************
		// the main tetrahedralise call
		tetrahedralize(&tb,&in,&out);

		// *****************************************
		// format the output data appropriately

		// any extra points are appended to the points data structure
		// XXX: we are assuming that extra points are appended to out.pointlist
		// i think this should always be the case
		for(int i=in.numberofpoints;i<out.numberofpoints;i++)
		{
			REAL* p = &out.pointlist[i*3];
			points.push_back(Vector3d(p[0],p[1],p[2]));
		}

		// hash the edges into a unique number
		int nop = out.numberofpoints;
		static const int edgePairs[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
		std::vector<bool> edgeExists(out.numberofpoints*out.numberofpoints);
		std::fill(edgeExists.begin(),edgeExists.end(),false);

		for(int i=0;i<out.numberoftetrahedra;i++)
		{
			int* tet = &out.tetrahedronlist[i*4];
			tetrahedra.push_back(boost::make_tuple(tet[0],tet[1],tet[3],tet[2]));

			int* nb = &out.neighborlist[i*4];
			tetrahedraNeighbours.push_back(boost::make_tuple(nb[0],nb[1],nb[3],nb[2]));

			// edge list
			for(int j=0;j<6;j++)
			{
				const int* e = &edgePairs[j][0];
				int e1 = tet[e[0]], e2 = tet[e[1]];
				int id = std::min(e1,e2)*nop + std::max(e1,e2);
				if (!edgeExists[id])
				{
					edgeExists[id] = true;
					edges.push_back(boost::make_tuple(e1,e2));
				}
			}
		}

		// NOTE: the structures within in and out are automatically freed upon deletion

	}
};


