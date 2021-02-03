#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include "GEL/HMesh/Manifold.h"

class MeshObject
{
    HMesh::Manifold manifold_core;

    // Scaling
    float projectorWidth;
    float projectorHeight;
public:
    MeshObject();

    void draw();

    void drawTriangle();
};

#endif // MESHOBJECT_H
