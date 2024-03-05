#include "viewer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
namespace COL781 {
    namespace Viewer {

        namespace GL = COL781::OpenGL;

        void Camera::initialize(float aspect) {
            firstMouse = true;
            yaw   = -90.0f;    
            pitch =  0.0f;
            lastX =  800.0f / 2.0;
            lastY =  600.0 / 2.0;
            fov   =  60.0f;

            this->aspect = aspect;

            position = glm::vec3(0.0f, 0.0f,  2.5f);
            lookAt = glm::vec3(0.0f, 0.0f, 0.0f);
            up = glm::vec3(0.0f, 1.0f,  0.0f);

            updateViewMatrix();
        }

        glm::mat4 Camera::getViewMatrix() {
            return viewMatrix;
        }

        void Camera::updateViewMatrix() {
            viewMatrix = glm::lookAt(position, lookAt, up);
        }

        glm::mat4 Camera::getProjectionMatrix() {
            return glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
        }
            glm::vec3 getRightVector();

        glm::vec3 Camera:: getViewDir() {
            return -glm::transpose(viewMatrix)[2];
        }

        glm::vec3 Camera::getRightVector() {
            return glm::transpose(viewMatrix)[0];
        }

        void Camera::setCameraView(glm::vec3 position_vector, glm::vec3 lookat_vector, glm::vec3 up_vector) {
            position = std::move(position_vector);
            lookAt = std::move(lookat_vector);
            up = std::move(up_vector);

            viewMatrix = glm::lookAt(position, lookAt, up);
        }

        bool Viewer::initialize(const std::string &title, int width, int height) {
            if (!r.initialize(title.c_str(), width, height))
                return false;
            program = r.createShaderProgram(
                r.vsBlinnPhong(),
                r.fsBlinnPhong()
            );
            r.useShaderProgram(program);
            object = r.createObject();
            r.enableDepthTest();
            camera.initialize((float)width/(float)height);
            return true;
        }

        void Viewer::setVertices(int n, const glm::vec3* vertices) {
            r.setVertexAttribs(object, 0, n, vertices);
        }

        void Viewer::setNormals(int n, const glm::vec3* normals) {
            r.setVertexAttribs(object, 1, n, normals);
        }

        void Viewer::setTriangles(int n, const glm::ivec3* triangles) {
            r.setTriangleIndices(object, n, triangles);
        }

