#include "meshobject.h"

#include "glhelper.h"
#include "GEL/CGLA/Vec3d.h"
#include "GEL/HMesh/obj_load.h"
#include "GEL/HMesh/quadric_simplify.h"
#include "GEL/HMesh/triangulate.h"

extern int mainAxis;

#define add3(vv, p) \
    vv.push_back(p[0]); \
    vv.push_back(p[1]); \
    vv.push_back(p[2]);


using namespace std;



MeshObject & meshInstance(){return MeshObject::get_instance();}

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

    bound_size = bound_rightup - bound_leftdown;
    center_bound = (bound_rightup + bound_leftdown) * 0.5;
    scale_bound = vec3gel(1);

    GL()->view_distance() = 2 * std::max(bound_size[0],
            std::max(bound_size[1], bound_size[2]));

    qInfo() << "Num triangle: " << manifold_core.no_faces();
}

MeshObject::MeshObject(std::string path)
{
    HMesh::obj_load(path, manifold_core);

    HMesh::triangulate(manifold_core);
    manifold_origin = manifold_core;

    updateViewParam();

//    buildCutOrderDoubleTri();
//    computeScaleFactor();
}

MeshObject &MeshObject::get_instance()
{
    static MeshObject instance("cat.obj");
    return instance;
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
    transform.scale(scale_bound[0], scale_bound[1], scale_bound[2]);

    GL()->drawGourand(points, normals, GL_TRIANGLES, &transform);
}

void MeshObject::drawTriangle()
{
    auto pos2 = get_flated_tri_pos();


    vector<vector<int>> lines = {{0,1},{1,2}, {2,0}, {0,3}, {3,1}};
    vec3gel c(1,1,1);
    floatVector points, colors;
    for(auto l : lines)
    {
        for(auto pIdx : l)
        {
            auto p = pos2[pIdx];
            points.push_back(p[0]);
            points.push_back(p[1]);
            points.push_back(0);
            add3(colors, c);
        }
    }

   QMatrix4x4 transform;
   glLineWidth(2);
   GL()->drawSimple(points, colors, transform, GL_LINES);
}

void MeshObject::drawTriangleSingle()
{
    auto pos2 = get_flated_tri_pos_single();

    vector<vector<int>> lines = {{0,1},{1,2}, {2,0}};
    vec3gel c(1,1,1);
    floatVector points, colors;
    for(auto l : lines)
    {
        for(auto pIdx : l)
        {
            auto p = pos2[pIdx];
            points.push_back(p[0]);
            points.push_back(p[1]);
            points.push_back(0);
            add3(colors, c);
        }
    }

   QMatrix4x4 transform;
   glLineWidth(2);
   GL()->drawSimple(points, colors, transform, GL_LINES);
}

void MeshObject::drawBased()
{
    static QVector<QVector3D> axis = {{1,0,0}, {0,1,0}, {0,0,1}};

    auto activeAxis = axis[mainAxis];

    QMatrix4x4 transform;

    vec3gel translate(0);
    translate[mainAxis] -= bound_size[mainAxis]/2;
    transform.translate(translate[0], translate[1], translate[2]);

    vec3gel scale = bound_size;
    scale[mainAxis] = 1;
    transform.scale(scale[0], scale[1], scale[2]);

    GL()->drawWiredCube(transform, activeAxis);
}

void MeshObject::draw_ordered_cutEdges()
{
    floatVector points, colors;
    float colorIter = 0;
    for(auto eid : ordered_cutEdge)
    {
        vec3gel c(1,0,0);//(colorIter++ / ordered_cutEdge.size());

        auto hew = manifold_core.walker(eid);
        auto p0 = manifold_core.pos(hew.vertex());
        auto p1 = manifold_core.pos(hew.opp().vertex());

        add3(points, p0);
        add3(points, p1);
        add3(colors, c);
        add3(colors, c);
    }

    QMatrix4x4 transform;
    transform.translate(-center_bound[0], -center_bound[1], -center_bound[2]);
    transform.scale(scale_bound[0], scale_bound[1], scale_bound[2]);
    GL()->drawSimple(points, colors, transform, GL_LINES);
}

