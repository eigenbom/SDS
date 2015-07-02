#include "gmath.h"
#include "hmath.h"
#include "vmath.h"
#include "random.h"
#include "vector3.h"

#include <iostream>
#include <cassert>
#include <iomanip>
#include <vector>
#include <cfloat>

#include <boost/foreach.hpp>
#include <boost/timer.hpp>

#include "random.h"

using std::cout;

void fan(int NUMPOINTS);
void fan2(int NUMPOINTS);
void draw(Vector3d& e, Vector3d& f, Vector3d& g, double width, int sample_width);
void draw2(Vector3d& e, Vector3d& f, Vector3d& g, double width, int sample_width);
void draw(std::vector<Vector3i>& faces, std::vector<Vector3d>& verts, Vector3d center, double width, int sample_width);

int main()
 {
	std::cout << "Vol radius check" << std::endl;

	double rad[10];
	for(int i=0;i<10;i++)
	{
		rad[i] = i;
		double vol = Math::volumeOfSphereGivenRadius(rad[i]);
		double radius = Math::radiusOfSphereGivenVolume(vol);

		std::cout << "rad: " << rad[i] << ",vol: " << vol << ",radius: " << radius << std::endl;
	}


	std::cout << "Barycentric coordinate test" << std::endl;

	Random r;

	// draw the triangle efg by calculating barycentric coords and testing the sum
	Vector3d e(r.getDouble(),r.getDouble(),r.getDouble());
	Vector3d f(r.getDouble(),r.getDouble(),r.getDouble());
	Vector3d g(r.getDouble(),r.getDouble(),r.getDouble());

	draw(e,f,g,1,30);

	// do a center point test

	double b1, b2;
	Math::baryTri(e,g,f,(e+f+g)/3,b1,b2);
	assert(b1>=0 and b2>=0 and (b1+b2)<=1);

	// triangulate some polygons
	std::cout << "Polygon triangulation test" << std::endl;

	// simple square triangulation
	std::cout << "Square triangulation test" << std::endl;

	std::vector<Vector3d> square_vertices;
	square_vertices.push_back(Vector3d(0,0,0));
	square_vertices.push_back(Vector3d(1,0,0));
	square_vertices.push_back(Vector3d(1,1,0));
	square_vertices.push_back(Vector3d(0,1,0));

	std::vector<Vector3i> faces = Math::triangulate(square_vertices);
	draw(faces,square_vertices,Vector3d(0,0,0),2.1,20);

	std::cout << "Regular polygon triangulation test" << std::endl;
	std::vector<Vector3d> reg_poly;
	double tf = 3/4.;
	reg_poly.push_back(Vector3d(1,0,0));
	reg_poly.push_back(Vector3d(tf,tf,0));
	reg_poly.push_back(Vector3d(0,1,0));
	reg_poly.push_back(Vector3d(-tf,tf,0));
	reg_poly.push_back(Vector3d(-1,0,0));
	reg_poly.push_back(Vector3d(-tf,-tf,0));
	reg_poly.push_back(Vector3d(0,-1,0));
	reg_poly.push_back(Vector3d(tf,-tf,0));
	faces = Math::triangulate(reg_poly);
	draw(faces,reg_poly,Vector3d(0,0,0),2.1,30);

	std::cout << "Concave polygon triangulation test" << std::endl;
	std::vector<Vector3d> cave;
	cave.push_back(Vector3d(0,0,0));
	cave.push_back(Vector3d(tf,tf,0));
	cave.push_back(Vector3d(0,1,0));
	cave.push_back(Vector3d(-tf,tf,0));
	cave.push_back(Vector3d(-1,0,0));
	cave.push_back(Vector3d(-tf,-tf,0));
	cave.push_back(Vector3d(0,-1,0));
	cave.push_back(Vector3d(tf,-tf,0));
	faces = Math::triangulate(cave);
	draw(faces,cave,Vector3d(0,0,0),2.1,60);

	int numpoints = 20;
	std::cout << "Large random simple polygon set of " << numpoints << " points" << std::endl;
	std::vector<Vector3d> randPoly;

	Random random;
	Vector3d v(1,0,0);
	for(int i=0;i<numpoints;i++)
	{
		randPoly.push_back(Math::rotateZ(v,Math::TWO_PI*i/numpoints)*(1+0.1*random.getDouble()));
	}
	faces = Math::triangulate(randPoly);
	draw(faces,randPoly,Vector3d::ZERO,2.1,numpoints*4);

	// test speed of triangulation
	int numtests = 100;
	int testsize = 100;
	std::cout << "Compute time for random simple polygons [" << numtests << " tests of " << testsize << " points]" << std::endl;

	double tmin = DBL_MAX, tmax = -DBL_MAX, tavg = 0;
	for(int i=0;i<numtests;i++)
	{
		std::vector<Vector3d> randomPoly;
		Vector3d v(1,0,0);
		for(int i=0;i<testsize;i++)
		{
			randomPoly.push_back(Math::rotateZ(v,Math::TWO_PI*i/testsize)*(1+0.1*random.getDouble()));
		}

		boost::timer timer;
		faces = Math::triangulate(randomPoly);
		double seconds = timer.elapsed();
		if (seconds < tmin) tmin = seconds;
		if (seconds > tmax) tmax = seconds;
		tavg += seconds;
	}
	tavg /= numtests;
	std::cout << "Average: " << tavg << "s, Min: " << tmin << "s, Max: " << tmax << "s" << std::endl;


	// TEST commonTriangulation routine
	std::cout << "Testing commonTriangulation" << std::endl;

	double tripoints[][2] = {{-1,0},{1,0},{0,1}};
	std::vector<Vector3d> tri1, tri2;
	std::vector<int> triIndices;

	for(int i=0;i<3;i++)
	{
		tri1.push_back(Vector3d(tripoints[i][0],tripoints[i][1],0));
		tri2.push_back(Vector3d(tripoints[i][0],tripoints[i][1],0));
		triIndices.push_back((i+1)%3);
	}

	std::vector<Vector3i> commonFaces;
	if (!Math::commonTriangulation(triIndices,tri1,tri2,commonFaces))
		assert(false);
	assert(commonFaces.size()==1);
	draw(commonFaces,tri1,Vector3d::ZERO,2,10);
	draw(commonFaces,tri2,Vector3d::ZERO,2,10);

	std::cout << "\nTesting commonTriangulation again" << std::endl;

	double sqPoints[][2] = {{-1,-1},{1,-1},{1,1},{-1,1}};
	double sqshedPoints[][2] = {{-1,-1},{1,-1},{-.5,-.5},{-1,1}};
	std::vector<Vector3d> sqP, sqshP;
	std::vector<int> sqIndices;

	for(int i=0;i<4;i++)
	{
		sqP.push_back(Vector3d(sqPoints[i][0],sqPoints[i][1],0));
		sqshP.push_back(Vector3d(sqshedPoints[i][0],sqshedPoints[i][1],0));
		sqIndices.push_back((i+1)%4);
	}

	commonFaces.clear();
	if (!Math::commonTriangulation(sqIndices,sqP,sqshP,commonFaces))
		assert(false);
	assert(commonFaces.size()==2);
	draw(commonFaces,sqP,Vector3d::ZERO,2,20);
	draw(commonFaces,sqshP,Vector3d::ZERO,2,20);

	// more tests

	fan(1);
	fan(2);
	fan(3);
	fan(10);

	fan2(1);
	fan2(3);

	for(int i=5; i < 10; i++)
		fan2(i);

	return 0;
}

