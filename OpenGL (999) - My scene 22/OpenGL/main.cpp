#include <iostream>
#include <cmath>
#include <vector>
#include <map>


// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

#include "SOIL2/SOIL2.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MATRIX_INIT.h"
#include "SHADER.h"
#include "CAMERA.h"

// Window dimensions
//const GLuint WIDTH = 800, HEIGHT = 600;
const GLuint WIDTH = 1440, HEIGHT = 900;

//for parallax mapping
float heightScale = 0.1;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

//Камера
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];//буфер нажатых клавиш
GLfloat deltaTime = 0.0f;    // Время, прошедшее между последним и текущим кадром
GLfloat lastFrame = 0.0f;      // Время вывода последнего кадра

//для мыши
// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat yaw   = -90.0f;//движения при фиксированной оси OY
//pitch - движения при фиксированной оси OZ
GLfloat pitch =   0.0f;//хранение углов камеры для мышиx

GLfloat lastX =  WIDTH  / 2.0;//последние координаты мыши
GLfloat lastY =  HEIGHT / 2.0;

//Lighting attributes
//glm::vec3 lightPos(0.5f, 1.0f, 0.3f);//Положение источника света
glm::vec3 lightPos(3.0f, 4.0f,-1.0f);

// meshes
unsigned int planeVAO;

//GLfloat fov =  45.0f;//угол обзора камеры

//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void key_callback(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void Do_movement();
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);

void renderScene( Shader &shader);
void renderCube();
void renderQuad();


GLFWwindow* global_window;

