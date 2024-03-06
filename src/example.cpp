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
    int rows = 1, columns = 1;
    V::Mesh mesh;
    // mesh = *mesh.createSquare(rows, columns);
    // mesh = *mesh.createSphere(10,10);
    std::string filePath = "../meshes/cube.obj";
    mesh = mesh.loadMesh(filePath);
    if (!v.initialize("Mesh viewer", 640, 480)) {
        return EXIT_FAILURE;
    }
    
    mesh.loopSubdivision(&mesh, 1);
   
    // mesh.flipEdge(&mesh, 119);
    // mesh.collapseEdge(&mesh, 45);
    mesh.createScene(&v);
    v.view();
    return 0;
}