void fan(int NUMPOINTS)
{
	static Random random;
	// harder triangulation
		std::cout << "\nTesting commonTriangulation on FAN(" << NUMPOINTS << ")" << std::endl;

		std::vector<Vector3d> sinPoints;
		std::vector<int> sinIndices;

		double mid = 0.5, amp = 0.25, period = 4*M_PI;
		sinPoints.push_back(Vector3d(0,0,0));
		sinPoints.push_back(Vector3d(1,0,0));

		for(int i=0; i < NUMPOINTS; i++)
		{
			double x = 1 - double(i+1)/(NUMPOINTS);
			double sinx = Math::sin(x*period)*amp + mid;
			// sinPoints.push_back(Vector3d(x,1 + random.getDouble()*.5,0));
			sinPoints.push_back(Vector3d(x,sinx,0));
		}

		for(int i=0; i < (NUMPOINTS + 2); i++)
			sinIndices.push_back(i);

		std::vector<Vector3i> commonFaces;
		if (!Math::commonTriangulation(sinIndices,sinPoints,sinPoints,commonFaces))
				assert(false);

		std::cout << "faces: ";
		for(std::vector<Vector3i>::iterator it = commonFaces.begin();it!=commonFaces.end();it++)
		{
			Vector3i i = *it;
			std::cout << "(" << i.x() << "," << i.y() << "," << i.z() << ") ";
		}
		std::cout << "\n";

		draw(commonFaces,sinPoints,Vector3d(.5,.5,0),1,40);
}

