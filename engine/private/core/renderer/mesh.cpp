#include "core/renderer/mesh.h"
//
// Mesh::Mesh() : VAO(0), VBO(0), EBO(0) {
//     glGenVertexArrays(1, &VAO);
//     glGenBuffers(1, &VBO);
//     glBindVertexArray(VAO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Position));
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, UV));
//     glBindVertexArray(0);
// }
//
// Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Uint32>& indices)
//     : vertices(vertices), indices(indices), bIsDirty(true) {
//     Setup();
// }
//
// Mesh::~Mesh() {
//     glDeleteVertexArrays(1, &VAO);
//     glDeleteBuffers(1, &VBO);
//     if (EBO) {
//         glDeleteBuffers(1, &EBO);
//     }
// }
//
// void Mesh::Bind() const {
//     glBindVertexArray(VAO);
// }
//
// size_t Mesh::GetVertexCount() const {
//     return vertices.size();
// }
//
// void Mesh::Draw(unsigned int mode) const {
//     glBindVertexArray(VAO);
//
//     if (!indices.empty()) {
//         glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);
//     } else {
//         glDrawArrays(mode, 0, vertices.size());
//     }
//
//     glBindVertexArray(0);
// }
//
// void Mesh::Update(const std::vector<Vertex>& new_vertices, const std::vector<Uint32>& new_indices) {
//     bool vertex_changed = (new_vertices.size() != vertices.size());
//     bool index_changed = (new_indices.size() != indices.size());
//
//     if (vertex_changed) {
//         vertices = new_vertices;
//     }
//
//     if (index_changed) {
//         indices = new_indices;
//     }
//
//     if (vertex_changed) {
//         glBindBuffer(GL_ARRAY_BUFFER, VBO);
//         glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
//     }
//
//     if (index_changed && !indices.empty()) {
//         if (EBO == 0) {
//             glGenBuffers(1, &EBO);
//         }
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_STATIC_DRAW);
//     }
//
//     bIsDirty = false;
// }
//
// void Mesh::Setup() {
//     glGenVertexArrays(1, &VAO);
//     glGenBuffers(1, &VBO);
//     glGenBuffers(1, &EBO);
//
//     glBindVertexArray(VAO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
//
//     if (!indices.empty()) {
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_STATIC_DRAW);
//     }
//
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Position));
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, UV));
//     glEnableVertexAttribArray(1);
//
//     glBindVertexArray(0);
// }
