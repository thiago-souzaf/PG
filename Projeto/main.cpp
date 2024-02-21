// includes c++
#include <iostream>
#include <cmath>
#include <vector>

// includes bibliotecas
#include "../External/glm/glm.hpp" // forma de importar o glm.hpp
#include "../External/glm/gtc/matrix_transform.hpp" // essa diretiva é necessária pra executar o código da linha 12

// includes .h e C
#include "./Includes/ray.h"
#include "./Includes/color.h"
#include "./Includes/hitable.h"
#include "./Includes/spheres.h"
#include "./Includes/plane.h"
#include "./Includes/hitable_list.h"
#include "./Includes/camera.h"
#include "./Includes/triangle.h"
#include "./Includes/tmesh.h"
#include "./Includes/material.h"
#include "./Includes/environment.h"
#include "./Includes/light.h"
#include "float.h"
#include "./Tools/MatrixOperations.h"
#include "./Tools/Matrix4X4.h"

#define M_PI 3.14159265358979323846

using std::vector;

// materiais básicos para testes com objetos
//                               d     a     s     r     t     n 
material* matte = new material(0.8f, 0.1f, 0.0f, 0.0f, 0.0f, 0.2f);

// luzes da cena 
// cor branca para o ambiente e luzes locais
color white = color(1,1,1);
Environment* ambientLight = new Environment(white);

Light* light_point1 = new Light(glm::vec3(4,0,-2),white);
Light* light_point2 = new Light(glm::vec3(-3,1,-1),white);

vector<Light*> scene_lights;

// cores basicas para testes com objetos
const color red(1.0f,0.0f,0.0f);
const color green(0.0f,1.0f,0.0f);
const color blue(0.0f,0.0f,1.0f);
// lista de objetos
std::vector<hitable*> lista;

vec3 phong(hit_record rec, color amb_light, vector<Light*> point_lights){

    // parte ambiental da iluminação de phong
    vec3 fator_ambiental = rec.kamb * amb_light;

    // esse sum vai ser o somatorio dos componentes difusos e especulares para cada luz da cena.
    vec3 sum = vec3(0.0f, 0.0f, 0.0f);
    for(Light* cur_light : point_lights){

        // vetor normalizado que sai do ponto de interseção em direção ao ponto de luz 
        vec3 L_dir = normalize(cur_light->getPosition() - rec.p);
        
        // o produto (N . Li) da equação de phong, precisa do clamp para ele não ser negativo e inverter as cores
        float dot_prod = glm::dot(rec.normal, L_dir);
        dot_prod = glm::clamp(dot_prod, 0.0f, 1.0f);

        // parte difusa da iluminação de phong
        vec3 fator_difuso = cur_light->getIntensity() * rec.cor * rec.kdif * dot_prod;

        sum += fator_difuso;
    }
    
    vec3 result = fator_ambiental + sum;
    result = clamp(result, 0.0f, 1.0f);
    
    return result;
}

// função que define a cor que será exibida
color ray_color(const ray& r, hitable *world)
{
    hit_record rec;
    if(world->hit(r, 0.0f, FLT_MAX, rec)){
        return phong(rec, ambientLight->getAmbientLight(), scene_lights);
    }

    color backgroundColor = glm::vec3(0.0,0.0,0.0); // cor preta pro background

    return backgroundColor;
}

void readfile();
// main
int main() {
    readfile();
    // largura e altura da tela respectivamente // resolução
    int nx = 500; // hres
    int ny = 500;  // vres

    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    // localização
    glm::vec3 origin(0.0f, 0.0f, 5.0f);

    // para onde a camera esta olhando
    glm::vec3 lookingat(0.0f, 0.0f, -1.0f);

    // vup
    glm::vec3 vup(0.0f, 1.0f, 0.0f);

    // distancia da camera pra tela pra tela
    float distance = 2.0f;

    float vfov = 100.0f; // Campo de visão vertical em graus
    
    scene_lights.push_back(light_point1);
    scene_lights.push_back(light_point2);
    hitable *world = new hitable_list(lista, lista.size());
    camera *cam = new camera(origin, lookingat, vup, ny, nx, distance,vfov);

    // printando os pixels
    for(int j = ny-1; j >= 0 ; j--)
    {
        for(int i = 0; i < nx; i++)
        {
            float u = float(i) / float(nx);
            float v = float(j)/ float(ny);
            ray r = cam->get_ray(u,v);

            glm::vec3 p = r.point_at_parameter(2.0f);

            color pixel_color = ray_color(r, world);
            write_color(std::cout, pixel_color);
        }
    }


    return 0;
}

void readfile(){

    std::string line;
    std::ifstream myfile ("./cena.txt");

    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            float Or, Og, Ob;
            if(line[0] == 's'){
                float x, y, z, r;
                sscanf(line.c_str(), "s %f %f %f %f %f %f %f", &x, &y, &z, &r, &Or, &Og, &Ob);
                color cor = glm::vec3(Or, Og, Ob);
                lista.push_back(new sphere(glm::vec3(x, y, z), r, cor, matte));
            }
            if(line[0] == 'p'){
                float x, y, z, nx, ny, nz;
                sscanf(line.c_str(), "p %f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &Or, &Og, &Ob);
                color cor = glm::vec3(Or, Og, Ob);
                lista.push_back(new plane(glm::vec3(x, y, z), glm::vec3(nx, ny, nz), cor, matte));
            }
            if(line[0] == 't'){
                // Quantidade de vertices (pontos) na mesh
                int v;
                // Quantidade de triangulos na mesh
                int t;
                sscanf(line.c_str(), "t %d %d", &v, &t);
                // Lista de vértices dos triângulos, cada vec3 representa a posição (x,y,z) de um ponto da mesh no espaço
                vec3 pontos[v];
                // Uma lista com triplas de índices de vértices (cada tripla possui os índices dos vértices (na lista de vértices) que fazem parte de um triângulo)
                triple vertices_index[t];
                for(int i = 0; i < v; i++){
                    std::getline(myfile, line);
                    float x, y, z;
                    sscanf(line.c_str(), "%f %f %f", &x, &y, &z);
                    pontos[i] = vec3(x, y, z);
                }
                for(int i = 0; i < t; i++){
                    std::getline(myfile, line);
                    int x, y, z;
                    sscanf(line.c_str(), "%d %d %d", &x, &y, &z);
                    vertices_index[i] = triple(x, y, z);
                }
                lista.push_back(new tmesh(v, t, pontos, vertices_index, green+red, matte));
            }
        }
        myfile.close();
    }

    else std::cout << "Unable to open file";
}