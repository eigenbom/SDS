#include "meshtools.h"

#include "vector3.h"
#include "vmath.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cfloat>

#include <boost/regex.hpp>
#include <boost/foreach.hpp>

using std::ifstream;
using std::string;

MeshTools::ParseException::ParseException(std::string file_, int line_, std::string error_) throw()
		:file(file_),line(line_),error(error_)
		{
			std::ostringstream oss;
			oss << "Parse Exception in " << file;
			if (line>=0) oss << ":" << line;
			oss << " " << error;
			formatted = oss.str();
		}

const char* MeshTools::ParseException::what() const throw()
{
	return formatted.c_str();
}


Mesh* MeshTools::Load(std::string prefix, PhysicalPropertiesMode mode)
{
	MeshTools::FileType ftype = DetermineFileType(prefix);
	Mesh* m = NULL;
	switch (ftype)
	{
		case Tetgen: m = MeshTools::LoadTetgenMesh(prefix);	break;
		case DotTet: // (broken!) m = MeshTools::LoadTetMesh(name); break;
		case Unknown:
		default:
			std::cerr << "Mesh type unknown.\n";
			return NULL;
			break;
	}
    if (m==NULL) return m;

	switch (mode)
	{
		case StaticMesh: CalculatePhysicalParametersStatic(m); break;

		case CellMesh:
		default:
			std::cerr << "PhysicalPropertiesMode not yet implemented!\n";
			delete m;
			return NULL;
	}

	return m;
}

MeshTools::FileType MeshTools::DetermineFileType(std::string name)
{
	// Check the string for obvious signs of file type
	// Depending on format, check the start of the files contents
	// (if it doesn't exist return error)

	static const boost::regex dottet(".*\\.tet");
	static const boost::regex tetgen(".*\\.1");

	if (boost::regex_match(name,dottet))
	{
		return DotTet;
	}
	else if (boost::regex_match(name,tetgen))
	{
		return Tetgen;
	}
	else
		return Unknown;
}

/** MeshTools::LoadTetgenMesh (name)
 * Loads a tetrahedral mesh (in 3-space) stored in the following (Tetgen format) files:
 * name.node (list of vertices)
 * name.ele (tetrahedra)
 * name.neigh (tetrahedra neighbours)
 *
 * updated: now removed .face dependency, and list of faces in Mesh is now invalid, only mOuterFaces stores anything
 * XXX: this change might break all existing simulation data
 */

// helper function (ignores comments)
template <typename T>
ifstream& readTok(ifstream& i, T& t)
{
	while(1)
	{
		while (i.peek()==' ')
			i.get();

		if (i.peek()=='#')
			i.ignore(256,'\n');
		else break;
	}

	i >> t;
	return i;
}