void MeshObject::drawActiveEdge()
{
    floatVector points, normals, apoints, acolors;

    auto hew = manifold_core.walker(currentEdge());
    vector<HMesh::FaceID> neighbor = {hew.face(), hew.opp().face()};

    vec3gel ca[2] = {vec3gel(0,1,0), vec3gel(0,0,1)};
    vec3gel cp(0.3, 0.3, 0.3);

    for(int i = 0; i <= currentEdgeIdx; i++)
    {
        auto hew = manifold_core.walker(ordered_cutEdge[i]);
        vector<HMesh::FaceID> neighbor = {hew.face(), hew.opp().face()};

        auto c = i==currentEdgeIdx? ca[1] : cp;

        for(auto f : neighbor)
        {
            auto norm = HMesh::normal(manifold_core, f);
            for(auto hew = manifold_core.walker(f);
                !hew.full_circle(); hew = hew.circulate_face_ccw())
            {
                auto p = manifold_core.pos(hew.vertex());

                for(int j = 0; j < 3; j++)
                {
                    if(i == currentEdgeIdx)
                    {
                        apoints.push_back(p[j]);
                        acolors.push_back(c[j]);
                    }
                    else
                    {
                        points.push_back(p[j]);
                        normals.push_back(norm[j]);
                    }
                }
            }
        }
    }



    QMatrix4x4 transform;
    transform.translate(-center_bound[0], -center_bound[1], -center_bound[2]);
    transform.scale(scale_bound[0], scale_bound[1], scale_bound[2]);

    GL()->drawGourand(points, normals, GL_TRIANGLES, &transform);
    GL()->drawSimple(apoints, acolors, transform, GL_TRIANGLES);
}

void MeshObject::draw_ordered_cutFaces()
{
    if(currentFaceIdx < 0)
        return;

    floatVector points, normals, apoints, acolors;

//    auto hew = manifold_core.walker(currentFace());
//    vector<HMesh::FaceID> neighbor = {hew.face(), hew.opp().face()};

    vec3gel c(0,1,0);

    for(int i = 0; i <= currentFaceIdx; i++)
    {
        auto f = ordered_cutFaces[i];

        auto norm = HMesh::normal(manifold_core, f);
        for(auto hew = manifold_core.walker(f);
            !hew.full_circle(); hew = hew.circulate_face_ccw())
        {
            auto p = manifold_core.pos(hew.vertex());

            for(int j = 0; j < 3; j++)
            {
                if(i == currentFaceIdx)
                {
                    apoints.push_back(p[j]);
                    acolors.push_back(c[j]);
                }
                else
                {
                    points.push_back(p[j]);
                    normals.push_back(norm[j]);
                }
            }
        }
    }



    QMatrix4x4 transform;
    transform.translate(-center_bound[0], -center_bound[1], -center_bound[2]);
    transform.scale(scale_bound[0], scale_bound[1], scale_bound[2]);

    GL()->drawGourand(points, normals, GL_TRIANGLES, &transform);
    GL()->drawSimple(apoints, acolors, transform, GL_TRIANGLES);
}

void MeshObject::reduceMesh(float ratio)
{
    manifold_core = manifold_origin;
    HMesh::quadric_simplify(manifold_core, ratio);

    manifold_core.cleanup();

    updateViewParam();
}

void MeshObject::buildCutOrderTri()
{
    // Find seed faces
    float zMin = INFINITY;
    HMesh::FaceID lowestFace;
    for(auto fid : manifold_core.faces())
    {
        auto p = HMesh::centre(manifold_core, fid);

        if(p[mainAxis] < zMin)
        {
            zMin = p[mainAxis];
            lowestFace = fid;
        }
    }
    // TODO

}

