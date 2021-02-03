#include "meshobject.h"

#include "glhelper.h"
#include "GEL/CGLA/Vec3d.h"
#include "GEL/HMesh/obj_load.h"
#include "GEL/HMesh/quadric_simplify.h"
#include "GEL/HMesh/triangulate.h"

typedef CGLA::Vec3d vec3gel ;

using namespace std;

vec3gel scale(1,1,1);
vec3gel center_bound;

void MeshObject::updateViewParam()
{

    vec3gel bound_leftdown(INFINITY), bound_rightup(-INFINITY);
    for(auto v : manifold_core.vertices())
    {
        auto p = manifold_core.pos(v);
        for(int i = 0; i < 3; i++)
        {
            bound_leftdown[i] = std::min(bound_leftdown[i], p[i]);
            bound_rightup[i] = std::max(bound_rightup[i], p[i]);
        }
    }

    vec3gel bound_size = bound_rightup - bound_leftdown;
    center_bound = (bound_rightup + bound_leftdown) * 0.5;
//    scale = vec3gel(1./ bound_size[0], 1./bound_size[1], 1./bound_size[2]);
    scale = vec3gel(1);

    GL()->view_distance() = 2 * std::max(bound_size[0],
            std::max(bound_size[1], bound_size[2]));

    qInfo() << "Num triangle: " << manifold_core.no_faces();
}

MeshObject::MeshObject()
{
    HMesh::obj_load("cat.obj", manifold_core);



    HMesh::triangulate(manifold_core);
    manifold_origin = manifold_core;

    updateViewParam();
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

    QMatrix4x4 transform;
    transform.translate(-center_bound[0], -center_bound[1], -center_bound[2]);
    transform.scale(scale[0], scale[1], scale[2]);

    GL()->drawGourand(points, normals, GL_TRIANGLES, &transform);
}

void MeshObject::drawTriangle()
{
    vector<vec3gel> vPos;
    for(auto hew = manifold_core.walker(HMesh::FaceID(currentTriIndex));
        !hew.full_circle(); hew.circulate_face_cw())
    {

    }

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

void MeshObject::reduceMesh(float ratio)
{
    manifold_core = manifold_origin;
    HMesh::quadric_simplify(manifold_core, ratio);

    updateViewParam();
}
