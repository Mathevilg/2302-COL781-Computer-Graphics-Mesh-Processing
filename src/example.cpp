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
    // mesh = *mesh.createSquare(rows, columns);
    // mesh = *mesh.createSphere(10,10);
    std::string filePath = "../meshes/teapot.obj";
    mesh = mesh.loadMesh(filePath);
    if (!v.initialize("Mesh viewer", 640, 480)) {
        return EXIT_FAILURE;
    }
    
    mesh.loopSubdivision(&mesh, 2);
    
    mesh.createScene(&v);
    v.view();
    return 0;
}