void MeshObject::buildCutOrderDoubleTri()
{


    // Cut two triangles at a time
    // Cut from bottom to top
    vector<HMesh::HalfEdgeID> potentialList;
    std::vector<bool> halfEdge_used(manifold_core.no_halfedges(), false);
    vector<HMesh::HalfEdgeID> ordered_cutEdge_all;

    // Find seed edge
    float zMin = INFINITY;
    HMesh::HalfEdgeID lowestEdge = HMesh::InvalidHalfEdgeID;
    for(auto e : manifold_core.halfedges())
    {
        auto hew = manifold_core.walker(e);
        auto p = manifold_core.pos(hew.vertex());

        if(p[mainAxis] < zMin)
        {
            zMin = p[mainAxis];
            lowestEdge = e;
        }
    }

    assert(lowestEdge != HMesh::InvalidHalfEdgeID);

    // Start finding
    auto hew = manifold_core.walker(lowestEdge);
    ordered_cutEdge_all.push_back(lowestEdge);
    halfEdge_used[lowestEdge.get_index()] = true;
    halfEdge_used[hew.opp().halfedge().get_index()] = true;

    while (1) {
        auto hew = manifold_core.walker(ordered_cutEdge_all.back());

        // add potential edges
        auto neighbor1 = edge_incident_to_vertex(hew.vertex());
        auto neighbor2 = edge_incident_to_vertex(hew.opp().vertex());
        neighbor1.insert(neighbor1.end(), neighbor2.begin(), neighbor2.end());

        for(auto & hh : neighbor1)
        {
            if(!halfEdge_used[hh.get_index()])
            {
                // Only add if not in potentialList
                bool exist = false;
                for(auto & hepoid : potentialList)
                {
                    auto hepo = manifold_core.walker(hepoid);
                    if(hepo.halfedge().get_index() == hh.get_index()
                            || hepo.opp().halfedge().get_index() == hh.get_index())
                    {
                        exist = true;
                        break;
                    }
                }
                if(!exist)
                    potentialList.push_back(hh);
            }
        }

        // Find the lowest
        float zMin = INFINITY;
        int minIdx = -1;
        for(int i = 0; i < potentialList.size(); i++)
        {
            auto hewp = manifold_core.walker(potentialList[i]);

            auto midPoint = (
                    manifold_core.pos(hewp.vertex())
                    + manifold_core.pos(hewp.opp().vertex())
                        ) / 2;
            if(midPoint[mainAxis] < zMin)
            {
                zMin = midPoint[mainAxis];
                minIdx = i;
            }
        }

        if(minIdx == -1)
            break;
        else
        {
            auto newHew = manifold_core.walker( potentialList[minIdx] );
            potentialList.erase(potentialList.begin() + minIdx);

            ordered_cutEdge_all.push_back(newHew.halfedge());
            halfEdge_used[newHew.halfedge().get_index()] = true;
            halfEdge_used[newHew.opp().halfedge().get_index()] = true;
        }
    }

    qInfo() << "Ordered all edges: " << ordered_cutEdge_all.size();
    qInfo() << "all he: " << manifold_core.no_halfedges();

    // Edges share tri are dupplicated
    halfEdge_used = std::vector<bool>(manifold_core.no_halfedges(), false);
    for(auto eid : ordered_cutEdge_all)
    {
        if(!halfEdge_used[eid.get_index()])
        {
            HMesh::HalfEdgeID longestEdge;
            float length = 0;
            // find longest edge
            for(auto hew = manifold_core.walker(eid); !hew.full_circle();
                hew = hew.circulate_face_ccw())
            {
                if(halfEdge_used[hew.halfedge().get_index()])
                    continue;

                auto l = HMesh::length(manifold_core, hew.halfedge());
                if(l > length)
                {
                    length = l;
                    longestEdge = hew.halfedge();
                }
            }

            Q_ASSERT(length != 0);

            ordered_cutEdge.push_back(longestEdge);

            for(auto hew = manifold_core.walker(longestEdge); !hew.full_circle();
                hew = hew.circulate_face_ccw())
            {
                if(hew.face() == HMesh::InvalidFaceID)
                    break;

                halfEdge_used[hew.halfedge().get_index()] = true;
                halfEdge_used[hew.opp().halfedge().get_index()] = true;
            }
            for(auto hew =manifold_core.walker( manifold_core.walker(longestEdge).opp().halfedge());
                !hew.full_circle();
                hew = hew.circulate_face_ccw())
            {
                if(hew.face() == HMesh::InvalidFaceID)
                    break;

                halfEdge_used[hew.halfedge().get_index()] = true;
                halfEdge_used[hew.opp().halfedge().get_index()] = true;
            }
        }
    }

    qInfo() << ordered_cutEdge.size() << " edges have been sorted";
    qInfo() << manifold_core.no_faces() << " num tris";

    // Make sure no triangle is bigger than display
    computeScaleFactor();
    validateTriangleSize();


}

