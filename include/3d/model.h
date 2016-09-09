#ifndef __MODEL__
#define __MODEL__

/**
 * Load models using Assimp
 * @author: Methusael Murmu
 */

#include <base.h>
#include <base_util.h>
#include <base_shader.h>
#include <3d/object.h>
#include <3d/type_common.h>
#include <external/soil/SOIL.h>

#include <external/assimp/Importer.hpp>
#include <external/assimp/scene.h>
#include <external/assimp/postprocess.h>

#include <string>
#include <vector>
#include <algorithm>

#if 1  // 0 to disable to debugging, 1 to enable
#define _MODEL_DEBUG_
#endif

namespace glf3d {

#define MAX_UNIFORM_SAMPLER 2
// Get offset of a struct member
#define getOffset(type, member) (&(((type*)0)->member))

struct Vertex {
    t_v3 position;
    t_v3 normal;
    t_v2 tex_coord;
    t_v3 tangent, bitangent;
};

struct Texture {
    GLuint id;
    glf::TextureType type;
    aiString path;
};

class Mesh;

typedef std::vector<GLuint>  t_vuint;
typedef std::vector<Vertex>  t_vvert;
typedef std::vector<Texture> t_vtex;
typedef std::vector<Mesh>    t_vmesh;

// Prototypes
GLuint loadTextureFromFile(const char* _path, std::string _dir, bool sRGB);

class Mesh {
 public:
    Mesh(const t_vvert& _verts, const t_vuint& _vidx,
         const t_vuint& _tidx, RenderContext* _renderContext):
        vertices(_verts), tex_ids(_tidx), vert_ids(_vidx), mpContext(_renderContext) {
        vAO = vBO = eBO = tex_sz = 0;
        bindMesh();
    }

    ~Mesh() {
        glDeleteBuffers(1, &eBO); glDeleteBuffers(1, &vBO);
        glDeleteBuffers(1, &vAO);
    }

    inline const GLuint VAO() const { return vAO; }

    void render(glf::ShaderTextureInfo* _texinfo, const t_vtex& _textures) {
        for (i = 0; i < glf::TEX_TYPE_SZ; ++i)
            tex_type_c[i] = 0;

        // Load appropriate textures to texture units
        glf::TextureType _type;
        for (i = 0; i < tex_sz; ++i) {
            _type = _textures[tex_ids[i]].type;
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, _textures[tex_ids[i]].id);
            glUniform1i(_texinfo->textureUniformLocation(_type, tex_type_c[_type]++), i);
        }

