#include "viewer.hpp"

namespace V = COL781::Viewer;
using namespace glm;

int main() {
    vec3 vertices[] = {
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.5, -0.5, 0.0),
        vec3(-0.5,  0.5, 0.0),
        vec3( 0.5,  0.5, 0.0)
    };
    vec3 normals[] = {
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0)
    };
    ivec3 triangles[] = {
        ivec3(0, 1, 2),
        ivec3(1, 2, 3)
    };

    V::Viewer v;
    int rows = 5, columns = 5;
    V::Mesh mesh;
    // mesh = *mesh.createSquare(3, 3);
    // mesh = *mesh.createSphere(10,10);
    std::string filePath = "../meshes/teapot.obj";
    mesh = mesh.loadMesh(filePath);
    // mesh.taubinSmoothing(&mesh, 0.33, -0.34, 25);
    // mesh.naiveSmoothing(&mesh, 0.33, 25);
    // mesh.collapseEdge(&mesh, 27);
    // mesh.flipEdge(&mesh, 27);
    // mesh.splitEdge(&mesh, 27, 0.5);
    // mesh.loopSubdivision(&mesh, 3);
    mesh.recomputeVertexNormals(&mesh);
    if (!v.initialize("Mesh viewer", 640, 480)) {
        return EXIT_FAILURE;
    }
    
    // mesh.loopSubdivision(&mesh, 1);
    
    mesh.createScene(&v);
    v.view();
    return 0;
}