#define updateUsedEdge(fid) \
{ \
    faces_used[fid.get_index()] = true;\
    for(auto hew2 = manifold_core.walker(fid); !hew2.full_circle(); \
        hew2 = hew2.circulate_face_cw()) \
    { \
        halfEdge_used[hew2.halfedge().get_index()] = true; \
        halfEdge_used[hew2.opp().halfedge().get_index()] = true; \
    } \
}

#define updateUsedEdgeByEdge(eid) \
{ \
    auto hew1 = manifold_core.walker(eid); \
    updateUsedEdge(hew1.face()); \
    updateUsedEdge(hew1.opp().face()); \
}
void MeshObject::buildCutOrderDoubleTri2()
{
    // Find seed edge
    float zMin = INFINITY;
    HMesh::FaceID lowestFace;
    for(auto fid : manifold_core.faces())
    {
        auto p = HMesh::centre(manifold_core, fid);

        if(p[mainAxis] < zMin)
        {
            zMin = p[mainAxis];
            lowestFace = fid;
        }
    }

    // grow it
    std::vector<bool> halfEdge_used(manifold_core.no_halfedges(), false);
    std::vector<bool> faces_used(manifold_core.no_faces(), false);
    vector<HMesh::HalfEdgeID> ordered_cutEdge_all;
    vector<HMesh::FaceID> potentialList;

    potentialList.push_back(lowestFace);

    while (potentialList.size() > 0) {
        // Find lowest potential in potential list
        float aMin = INFINITY;
        int lowestIdx = -1;
        for(int i = 0; i < potentialList.size(); i++)
        {
            auto center = HMesh::centre(manifold_core, potentialList[i]);
            if(center[mainAxis] < aMin)
            {
                aMin = center[mainAxis];
                lowestIdx = i;
            }
        }

        Q_ASSERT(lowestIdx != -1);

        auto lowestFaceID = potentialList[lowestIdx];
        // Find Longest edge
        float longestD = -INFINITY;
        HMesh::HalfEdgeID longestEdgeID = HMesh::InvalidHalfEdgeID;
        for(auto hew = manifold_core.walker(lowestFaceID);
            !hew.full_circle();
            hew = hew.circulate_face_ccw())
        {
            if(halfEdge_used[hew.halfedge().get_index()])
                continue;

            auto l = HMesh::length(manifold_core, hew.halfedge());
            if(l > longestD)
            {
                longestD = l;
                longestEdgeID = hew.halfedge();
            }
        }

        if(longestEdgeID == HMesh::InvalidHalfEdgeID)
        { // The face covered by used edges
//            ordered_cutFaces.push_back(lowestFaceID);
//            faces_used[lowestFaceID.get_index()] = true;
            potentialList.erase(potentialList.begin() + lowestIdx);
            continue;
        }

        ordered_cutEdge.push_back(longestEdgeID);

        // Update used edges
        updateUsedEdgeByEdge(longestEdgeID);

        auto curhew = manifold_core.walker(longestEdgeID);

        // Update potential list
        potentialList.erase(potentialList.begin() + lowestIdx);
        // Add 4 potential faces
        for(auto hew = manifold_core.walker(curhew.face());
            !hew.full_circle();
            hew = hew.circulate_face_ccw())
        {
            auto otherFid = hew.opp().face();
            if(otherFid != HMesh::InvalidFaceID
               && !faces_used[otherFid.get_index()])
            {
                // check if exist
                bool exist = false;
                for(auto pf : potentialList)
                    if(pf == otherFid)
                        exist = true;

                if(!exist)
                    potentialList.push_back(otherFid);
            }
        }
        for(auto hew = manifold_core.walker(curhew.opp().face());
            !hew.full_circle();
            hew = hew.circulate_face_ccw())
        {
            auto otherFid = hew.opp().face();
            if(otherFid != HMesh::InvalidFaceID
               && !faces_used[otherFid.get_index()])
            {
                // check if exist
                bool exist = false;
                for(auto pf : potentialList)
                    if(pf == otherFid)
                        exist = true;

                if(!exist)
                    potentialList.push_back(otherFid);
            }
        }
    }

    for(int i = 0; i < faces_used.size(); i++)
    {
        if(!faces_used[i])
            ordered_cutFaces.push_back(HMesh::FaceID(i));
    }

    qInfo() << ordered_cutEdge.size() << " edges";
    qInfo() << ordered_cutFaces.size() << " faces";
    qInfo() << manifold_core.no_faces() << " total faces";


    if(ordered_cutFaces.size() > 0)
        currentFaceIdx = 0;
    else
        currentFaceIdx = -1;
    // Make sure no triangle is bigger than display
    computeScaleFactor();
    validateTriangleSize();
}

