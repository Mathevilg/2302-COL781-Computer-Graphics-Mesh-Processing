#ifndef VIEWER_HPP
#define VIEWER_HPP

#include "hw.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <string>

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
            void createScene(Mesh* mesh);
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
            Vertex* head;
            Face* left;
        };

        class Vertex {
        public:
            glm::vec3 position;
            glm::vec4 colour;
            glm::vec3 normal;
            int index;
            HalfEdge* halfEdge;
            void traverseNeighbouringTriangles(Vertex* v);
        };

        class Face {
        public:
            HalfEdge* halfEdge;
        };

        class Mesh {
        public:
            Mesh* createSqaure(int rows, int columns);
            Mesh* createSphere(int longitudes, int latitudes);
            std::vector<Vertex> vertices;
            std::vector<Face> faces;
            std::vector<HalfEdge> halfEdges;
            std::vector<glm::vec3> getVertices(Mesh mesh);
            std::vector<glm::vec3> getNormals(Mesh mesh);
            std::vector<glm::vec4> getColours(Mesh mesh);
            std::vector<glm::ivec3> getTriangles(Mesh mesh);
            void createScene(Mesh* mesh);
            // std::vector<Face> faces;



            // Part 1.3 and 1.4
            Mesh loadMesh(std::string filePath); // Parser for part 1.3
            void recomputeVertexNormals(Mesh* mesh); // recompute normals for part 1.4
        };

        

    }
}

#endif
