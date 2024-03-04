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
    V::Mesh mesh = V::Mesh::createSquare(rows, columns);
    if (!v.initialize("Mesh viewer", 640, 480)) {
        return EXIT_FAILURE;
    }
    v.createScene(&mesh);
    v.view();
    return 0;
}