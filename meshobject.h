#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include "GEL/HMesh/Manifold.h"

class MeshObject
{
    HMesh::Manifold manifold_origin;
    HMesh::Manifold manifold_core;

    // Scaling
    float projectorSize;


    void updateViewParam();

    int currentTriIndex = 0;
public:
    MeshObject();

    void draw();

    void drawTriangle();

    void reduceMesh(float ratio);
};

#endif // MESHOBJECT_H
