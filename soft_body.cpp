#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <cstddef>

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct SimpleVertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Particle
{
    glm::vec3 position;
    glm::vec3 previousPosition;
    glm::vec3 velocity;
    float invMass;
};

struct DistanceConstraint
{
    int p0;
    int p1;
    float restLength;
};

struct VolumeConstraint {
    int p0, p1, p2, p3;
    float restVolume;
};
float calculateVolume(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    return glm::dot(p1 - p0, glm::cross(p2 - p0, p3 - p0)) / 6.0f;
}
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
    Shader simpleShader("simple.vs", "simple.fs");

    std::vector<SimpleVertex> tetrahedronVertices = {

    { glm::vec3(0.0f,  0.5f,  0.0f), glm::vec3(0.0f,  0.447f,  0.894f) },
    { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f,  0.447f,  0.894f) },
    { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f,  0.447f,  0.894f) },

    { glm::vec3(0.0f,  0.5f,  0.0f), glm::vec3(0.816f, 0.447f, -0.408f) },
    { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.816f, 0.447f, -0.408f) },
    { glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.816f, 0.447f, -0.408f) },

    { glm::vec3(0.0f,  0.5f,  0.0f), glm::vec3(-0.816f, 0.447f, -0.408f) },
    { glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(-0.816f, 0.447f, -0.408f) },
    { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.816f, 0.447f, -0.408f) },

    { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f) },
    { glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f) },
    { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f) },
    };

    // Generate VAO and VBO
    unsigned int tetraVAO, tetraVBO;
    glGenVertexArrays(1, &tetraVAO);
    glGenBuffers(1, &tetraVBO);

    // Bind VAO first
    glBindVertexArray(tetraVAO);

    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, tetraVBO);
    glBufferData(GL_ARRAY_BUFFER,
        tetrahedronVertices.size() * sizeof(SimpleVertex),
        tetrahedronVertices.data(),
        GL_DYNAMIC_DRAW);

    // ---- Vertex Attribute Setup ----

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(SimpleVertex),
        (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(SimpleVertex),
        (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);

    


    // ---- Plane ----

    // plane
    float planeVertices[] = {
        // positions            // normals
         5.0f, -3.5f,  5.0f,    0.0f, 1.0f, 0.0f,
        -5.0f, -3.5f,  5.0f,    0.0f, 1.0f, 0.0f,
        -5.0f, -3.5f, -5.0f,    0.0f, 1.0f, 0.0f,

         5.0f, -3.5f,  5.0f,    0.0f, 1.0f, 0.0f,
        -5.0f, -3.5f, -5.0f,    0.0f, 1.0f, 0.0f,
         5.0f, -3.5f, -5.0f,    0.0f, 1.0f, 0.0f
    };

    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::vector<Particle> particles =
    {
        { glm::vec3(0.0f,  -0.5f,  0.0f), glm::vec3(0.0f,  0.5f,  0.0f), glm::vec3(0.0f), 1.0f },
        { glm::vec3(-0.5f, 0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f), 1.0f },
        { glm::vec3(0.5f, 0.5f,  0.5f), glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f), 1.0f },
        { glm::vec3(0.0f, 0.5f, -0.5f), glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.0f), 1.0f }
    };

    std::vector<DistanceConstraint> constraints;

    // helper to create constraint
    auto addConstraint = [&](int i, int j)
        {
            float rest = glm::length(particles[i].position - particles[j].position);
            constraints.push_back({ i, j, rest });
        };

    // tetrahedron edges
    addConstraint(0, 1);
    addConstraint(0, 2);
    addConstraint(0, 3);
    addConstraint(1, 2);
    addConstraint(1, 3);
    addConstraint(2, 3);

    // V0
    float restVolume = glm::dot(particles[1].position - particles[0].position,
        glm::cross(particles[2].position - particles[0].position,
            particles[3].position - particles[0].position)) / 6.0f;

    glm::vec3 gravity(0.0f, -1.0f, 0.0f);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        float dt = deltaTime;


        // calculate position by velocity
        for (auto& p : particles)
        {
            if (p.invMass == 0.0f) continue;

            p.velocity += gravity * dt;
            p.previousPosition = p.position;
            p.position += p.velocity * dt;
        }

        // Presolve
        // detect ground
        for (auto& p : particles)
        {
            float groundY = -3.5f;

            if (p.position.y < groundY)
            {
                p.position.y = groundY;
            }
        }

        int solverIterations = 1;   // more = stiffer


        for (int iter = 0; iter < solverIterations; ++iter)
        {
            // Distance Constraint
            for (auto& c : constraints)
            {
                Particle& p0 = particles[c.p0];
                Particle& p1 = particles[c.p1];

                glm::vec3 delta = p0.position - p1.position;
                
                float l = glm::length(delta);

                if (l < 1e-6f) continue; // Prevent division by zero

                glm::vec3 gradC1 = (p0.position - p1.position) / glm::length(p0.position - p1.position);
                glm::vec3 gradC2 = (p1.position - p0.position) / glm::length(p1.position - p0.position);

                float l0 = c.restLength;
                float w1 = p0.invMass;
                float w2 = p1.invMass;
                float wSum = w1 + w2;

                if (wSum < 1e-6f) continue;

                float complianceDist = 0.03f;
                float alphaTerm = complianceDist / (deltaTime * deltaTime);
                float lambda = -(l - l0) / (wSum + alphaTerm);

                p0.position += (lambda * w1 * gradC1);
                p1.position += (lambda * w2 * gradC2);

            }

            // Volume Constraint
            

            Particle& p0 = particles[0];
            Particle& p1 = particles[1];
            Particle& p2 = particles[2];
            Particle& p3 = particles[3];

            // solve constraint by finding gradient 
            glm::vec3 grad0 = glm::cross(p2.position - p1.position, p3.position - p1.position);
            glm::vec3 grad1 = glm::cross(p3.position - p0.position, p2.position - p0.position);
            glm::vec3 grad2 = glm::cross(p0.position - p1.position, p3.position - p1.position);
            glm::vec3 grad3 = glm::cross(p1.position - p0.position, p2.position - p0.position);

            // V
            float currentVol = glm::dot
            (
                p1.position - p0.position,
                glm::cross(p2.position - p0.position, p3.position - p0.position)
            ) / 6.0f;

            // V - V0
            float C = currentVol - restVolume;

            float wSum = (p0.invMass * glm::dot(grad0, grad0)) +
                (p1.invMass * glm::dot(grad1, grad1)) +
                (p2.invMass * glm::dot(grad2, grad2)) +
                (p3.invMass * glm::dot(grad3, grad3));

            float complianceVol = 0.9f;
            float alphaVol = complianceVol / (dt * dt);
            float lambda = -6.0 * C / (wSum + alphaVol);

            if (glm::abs(wSum) > 1e-6f)
            {

                p0.position += lambda * p0.invMass * grad0;
                p1.position += lambda * p1.invMass * grad1;
                p2.position += lambda * p2.invMass * grad2;
                p3.position += lambda * p3.invMass * grad3;
            }

            
        }

        for (auto& p : particles)
        {
            p.velocity = (p.position - p.previousPosition) / dt;
        }

        // Update vertices using particles
        // Front face
        tetrahedronVertices[0].Position = particles[0].position;
        tetrahedronVertices[1].Position = particles[1].position;
        tetrahedronVertices[2].Position = particles[2].position;

        // Right
        tetrahedronVertices[3].Position = particles[0].position;
        tetrahedronVertices[4].Position = particles[2].position;
        tetrahedronVertices[5].Position = particles[3].position;

        // Left
        tetrahedronVertices[6].Position = particles[0].position;
        tetrahedronVertices[7].Position = particles[3].position;
        tetrahedronVertices[8].Position = particles[1].position;

        // Bottom
        tetrahedronVertices[9].Position = particles[1].position;
        tetrahedronVertices[10].Position = particles[3].position;
        tetrahedronVertices[11].Position = particles[2].position;

        glBindBuffer(GL_ARRAY_BUFFER, tetraVBO);
        glBufferSubData(GL_ARRAY_BUFFER,
            0,
            tetrahedronVertices.size() * sizeof(SimpleVertex),
            tetrahedronVertices.data());

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        simpleShader.use();

        simpleShader.setMat4("projection", projection);
        simpleShader.setMat4("view", view);

        simpleShader.setVec3("lightColor", glm::vec3(1.0f));
        simpleShader.setVec3("lightDir", glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)));

        model = glm::mat4(1.0f);
        simpleShader.setMat4("model", model);
        simpleShader.setVec3("objectColor", glm::vec3(0.6f, 0.6f, 0.6f));

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = glm::mat4(1.0f);

        simpleShader.setMat4("model", model);
        simpleShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.2f));

        glBindVertexArray(tetraVAO);
        glDrawArrays(GL_TRIANGLES, 0, 12);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