void fan2(int NUMPOINTS)
{
	static Random random;
	// harder triangulation
		std::cout << "\nTesting commonTriangulation on FAN2(" << NUMPOINTS << ")" << std::endl;

		std::vector<Vector3d> sinPoints;
		std::vector<Vector3d> cosPoints;
		std::vector<int> sinIndices;

		double mid = 0.5, amp = 0.25, period = 4*M_PI;
		sinPoints.push_back(Vector3d(0,0,0));
		sinPoints.push_back(Vector3d(1,0,0));

		cosPoints.push_back(Vector3d(0,0,0));
		cosPoints.push_back(Vector3d(1,0,0));

		for(int i=0; i < NUMPOINTS; i++)
		{
			double x = 1 - double(i+1)/(NUMPOINTS);
			double sinx = Math::sin(x*period)*amp + mid;
			double cosx = Math::cos(x*period)*amp + mid;
			// sinPoints.push_back(Vector3d(x,1 + random.getDouble()*.5,0));
			sinPoints.push_back(Vector3d(x,sinx,0));
			cosPoints.push_back(Vector3d(x,cosx,0));
		}

		for(int i=0; i < (NUMPOINTS + 2); i++)
			sinIndices.push_back(i);

		std::vector<Vector3i> commonFaces;
		if (!Math::commonTriangulation(sinIndices,sinPoints,cosPoints,commonFaces))
		{
			std::cout << "No common triangulation\n";
		}
		else
		{
			std::cout << "faces: ";
			for(std::vector<Vector3i>::iterator it = commonFaces.begin();it!=commonFaces.end();it++)
			{
				Vector3i i = *it;
				std::cout << "(" << i.x() << "," << i.y() << "," << i.z() << ") ";
			}
			std::cout << "\n";

			draw(commonFaces,sinPoints,Vector3d(.5,.5,0),1,NUMPOINTS*5);
			draw(commonFaces,cosPoints,Vector3d(.5,.5,0),1,NUMPOINTS*5);
		}
}

void draw(Vector3d& e, Vector3d& f, Vector3d& g, double width, int sample_width)
{
	double y = width;
	double dy = width / sample_width;

	double x = 0;
	double dx = width / sample_width;

	for(int i=0;i<=sample_width;++i,y-=dy)
	{
		x = 0;
		for(int j=0;j<=sample_width;++j,x+=dx)
		{
			Vector3d p(x,y,0);

			if ((Math::abs(p.x()-e.x()) < dx/2 and
				Math::abs(p.y()-e.y()) < dy/2))
				cout << "e";
			else if (Math::abs(p.x()-f.x()) < dx/2  and
				Math::abs(p.y()-f.y()) < dy/2)
				cout << "f";
			else if (Math::abs(p.x()-g.x()) < dx/2 and
				Math::abs(p.y()-g.y()) < dy/2)
				cout << "g";
			else
			{
				double b1, b2;
				Math::baryTri(e,f,g,p,b1,b2);

				if (b1>=0 and b2>=0 and b1+b2<=1)
					cout << ".";
				else
					cout << "_";
			}
		}
		cout << "\n";
	}
}

void draw2(Vector3d& e, Vector3d& f, Vector3d& g, double width, int sample_width)
{
	double y = width;
	double dy = width / sample_width;

	double x = 0;
	double dx = width / sample_width;

	for(int i=0;i<=sample_width;++i,y-=dy)
	{
		x = 0;
		for(int j=0;j<=sample_width;++j,x+=dx)
		{
			Vector3d p(x,y,0);

			if (
				(Math::abs(p.x()-e.x()) < dx/2 and
				Math::abs(p.y()-e.y()) < dy/2)  or
				(Math::abs(p.x()-f.x()) < dx/2  and
				Math::abs(p.y()-f.y()) < dy/2)  or
				(Math::abs(p.x()-g.x()) < dx/2 and
				Math::abs(p.y()-g.y()) < dy/2))
			{
				cout << "   XX   ";
			}
			else
			{
				double b1, b2;
				Math::baryTri(e,f,g,p,b1,b2);

				cout << std::setw(3) << b1 << "," << std::setw(3) << b2 << " ";
			}
		}
		cout << "\n";
	}

}

void draw(std::vector<Vector3i>& faces, std::vector<Vector3d>& verts, Vector3d center, double width, int sample_width)
{
	double y = width/2;
	double dy = width / sample_width;

	double x = -width/2;
	double dx = width / sample_width;

	for(int i=0;i<=sample_width;++i,y-=dy)
	{
		x = -width/2;
		for(int j=0;j<=sample_width;++j,x+=dx)
		{
			Vector3d p(center.x()+x,center.y()+y,0);

			// for each face, we determine which one it falls into
			char facesymbol = 'a';
			bool wasinface = false;
			BOOST_FOREACH(Vector3i face, faces)
			{
				double b1, b2;
				Math::baryTri(verts[face[0]],verts[face[1]],verts[face[2]],p,b1,b2);

				if (b1>=0 and b2>=0 and b1+b2<=1)
				{
					cout << facesymbol;
					wasinface = true;
					break;
				}
				else
					facesymbol++;
			}

			if (not wasinface) cout << "_";

		}
		cout << "\n";
	}
}