int main()
{
    //СОЗДАНИЕ ОКНА
    
    int screenWidth,screenHeight;
    
    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    
    GLFWwindow *window=glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL", nullptr, nullptr);
    global_window=window;
    {
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    
    if (nullptr==window)
    {
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    //glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glewExperimental=GL_TRUE;
    
    if ( GLEW_OK != glewInit( ) )
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }
    
    glViewport(0, 0, screenWidth, screenHeight);//преобразуем координаты отсечения от -1.0 до 1.0 в область экранных координат
    }
    
    //say to OpenGL to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // build and compile shaders
    // -------------------------
    //lightning
    Shader lightingShader("shaders/lighting.vs","shaders/lighting.frag");
    Shader lampShader("shaders/lamp.vs","shaders/lamp.frag");
    
    //posteffect
    Shader posteffect_shader("shaders/framebuffers.vs", "shaders/framebuffers.frag");
    Shader screen_posteffect_Shader("shaders/framebuffers_screen.vs", "shaders/framebuffers_screen.frag");
    
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.frag");
    
    // LIGHTNING BEGIN
    
    //координаты куба
    GLfloat vertices[] =
    {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    
    GLuint VBO,BigCubeVAO;//кубик с эмиссией
     //,EBO;
     
     /*
     //VAO-vertex array obects
     //Объект вершинного массива
     
     //VBO-vertex buffer objects
     //объекты вершинного буффера
     //могут хранить большое количество вершин в памяти GPU
     
     //(VAO) может быть также привязан как и VBO и после этого все последующие вызовы вершинных атрибутов будут храниться в VAO
     // кароче VAO - это структура указателей на (первые)объекты VBO,(а потом просто пошагово двигается по объектам VBO, т.к. расстояние между ними задано)
      */
     glGenVertexArrays(1,&BigCubeVAO);
     glGenBuffers(1,&VBO);//создание VBO буффера
     //glGenBuffers(1, &EBO);
     
     
     glBindBuffer(GL_ARRAY_BUFFER,VBO);//привязать GL_ARRAY_BUFFER к нашему буферу VBO

     glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
     //копирования вершинных данных в этот буфер
     /*
     //vertices- это сами данные, которые копируем
     //GL_STATIC_DRAW-данные либо никогда не будут изменяться, либо будут изменяться очень редко
     
     
      указываем OpenGL как он должен интерпретировать вершинные данные
      
      Первый аргумент описывает какой аргумент шейдера мы хотим настроить. Мы хотим специфицировать значение аргумента position, позиция которого была указана следующим образом: layout (location = 0).
      
      Следующий аргумент описывает размер аргумента в шейдере. Поскольку мы использовали vec3 то мы указываем 3
      
      Третий аргумент описывает используемый тип данных. Мы указываем GL_FLOAT, поскольку vec в шейдере использует числа с плавающей точкой
      
      Четвертый аргумент указывает необходимость нормализовать входные данные. Если мы укажем GL_TRUE, то все данные будут расположены между 0 (-1 для знаковых значений) и 1. Нам нормализация не требуется, поэтому мы оставляем GL_FALSE
      
      Пятый аргумент называется шагом и описывает расстояние между наборами данных. Мы также можем указать шаг равный 0 и тогда OpenGL высчитает шаг (работает только с плотно упакованными наборами данных)
      Последний параметр имеет тип GLvoid* и поэтому требует такое странное приведение типов. Это смещение начала данных в буфере. У нас буфер не имеет смещения и поэтому мы указываем 0.
      */
     
     glBindVertexArray(BigCubeVAO);
     //position attribute
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
     glEnableVertexAttribArray(0);
     //normal attribute
     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
     glEnableVertexAttribArray(1);
     //texture attribute
     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
     glEnableVertexAttribArray(2);

     /*
     //Атрибут с координатами
     glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE, 5*sizeof(GLfloat),(GLvoid*)0);
     //glVertexbPointer(0,3,GL_FLOAT,GL_FALSE, 8*sizeof(GLfloat),(GLvoid*)0);
     //0 - это layout
     //3 - т.к. у нас 3 подряд координаты хранятся
     //6*"sizeof(GLfloat)"
     //6 т.к. теперь у нас 6 позиций
     //и через каждые 6 позиций новые элементы массива расположены
     glEnableVertexAttribArray(0);//ВКЛЮЧАЕМ АТРИБУТ. Таким образом мы передадим вершинному атрибуту позицию аргумента
     /*
     Атрибут с цветом
     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
     1 - это layout
     3-т.к. у нас 3 цвета подрад хранятся
     glEnableVertexAttribArray(1);
     /*
     
     glBindVertexArray(0);
     
     Аттрибут с текстурами
     
     //создадим текстуру - (массив GLuint в котором будут храниться идентификаторы этих текстур)
     //GLuint texture;
     //glGenTextures(1, &texture);
     //1-количество текстур для генерации
     //texture-массив GLuint в котором будут храниться идентификаторы текстур
     
     //Привяжем объект для того, чтобы функции, использующие текстуры, знали какую текстуру использовать
     //glBindTexture(GL_TEXTURE_2D, texture);
     
     //ЗАГРУЗКА 2-Х ТЕКСТУР
     
     //TEXTURE 1
     GLuint texture1;
     glGenTextures(1, &texture1);
     glBindTexture(GL_TEXTURE_2D, texture1);// All upcoming GL_TEXTURE_2D operations now have effect on our texture object
     
     // Set our texture parameters
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     
     // Set texture wrapping to GL_REPEAT
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     
     // Set texture filtering
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     
    //Загрузка изображения
    unsigned char* image=SOIL_load_image("res/images/container.jpg",&screenWidth,
                                         &screenHeight,0,SOIL_LOAD_RGB);
    /*
     Первый аргумент функции — это местоположение файла изображения.
     Второй и третий аргументы — это указатели на int в которые будут помещены размеры изображения: ширина и высота. Они нам понадобятся для генерации текстуры.
     Четвертый аргумент — это количество каналов изображения, но мы оставим там просто 0.
     Пятый аргумент сообщает SOIL как ему загружать изображение: нам нужна только RGB информация изображения. Результат будет храниться в большом массиве байт.
     */
     
    // Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
     GLuint lightVAO;
     glGenVertexArrays(1, &lightVAO);
     glBindVertexArray(lightVAO);
     // We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
     glBindBuffer(GL_ARRAY_BUFFER, VBO);
     // Set the vertex attributes (only position data for the lamp))
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the normal vectors
     glEnableVertexAttribArray(0);
     glBindVertexArray(0);
     
     unsigned int diffuseMap=loadTexture("res/images/container2.png");
    //unsigned int diffuseMap=loadTexture("my textures/wood.jpg");
     //unsigned int specularMap =
                     //loadTexture("res/images/lighting_maps_specular_color.png");
    unsigned int specularMap =
    loadTexture("res/images/container2_specular.png");
    

     //unsigned int emissionMap =
                     //loadTexture("res/images/matrix.jpg");
     unsigned int emissionMap =
                   loadTexture("my textures/emission_rithm.jpg");
    
    // shader configuration
    // --------------------
    lightingShader.Use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("material.emission", 2);
    
    // LIGHTNING END
    
    // POSTEFFECT BEGIN
    
    //координаты куба
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    float planeVertices[] = {
        // positions          // texture Coords
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };
    float quadVertices[] =
    { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    /*
     в этом тесте квад в 2 раза меньше размера окна
    float quadVertices[] =
    { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,

        -0.5f,  0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f
    };
     */
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    // load textures
    // -------------
    //unsigned int cubeTexture =loadTexture("res/images/container.jpg");
    unsigned int cubeTexture =loadTexture("my textures/magma.jpg");
    
    //unsigned int floorTexture =loadTexture("res/images/container2.png");
    unsigned int floorTexture =loadTexture("my textures/epic_cube.jpeg");
    

    
    // shader configuration
    // --------------------
    posteffect_shader.Use();
    posteffect_shader.setInt("texture1", 0);
    
    screen_posteffect_Shader.Use();
    screen_posteffect_Shader.setInt("screenTexture", 0);
    
    // framebuffer configuration
    // -------------------------
    //создание объекта кадрового буффера
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);//привязка
    //GL_FRAMEBUFFER - точка привязки
    
    // создадим текстурный объект, который присоединим к прикреплению цвета кадрового буфера
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);//null - потомучто мы просто запросили память, но пока ничего не записываем
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //прикрепляем объект текстуры к буферу кадров
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    // !!! textureColorbuffer - туда мы заполням цвет, а потом заливаем это на quad экрана
    
    unsigned int rbo;//render buffer (типа текстуры, только удобнее)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);// use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // POSTEFFECT END
    
    
    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    
    
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    
    /*
     lesson's skybox
    std::vector<std::string> faces
       {
           "skybox/right.jpg",
           "skybox/left.jpg",
           "skybox/top.jpg",
           "skybox/bottom.jpg",
           "skybox/front.jpg",
           "skybox/back.jpg"
       };
    */
    /*
    std::vector<std::string> faces
    {
       
        "myskybox/left.png", "myskybox/right.png",
        "myskybox/top.png",
        "myskybox/bottom.png",
        "myskybox/front.png",
        "myskybox/back.png"
    };
    */
    // +X (right)
    // -X (left)
    // +Y (top)
    // -Y (bottom)
    // +Z (front)
    // -Z (back)
    /*std::vector<std::string> faces
    {
       
        "myskybox2/posx.jpg",
        "myskybox2/negx.jpg",
        "myskybox2/posy.jpg",
        "myskybox2/negy.jpg",
        "myskybox2/posz.jpg",
        "myskybox2/negz.jpg"
    };*/
    std::vector<std::string> faces
    {
        "skybox/front.jpg",
        "skybox/back.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/right.jpg",
        "skybox/left.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    
    
    
    skyboxShader.Use();
    skyboxShader.setInt("skybox", 0);
    
    // render loop
    // -----------
    
    while(!glfwWindowShouldClose(window))
    {
        
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        glfwPollEvents();//обабатываем события
        key_callback(window);
        //Do_movement();//обрабатываем перемещения(нажатие WASD)

        // change light position over time
        //lightPos.x = sin(glfwGetTime()) * 3.0f;
        //lightPos.z = cos(glfwGetTime()) * 2.0f;
        //lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;
        
        /*
        !!!
        порядок действий при рендере сцены, содержащей как непрозрачные, так и прозрачные объекты, выглядит следующим образом:

        Вывести все непрозрачные объекты.
        Отсортировать прозрачные объекты по удалению.
        Нарисовать прозрачные объекты в отсортированном порядке.
        */
        
        //сортируем окна по удаленности и будем выводить от дальнего к ближнему, чтобы через ближние было видно дальние
        // sort the transparent windows before rendering
        // ---------------------------------------------
        
        
        
        // render
        // ------
        // make sure we clear the framebuffer's content
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // POPIXEL BEGIN
         
        // render
         // ------
         //
        //привязываем наш другой буфер и рендерим, как обычно в цветную текстуру
         glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
         glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

         // make sure we clear the framebuffer's content
         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
         //отчистка буфера цветов,буфера глубины,ТРАФАРЕТНОГО БУФЕРА
         
         // set uniforms
         posteffect_shader.Use();
         
         glm::mat4 model ; matrix_init4(model);
         glm::mat4 view ;  matrix_init4(view);
         glm::mat4 projection; matrix_init4(projection);
        
         
         model = glm::mat4(1.0f);
         view = camera.GetViewMatrix();
         projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH/ (float)HEIGHT, 0.1f, 100.0f);
         posteffect_shader.setMat4("view", view);
         posteffect_shader.setMat4("projection", projection);

         posteffect_shader.Use();
         posteffect_shader.setMat4("view", view);
         posteffect_shader.setMat4("projection", projection);
         
         /*
          шаги, необходимые для вывода сцены в текстуру:

          1. Привязать наш объект буфера кадра как текущий и вывести сцену обычным образом.
          2. Привязать буфер кадра по умолчанию.
          3. Вывести полноэкранный квад с наложением текстуры из буфера цвета нашего объекта кадрового буфера.

          */
         
         // cubes
         glBindVertexArray(cubeVAO);
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, cubeTexture);
         model = glm::translate(model, glm::vec3(-4.0f, 0.0f, 0.0f));
         posteffect_shader.setMat4("model", model);
         glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 4.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 0.0f, 0.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //mid
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 2.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, -2.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 2.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -2.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        //centre
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 1.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, -1.0f));
        posteffect_shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
         // floor
         glBindVertexArray(planeVAO);
         glBindTexture(GL_TEXTURE_2D, floorTexture);
         model = glm::mat4(1.0f);
         posteffect_shader.setMat4("model", model);
         glDrawArrays(GL_TRIANGLES, 0, 6);
         glBindVertexArray(0);
         
         // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
         glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
         // clear all relevant buffers
         glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
         glClear(GL_COLOR_BUFFER_BIT);

         screen_posteffect_Shader.Use();
         glBindVertexArray(quadVAO);
         glBindTexture(GL_TEXTURE_2D, textureColorbuffer);    // use the color attachment texture as the texture of the quad plane
         glDrawArrays(GL_TRIANGLES, 0, 6);
         // POPIXEL END

        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        
        //lamp move begin
            lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
            lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f +3.0f;
            lightPos.z = -1.0f + 1.0f;
            //lightPos.z = sin(glfwGetTime()/3.0f) * 3.0f;
            //lamp move end

            // Use cooresponding shader when setting uniforms/drawing objects
            lightingShader.Use();
            lightingShader.setVec3("light.position", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);
            
            //time uniform
            glUniform1f(glGetUniformLocation(lightingShader.Program, "time"), (glfwGetTime()));
            
            // light properties
            /*
            glm::vec3 lightColor;
            lightColor.x = sin(glfwGetTime() * 2.0f) / 2.0f + 0.5f;
            lightColor.y = sin(glfwGetTime() * 0.7f) / 2.0f + 0.5f;
            lightColor.z = sin(glfwGetTime() * 1.3f) / 2.0f + 0.5f;//делим на 2.0f для нормировки и сдвигаем на 0.5f, чтобы цвет отрицательным не был
            
            glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
            glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
            lightingShader.setVec3("light.ambient", ambientColor);
            lightingShader.setVec3("light.diffuse", diffuseColor);
            lightingShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
            */
            lightingShader.setVec3("light.ambient",glm::vec3(0.2f, 0.2f, 0.2f));
            lightingShader.setVec3("light.diffuse",glm::vec3(0.5f, 0.5f, 0.5f));
            lightingShader.setVec3("light.specular",glm::vec3(1.0f, 1.0f, 1.0f));
            
            // material properties
            lightingShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f)); // specular lighting doesn't have full effect on this object's material
            lightingShader.setFloat("material.shininess", 32.0f);
            //lightingShader.setVec3("material.ambient", glm::vec3(0.19225f, 0.19225f, 0.19225f));
            //lightingShader.setVec3("material.diffuse", glm::vec3(0.50754f, 0.50754f, 0.50754f));
            //lightingShader.setVec3("material.specular", glm::vec3(0.508273f, 0.508273f, 0.508273f));
            //lightingShader.setFloat("material.shininess", 0.4f);
            
            /*
             GLint objectColorLoc = glGetUniformLocation(lightingShader.Program, "objectColor");
            GLint lightColorLoc  = glGetUniformLocation(lightingShader.Program, "lightColor");
            GLint lightPosLoc    = glGetUniformLocation(lightingShader.Program, "lightPos");
            glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
            glUniform3f(lightColorLoc,  1.0f, 1.0f, 1.0f);
            glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
            
            GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
            glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
            */
             /*
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);
            glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

            ourShader.Use();
            */
            
            // Camera/View transformation
            //glm::mat4 view;
            matrix_init4(view);
            view = camera.GetViewMatrix();
            /*
             GLfloat radius=10.f;
            GLfloat camX = sin(glfwGetTime()) * radius;
            GLfloat camZ = cos(glfwGetTime()) * radius;
             //view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));//сделали матрицу вида
             */
            //view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            /*
             Сначала мы устанавливаем камеру в позицию, координата которой хранится в переменной cameraPos. Направление камеры вычисляется как её текущая позиция + вектор cameraFront, который мы только что объявили и инициализировали. Это гарантирует, что не смотря на перемещение камеры, она всегда будет смотреть в направлении цели
             */
            
            
            //МАТРИЦА ПРОЕКЦИИ(переспективную)
            //glm::mat4 projection;
            matrix_init4(projection);
            projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH/(GLfloat)HEIGHT, 0.1f, 100.0f);
            
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);
            
            // world transformation
            //glm::mat4 model;
            matrix_init4(model);
            model= glm::mat4(1.0f);
            //model = glm::translate(model, glm::vec3 (1.0f,3.0f,2.0f));//сдвинул кубик
            model = glm::translate(model, glm::vec3 (1.0f*sin(glfwGetTime()),3.0f,2.0f*sin(glfwGetTime())));//сдвинул кубик
            lightingShader.setMat4("model", model);
            
            // bind diffuse map - карта интенсивности света на каждый уч-к пов-сти
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            
            // bind specular map - карта бликов
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMap);
            
            // bind emission map
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, emissionMap);
            
            // render the cube
            glBindVertexArray(BigCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // also draw the lamp object
            lampShader.Use();
            lampShader.setMat4("projection", projection);
            lampShader.setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
            lampShader.setMat4("model", model);

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            /*
            GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
            GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
            GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
            
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            
            // Draw the container (using container's vertex attributes)
            glBindVertexArray(containerVAO);
           
            glm::mat4 model;
            matrix_init4(model);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            /*
            for (GLuint i=0;i<10;i++)//отрисовка 10ти кубов
            {
                glm::mat4 model;
                matrix_init4(model);
                model = glm::translate(model, cubePositions[i]);
                GLfloat angle=glm::radians(20.0f)*i;
                //if (i%3==0)
                    angle=glfwGetTime()*glm::radians(40.0f);
                
                model = glm::rotate(model,angle, glm::vec3(1.0f, 0.3f, 0.5f));
                
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            */
            /*
             glDrawArrays(GL_TRIANGLES, 0, 36);
            //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            //6 = 3 вершины + 3 цвета
             */
            /*
            // Also draw the lamp object, again binding the appropriate shader
            lampShader.Use();
            // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
            modelLoc = glGetUniformLocation(lampShader.Program, "model");
            viewLoc  = glGetUniformLocation(lampShader.Program, "view");
            projLoc  = glGetUniformLocation(lampShader.Program, "projection");
            // Set matrices
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            //model = glm::mat4();
            matrix_init4(model);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            // Draw the light object (using light's vertex attributes)
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        */

        
        
        
         
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    
    
    
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
    glDeleteVertexArrays(1,&BigCubeVAO);
    glDeleteBuffers(1,&VBO);
    
    glfwTerminate();
    
    return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        
        glm::vec3 pos1(-1.0f,  1.0f, -3.0f);
        glm::vec3 pos2(-1.0f, -1.0f, -3.0f);
        glm::vec3 pos3( 1.0f, -1.0f, -3.0f);
        glm::vec3 pos4( 1.0f,  1.0f, -3.0f);
        
        /*
        glm::vec3 pos1(-1.0f, 0.0f, 1.0f);
        glm::vec3 pos2(-1.0f, 0.0f, -1.0f);
        glm::vec3 pos3( 1.0f, 0.0f, -1.0f);
        glm::vec3 pos4( 1.0f, 0.0f, 1.0f);
         */
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);
        //glm::vec3 nm(0.0f,1.0f,0.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}




// renders the 3D scene
// --------------------

void renderScene(Shader &shader)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
    shader.setMat4("model", model);
    renderCube();
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
/*
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
*/



// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }
}


bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Обратный порядок вычитания потому что оконные Y-координаты возрастают с верху вниз
    
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int screenWidth,screenHeight;
    glfwGetFramebufferSize(global_window, &screenWidth, &screenHeight);
    
    //Загрузка изображения
    unsigned char* image=SOIL_load_image(path,&screenWidth,
                                         &screenHeight,0,SOIL_LOAD_RGBA);
    std::cout<<"Loaded succsessfully: "<<path<<std::endl;
    if (image)
    {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(image);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        SOIL_free_image_data(image);
    }
    
    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *image = SOIL_load_image(faces[i].c_str(),
                                         &width, &height, &nrChannels, 0);
        
        if (image)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            SOIL_free_image_data(image);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            SOIL_free_image_data(image);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