        glBindVertexArray(vAO);
        if (mpContext->shouldDrawInstanced) {
            glDrawElementsInstanced(
                GL_TRIANGLES, vert_ids.size(), GL_UNSIGNED_INT, 0, mpContext->instanceAmount);
        } else {
            glDrawElements(GL_TRIANGLES, vert_ids.size(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);

        // Messes with texture bindings for shadow maps
#if 0
        // Reset texture units
        for (i = 0; i < glf::TEX_TYPE_SZ; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
#endif
    }

    t_vvert             vertices;
    std::vector<GLuint> tex_ids;
    std::vector<GLuint> vert_ids;

 private:
    GLuint vAO, vBO, eBO;
    RenderContext* mpContext;

    // tex_type:    used as counters for texture types during render
    GLint i, tex_sz, tex_type_c[glf::TEX_TYPE_SZ];

    // Bind mesh data to OpenGL context
    void bindMesh() {
        glGenVertexArrays(1, &vAO);
        glGenBuffers(1, &vBO); glGenBuffers(1, &eBO);

        // Configure vAO
        glBindVertexArray(vAO);
        // Fill buffer objects
        glBindBuffer(GL_ARRAY_BUFFER, vBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
            &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * vert_ids.size(),
            &vert_ids[0], GL_STATIC_DRAW);

        // Assign vertex attributes
        glVertexAttribPointer(glf::ATTR_POS_ID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            BUFFER_OFFSET(getOffset(Vertex, position)));
        glEnableVertexAttribArray(glf::ATTR_POS_ID);
        glVertexAttribPointer(glf::ATTR_NORM_ID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            BUFFER_OFFSET(getOffset(Vertex, normal)));
        glEnableVertexAttribArray(glf::ATTR_NORM_ID);
        glVertexAttribPointer(glf::ATTR_TEX_ID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            BUFFER_OFFSET(getOffset(Vertex, tex_coord)));
        glEnableVertexAttribArray(glf::ATTR_TEX_ID);
        glVertexAttribPointer(glf::ATTR_TAN_ID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            BUFFER_OFFSET(getOffset(Vertex, tangent)));
        glEnableVertexAttribArray(glf::ATTR_TAN_ID);
        glVertexAttribPointer(glf::ATTR_BTAN_ID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            BUFFER_OFFSET(getOffset(Vertex, bitangent)));
        glEnableVertexAttribArray(glf::ATTR_BTAN_ID);

        // Unbind
        glBindVertexArray(0);
        tex_sz = tex_ids.size();
    }
};

class Model: public Object {
 public:
    explicit Model(t_rcv3 _pos = Object::kDefaultTranslate): mesh_sz(0) {
        std::fill_n(bounds, BOUNDS_SZ, 0);  // For 6 faces of the box bound
    }

    ~Model() {
        for (GLint i = 0; i < textures.size(); ++i)
            glDeleteTextures(1, &textures[i].id);
    }

    inline bool load(const char* _path, bool sRGB = false, bool _flipuv = true) {
        sRGBSpace = sRGB;
        return loadModel(_path, _flipuv);
    }

    inline const t_vmesh& getMeshes() const { return meshes; }
    inline RenderContext& renderContext() { return mRenderContext; }

    void render(glf::ShaderTextureInfo* _texinfo) {
        // Perform per-model setup here

        for (GLint i = 0; i < mesh_sz; ++i)
            meshes[i].render(_texinfo, textures);
    }

    Material material;

 private:
    // Used for referring to Box bound faces
    enum BoxBoundFaces { RIGHT, LEFT, TOP, BOTTOM, BACK, FRONT, BOUNDS_SZ };

    t_vmesh meshes;
    t_vtex  textures;
    GLfloat bounds[BOUNDS_SZ];
    RenderContext mRenderContext;

    GLint mesh_sz;
    bool sRGBSpace;
    std::string directory;  // Directory for this model

    // Prevent copy
    const Model& operator=(const Model& rhs) {}

    bool loadModel(std::string _path, bool _flipuv) {
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Loading model: %s\n", _path.c_str());
#endif
        Assimp::Importer importer;
        unsigned int importFlags = aiProcess_Triangulate | aiProcess_ImproveCacheLocality |
            aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace;
        importFlags |= _flipuv ? aiProcess_FlipUVs : 1;

        const aiScene* scene = importer.ReadFile(_path, importFlags);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            fprintf(stdout, "Error [Assimp]: %s\n", importer.GetErrorString());
            return false;
        }

        directory = _path.substr(0, _path.find_last_of('/'));
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Model directory: %s\n", directory.c_str());
        fprintf(stdout, "Processing nodes...\n");
#endif
        processNode(scene->mRootNode, scene);

        mesh_sz = meshes.size();
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Meshes loaded: %d\n", mesh_sz);
#endif
        return true;
    }

    void processNode(aiNode* _node, const aiScene* _scene) {
        GLuint i;
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Meshes found: %u\n", _node->mNumMeshes);
#endif
        for (i = 0; i < _node->mNumMeshes; ++i) {
            aiMesh* mesh = _scene->mMeshes[_node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, _scene));
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Processed mesh: (%u of %u)\n", i + 1, _node->mNumMeshes);
#endif
        }

        for (i = 0; i < _node->mNumChildren; ++i) {
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Processing child (%u of %u)\n", i + 1, _node->mNumChildren);
#endif
            processNode(_node->mChildren[i], _scene);
        }
    }