Mesh* MeshTools::LoadTetgenMesh(std::string name)
{
	// open all streams and return if any errors while opening

	ifstream fnode((name + ".node").c_str());
	ifstream ftetra((name + ".ele").c_str());
	ifstream fneigh((name + ".neigh").c_str());

	if (not (fnode and ftetra and fneigh))
	{
		std::cerr << "Could not load mesh file: " << name << ".node/.ele/.neigh" << std::endl;
		return NULL;
	}

	// the files exist, so make a new mesh
	Mesh* mesh = new Mesh;

	// read node file
	int numNodes,numDims,numAtt,numB;
	readTok(fnode,numNodes);
	readTok(fnode,numDims);
	readTok(fnode,numAtt);
	readTok(fnode,numB);

	int numberOfVertices = numNodes;

	// temporarily store the vertex pointers in a RAM ADT
	if (numNodes<=0) throw(ParseException(name,-1,"numNodes < 0"));

	typedef Vertex* PVERT;
	PVERT* vertexArray = new PVERT[numberOfVertices];
	std::map<Vertex*, unsigned int> vertexIdMap;

	// edgeArray[i][j] = true if there is an edge from vertex i to vertex j
	bool* edgeArray = new bool[numberOfVertices*numberOfVertices];
	for(int i=0;i<numberOfVertices*numberOfVertices;i++)
		edgeArray[i] = false;

	int node = 0;
	while (fnode and node<numNodes)
	{
		int index;
		readTok(fnode,index);
		if (index!=node) throw(ParseException(name,-1,"index!=node"));

		double x,y,z;
		readTok(fnode,x);
		readTok(fnode,y);
		readTok(fnode,z);

		Vertex* v = new Vertex(x,y,z,0);
		mesh->mVertices.push_back(v);
		vertexArray[node] = v;
		vertexIdMap[v] = node;

		node++;
	}

	mesh->updateAABB();

	// Read in tetrahedra

	int numTetra, numAttribs; // ,numNodes already declared
	readTok(readTok(readTok(ftetra,numTetra),numNodes),numAttribs);
	if (numTetra==0) throw(ParseException(name,-1,"No tetrahedra"));
	if (numNodes!=4) throw(ParseException(name,-1,"File not in 4 node format."));

	int numberOfTetras = numTetra;
	Tetra** tetrahedra = new Tetra*[numberOfTetras];

	int tetra = 0;
	while (ftetra and tetra<numTetra)
	{
		int index;
		readTok(ftetra,index);
		if (index!=tetra) throw(ParseException(name,-1,"tetrahedra indexing error"));

		int v[4];
		readTok(readTok(readTok(readTok(ftetra,v[0]),v[1]),v[2]),v[3]);

		int &v1 = v[0], &v2 = v[1], &v3 = v[2], &v4 = v[3];

		if (not (v1>=0 && v2>=0 && v3>=0 && v4>=0)) throw(ParseException(name,-1, "tetrahedra vertex indices < 0"));

		if (not (v1<numberOfVertices &&
				v2<numberOfVertices &&
				v3<numberOfVertices &&
				v4<numberOfVertices))
		{
			throw(ParseException(name,-1, "tetrahedra vertex indices >= num vertices"));
		}

		Tetra* pTetra = new Tetra(vertexArray[v1],vertexArray[v2],vertexArray[v3],vertexArray[v4]);
		mesh->mTetras.push_back(pTetra);
		tetrahedra[index] = pTetra;

		for(int i=0;i<numAttribs;++i) // ignore attributes for the moment
		{
			double att;
			readTok(ftetra,att);
		}

		// add the edges
		for(int i=0;i<4;i++)
		{
			for(int j=i+1;j<4;j++)
			{
				int vi = v[i], vj = v[j];

				// avoid duplicate edges being added
				if (not (edgeArray[vi*numberOfVertices + vj] or edgeArray[vj*numberOfVertices + vi]))
				{
					edgeArray[vi*numberOfVertices + vj] = true;
					mesh->mEdges.push_back(new Edge(vertexArray[vi],vertexArray[vj]));

					vertexArray[vi]->addNeighbour(vertexArray[vj]);
					vertexArray[vj]->addNeighbour(vertexArray[vi]);

					//vertexArray[vi]->mNeighbours.push_back(vertexArray[vj]);
					//vertexArray[vj]->mNeighbours.push_back(vertexArray[vi]);
				}
			}
		}

		tetra++;
	}

	//read neighbourhood data
	int numNeighbours, numEntries;
	readTok(readTok(fneigh,numNeighbours),numEntries);

	if (not numNeighbours>0) throw(ParseException(name,-1,"num neighbours <= 0"));
	if (numEntries!=4) throw(ParseException(name,-1,"neighbour file not in 4 node format"));

	int neigh = 0;
	while (fneigh and neigh<numNeighbours)
	{
		int index;
		readTok(fneigh,index);
		if (index!=neigh) throw(ParseException(name,-1,"neighbour indexes incorrect"));


		int t1, t2, t3, t4;
		readTok(readTok(readTok(readTok(fneigh,t1),t2),t3),t4);

		if (not (t1>=-1 && t2>=-1 && t3>=-1 && t4>=-1) ) throw(ParseException(name,-1,"neighbour indexes < -1"));
		if (not (t1<numberOfTetras && t2<numberOfTetras && t3<numberOfTetras && t4<numberOfTetras) ) throw(ParseException(name,-1,"neighbour indexes >= numneighbours"));

		Tetra* tn[] = {
			t1>=0?tetrahedra[t1]:NULL,
			t2>=0?tetrahedra[t2]:NULL,
			t3>=0?tetrahedra[t3]:NULL,
			t4>=0?tetrahedra[t4]:NULL};

		tetrahedra[index]->setNeighbours(tn[0],tn[1],tn[2],tn[3]);

		// set up vertex surface data
		// for visualisation purposes we distinguish surface vertices from non-surface vertices
		// each surface vertex also maintains a list of all neighbouring surface vertices
		// this is used to approximate the surface normal to create a smooth surface

		// foreach NULL face in tn mark the adjoined vertices as "surface" vertices ...
		// foreach NULL face add it to the list of outer faces ...

		for(int i=0;i<4;i++)
		{
			if (tn[i]==NULL)
			{


				unsigned int vid[] = {
						vertexIdMap[&tetrahedra[index]->v(0)],
						vertexIdMap[&tetrahedra[index]->v(1)],
						vertexIdMap[&tetrahedra[index]->v(2)],
						vertexIdMap[&tetrahedra[index]->v(3)]};
					/*
					vertexIdMap[tetrahedra[index]->mV[0]],
					vertexIdMap[tetrahedra[index]->mV[1]],
					vertexIdMap[tetrahedra[index]->mV[2]],
					vertexIdMap[tetrahedra[index]->mV[3]]};
					*/

				// create the appropriate face (facing outwards from the tetrahedra)
				// note that this face is unique to this tetra
				// so duplicates won't be created
				unsigned int v1;
				unsigned int v2;
				unsigned int v3;
				switch(i)
				{
					case 3: v1 = vid[0]; v2 = vid[2]; v3 = vid[1]; break; //0,2,1
					case 2: v1 = vid[0]; v2 = vid[1]; v3 = vid[3]; break; //0,1,3
					case 1: v1 = vid[3]; v2 = vid[2]; v3 = vid[0]; break; //3,2,0
					case 0: v1 = vid[1]; v2 = vid[2]; v3 = vid[3]; break; //1,2,3
					default: v1 = v2 = v3 = 0; break;
				}

				if (v1==0 and v2==0 and v3==0)
				{
					throw(ParseException(name,-1,"error in file, something to do with vertex and face indices?"));
				}
				else
				{
					Face* f = new Face(vertexArray[v1],vertexArray[v2],vertexArray[v3]);
					mesh->addOuterFace(f);
					mesh->addFaceNeighbourLinks(f);
					vertexArray[v1]->setSurface(true);
					vertexArray[v2]->setSurface(true);
					vertexArray[v3]->setSurface(true);

					/*
					vertexArray[v1]->addFaceNeighbour(f);
					vertexArray[v2]->addFaceNeighbour(f);
					vertexArray[v3]->addFaceNeighbour(f);

					//vertexArray[v1]->mFaceNeighbours.push_back(f);
					//vertexArray[v2]->mFaceNeighbours.push_back(f);
					//vertexArray[v3]->mFaceNeighbours.push_back(f);
					f->setOuter(true);
					mesh->mOuterFaces.push_back(f);

					// label verts as outer..
					// then face i is a boundary face
					vertexArray[v1]->setSurface(true);
					vertexArray[v2]->setSurface(true);
					vertexArray[v3]->setSurface(true);

					// XXX: kept for backwards compatibility
					mesh->mFaces.push_back(f);
					*/
				}
			}
		}

		neigh++;
	}

	// foreach surface vertex, construct a list of its neighbours...
	for(int i=0;i<numberOfVertices;++i)
	{
		Vertex* v = vertexArray[i];
		if (v->surface())
		{
			// build list of neighbours
			//std::list<Face*>& nb = v->surfaceFaces();
			//std::list<Face*>& nb = v->mFaceNeighbours;

			// also maintain a list of attached OUTER faces...
			BOOST_FOREACH(Face* f, mesh->outerFaces())
			{
				if (f->contains(v))
					v->addFaceNeighbour(f);
					//nb.push_back(f);
			}
		}
	}

	delete[]vertexArray;
	delete[]tetrahedra;
	delete[]edgeArray;

	return mesh;
}