        void Viewer::view() {
            // The transformation matrix.
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view;    
            glm::mat4 projection = camera.getProjectionMatrix();

            float deltaAngleX = 2.0 * 3.14 / 800.0;
            float deltaAngleY = 3.14 / 600.0;

            int lastxPos, lastyPos, xPos, yPos;

            SDL_GetMouseState(&lastxPos, &lastyPos);

            while (!r.shouldQuit()) {
                r.clear(glm::vec4(1.0, 1.0, 1.0, 1.0));

                camera.updateViewMatrix();

                Uint32 buttonState = SDL_GetMouseState(&xPos, &yPos);
                if( buttonState & SDL_BUTTON(SDL_BUTTON_LEFT) ) {
                    glm::vec4 pivot = glm::vec4(camera.lookAt.x, camera.lookAt.y, camera.lookAt.z, 1.0f);
                    glm::vec4 position = glm::vec4(camera.position.x, camera.position.y, camera.position.z, 1.0f);

                    float xAngle = (float)(lastxPos - xPos) * deltaAngleX;
                    float yAngle = (float)(lastyPos - yPos) * deltaAngleY;

                    float cosAngle = dot(camera.getViewDir(), camera.up);

                    if(cosAngle * signbit(deltaAngleY) > 0.99f)
                        deltaAngleY = 0.0f;

                    glm::mat4 rotationMatX(1.0f);
                    rotationMatX = glm::rotate(rotationMatX, xAngle, camera.up);
                    position = (rotationMatX * (position - pivot)) + pivot;

                    glm::mat4 rotationMatY(1.0f);
                    rotationMatY = glm::rotate(rotationMatY, yAngle, camera.getRightVector());
                    glm::vec3 finalPosition = (rotationMatY * (position - pivot)) + pivot;
                    camera.position = finalPosition;
                    camera.updateViewMatrix();
                }

                buttonState = SDL_GetMouseState(&xPos, &yPos);
                if( buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                    // Update camera parameters

                    float deltaY =  (float)(lastyPos - yPos) * 0.01f;
                    glm::mat4 dollyTransform = glm::mat4(1.0f);
                    dollyTransform = glm::translate(dollyTransform, normalize(camera.lookAt - camera.position) * deltaY);
                    glm::vec3 newCameraPosition = dollyTransform * glm::vec4(camera.position, 1.0f);
                    float newCameraFov = 2 * glm::atan(600.0f / (2 * deltaY)); // TODO Ask
                    
                    if(signbit(newCameraPosition.z) == signbit(camera.position.z)) {
                        camera.position = newCameraPosition;
                        camera.fov = newCameraFov; // TODO Ask
                        }
                }

                lastxPos = xPos;
                lastyPos = yPos;

                view = camera.getViewMatrix();
                
                r.setUniform(program, "model", model);
                r.setUniform(program, "view", view);
                r.setUniform(program, "projection", projection);
                r.setUniform(program, "lightPos", camera.position);
                r.setUniform(program, "viewPos", camera.position);
                r.setUniform(program, "lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

                r.setupFilledFaces();
                glm::vec3 orange(1.0f, 0.6f, 0.2f);
                glm::vec3 white(1.0f, 1.0f, 1.0f);
                r.setUniform(program, "ambientColor", 0.4f*orange);
                r.setUniform(program, "diffuseColor", 0.9f*orange);
                r.setUniform(program, "specularColor", 0.8f*white);
                r.setUniform(program, "phongExponent", 100.f);
                r.drawObject(object);

                r.setupWireFrame();
                glm::vec3 black(0.0f, 0.0f, 0.0f);
                r.setUniform(program, "ambientColor", black);
                r.setUniform(program, "diffuseColor", black);
                r.setUniform(program, "specularColor", black);
                r.setUniform(program, "phongExponent", 0.f);
                r.drawObject(object);
                r.show();
            }
        }

        // Part 1.1 and 1.2
        

        void Vertex::traverseNeighbouringTriangles(std::vector<HalfEdge> &halfEdges, Vertex* v){
            HalfEdge h = halfEdges[v->halfEdge];
            bool f = 0;
            do {
                // do something with h->left;
                if(halfEdges[h.halfEdgeNext].halfEdgePair==-1){
                    f = 1;
                    break;
                }
                h = halfEdges[halfEdges[h.halfEdgeNext].halfEdgePair];
            }
            while (h.index != halfEdges[v->halfEdge].index);

            if(f){
                h = halfEdges[v->halfEdge];
                while(h.halfEdgePair!=-1){
                    h = halfEdges[halfEdges[halfEdges[h.halfEdgePair].halfEdgeNext].halfEdgeNext];
                }
            }
        }

        void Mesh::createScene(Viewer* viewer){   
            std::vector<glm::vec3> vertices = this->getVertices(this);
            std::vector<glm::vec3> normals = this->getNormals(this);
            std::vector<glm::vec4> colours = this->getColours(this);
            std::vector<glm::ivec3> triangles = this->getTriangles(this);

            viewer->setVertices(vertices.size(), vertices.data());
            viewer->setNormals(normals.size(), normals.data());
            viewer->setTriangles(triangles.size(), triangles.data());
        }


        Mesh* Mesh::createSquare(int rows, int columns){
            Mesh* mesh = new Mesh();
        
            // Step 1: Initialize vertices
            for (int i = 0; i <= rows; i++) {
                for (int j = 0; j <= columns; j++) {
                    Vertex v;
                    v.position = glm::vec3((float)j / columns, (float)i / rows, 0.0f);
                    v.colour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
                    v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
                    v.index = i * (columns + 1) + j;
                    
                    mesh->vertices.push_back(v);
                    mesh->positions.push_back(v.position);
                    mesh->vertexNormals.push_back(v.normal);
                }
            }

            // Step 2: Initialize triangles
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < columns; j++) {
                    Face upperTriangle;
                    Face lowerTriangle;
                    HalfEdge e1, e2, e3, e4, e5, e6;
                    Vertex v1, v2, v3, v4;
                    v1 = mesh->vertices[i * (columns + 1) + j];
                    v2 = mesh->vertices[i * (columns + 1) + j + 1];
                    v3 = mesh->vertices[(i + 1) * (columns + 1) + j + 1];
                    v4 = mesh->vertices[(i + 1) * (columns + 1) + j];

                    int start_index = mesh->halfEdges.size();
                    e1.head = v2.index; e1.index = start_index;
                    e2.head = v1.index; e2.index = start_index + 1;
                    e3.head = v4.index; e3.index = start_index + 2;
                    e4.head = v4.index; e4.index = start_index + 3;
                    e5.head = v3.index; e5.index = start_index + 4;
                    e6.head = v2.index; e6.index = start_index + 5;

                    if(i==0){
                        v1.halfEdge = e2.index;
                    }
                    if(j==columns-1){
                        v2.halfEdge = e6.index;
                    }

                    if(i==rows-1){
                        if(j==0){
                            v4.halfEdge = e3.index;
                        }
                        v3.halfEdge = e5.index;
                    }
                    else{
                        v4.halfEdge = e3.index;
                    }
                    

                    upperTriangle.index = 2*(i*columns + j);
                    lowerTriangle.index = 2*(i*columns + j) + 1;
                    upperTriangle.halfEdge = e1.index;
                    lowerTriangle.halfEdge = e4.index;

                    e1.left = upperTriangle.index; e1.halfEdgeNext = e2.index;
                    e2.left = upperTriangle.index; e2.halfEdgeNext = e3.index;
                    e3.left = upperTriangle.index; e3.halfEdgeNext = e1.index;
                    e4.left = lowerTriangle.index; e4.halfEdgeNext = e5.index;
                    e5.left = lowerTriangle.index; e5.halfEdgeNext = e6.index;
                    e6.left = lowerTriangle.index; e6.halfEdgeNext = e4.index;
                    e4.halfEdgePair = e1.index;
                    e1.halfEdgePair = e4.index;

                    mesh->halfEdges.push_back(e1);
                    mesh->halfEdges.push_back(e2);
                    mesh->halfEdges.push_back(e3);
                    mesh->halfEdges.push_back(e4);
                    mesh->halfEdges.push_back(e5);
                    mesh->halfEdges.push_back(e6);

                    if(i!=0 && j!=0){
                        HalfEdge prevUp = mesh->halfEdges[mesh->halfEdges[mesh->faces[2*((i-1)*columns + j) + 1].halfEdge].halfEdgeNext];
                        HalfEdge prevLeft = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(i*columns + j) - 1].halfEdge].halfEdgeNext].halfEdgeNext];
                        e2.halfEdgePair = prevUp.index;
                        prevUp.halfEdgePair = e2.index;
                        e3.halfEdgePair = prevLeft.index;
                        prevLeft.halfEdgePair = e3.index;
                    }
                    else if(i!=0){
                        HalfEdge prevUp = mesh->halfEdges[mesh->halfEdges[mesh->faces[2*((i-1)*columns + j) + 1].halfEdge].halfEdgeNext];
                        e2.halfEdgePair = prevUp.index;
                        prevUp.halfEdgePair = e2.index;
                    }
                    else if(j!=0){
                        HalfEdge prevLeft = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(i*columns + j) - 1].halfEdge].halfEdgeNext].halfEdgeNext];
                        e3.halfEdgePair = prevLeft.index;
                        prevLeft.halfEdgePair = e3.index;
                    }
                    mesh->faces.push_back(upperTriangle);
                    mesh->faces.push_back(lowerTriangle);
                    
                }
            }
            return mesh;
        }

        Mesh* Mesh::createSphere(int longitudes, int latitudes){
            Mesh* mesh = new Mesh();

            // Step 1: Initialize vertices
            for (int i = 1; i < latitudes; i++) {
                for (int j = 0; j < longitudes; j++) {
                    float theta = static_cast<float>(j) / longitudes * glm::two_pi<float>();
                    float phi = static_cast<float>(i) / latitudes * glm::pi<float>();
        
                    Vertex v;
                    v.position = glm::vec3(std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi));        
                    v.colour = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
                    v.normal = glm::normalize(v.position);
                    v.index = (i-1) * longitudes + j;

                    mesh->vertices.push_back(v);
                    mesh->positions.push_back(v.position);
                    mesh->vertexNormals.push_back(v.normal);
                }
            }
            Vertex northPole, southPole;
            northPole.position = glm::vec3(0.0f, 0.0f, 1.0f); southPole.position = glm::vec3(0.0f, 0.0f, -1.0f);
            northPole.colour = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); southPole.colour = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
            northPole.normal = glm::vec3(0.0f, 0.0f, 1.0f); southPole.normal = glm::vec3(0.0f, 0.0f, -1.0f);
            mesh->vertices.push_back(northPole); mesh->vertices.push_back(southPole);
            mesh->positions.push_back(northPole.position); mesh->positions.push_back(southPole.position);
            mesh->vertexNormals.push_back(northPole.normal); mesh->vertexNormals.push_back(southPole.normal);
            northPole.index = (latitudes-1) * longitudes; southPole.index = (latitudes-1) * longitudes + 1;
            
            
            // Step 2: Initialize triangles
            for (int i = 0; i < latitudes-2; i++) {
                for (int j = 0; j < longitudes; j++) {
                    Face upperTriangle;
                    Face lowerTriangle;
        
                    HalfEdge e1, e2, e3, e4, e5, e6;
        
                    Vertex v1 = mesh->vertices[i * (longitudes) + j];
                    Vertex v2 = mesh->vertices[i * (longitudes) + (j + 1)%longitudes];
                    Vertex v3 = mesh->vertices[(i + 1) * (longitudes) + (j + 1)%longitudes];
                    Vertex v4 = mesh->vertices[(i + 1) * (longitudes) + j];

                    int start_index = mesh->halfEdges.size();
                    e1.head = v2.index; e1.index = start_index;
                    e2.head = v1.index; e2.index = start_index + 1;
                    e3.head = v4.index; e3.index = start_index + 2;
                    e4.head = v4.index; e4.index = start_index + 3;
                    e5.head = v3.index; e5.index = start_index + 4;
                    e6.head = v2.index; e6.index = start_index + 5;
                    
                    v3.halfEdge = e5.index;
                    if(i==0){
                        v1.halfEdge = e2.index;
                    }

                    upperTriangle.index = 2*(i*longitudes + j);
                    lowerTriangle.index = 2*(i*longitudes + j) + 1;
                    upperTriangle.halfEdge = e1.index;
                    lowerTriangle.halfEdge = e4.index;
        
                    e1.left = upperTriangle.index; e1.halfEdgeNext = e2.index;
                    e2.left = upperTriangle.index; e2.halfEdgeNext = e3.index;
                    e3.left = upperTriangle.index; e3.halfEdgeNext = e1.index;
                    e4.left = lowerTriangle.index; e4.halfEdgeNext = e5.index;
                    e5.left = lowerTriangle.index; e5.halfEdgeNext = e6.index;
                    e6.left = lowerTriangle.index; e6.halfEdgeNext = e4.index;
        
                    e4.halfEdgePair = e1.index;
                    e1.halfEdgePair = e4.index;
        
                    mesh->halfEdges.push_back(e1);
                    mesh->halfEdges.push_back(e2);
                    mesh->halfEdges.push_back(e3);
                    mesh->halfEdges.push_back(e4);
                    mesh->halfEdges.push_back(e5);
                    mesh->halfEdges.push_back(e6);
        
        
                    if (i != 0 && j!=0) {
                        HalfEdge prevUp = mesh->halfEdges[mesh->halfEdges[mesh->faces[2 * ((i - 1) * longitudes + j) + 1].halfEdge].halfEdgeNext];
                        HalfEdge prevLeft = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2 * (i * longitudes + j) - 1].halfEdge].halfEdgeNext].halfEdgeNext];
                        e2.halfEdgePair = prevUp.index;
                        prevUp.halfEdgePair = e2.index;
                        e3.halfEdgePair = prevLeft.index;
                        prevLeft.halfEdgePair = e3.index;
                        if(j==longitudes-1){
                            HalfEdge Right = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2 * ((i -1)* longitudes + j + 1) ].halfEdge].halfEdgeNext].halfEdgeNext];
                            e6.halfEdgePair = Right.index;
                            Right.halfEdgePair = e6.index;
                        }
                    }
                    else if (j != 0) {
                        HalfEdge prevLeft = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2 * (i * longitudes + j) - 1].halfEdge].halfEdgeNext].halfEdgeNext];
                        e3.halfEdgePair = prevLeft.index;
                        prevLeft.halfEdgePair = e3.index;
                        if(j==longitudes-1){
                            HalfEdge Right = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2 * ((i -1)* longitudes + j + 1) ].halfEdge].halfEdgeNext].halfEdgeNext];
                            e6.halfEdgePair = Right.index;
                            Right.halfEdgePair = e6.index;
                        }
                    }
                    if (i != 0) {
                        HalfEdge prevUp = mesh->halfEdges[mesh->halfEdges[mesh->faces[2 * ((i - 1) * longitudes + j) + 1].halfEdge].halfEdgeNext];
                        e2.halfEdgePair = prevUp.index;
                        prevUp.halfEdgePair = e2.index;
                    }
        
        
                    mesh->faces.push_back(upperTriangle);
                    mesh->faces.push_back(lowerTriangle);
                }
            }

            for(int j =0; j<longitudes; j++){
                Face topTriangle;
                Face bottomTriangle;
                HalfEdge e1, e2, e3, e4, e5, e6;
                Vertex v1 = mesh->vertices[j];
                Vertex v2 = mesh->vertices[(j + 1)%longitudes];
                Vertex v3 = mesh->vertices[(latitudes - 2) * (longitudes) + (j + 1)%longitudes];
                Vertex v4 = mesh->vertices[(latitudes - 2) * (longitudes) + j];
                int start_index = mesh->halfEdges.size();
                e1.head = northPole.index; e1.index = start_index;
                e2.head = v1.index; e2.index = start_index + 1;
                e3.head = v2.index; e3.index = start_index + 2;
                e4.head = southPole.index; e4.index = start_index + 3;
                e5.head = v4.index; e5.index = start_index + 4;
                e6.head = v3.index; e6.index = start_index + 5;

                if(j==0){
                    northPole.halfEdge = e1.index;
                    southPole.halfEdge = e4.index;
                }

                topTriangle.index = 2*(latitudes-2)*longitudes+2*j;
                bottomTriangle.index = 2*(latitudes-2)*longitudes+2*j + 1;
                topTriangle.halfEdge = e3.index;
                bottomTriangle.halfEdge = e6.index;

                e1.left = topTriangle.index; e1.halfEdgeNext = e2.index;
                e2.left = topTriangle.index; e2.halfEdgeNext = e3.index;
                e3.left = topTriangle.index; e3.halfEdgeNext = e1.index;
                e4.left = bottomTriangle.index; e4.halfEdgeNext = e5.index;
                e5.left = bottomTriangle.index; e5.halfEdgeNext = e6.index;
                e6.left = bottomTriangle.index; e6.halfEdgeNext = e4.index;

                mesh->halfEdges.push_back(e1);
                mesh->halfEdges.push_back(e2);
                mesh->halfEdges.push_back(e3);
                mesh->halfEdges.push_back(e4);
                mesh->halfEdges.push_back(e5);
                mesh->halfEdges.push_back(e6);

                if(latitudes==2){
                    e3.halfEdgePair = e6.index;
                    e6.halfEdgePair = e3.index;
                    if(j!=0){
                        HalfEdge prevLeftTop = mesh->halfEdges[e2.index-7];
                        e2.halfEdgePair = prevLeftTop.index;
                        prevLeftTop.halfEdgePair = e2.index;
                        HalfEdge prevLeftBottom = mesh->halfEdges[e4.index-5];
                        e4.halfEdgePair = prevLeftBottom.index;
                        prevLeftBottom.halfEdgePair = e4.index;
                        if(j==longitudes-1){
                            HalfEdge prevRightTop = mesh->halfEdges[1];
                            e1.halfEdgePair = prevRightTop.index;
                            prevRightTop.halfEdgePair = e1.index;

                            HalfEdge prevRightBottom = mesh->halfEdges[3];
                            e5.halfEdgePair = prevRightBottom.index;
                            prevRightBottom.halfEdgePair = e5.index;
                        }
                    }
                }
                else{
                    HalfEdge prevDown = mesh->halfEdges[mesh->halfEdges[mesh->faces[2*j].halfEdge].halfEdgeNext];
                    e3.halfEdgePair = prevDown.index;
                    prevDown.halfEdgePair = e3.index;
    
                    HalfEdge prevUp = mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(latitudes-3)*longitudes + 2*j + 1].halfEdge].halfEdgeNext];
                    e6.halfEdgePair = prevUp.index;
                    prevUp.halfEdgePair = e6.index;
    
                    if(j!=0){
                        HalfEdge prevLeftTop = mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(latitudes-2)*longitudes + 2*j - 2].halfEdge].halfEdgeNext];
                        e2.halfEdgePair = prevLeftTop.index;
                        prevLeftTop.halfEdgePair = e2.index;
                        HalfEdge prevLeftBottom = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(latitudes-2)*longitudes + 2*j - 1].halfEdge].halfEdgeNext].halfEdgeNext];
                        e4.halfEdgePair = prevLeftBottom.index;
                        prevLeftBottom.halfEdgePair = e4.index;
                        if(j==longitudes-1){
                            HalfEdge prevRightTop = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(latitudes-2)*longitudes].halfEdge].halfEdgeNext].halfEdgeNext];
                            e1.halfEdgePair = prevRightTop.index;
                            prevRightTop.halfEdgePair = e1.index;
                            HalfEdge prevRightBottom = mesh->halfEdges[mesh->halfEdges[mesh->faces[2*(latitudes-2)*longitudes + 1].halfEdge].halfEdgeNext];
                            e5.halfEdgePair = prevRightBottom.index;
                            prevRightBottom.halfEdgePair = e5.index; 
                        } 
    
                    }
                }

                mesh->faces.push_back(topTriangle);
                mesh->faces.push_back(bottomTriangle);
            
            }
            
            
            return mesh;
        }

        std::vector<glm::vec3> Mesh::getVertices(Mesh* mesh){
            std::vector<glm::vec3> pos;
            for (auto v : mesh->vertices) pos.push_back(v.position);
            // return mesh->positions;
            return pos;
        }
        std::vector<glm::vec3> Mesh::getNormals(Mesh* mesh){
            // return mesh->vertexNormals;
            std::vector<glm::vec3> nor;
            for (auto v : mesh->vertices) nor.push_back(v.normal);
            return nor;
        }
        std::vector<glm::vec4> Mesh::getColours(Mesh* mesh){
            std::vector<glm::vec4> colours;

            for(auto vertex : mesh->vertices){
                // vertex.colour = glm::vec4(1.0, 0.0, 0.0, 1.0);
                colours.push_back(vertex.colour);
            }
            return colours;
        }
        std::vector<glm::ivec3> Mesh::getTriangles(Mesh* mesh){
            std::vector<glm::ivec3> triangles;

            for(auto face : mesh->faces){
                auto edge = mesh->halfEdges[face.halfEdge];
                triangles.push_back(glm::ivec3(edge.head, mesh->halfEdges[edge.halfEdgeNext].head, mesh->halfEdges[mesh->halfEdges[edge.halfEdgeNext].halfEdgeNext].head));
            }
            return triangles;
        }

        // Parts 1.3 and 1.4

        // Parser for part 1.3
        Mesh Mesh::loadMesh(std::string filePath){
            Mesh mesh;
            // std::cout << filePath << "\n";
            std::ifstream inputFile(filePath);
            std::string line;
            int intValue;
            int vertexCount=0;
            int edgeCount = 0;
            int faceCount = 0;
            int normalCount = 0;
            std::map<std::pair<int, int>,int> edgeMap;
            bool normalsPresent = 0;
            while (std::getline(inputFile, line)){
                // std::cout << line << "\n";
                std::istringstream iss(line);
                std::string type;
                iss >> type;
                float x,y,z;
                // std::cout << type << "\n";
                if (type=="f"){
                    // for eavery face, we would have exactly 3 half edges
                    Face f;
                    HalfEdge e1,e2,e3;
                    Vertex v1, v2, v3;
                    e1.index = edgeCount;
                    e2.index = edgeCount+1;
                    e3.index = edgeCount+2;
                    f.index = faceCount;

                    int i,j,k;
                    iss >> i >> j >> k;
                    v1.position = mesh.positions[i-1];
                    v2.position = mesh.positions[j-1];
                    v3.position = mesh.positions[k-1];
                    if (normalsPresent){
                        v1.normal = mesh.vertexNormals[i-1];
                        v2.normal = mesh.vertexNormals[j-1];
                        v3.normal = mesh.vertexNormals[k-1];
                    }
                        
                    // if (edgeMap.find(std::pair<int,int>(std::min(j,i), std::max(j,i))) == edgeMap.end()) edgeMap[std::pair<int,int>(std::min(j,i), std::max(j,i))] = e1.index;
                    //     else { // if found 
                    //         (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,i), std::max(j,i))]].halfEdgePair = e1.index;
                    //         e1.halfEdgePair = (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,i), std::max(j,i))]].index;}

                    // if (edgeMap.find(std::pair<int,int>(std::min(j,k), std::max(j,k))) == edgeMap.end()) edgeMap[std::pair<int,int>(std::min(j,k), std::max(j,k))] = e1.index;
                    //     else { // if found 
                    //         (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,k), std::max(j,k))]].halfEdgePair = e1.index;
                    //         e1.halfEdgePair = (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,k), std::max(j,k))]].index;}

                    // if (edgeMap.find(std::pair<int,int>(std::min(k,i), std::max(k,i))) == edgeMap.end()) edgeMap[std::pair<int,int>(std::min(i,k), std::max(i,k))] = e1.index;
                    //     else { // if found 
                    //         (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(i,k), std::max(i,k))]].halfEdgePair = e1.index;
                    //         e1.halfEdgePair = (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(i,k), std::max(i,k))]].index;}

                    f.halfEdge = edgeCount;

                    e1.halfEdgeNext = edgeCount+1;
                    e2.halfEdgeNext = edgeCount+2;
                    e3.halfEdgeNext = edgeCount;

                    e1.left = faceCount;
                    e2.left = faceCount;
                    e3.left = faceCount;

                    e1.head = vertexCount+1;
                    e2.head = vertexCount+2;
                    e3.head = vertexCount;
                    
                    v1.halfEdge = edgeCount+2;
                    v2.halfEdge = edgeCount;
                    v3.halfEdge = edgeCount+1;

                    mesh.faces.push_back(f);
                    mesh.halfEdges.push_back(e1);
                    mesh.halfEdges.push_back(e2);
                    mesh.halfEdges.push_back(e3);
                    mesh.vertices.push_back(v1);
                    mesh.vertices.push_back(v2);
                    mesh.vertices.push_back(v3);

                    vertexCount+=3;
                    edgeCount+=3;
                    faceCount++;
                }
                // assuming that the object file contains vn first followed by v.
                else if (type =="vn"){
                    Vertex v;
                    // float x,y,z;
                    iss >> x >> y >> z;
                    mesh.vertexNormals.push_back(glm::vec3(x,y,z));
                    normalsPresent=1;
                    // v.normal = glm::vec3(x,y,z);
                    // v.index = vertexCount;
                    // mesh.vertices.push_back(v);
                    normalCount++;
                }

                else if (type == "v"){
                    // float x,y,z;
                    if (normalCount==0){
                        // Vertex v;
                        // // float x,y,z;
                        iss >> x >> y >> z;
                        // // std ::cout << x << y << z << "\n";
                        // v.position = glm::vec3(x,y,z);
                        // v.index = vertexCount;
                        // mesh.vertices.push_back(v);
                        // vertexCount++;
                        mesh.positions.push_back(glm::vec3(x,y,z));
                    }
                    else {
                        iss >> x >> y >> z;
                        mesh.vertices[vertexCount].position = glm::vec3(x,y,z);
                        vertexCount++;
                    }
                }
            }

            // Now traverse the edges once agan to set the pairs
            for (int i=0; i<mesh.halfEdges.size(); i++) {
                for (int j=i+1; j<mesh.halfEdges.size(); j++) {
                    glm::vec3 a1 = mesh.vertices[mesh.halfEdges[i].head].position;
                    glm::vec3 a2 = mesh.vertices[mesh.halfEdges[mesh.halfEdges[mesh.halfEdges[i].halfEdgeNext].halfEdgeNext].head].position;
                    glm::vec3 b1 = mesh.vertices[mesh.halfEdges[j].head].position;
                    glm::vec3 b2 = mesh.vertices[mesh.halfEdges[mesh.halfEdges[mesh.halfEdges[j].halfEdgeNext].halfEdgeNext].head].position;
                    if (a1==b2 && a2==b1){
                        std::cout << "Match!!" << a1.y << a1.z << a2.y << a2.z << "\n";
                        mesh.halfEdges[i].halfEdgePair = j;
                        mesh.halfEdges[j].halfEdgePair = i;
                    }
                    
                }
            }
            
            return mesh;
        }

        // recompute normals for part 1.4
        void Mesh::recomputeVertexNormals(Mesh* mesh){
            for (int i=0; i<(mesh->vertices).size(); i++){
                // for each vertex take the weighted average of normals of each faces with weights proportional to the areas of faces

                int he = (mesh->vertices[i]).halfEdge;
                std::vector<float> areas;
                std::vector<glm::vec3> normls;
                do {
                // do something with h->left;
                    glm::vec3 v1 = mesh->vertices[mesh->halfEdges[he].head].position;
                    glm::vec3 v2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].head].position;
                    glm::vec3 v3 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgeNext].head].position;
                    // std::cout << "v1: " << v1.x << " " << v1.y << " " << v1.z << "\n";
                    // std::cout << "v2: " << v2.x << " " << v2.y << " " << v2.z << "\n";
                    // std::cout << "v3: " << v3.x << " " << v3.y << " " << v3.z << "\n";
                    // std::cout <<"\n";
                    glm::vec3 n = glm::cross(v1-v2, v1-v3);
                    // std::cout << "area : " << glm::length(n) << "\n";
                    areas.push_back(glm::length(n));
                    normls.push_back(n/glm::length(n));
                    // std::cout << "n: " << n.x << " " << n.y << " " << n.z << "\n";
                    he = mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgePair;  // h = h->next->pair;
                } while (he != (mesh->vertices[i]).halfEdge);

                glm::vec3 interpolatedNormal = glm::vec3(0.0, 0.0, 0.0);
                float areaSum=0.0;
                for (int i=0; i<normls.size(); i++) {
                    interpolatedNormal+=normls[i]*areas[i];
                    areaSum+=areas[i];
                }
                interpolatedNormal /= areaSum;
                mesh->vertices[i].normal = interpolatedNormal/glm::length(interpolatedNormal);
                // std::cout << "n: " << interpolatedNormal.x << " " << interpolatedNormal.y << " " << interpolatedNormal.z << "\n";
                // std::cout << "n: " << mesh->vertices[i].normal.x << " " << mesh->vertices[i].normal.y << " " << mesh->vertices[i].normal.z << "\n";
            }
        }


        void Mesh::naiveSmoothing(Mesh* mesh, float lambda, int iter){
            for (int iterations=0; iterations<iter; iterations++){
                for (int i=0; i<(mesh->vertices).size(); i++){
                    int he = (mesh->vertices[i]).halfEdge;
                    glm::vec3 sumNeighbours = glm::vec3(0.0,0.0,0.0);
                    int neighbours=0;
                    do {
                    // do something with h->head;
                        glm::vec3 v1 = mesh->vertices[mesh->halfEdges[he].head].position;
                        glm::vec3 v2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].head].position;
                        glm::vec3 v3 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgeNext].head].position;
                        sumNeighbours += v2;
                        sumNeighbours += v3;
                        he = mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgePair;  // h = h->next->pair;
                        neighbours++;
                    } while (he != (mesh->vertices[i]).halfEdge);
                    sumNeighbours /= (2.0*neighbours);
                    glm::vec3 delta = sumNeighbours - mesh->vertices[mesh->halfEdges[he].head].position;
                    mesh->vertices[mesh->halfEdges[he].head].position += lambda*delta;
                }
            }
        }

        void Mesh::taubinSmoothing(Mesh* mesh, float lambda, float mu, int iter){
            for (int iterations=0; iterations<iter; iterations++){
                for (int i=0; i<(mesh->vertices).size(); i++){
                    int he = (mesh->vertices[i]).halfEdge;
                    glm::vec3 sumNeighbours = glm::vec3(0.0,0.0,0.0);
                    int neighbours=0;
                    do {
                    // do something with h->head;
                        glm::vec3 v1 = mesh->vertices[mesh->halfEdges[he].head].position;
                        glm::vec3 v2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].head].position;
                        glm::vec3 v3 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgeNext].head].position;
                        sumNeighbours += v2;
                        sumNeighbours += v3;
                        he = mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgePair;  // h = h->next->pair;
                        neighbours++;
                    } while (he != (mesh->vertices[i]).halfEdge);
                    sumNeighbours /= (2.0*neighbours);
                    glm::vec3 delta = sumNeighbours - mesh->vertices[mesh->halfEdges[he].head].position;
                    mesh->vertices[mesh->halfEdges[he].head].position += lambda*delta;
                }
                for (int i=0; i<(mesh->vertices).size(); i++){
                    int he = (mesh->vertices[i]).halfEdge;
                    glm::vec3 sumNeighbours = glm::vec3(0.0,0.0,0.0);
                    int neighbours=0;
                    do {
                    // do something with h->head;
                        glm::vec3 v1 = mesh->vertices[mesh->halfEdges[he].head].position;
                        glm::vec3 v2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].head].position;
                        glm::vec3 v3 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgeNext].head].position;
                        sumNeighbours += v2;
                        sumNeighbours += v3;
                        he = mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgePair;  // h = h->next->pair;
                        neighbours++;
                    } while (he != (mesh->vertices[i]).halfEdge);
                    sumNeighbours /= (2.0*neighbours);
                    glm::vec3 delta = sumNeighbours - mesh->vertices[mesh->halfEdges[he].head].position;
                    mesh->vertices[mesh->halfEdges[he].head].position += lambda*mu;
                }
            }
        }

        // part 2.2 edge flipping, splitting, collapsing, testing mesh connectivity
        void Mesh::flipEdge(Mesh* mesh, int halfEdgeIndex){
            glm::vec3 newPos1 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].head].position;
            glm::vec3 newPos2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext].head].position;

            int left = mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].halfEdgePair;
            int leftRight = mesh->halfEdges[halfEdgeIndex].halfEdgeNext;

            int right = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext].halfEdgePair;
            int rightLeft = mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext;
            // int pair = mesh->halfEdges[halfEdgeIndex].halfEdgeNext;
            int a = halfEdgeIndex;
            int b = mesh->halfEdges[halfEdgeIndex].halfEdgePair;

            mesh->vertices[mesh->halfEdges[halfEdgeIndex].head].position = newPos2;
            mesh->vertices[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].head].position = newPos1;

            mesh->halfEdges[halfEdgeIndex].halfEdgePair = right;
            mesh->halfEdges[right].halfEdgePair = a;
            mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgePair = left;
            mesh->halfEdges[left].halfEdgePair = b;  
            mesh->halfEdges[rightLeft].halfEdgePair = leftRight;
            mesh->halfEdges[leftRight].halfEdgePair = rightLeft;
        }
        void Mesh::splitEdge(Mesh* mesh, int halfEdgeIndex, float ratio){
            Vertex v1,v2,v3,v4,v5,v6;
            HalfEdge e1,e2,e3,e4,e5,e6;
            Face f1,f2;
            glm::vec3 pos1 = mesh->vertices[mesh->halfEdges[halfEdgeIndex].head].position;
            glm::vec3 pos2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].head].position;
            glm::vec3 posleft = mesh->vertices[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].head].position;
            glm::vec3 posright = mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext].head].position;
            glm::vec3 mid = (pos1+pos2);
            mid/=2.0;
            mesh->vertices[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].halfEdgeNext].head].position = mid;
            mesh->vertices[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].head].position = mid;
            v1.position = pos2;
            v2.position = pos1;
            v3.position = posleft;
            v4.position = pos1;
            v5.position = pos2;
            v6.position = posright;
            int currLen = mesh->vertices.size();
            int currFaceLen = mesh->faces.size();
            e1.head = currLen+1;
            e2.head = currLen+2;
            e3.head = currLen;
            e4.head = currLen+4;
            e5.head = currLen+5;
            e6.head = currLen+3;
            e1.halfEdgeNext = currLen+1;
            e2.halfEdgeNext = currLen+2;
            e3.halfEdgeNext = currLen;
            e4.halfEdgeNext = currLen+4;
            e5.halfEdgeNext = currLen+5;
            e6.halfEdgeNext = currLen+3;
            f1.halfEdge = currLen;
            f2.halfEdge = currLen+3;
            e1.left = currFaceLen;
            e2.left = currFaceLen;
            e3.left = currFaceLen;
            e4.left = currFaceLen+1;
            e5.left = currFaceLen+1;
            e6.left = currFaceLen+1;
            e1.halfEdgePair = currLen+3;
            mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].halfEdgeNext].halfEdgePair].halfEdgePair = currLen+2;
            e3.halfEdgePair = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].halfEdgeNext].halfEdgePair;
            e2.halfEdgePair = mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].halfEdgeNext;
            mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgeNext].halfEdgeNext].halfEdgePair = currLen+1;
            e4.halfEdgePair = currLen;
            e5.halfEdgePair = mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext].halfEdgePair;
            mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext].halfEdgePair].halfEdgePair = currLen+4;
            e6.halfEdgePair = mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext;
            mesh->halfEdges[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].halfEdgeNext].halfEdgePair = currLen+5;
            v1.halfEdge = currLen+2;
            v2.halfEdge = currLen;
            v3.halfEdge = currLen+1;
            v4.halfEdge = currLen+5;
            v5.halfEdge = currLen+3;
            v6.halfEdge = currLen+4;
            mesh->vertices.push_back(v1);
            mesh->vertices.push_back(v2);
            mesh->vertices.push_back(v3);
            mesh->vertices.push_back(v4);
            mesh->vertices.push_back(v5);
            mesh->vertices.push_back(v6);
            mesh->halfEdges.push_back(e1);
            mesh->halfEdges.push_back(e2);
            mesh->halfEdges.push_back(e3);
            mesh->halfEdges.push_back(e4);
            mesh->halfEdges.push_back(e5);
            mesh->halfEdges.push_back(e6);
            mesh->faces.push_back(f1);
            mesh->faces.push_back(f2);
            std::cout<<mesh->halfEdges.size()<< " " <<mesh->vertices.size()<<"\n";
        }
        void Mesh::collapseEdge(Mesh* mesh, int halfEdgeIndex){
            glm::vec3 pos1 = mesh->vertices[mesh->halfEdges[halfEdgeIndex].head].position;
            glm::vec3 pos2 = mesh->vertices[mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].head].position;
            glm::vec3 mid = pos1+pos2;
            mid/=2.0;
            
            int v1 = mesh->halfEdges[halfEdgeIndex].head;
            int v2 = mesh->halfEdges[mesh->halfEdges[halfEdgeIndex].halfEdgePair].head;
            int he = (mesh->vertices[v1]).halfEdge;
            do {
                mesh->vertices[mesh->halfEdges[he].head].position = mid;
                he = mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgePair;  // h = h->next->pair;
            } while (he != (mesh->vertices[v1]).halfEdge);

            he = (mesh->vertices[v2]).halfEdge;
            do {
                mesh->vertices[mesh->halfEdges[he].head].position = mid;
                he = mesh->halfEdges[mesh->halfEdges[he].halfEdgeNext].halfEdgePair;  // h = h->next->pair;
            } while (he != (mesh->vertices[v2]).halfEdge);

        }
        bool Mesh::testMeshConnectivity(Mesh* mesh){return true;}
        // invariants : vertices, edges, faces count
        // 

        // part 2.3
        
        void Mesh::loopSubdivision(Mesh* mesh){
            std::map<int, std::pair<std::pair<int, int>,std::pair<int, int>>> edgeMap;
            std::vector<Vertex> newVertices;
            std::vector<int> oldVertices;
            std::vector<HalfEdge> newEdges;
            std::vector<Face> newFaces;

            //Splitting to create new vertices
            for(auto face:mesh->faces){
                HalfEdge e1 = mesh->halfEdges[face.halfEdge];
                HalfEdge e2 = mesh->halfEdges[e1.halfEdgeNext];
                HalfEdge e3 = mesh->halfEdges[e2.halfEdgeNext];
                
                Vertex v1 = mesh->vertices[e1.head];
                Vertex v2 = mesh->vertices[e2.head];
                Vertex v3 = mesh->vertices[e3.head];

                glm::vec3 newPos1 = (v1.position+v2.position);
                glm::vec3 newPos2 = (v2.position+v3.position);
                glm::vec3 newPos3 = (v3.position+v1.position);
                newPos1/=2.0;newPos2/=2.0;newPos3/=2.0;

                Vertex v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15;
                HalfEdge e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15;
                Face f1,f2,f3,f4;

                v4.position = v9.position = v14.position = newPos3;
                v6.position = v7.position = v11.position = newPos1;
                v8.position = v10.position = v15.position = newPos2;
                v5.position = v1.position; v12.position = v2.position; v13.position = v3.position;
                
                int start_index = newEdges.size();
                v4.index = start_index; v5.index = start_index+1; v6.index = start_index+2; v7.index = start_index+3; 
                v8.index = start_index+4; v9.index = start_index+5; v10.index = start_index+6; v11.index = start_index+7;
                v12.index = start_index+8; v13.index = start_index+9; v14.index = start_index+10; v15.index = start_index+11;

                oldVertices.push_back(v5.index);
                oldVertices.push_back(v12.index);
                oldVertices.push_back(v13.index);

                e4.index = start_index; e5.index = start_index+1; e6.index = start_index+2; e7.index = start_index+3;
                e8.index = start_index+4; e9.index = start_index+5; e10.index = start_index+6; e11.index = start_index+7;
                e12.index = start_index+8; e13.index = start_index+9; e14.index = start_index+10; e15.index = start_index+11;

                v4.halfEdge = e4.index; v5.halfEdge = e5.index; v6.halfEdge = e6.index; v7.halfEdge = e7.index; v8.halfEdge = e8.index; v9.halfEdge = e9.index;
                v10.halfEdge = e10.index; v11.halfEdge = e11.index; v12.halfEdge = e12.index; v13.halfEdge = e13.index; v14.halfEdge = e14.index; v15.halfEdge = e15.index;

                f1.index = start_index; f2.index = start_index+1; f3.index = start_index+2; f4.index = start_index+3;
                f1.halfEdge = e4.index; f2.halfEdge = e7.index; f3.halfEdge = e10.index; f4.halfEdge = e13.index;

                e4.head = v4.index; e5.head = v5.index; e6.head = v6.index; e7.head = v7.index; e8.head = v8.index; e9.head = v9.index;
                e10.head = v10.index; e11.head = v11.index; e12.head = v12.index; e13.head = v13.index; e14.head = v14.index; e15.head = v15.index;

                e4.halfEdgeNext = e5.index; e5.halfEdgeNext = e6.index; e6.halfEdgeNext = e4.index;
                e7.halfEdgeNext = e8.index; e8.halfEdgeNext = e9.index; e9.halfEdgeNext = e7.index;
                e10.halfEdgeNext = e11.index; e11.halfEdgeNext = e12.index; e12.halfEdgeNext = e10.index;
                e13.halfEdgeNext = e14.index; e14.halfEdgeNext = e15.index; e15.halfEdgeNext = e13.index;

                e4.left = f1.index; e5.left = f1.index; e6.left = f1.index; e7.left = f2.index; e8.left = f2.index; e9.left = f2.index;
                e10.left = f3.index; e11.left = f3.index; e12.left = f3.index; e13.left = f4.index; e14.left = f4.index; e15.left = f4.index;

                e4.halfEdgePair = e7.index; e7.halfEdgePair = e4.index; 
                e8.halfEdgePair = e11.index; e11.halfEdgePair = e8.index;
                e9.halfEdgePair = e15.index; e15.halfEdgePair = e9.index;

                newVertices.push_back(v4); newVertices.push_back(v5); newVertices.push_back(v6); newVertices.push_back(v7); newVertices.push_back(v8); newVertices.push_back(v9);
                newVertices.push_back(v10); newVertices.push_back(v11); newVertices.push_back(v12); newVertices.push_back(v13); newVertices.push_back(v14); newVertices.push_back(v15);
                newEdges.push_back(e4); newEdges.push_back(e5); newEdges.push_back(e6); newEdges.push_back(e7); newEdges.push_back(e8); newEdges.push_back(e9);
                newEdges.push_back(e10); newEdges.push_back(e11); newEdges.push_back(e12); newEdges.push_back(e13); newEdges.push_back(e14); newEdges.push_back(e15);

                newFaces.push_back(f1); newFaces.push_back(f2); newFaces.push_back(f3); newFaces.push_back(f4);

                if(edgeMap.find(e1.halfEdgePair)!=edgeMap.end()){
                    std::pair<int,int> pair1 = edgeMap[e1.halfEdgePair].first;
                    std::pair<int,int> pair2 = edgeMap[e1.halfEdgePair].second;
                    if(pair1.first == v1.index){
                        e5.halfEdgePair = pair1.second;
                        newEdges[pair1.second].halfEdgePair = e5.index;
                        e14.halfEdgePair = pair2.second;
                        newEdges[pair2.second].halfEdgePair = e14.index;
                    }
                    else{
                        e5.halfEdgePair = pair2.second;
                        newEdges[pair2.second].halfEdgePair = e5.index;
                        e14.halfEdgePair = pair1.second;
                        newEdges[pair1.second].halfEdgePair = e14.index;
                    }
                }
                else{
                    edgeMap[e1.index] = {{v1.index,e5.index},{v3.index,e14.index}};
                }

                if(edgeMap.find(e2.halfEdgePair)!=edgeMap.end()){
                    std::pair<int,int> pair1 = edgeMap[e2.halfEdgePair].first;
                    std::pair<int,int> pair2 = edgeMap[e2.halfEdgePair].second;
                    if(pair1.first == v1.index){
                        e6.halfEdgePair = pair1.second;
                        newEdges[pair1.second].halfEdgePair = e6.index;
                        e12.halfEdgePair = pair2.second;
                        newEdges[pair2.second].halfEdgePair = e12.index;
                    }
                    else{
                        e6.halfEdgePair = pair2.second;
                        newEdges[pair2.second].halfEdgePair = e6.index;
                        e12.halfEdgePair = pair1.second;
                        newEdges[pair1.second].halfEdgePair = e12.index;
                    }
                }
                else{
                    edgeMap[e2.index] = {{v1.index,e6.index},{v2.index,e12.index}};
                }

                if(edgeMap.find(e3.halfEdgePair)!=edgeMap.end()){
                    std::pair<int,int> pair1 = edgeMap[e3.halfEdgePair].first;
                    std::pair<int,int> pair2 = edgeMap[e3.halfEdgePair].second;
                    if(pair1.first == v2.index){
                        e10.halfEdgePair = pair1.second;
                        newEdges[pair1.second].halfEdgePair = e10.index;
                        e13.halfEdgePair = pair2.second;
                        newEdges[pair2.second].halfEdgePair = e13.index;
                    }
                    else{
                        e10.halfEdgePair = pair2.second;
                        newEdges[pair2.second].halfEdgePair = e10.index;
                        e13.halfEdgePair = pair1.second;
                        newEdges[pair1.second].halfEdgePair = e13.index;
                    }
                }
                else{
                    edgeMap[e3.index] = {{v2.index,e10.index},{v3.index,e13.index}};
                }
            }

            mesh->vertices = newVertices;
            mesh->halfEdges = newEdges;
            mesh->faces = newFaces;

            //Updating the old vertices
            for(auto vertex:oldVertices){
                glm::vec3 newPos = glm::vec3(0.0,0.0,0.0);
                int count = 0;
                HalfEdge he = mesh->halfEdges[mesh->vertices[vertex].halfEdge];
                bool f =0;
                do {
                    if(mesh->halfEdges[he.halfEdgeNext].halfEdgePair==-1){
                        f = 1;
                        break;
                    }
                    HalfEdge nextEdge = mesh->halfEdges[he.halfEdgeNext];
                    newPos+=mesh->vertices[nextEdge.head].position;
                    he = mesh->halfEdges[nextEdge.halfEdgePair];
                    count++;
                }
                while (he.index != mesh->halfEdges[mesh->vertices[vertex].halfEdge].index);

                if(f){
                    he = mesh->halfEdges[mesh->vertices[vertex].halfEdge];
                    while(he.halfEdgePair!=-1){
                        HalfEdge pairEdge = mesh->halfEdges[he.halfEdgePair];
                        newPos+=mesh->vertices[pairEdge.head].position;
                        he = mesh->halfEdges[mesh->halfEdges[pairEdge.halfEdgeNext].halfEdgeNext];
                        count++;
                    }
                }

                float x = (3.0+2.0*cos(glm::two_pi<float>()/count));
                float a = (x*x)/32.0f - 0.25f;
                float b = (1-a)/count;
                
                newPos*=a;
                newPos+=mesh->vertices[vertex].position*b;
                mesh->vertices[vertex].position = newPos;
            }


        }

    }
}