std::vector<vec2> MeshObject::get_flated_tri_pos(int idx)
{
    if(ordered_cutEdge.size() == 0)
        return std::vector<vec2>(4);

    if(idx == -1)
        idx = currentEdgeIdx;

    auto hew = manifold_core.walker(ordered_cutEdge[idx]);
    vector<vec3gel> pos(4);
    pos[0] = manifold_core.pos(hew.vertex());
    pos[1] = manifold_core.pos(hew.opp().vertex());
    pos[2] = manifold_core.pos(hew.next().vertex());
    pos[3] = manifold_core.pos(hew.opp().next().vertex());

    auto z2 = HMesh::normal(manifold_core, hew.face());
    auto z3 = HMesh::normal(manifold_core, hew.opp().face());
    auto x01 = pos[1] - pos[0]; x01.normalize();
    auto y2 = CGLA::cross(z2, x01);
    auto y3 = CGLA::cross(z3, x01);



    std::vector<vec2> pos2(4);
    pos2[0] = vec2(0);
    pos2[1][1] = 0;
    pos2[1][0] = (pos[1] - pos[0]).length();
    pos2[2][0] = CGLA::dot(pos[2] - pos[0], x01);
    pos2[2][1] = CGLA::dot(pos[2] - pos[0], y2);
    pos2[3][0] = CGLA::dot(pos[3] - pos[0], x01);
    pos2[3][1] = CGLA::dot(pos[3] - pos[0], y3);


    // scale
    for(auto &p : pos2)
        p *= scale2D;

    // translate
    vec2 bound_leftdown(INFINITY);
    vec2 bound_rightup(-INFINITY);

    for(auto p : pos2)
    {
        for(int i = 0; i < 2; i++)
        {
            bound_leftdown[i] = std::min(bound_leftdown[i], p[i]);
            bound_rightup[i] = std::max(bound_rightup[i], p[i]);
        }
    }

    vec2 center = (bound_rightup + bound_leftdown)/2;
    vec2 size = bound_rightup - bound_leftdown;

    if(size[0] > 2 || size[1] > 2)
    {
        qInfo() << "Triangle is too big";
        exit(1);
    }

    for(auto &p : pos2)
        p -= center;

    return pos2;
}

std::vector<vec2> MeshObject::get_flated_tri_pos_single(int idx)
{
    if(ordered_cutFaces.size() == 0)
        return std::vector<vec2>();

    if(idx == -1)
        idx = currentFaceIdx;

    auto hew = manifold_core.walker(ordered_cutFaces[idx]);

    vector<vec3gel> pos(3);
    pos[0]= manifold_core.pos(hew.vertex());
    pos[1] = manifold_core.pos(hew.next().vertex());
    pos[2] = manifold_core.pos(hew.next().next().vertex());

    auto z = HMesh::normal( manifold_core, hew.face());
    auto x = pos[1] - pos[0]; x.normalize();
    auto y = CGLA::cross(z,x);

    vector<vec2> pos2(3);
    pos2[0] = vec2(0);
    pos2[1][0] = 0;
    pos2[1][1] = (pos[1] - pos[0]).length();
    pos2[2][0] = CGLA::dot(x, pos[2] - pos[0]);
    pos2[2][1] = CGLA::dot(y, pos[2] - pos[0]);

    // scale
    for(auto &p : pos2)
        p *= scale2D;

    // translate
    vec2 bound_leftdown(INFINITY);
    vec2 bound_rightup(-INFINITY);

    for(auto p : pos2)
    {
        for(int i = 0; i < 2; i++)
        {
            bound_leftdown[i] = std::min(bound_leftdown[i], p[i]);
            bound_rightup[i] = std::max(bound_rightup[i], p[i]);
        }
    }

    vec2 center = (bound_rightup + bound_leftdown)/2;
    vec2 size = bound_rightup - bound_leftdown;

    for(auto &p : pos2)
        p -= center;

    return pos2;
}

