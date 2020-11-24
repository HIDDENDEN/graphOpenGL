//Когда мы рендерим сцену для источника света, нам нужны только координаты вершин и ничего больше
//Для каждой модели этот шейдер отображает вершины модели в пространство источника света с помощью lightSpaceMatrix
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    //нам нужны только координаты вершин и ничего больше
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    //отображаем вершины модели в пространство источника света с помощью lightSpaceMatrix
}
//Местоположение текстуры чаще называется !!!текстурным блоком!!!
