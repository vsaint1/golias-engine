#pragma once

#include "helpers/logging.h"

/*! @brief Vertex struct

    @version 0.0.3
*/
struct Vertex {
    glm::vec2 position;
    glm::vec2 tex_coord;
    glm::vec4 color;
};


/*! @brief Mesh class
    - A mesh is a collection of vertices and indices
    - VAO, VBO and EBO related to the mesh
    - Material (SOON)

    @version 0.0.3
*/
class Mesh {
public:
    Mesh();

    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<Uint32>& indices = {});

    ~Mesh();

    void Bind() const;

    void Draw(unsigned int mode = GL_TRIANGLES) const;

    void Update(const std::vector<Vertex>& new_vertices, const std::vector<Uint32>& new_indices = {});

    size_t GetVertexCount() const;

private:
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;

    void Setup();

    bool bIsDirty = false;
};