    Mesh processMesh(aiMesh* _mesh, const aiScene* _scene) {
        t_vvert vertices;
        t_vuint tex_ids, vert_ids;
        GLuint i, j;

#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Vertices found: %u\n", _mesh->mNumVertices);
#endif

        Vertex vertex;
        for (i = 0; i < _mesh->mNumVertices; ++i) {
            vertex.position.x = _mesh->mVertices[i].x;
            vertex.position.y = _mesh->mVertices[i].y;
            vertex.position.z = _mesh->mVertices[i].z;
            addToBounds(vertex.position);

            vertex.normal.x = _mesh->mNormals[i].x;
            vertex.normal.y = _mesh->mNormals[i].y;
            vertex.normal.z = _mesh->mNormals[i].z;

            vertex.tangent.x = _mesh->mTangents[i].x;
            vertex.tangent.y = _mesh->mTangents[i].y;
            vertex.tangent.z = _mesh->mTangents[i].z;

            // Re-orthogonalize tangents
            vertex.tangent = vertex.tangent -
                             glm::dot(vertex.tangent, vertex.normal) * vertex.normal;
            vertex.bitangent = glm::cross(vertex.tangent, vertex.normal);

            if (_mesh->mTextureCoords[0]) {
                vertex.tex_coord.x = _mesh->mTextureCoords[0][i].x;
                vertex.tex_coord.y = _mesh->mTextureCoords[0][i].y;
            } else {
                vertex.tex_coord.x = vertex.tex_coord.y = 0.0f;
            }

            vertices.push_back(vertex);
        }

        for (i = 0; i < _mesh->mNumFaces; ++i) {
            aiFace face = _mesh->mFaces[i];
            for (j = 0; j < face.mNumIndices; ++j)
                vert_ids.push_back(face.mIndices[j]);
        }

        if (_mesh->mMaterialIndex >= 0) {
            aiMaterial* material = _scene->mMaterials[_mesh->mMaterialIndex];
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Found materials: %d\n", _mesh->mMaterialIndex);
#endif
            t_vuint diff_ids, spec_ids, norm_ids;
            // Diffuse maps
            loadTextures(material, aiTextureType_DIFFUSE, diff_ids, glf::TEX_DIFFUSE);
            tex_ids.insert(tex_ids.end(), diff_ids.begin(), diff_ids.end());
            // Specular maps
            loadTextures(material, aiTextureType_SPECULAR, spec_ids, glf::TEX_SPECULAR);
            tex_ids.insert(tex_ids.end(), spec_ids.begin(), spec_ids.end());
            // Normal maps
            loadTextures(material, aiTextureType_HEIGHT, norm_ids, glf::TEX_NORMAL);
            tex_ids.insert(tex_ids.end(), norm_ids.begin(), norm_ids.end());
        }

        return Mesh(vertices, vert_ids, tex_ids, &mRenderContext);
    }

    void loadTextures(aiMaterial* _material, aiTextureType _type,
                      t_vuint& _tex_ids, glf::TextureType _local_type) {
#ifdef _MODEL_DEBUG_
        fprintf(stdout, "Loading texture type: %d [%d available]\n",
            _local_type, _material->GetTextureCount(_type));
#endif
        GLuint i, j, idx;

        for (i = 0; i < _material->GetTextureCount(_type); ++i) {
            aiString str;
            _material->GetTexture(_type, i, &str);

            bool tex_available = false;
            for (j = 0; j < textures.size(); ++j) {
                if (textures[j].path == str) {
                    idx = j; tex_available = true;
                    break;
                }
            }

            if (!tex_available) {
                Texture _texture;
                // sRGB conversion not required for normal maps
                _texture.id = loadTextureFromFile(
                    str.C_Str(), directory, sRGBSpace && _local_type != glf::TEX_NORMAL);
                _texture.type = _local_type;
                _texture.path = str;

                textures.push_back(_texture);
                idx = textures.size() - 1;
            }

            _tex_ids.push_back(idx);
        }
    }

    inline void addToBounds(t_rcv3 _pos) {
        bounds[RIGHT]   = std::max(bounds[RIGHT], _pos.x);
        bounds[LEFT]    = std::min(bounds[LEFT], _pos.x);
        bounds[TOP]     = std::max(bounds[TOP], _pos.y);
        bounds[BOTTOM]  = std::min(bounds[BOTTOM], _pos.y);
        bounds[BACK]    = std::max(bounds[BACK], _pos.z);
        bounds[FRONT]   = std::min(bounds[FRONT], _pos.z);
    }
};

GLuint loadTextureFromFile(const char* _path, std::string _dir, bool sRGB) {
    std::string filename = _dir + '/' + _path;
#ifdef _MODEL_DEBUG_
    fprintf(stdout, "Loading texture: %s ", filename.c_str());
#endif

    GLuint tex_id;
    int width, height;
    unsigned char* image = SOIL_load_image(
        filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);

    if (!image) {
        fprintf(stdout, "\x1b[31;1m[Error]\x1b[0m\n");
        return 0;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, sRGB ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB,
        GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture params
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(image);
#ifdef _MODEL_DEBUG_
    fprintf(stdout, "\x1b[32;1m[Loaded]\x1b[0m\n");
#endif

    return tex_id;
}

}  // namespace glf3d

#endif