HMesh::FaceID MeshObject::currentFace()
{
    if(ordered_cutFaces.size() == 0)
        return HMesh::InvalidFaceID;
    else
        return ordered_cutFaces[currentFaceIdx];
}

void MeshObject::changeCurrentFace(int dis)
{
    if(ordered_cutFaces.size() == 0)
        return;

    if(dis < 0 && currentFaceIdx + dis >= 0)
        currentFaceIdx += dis;

    if(dis > 0 && currentFaceIdx + dis < ordered_cutFaces.size())
        currentFaceIdx += dis;
}

void MeshObject::changeCurrentEdge(int dis)
{
    if(dis < 0 && currentEdgeIdx + dis >= 0)
        currentEdgeIdx += dis;

    if(dis > 0 && currentEdgeIdx + dis < ordered_cutEdge.size())
        currentEdgeIdx += dis;

    qInfo() << currentEdgeIdx;
}

QString MeshObject::progressMessage()
{
    return "Group " + QString::number(currentEdgeIdx)
            + "/" + QString::number(ordered_cutEdge.size())
            + " Face " + QString::number(currentFaceIdx)
            + "/" + QString::number(ordered_cutFaces.size());
}

std::vector<HMesh::HalfEdgeID> MeshObject::edge_incident_to_vertex(HMesh::VertexID v)
{
    std::vector<HMesh::HalfEdgeID> out;
    for(auto hew = manifold_core.walker(v); !hew.full_circle();
        hew = hew.circulate_vertex_cw())
    {
        out.push_back(hew.halfedge());
    }
    return out;
}

std::vector<HMesh::HalfEdgeID> MeshObject::edge_share_tri_with_edge(HMesh::HalfEdgeID eid)
{
    std::vector<HMesh::HalfEdgeID> out;
    auto hh = manifold_core.walker(eid);
    for(auto hew = manifold_core.walker(hh.halfedge()); !hew.full_circle();
        hew = hew.circulate_face_cw())
    {
        out.push_back(hew.halfedge());
    }
    for(auto hew = manifold_core.walker(hh.opp().halfedge()); !hew.full_circle();
        hew = hew.circulate_face_cw())
    {
        out.push_back(hew.halfedge());
    }

    return out;
}

void MeshObject::findConnectedComponents()
{
    vector<bool> triangleUsed(manifold_core.no_faces(), false);
    vector<vector<int>> connectedComponents;

    auto region = connectedRegion(* manifold_core.faces_begin(), triangleUsed);
}

std::vector<int> MeshObject::connectedRegion(HMesh::FaceID fid, std::vector<bool> &usedTri)
{
    std::vector<int> out;
    out.push_back(fid.get_index());
    usedTri[fid.get_index()] = true;

    while (1) {
        for(auto hew = manifold_core.walker(HMesh::FaceID(out.back()));
        !hew.full_circle(); hew = hew.circulate_face_ccw())
        {

        }
    }
}

void MeshObject::computeScaleFactor()
{
    float objHeight = bound_size[mainAxis];
    scale2D = _modelHeight / objHeight / projectorSize * 2; // screen is -1 to 1
}

void MeshObject::validateTriangleSize()
{
    for(int i = 0; i < ordered_cutEdge.size(); i++)
    {
        get_flated_tri_pos(i);
    }
}
