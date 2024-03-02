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

            position = glm::vec3(0.0f, 0.0f,  1.5f);
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
        void traverseNeighbouringTriangles(Vertex* v){

        }


        Mesh* Mesh::createSqaure(int rows, int columns){
            Mesh* mesh;
            for (int i=0; i<rows; i++){
                for (int j=0; j<columns; j++){
                    // Add two triangles
                    Face f;

                    mesh->faces.push_back(f);
                }
            }
            // connect the faces 
            return mesh;
        }

        Mesh* Mesh::createSphere(int longitudes, int latitudes){
            Mesh* mesh;
            return mesh;
        }

        // std::vector<glm::vec4> Mesh::getVertices(Mesh mesh){return;}
        // std::vector<glm::vec4> Mesh::getColours(Mesh mesh){return;}
        // std::vector<glm::ivec3> Mesh::getTriangles(Mesh mesh){return;}



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
                        e1.index = edgeCount;
                        e2.index = edgeCount+1;
                        e3.index = edgeCount+2;
                        f.index = faceCount;

                        int i,j,k;
                        iss >> i >> j >> k;
                            
                        if (edgeMap.find(std::pair<int,int>(std::min(j,i), std::max(j,i))) == edgeMap.end()) edgeMap[std::pair<int,int>(std::min(j,i), std::max(j,i))] = e1.index;
                            else { // if found 
                                (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,i), std::max(j,i))]].halfEdgePair = e1.index;
                                e1.halfEdgePair = (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,i), std::max(j,i))]].index;}

                        if (edgeMap.find(std::pair<int,int>(std::min(j,k), std::max(j,k))) == edgeMap.end()) edgeMap[std::pair<int,int>(std::min(j,k), std::max(j,k))] = e1.index;
                            else { // if found 
                                (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,k), std::max(j,k))]].halfEdgePair = e1.index;
                                e1.halfEdgePair = (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(j,k), std::max(j,k))]].index;}

                        if (edgeMap.find(std::pair<int,int>(std::min(k,i), std::max(k,i))) == edgeMap.end()) edgeMap[std::pair<int,int>(std::min(i,k), std::max(i,k))] = e1.index;
                            else { // if found 
                                (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(i,k), std::max(i,k))]].halfEdgePair = e1.index;
                                e1.halfEdgePair = (mesh.halfEdges)[edgeMap[std::pair<int,int>(std::min(i,k), std::max(i,k))]].index;}

                        f.halfEdge = edgeCount;

                        e1.halfEdgeNext = edgeCount+1;
                        e2.halfEdgeNext = edgeCount+2;
                        e3.halfEdgeNext = edgeCount;

                        e1.left = faceCount;
                        e2.left = faceCount;
                        e3.left = faceCount;

                        e1.head = j;
                        e2.head = k;
                        e3.head = i;
                        mesh.vertices[i-1].halfEdge = edgeCount+2;
                        mesh.vertices[j-1].halfEdge = edgeCount;
                        mesh.vertices[k-1].halfEdge = edgeCount+1;

                        (mesh.faces).push_back(f);
                        (mesh.halfEdges).push_back(e1);
                        (mesh.halfEdges).push_back(e2);
                        (mesh.halfEdges).push_back(e3);

                        edgeCount+=3;
                        faceCount++;
                    }
                    // assuming that the object file contains vn first followed by v.
                    else if (type =="vn"){
                        Vertex v;
                        // float x,y,z;
                        iss >> x >> y >> z;
                        v.normal = glm::vec3(x,y,z);
                        v.index = vertexCount;
                        mesh.vertices.push_back(v);
                        normalCount++;
                    }

                    else if (type == "v"){
                        // float x,y,z;
                        if (normalCount==0){
                            Vertex v;
                            // float x,y,z;
                            iss >> x >> y >> z;
                            // std ::cout << x << y << z << "\n";
                            v.position = glm::vec3(x,y,z);
                            v.index = vertexCount;
                            mesh.vertices.push_back(v);
                            vertexCount++;
                        }
                        else {
                            iss >> x >> y >> z;
                            mesh.vertices[vertexCount].position = glm::vec3(x,y,z);
                            vertexCount++;
                        }
                    }
                }
            
        return mesh;
        }

        // recompute normals for part 1.4
        void recomputeVertexNormals(Mesh* mesh){
            for (int i=0; i<(mesh->vertices).size(); i++){
                // for each vertex take the average of normals of each faces
            }

        }

    }
}
