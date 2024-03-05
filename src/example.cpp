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
    mesh = *mesh.createSphere(50,50);
    if (!v.initialize("Mesh viewer", 640, 480)) {
        return EXIT_FAILURE;
    }
    
        // mesh.loopSubdivision(&mesh, 30);
   
    // mesh.flipEdge(&mesh, 119);
    
    mesh.createScene(&v);
    v.view();
    return 0;
}