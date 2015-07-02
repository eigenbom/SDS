/*
 * annotation.h
 *
 * Annotations are visual information added to the organism viewer.
 *
 *  Created on: 23/09/2009
 *      Author: ben
 */

#ifndef ANNOTATION_H_
#define ANNOTATION_H_

#include <string>

#include <boost/any.hpp>
#include <boost/tuple/tuple.hpp>

#include "vector3.h"

#include "vertex.h"
#include "edge.h"
#include "face.h"

#include "cell.h"

class Annotation
{
public:
	Annotation(boost::any d):mData(d){}
	virtual ~Annotation(){}
	virtual void draw() = 0;

protected:
	virtual void setHighlightColour();

protected:
	boost::any mData;
};

template <typename T>
class AnnotationOfType: public Annotation
{
public:
	AnnotationOfType(T& d):Annotation(boost::any(d)){}
	virtual T data(){return boost::any_cast<T>(mData);}
};

class VertexAnnotation: public AnnotationOfType<Vertex*>
{
public:
	VertexAnnotation(Vertex* v):AnnotationOfType<Vertex*>(v){}
	virtual void draw();
};

class EdgeAnnotation: public AnnotationOfType<Edge*>
{
public:
	EdgeAnnotation(Edge* e):AnnotationOfType<Edge*>(e){}
	virtual void draw();
};

/**
 * FaceAnnotations are triangles: either Face* or a 3tuple of positions.
 */
class FaceAnnotation: public Annotation
{
public:
	FaceAnnotation(Face* f):Annotation(boost::any(f)),mMode(0){}
	FaceAnnotation(boost::tuple<Vector3d,Vector3d,Vector3d> t):Annotation(boost::any(t)),mMode(1){}
	FaceAnnotation(boost::tuple<Vertex*,Vertex*,Vertex*> t):Annotation(boost::any(t)),mMode(2){}
	virtual void draw();


private:
	int mMode; // mode of which format the face is stored in, 0 Face*, 1 Vector3d*3, 2 (Vertex*)*3
};

class CellAnnotation: public AnnotationOfType<Cell*>
{
public:
	CellAnnotation(Cell* c):AnnotationOfType<Cell*>(c){}
	virtual void draw();
};

class TetraAnnotation: public Annotation
{
public:
	TetraAnnotation(Tetra* t):Annotation(boost::any(t)),mMode(0){}
	virtual void draw();

private:
	int mMode;
};

#endif /* ANNOTATION_H_ */
