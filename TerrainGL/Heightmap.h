#pragma once
#ifndef HEIGHTMAP

// Used to disable fopen warning
// Should be using fopen_s ..
#pragma warning(disable:4996)

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include "OpenGL.h"
#include "Projection.h"

#define TERRAIN_PROJECTION_X 0
#define TERRAIN_PROJECTION_HEIGHT 1
#define TERRAIN_PROJECTION_Z 2

#define TERRAIN_SCALE	1.0f

constexpr auto TERRAIN_X = 16;
constexpr auto TERRAIN_Z = 16;
constexpr auto TERRAIN_PROJECTION = 3;
constexpr auto VB_DATA_SIZE = TERRAIN_X * TERRAIN_Z * 3;

struct UBOData
{
    glm::mat4 viewProjectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 normalMatrix;
    glm::vec4 lightDirViewSpace;
    glm::vec4 offset;
    // Maximum wave height in x
    glm::vec4 data;
    glm::vec4 ambiant;
    glm::vec4 foamAmbiant;
    glm::vec4 diffuse;
    glm::vec4 foamDiffuse;
    // specular color in rgb and specular power in a
    glm::vec4 specular;
    glm::vec4 foamSpecular;
};

struct VertexData
{
    glm::vec2 position;
    glm::vec2 uvs;
};

class Terrain
{
public:
    Terrain(GLFWwindow* window);
    ~Terrain();
    void generate();
    void render();
        
private:
    bool readBMP(const std::string& fileName, std::string& result);

public:
    GLuint m_VBO; // vertex buffer id
    GLuint m_VAO; // vertex array id
    GLuint m_IBO; // indices
    GLuint m_UBO; // indices

private:
    UBOData* m_UBOData;

    GLFWwindow* window;

    std::vector<float> m_bitmap;
    int m_bitmapHeight;
    int m_bitmapWidth;

    std::size_t m_HeightMapDataSize, m_NormalDataSize, m_FoamMaskDataSize;
    uint32_t m_IndexCount;
    uint32_t m_VertexCount;

    GLuint m_HeightTexture;

    Projection* m_Projection;
    GLuint m_ShaderProgram;

    glm::vec2 m_InvTextureSize;
    glm::vec4 m_LightDir;
};

#endif // !HEIGHTMAP
