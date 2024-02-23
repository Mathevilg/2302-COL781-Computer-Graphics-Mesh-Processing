#ifndef VIEWER_HPP
#define VIEWER_HPP

#include "hw.hpp"
#include <vector>

namespace COL781 {
    namespace Viewer {

        class Camera {
        public:
            glm::vec3 position;
            glm::vec3 front;
            glm::vec3 up; 
            glm::vec3 lookAt;
            glm::mat4 viewMatrix;

            float cameraSpeed, yaw, pitch, lastX, lastY, fov, aspect;
            bool firstMouse;
            void initialize(float aspect);
            glm::mat4 getViewMatrix();
            glm::mat4 getProjectionMatrix();
            glm::vec3 getViewDir();
            glm::vec3 getRightVector();

            void setCameraView(glm::vec3 position_vector, glm::vec3 lookat_vector, glm::vec3 up_vector);
            void updateViewMatrix();
        };

        class Viewer {
        public:
            bool initialize(const std::string &title, int width, int height);
            void setVertices(int n, const glm::vec3* vertices);
            void setNormals(int n, const glm::vec3* normals);
            void setTriangles(int n, const glm::ivec3* triangles);
            void view();
        private:
            COL781::OpenGL::Rasterizer r;
            COL781::OpenGL::ShaderProgram program;
            COL781::OpenGL::Object object;
            Camera camera;
        };

        class Point{
            float pos[3];
        };

        // class Vertex{
        //     Point position;
        //     Edge* edge;
        // };
        // class Edge{
        // private :
        //     Triangle *triangle;
        //     int index;
        // };

        // class Triangle{
        // private :
        //     Vertex *vertices[3];
        //     Edge *neighbors[3];

        // };

        // class Mesh{
        //     std::vector<Triangle> triangles;
        //     // std::vector<Point> points;
        // };

        class HalfEdge {
        public:
            HalfEdge *pair, *next;
            Vertex *head;
            Face *left;
        };

        class Vertex {
        public:
            vec4 position;
            vec4 colour;
            vec3 normal;
            HalfEdge *halfEdge;
            void traverseNeighbouringTriangles(Vertex* v);
        };

        class Face {
        public:
            HalfEdge *halfEdge;
        };

        class Mesh {
        public:
            Mesh* createSqaure(int rows, int columns);
            Mesh* createSphere(int longitudes, int latitudes);
            std::vector<glm::vec4> vertices(Mesh mesh);
            std::vector<glm::vec4> colours(Mesh mesh);
            std::vector<glm::ivec3> triangles(Mesh mesh);
            std::vector<Face> faces;



            // Part 1.3 and 1.4
            Mesh* loadMesh(std::string filePath); // Parser for part 1.3
            void recomputeVertexNormals(Mesh* mesh); // recompute normals for part 1.4
        };

        

    }
}

#endif