void MeshTools::CalculatePhysicalParametersStatic(Mesh* m)
{
	// calculate rest lengths of all edges and tetrahedra, also calculate mesh volume and mass
	// distribute mass from tetrahedra to vertices

	for (std::list<Vertex*>::iterator it = m->mVertices.begin(); it!=m->mVertices.end(); it++)
		(*it)->setM(0);

	for (std::list<Edge*>::iterator it = m->mEdges.begin(); it!=m->mEdges.end(); it++)
	{
		Edge& e = **it;
		e.setRest((e.v(0)->x() - e.v(1)->x()).length());
	}

	for (std::list<Tetra*>::iterator it = m->mTetras.begin(); it!=m->mTetras.end(); it++)
	{
		Tetra& t = **it;
		Vertex &a = t.v(0), &b = t.v(1), &c = t.v(2), &d = t.v(3);
		t.setRest(1/6. * dot(b.x()-a.x(),cross(c.x()-a.x(),d.x()-a.x())));
		double r = t.rest();

		// distribute mass to vertices
		a.addM(r/4.0);
		b.addM(r/4.0);
		c.addM(r/4.0);
		d.addM(r/4.0);
	}

	/* XXX: I think it is better to evenly distribute the masses
	Vector3d centroid = (vs[0]->x()+vs[1]->x()+vs[2]->x()+vs[3]->x()) / 4.0;
	double totalL = 0;
	for(int i=0;i<4;i++)
		totalL += (vs[i]->x() - centroid).length();
	double massRatio = pTetra->rest() / totalL;
	mesh->mVolume += pTetra->rest();
	for(int i=0;i<4;i++)
		vs[i]->addM((vs[i]->x() - centroid).length() * massRatio);
	*/
}

