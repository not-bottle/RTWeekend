#ifndef MESH_H
#define MESH_H

#include "vec3.h"
#include "rtw_stb_image.h"

#include <string>
#include <vector>
#include <memory>

struct Vertex {
    // position
    point3 Position;
    // normal
    vec3 Normal;
    // tex coords
    vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
    std::shared_ptr<rtw_image> texture;
};

class Mesh {
public:
    // mesh Data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
    }

    // Not a great way to smooth out vertex normals on imported model. 
    // It works but I don't really need this because assimp already has this option on import
    void smoothVertexNormals() {
        std::vector<std::tuple<Vertex, int, std::vector<vec3>>> vertex_normal_acc{};

        // Count vertices
        for (Vertex& vertex : vertices) {
            bool found = false;
            for (std::tuple<Vertex, int, std::vector<vec3>>& acc : vertex_normal_acc) {
                if (std::get<0>(acc).Position == vertex.Position) {
                    std::get<0>(acc).Normal += vertex.Normal;
                    std::get<1>(acc) += 1;
                    std::get<2>(acc).push_back(vertex.Normal);
                    found = true;
                }
            }
            if (!found) {
                vertex_normal_acc.push_back(std::tuple<Vertex, int, std::vector<vec3>>(vertex, 1, std::vector<vec3>()));
            }
        }

        // Set vertex normals to average
        for (Vertex& vertex : vertices) {
            for (std::tuple<Vertex, int, std::vector<vec3>>& acc : vertex_normal_acc) {
                if (std::get<0>(acc).Position == vertex.Position) {
                    vertex.Normal = std::get<0>(acc).Normal / std::get<1>(acc);
                }
            }
        }

        for (std::tuple<Vertex, int, std::vector<vec3>>& acc : vertex_normal_acc) {
            std::cerr << "Vertex: " << std::get<0>(acc).Position << std::endl;
            for (vec3 v : std::get<2>(acc)) {
                std::cerr << "\t" << v << std::endl;
            }
        }
    }
};

#endif // MESH_H