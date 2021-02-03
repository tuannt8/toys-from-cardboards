#include "meshobject.h"

#include "glhelper.h"
#include "GEL/HMesh/obj_load.h"

MeshObject::MeshObject()
{
    HMesh::obj_load("arrow.obj", manifold_core);
}

void MeshObject::draw()
{
    floatVector points, normals;
    for(auto f : manifold_core.faces())
    {
        auto norm = HMesh::normal(manifold_core, f);
        for(auto hew = manifold_core.walker(f);
            !hew.full_circle(); hew = hew.circulate_face_ccw())
        {
            auto p = manifold_core.pos(hew.vertex());

            for(int i = 0; i < 3; i++)
            {
                points.push_back(p[i]);
                normals.push_back(norm[i]);
            }
        }
    }

    GL()->drawGourand(points, normals, GL_TRIANGLES);
}

void MeshObject::drawTriangle()
{
    floatVector points, colors;
    points = {
        0,0,0,
        1,0,0,
        1,0,0,
        0,1,0,
        0,1,0,
        0,0,0
    };

   colors = {
    1,1,1,
    1,1,1,
    1,1,1,
    1,1,1,
    1,1,1,
    1,1,1
   };

   QMatrix4x4 transform;
   glLineWidth(2);
   GL()->drawSimple(points, colors, transform, GL_LINES);
}
