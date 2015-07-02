/*
 * glutils.h
 *
 *  Created on: 23/09/2009
 *      Author: ben
 */

#ifndef GLUTILS_H_
#define GLUTILS_H_

#include "aabb.h"
#include "tetra.h"
#include "vector3.h"

// common drawing operations
void drawAABB(const AABB& aabb);
/// draws a tetrahedra, possibly scaled around its center by some amount
void drawTetra(const Tetra* t, double scale = 1);
void drawTri(const Vector3d& a, const Vector3d& b, const Vector3d& c, bool outline);
// drawTriangle inside a GL_TRIANGLES draw state
void drawTriInline(const Vector3d& a, const Vector3d& b, const Vector3d& c, bool withNormal = true);
void drawSphere();
/// drawVector(vector, base colour, tip colour)
void drawVector(const Vector3d& vector, Vector3d baseColour = Vector3d::ZERO, Vector3d tipColour = Vector3d(1,1,1));

#endif /* GLUTILS_H_ */
