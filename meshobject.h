#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include <string>
#include "GEL/HMesh/Manifold.h"

#include "GEL/CGLA/Vec2d.h"
#include <QString>

typedef CGLA::Vec2d vec2 ;

typedef CGLA::Vec3d vec3gel ;

class MeshObject
{
    HMesh::Manifold manifold_origin;
    HMesh::Manifold manifold_core;

    // Scaling
    float projectorSize = 60; // cm
    float _modelHeight = 100;
    float scale2D;


    void updateViewParam();

    int currentTriIndex = 0;


public:
    MeshObject(std::string path);

    static MeshObject &get_instance();

    HMesh::Manifold & manifold(){return manifold_core;}
    float & modelHeight(){return _modelHeight;}

    void draw();
    void drawTriangle();
    void drawTriangleSingle();
    void drawBased();
    void draw_ordered_cutEdges();
    void drawActiveEdge();
    void draw_ordered_cutFaces();

    void reduceMesh(float ratio);
    void buildCutOrderTri();
    void buildCutOrderDoubleTri();
    void buildCutOrderDoubleTri2();

    std::vector<vec2> get_flated_tri_pos(int idx = -1);
    std::vector<vec2> get_flated_tri_pos_single(int idx = -1);

    HMesh::HalfEdgeID currentEdge(){return ordered_cutEdge[currentEdgeIdx];}
    HMesh::FaceID currentFace();

    void changeCurrentFace(int dis);
    void changeCurrentEdge(int dis);

    QString progressMessage();
private:
    vec3gel scale_bound = vec3gel(1,1,1);
    vec3gel center_bound;
    vec3gel bound_size;

    int currentEdgeIdx = 0;
    std::vector<HMesh::HalfEdgeID> ordered_cutEdge;
    int currentFaceIdx = 0;
    std::vector<HMesh::FaceID> ordered_cutFaces;
    std::vector<HMesh::HalfEdgeID> edge_incident_to_vertex(HMesh::VertexID v);

    std::vector<HMesh::HalfEdgeID> edge_share_tri_with_edge(HMesh::HalfEdgeID);

    void findConnectedComponents();
    std::vector<int> connectedRegion(HMesh::FaceID, std::vector<bool> & usedTri);

    void computeScaleFactor();
    void validateTriangleSize();
};

MeshObject & meshInstance();

#endif // MESHOBJECT_H