enum Segment{VERTICES,TRIANGLES,TETRAS,MATERIALS,HEADER,FIRST_LINE,UNKNOWN};

Segment toSegment(std::string var)
{
	if (var=="VERTICES") return VERTICES;
	else if (var=="MATERIALS") return MATERIALS;
	else if (var=="TETRAS") return TETRAS;
	else if (var=="TRIANGLES") return TRIANGLES;
	else return UNKNOWN;
}

// LoadTetMesh: load a mesh with a .tet extension
// XXX: BROKEN, need to add outerFace functionality
Mesh* MeshTools::LoadTetMesh(std::string name)
{
	return NULL;

	/*
	// file format as follows
	//
	// [first line]
	// "tet version <double>"
	//
	// [header]
	// "num_materials <int>"
	// "num_vertices <int>"
	// "num_tetras <int>"
	// "num_triangles <int>"

	// [section]
	// "MATERIALS/VERTICES/TETRAS/TRIANGLES"

	// [VERTICES]
	// <double> (*4) (x y z ?)

	// [MATERIALS]
	// <string> (material name) // XXX: Ignore data

	// [TETRAS]
	// <int> (*5) (v1,v2,v3,v4,?)

	// [TRIANGLES]
	// XXX: Ignore data

	// parser constants, types
	// char* var_data_strings[] = {"num_materials","num_vertices","num_tetras","num_triangles"};
	// char* section_header_strings[] = {"MATERIALS","VERTICES","TETRAS","TRIANGLES"};
	Segment curSegment = FIRST_LINE;

	// start parsing...

	ifstream f(name.c_str());
	if (not f) return NULL;

	// file is opened so make a new mesh
	Mesh* mesh = new Mesh;

	// state information for the parser
	int num_materials = 0, num_vertices = 0, num_tetras = 0, num_triangles = 0;
	int currentItem = 0;

	// maintain an array of vertices for the tetras to point to
	typedef Vertex* PVERT;
	PVERT* vertexArray = NULL;
	bool* edgeArray = NULL;

	// AABB info...
	double lx = DBL_MAX, mx = -DBL_MAX;
	double ly = DBL_MAX, my = -DBL_MAX;
	double lz = DBL_MAX, mz = -DBL_MAX;

	// stores a tag at edgeArray[v1*num_verts+v2] indicating whether we have created the edge (v1,v2) or not...
	// symmetric, i.e., ea[(v1,v2)]==ea[(v2,v1)]

	// parse the file, line by line, updating state if necessary
	while (f and curSegment!=UNKNOWN)
	{
		switch (curSegment)
		{
			case FIRST_LINE:
				{
					// std::cerr << "FIRST_LINE\n";
					string stet, sversion; double version;
					f >> stet >> sversion >> version;
					if (stet=="tet" and sversion=="version" and version >= 0.999)
						curSegment = HEADER;
					else
						curSegment = UNKNOWN;
					break;
				}
			case HEADER:
				{
					// std::cerr << "HEADER\n";
					string var;
					f >> var;
					if (var=="num_materials")
						f >> num_materials;
					else if (var=="num_vertices")
					{
						f >> num_vertices;

						if (vertexArray)
							delete[]vertexArray;
						if (edgeArray)
							delete[]edgeArray;

						vertexArray = new PVERT[num_vertices];
						edgeArray = new bool[num_vertices*num_vertices];

						std::fill(edgeArray,edgeArray+num_vertices*num_vertices,false);
					}
					else if (var=="num_tetras")
						f >> num_tetras;
					else if (var=="num_triangles")
						f >> num_triangles;
					else
					{
						curSegment = toSegment(var);
						string junk;
						getline(f,junk);
					}
					break;
				}
			case VERTICES:
				{
					// std::cerr << "V";
					if (currentItem > num_vertices)
					{
						curSegment = UNKNOWN;
					}
					else if (currentItem==num_vertices)
					{
						// we must read in a new segment header...
						string var;
						f >> var;
						curSegment = toSegment(var);
						currentItem = 0;

						string junk;
						getline(f,junk);
					}
					else
					{
						double x, y, z, junk;
						f >> x >> y >> z >> junk;
						vertexArray[currentItem] = new Vertex(x,y,z,0); // create with zero mass...
						mesh->mVertices.push_back(vertexArray[currentItem]);
						currentItem++;

						// update AABB
						if (x < lx) lx = x;
						if (x > mx) mx = x;

						if (y < ly) ly = y;
						if (y > my) my = y;

						if (z < lz) lz = z;
						if (z > mz) mz = z;
					}
					break;
				}
			case MATERIALS:
				{
					// std::cerr << "MATERIALS\n";
					// we ignore this info, we are just after the structure information
					if (currentItem > num_materials)
					{
						curSegment = UNKNOWN;
					}
					else if (currentItem == num_materials)
					{
						// we must read in a new segment header...
						string var;
						f >> var;
						curSegment = toSegment(var);
						currentItem = 0;

						// std::cerr << "v" << var << "\n";

						string junk;
						getline(f,junk);
					}
					else
					{
						string junk;
						getline(f,junk);
						currentItem++;
					}
					break;
				}
			case TETRAS:
				{
					// std::cerr << "T";

					// XXX: What order are the tetrahedra vertices arranged in?
					// PRE: we have read in the vertex data

					if (currentItem > num_tetras or vertexArray == NULL)
					{
						curSegment = UNKNOWN;
					}
					else if (currentItem == num_tetras)
					{
						string var;
						f >> var;
						curSegment = toSegment(var);
						currentItem = 0;

						string junk;
						getline(f,junk);
					}
					else
					{
						// XXX: mapping from .tet vert list, to my vert list is
						// 0->0, 1->1, 2->3, 3->2
						int vi[4]; double junk;
						f >> vi[0] >> vi[1] >> vi[3] >> vi[2] >> junk;
						PVERT v[4] = {vertexArray[vi[0]], vertexArray[vi[1]], vertexArray[vi[2]], vertexArray[vi[3]]};
						Tetra* t = new Tetra(v[0],v[1],v[2],v[3]);
						mesh->mTetras.push_back(t);

						// add the six edges to mesh
						// to ensure no duplicate edges, we tag each (v1,v2),(v2,v1) pair in an array
						bool* ea = edgeArray;
						int m = num_vertices;

						for(int i=0;i<4;i++)
						{
							for(int j=i+1;j<4;j++)
							{
								int v1 = vi[i], v2 = vi[j];
								if (not ea[v1*m+v2])
								{
									Edge* e = new Edge(v[i],v[j]);
									mesh->mEdges.push_back(e);
									ea[v1*m+v2] = ea[v1+v2*m] = true;

									v[i]->mNeighbours.push_back(v[j]);
									v[j]->mNeighbours.push_back(v[i]);
								}
							}
						}

						// distribute the mass to each vertex
						Vector3d centroid = (v[0]->x()+v[1]->x()+v[2]->x()+v[3]->x()) / 4.0;
						double totalL = 0;
						for(int i=0;i<4;i++)
						{
							totalL += (v[i]->x() - centroid).length();
						}

						double massRatio = std::abs(t->rest()) / totalL;


						for(int i=0;i<4;i++)
							v[i]->addM((v[i]->x() - centroid).length() * massRatio);

						// increment item no. and move on to the next line
						currentItem++;
					}
					break;
				}
			case TRIANGLES:
				{
					// std::cerr << "t";
					// XXX: Ignore this data, maybe include it in the future?

					// Currently just churn through the lines until we reach EOL or another section header
					string var;
					getline(f,var);
					if (toSegment(var)!=UNKNOWN)
						curSegment = toSegment(var);
					else
						currentItem++;
					break;
				}
			default:
				curSegment = UNKNOWN;
		}
	}

	// std::cerr << "fin\n";
	// clean up and exit

	if (vertexArray)
		delete[]vertexArray;
	if (edgeArray)
		delete[]edgeArray;

	if (f and (curSegment == UNKNOWN))
	{
		delete mesh;
		return NULL;
	}
	else
	{
		if (mesh)
		{
			mesh->mAABB[0] = lx;
			mesh->mAABB[1] = ly;
			mesh->mAABB[2] = lz;
			mesh->mAABB[3] = mx;
			mesh->mAABB[4] = my;
			mesh->mAABB[5] = mz;
			mesh->mAABB.valid(true);
		}
		return mesh;
	}
	*/
}


