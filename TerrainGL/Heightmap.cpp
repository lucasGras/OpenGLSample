#include "Heightmap.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_images.h"
#include "Shader.h"

using namespace std;

Terrain::Terrain(GLFWwindow* window)
{
    this->window = window;
    // Init Vertex Array
    glCreateVertexArrays(1, &this->m_VAO);
    glBindVertexArray(this->m_VAO);

    this->m_ShaderProgram = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
    this->m_Projection = new Projection(this->m_ShaderProgram, this->window);
}

Terrain::~Terrain()
{
    glDeleteProgram(this->m_ShaderProgram);
}

bool Terrain::readBMP(const std::string & fileName, std::string &result)
{
    std::ifstream ifs(fileName, std::ios::binary);

    if (ifs)
    {
        result = std::move(std::string(std::istreambuf_iterator<char>(ifs), {}));

        if (!ifs)
        {
            std::cerr << "Error: only " << ifs.gcount() << " could be read\n";
        }
    }
    else
    {
        std::cerr << "Cannot open file : " << fileName << '\n';
        return false;
    }

    return true;
}

void Terrain::generate() {

    // Heightmap read and coordonate matrice generation
    std::string content;
    this->readBMP("heightmap1.bmp", content);
    std::cout << content.size() << std::endl;
    this->m_bitmapWidth = 16;
    this->m_bitmapHeight = 16;


    // Compute vertex (VBO) and indices (optimized triangles => IBO) lenght
    this->m_VertexCount = this->m_bitmapWidth * this->m_bitmapHeight;
    this->m_IndexCount = (this->m_bitmapWidth - 1) * (this->m_bitmapHeight - 1) * 6;
    
    // Allocate vector memory for VBO and IBO
    // We have no need to store this as class property because it will be assign
    // to VBO and IBO buffer memory addresses
    std::vector<uint32_t> indicesData = std::vector<uint32_t>(this->m_IndexCount);
    std::vector<VertexData> vertexData = std::vector<VertexData>(this->m_VertexCount);

    // Generate vertex buffer

    for (uint32_t y = 0; y < this->m_bitmapHeight; y++) {
        const float yPos = -this->m_bitmapHeight / 2.0f + this->m_bitmapHeight * static_cast<float>(y) / static_cast<float>(this->m_bitmapHeight - 1);
        const float UVY = static_cast<float>(y * 2 + 1) / static_cast<float>(this->m_bitmapHeight * 2);

        for (uint32_t x = 0; x < this->m_bitmapWidth; x++) {
            const float xPos = -this->m_bitmapWidth / 2.0f + this->m_bitmapWidth * static_cast<float>(x) / static_cast<float>(this->m_bitmapWidth - 1);
            const float UVX = static_cast<float>(x * 2 + 1) / static_cast<float>(this->m_bitmapWidth * 2);
            const uint32_t offset = y * this->m_bitmapWidth + x;
            VertexData& vData = vertexData[offset];

            vData.uvs = glm::vec2(UVX, UVY);
            vData.position = glm::vec2(xPos, yPos);
        }
    }
    glCreateBuffers(1, &this->m_VBO);
    glNamedBufferStorage(this->m_VBO, sizeof(VertexData) * vertexData.size(), vertexData.data(), 0);


    // Generate indices buffer
    for (uint32_t y = 0; y < this->m_bitmapHeight - 1; ++y)
    {
        for (uint32_t x = 0; x < this->m_bitmapWidth - 1; ++x)
        {
            const uint32_t vertexOffset = (y * this->m_bitmapWidth + x);
            const uint32_t indexOffset = (y * (this->m_bitmapWidth - 1) + x) * 6;

            indicesData[indexOffset + 0] = vertexOffset;
            indicesData[indexOffset + 1] = vertexOffset + this->m_bitmapWidth;
            indicesData[indexOffset + 2] = vertexOffset + this->m_bitmapWidth + 1;

            indicesData[indexOffset + 3] = vertexOffset + this->m_bitmapWidth + 1;
            indicesData[indexOffset + 4] = vertexOffset + 1;
            indicesData[indexOffset + 5] = vertexOffset;
        }
    }
    glCreateBuffers(1, &this->m_IBO);
    glNamedBufferStorage(this->m_IBO, sizeof(uint32_t) * indicesData.size(), indicesData.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, this->m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_IBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), nullptr);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<GLvoid*>(sizeof(glm::vec2)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glCreateBuffers(1, &this->m_UBO);

    this->m_LightDir = glm::normalize(glm::vec4(-0.5f, -1.0f, 0.25f, 0.0f));

    UBOData uboData;

    // Use model view projection here even if it is an identity matrix (1)
    uboData.viewProjectionMatrix = this->m_Projection->modelViewProjectionMatrix;
    uboData.viewMatrix = this->m_Projection->viewMatrix;
    // Should be modelViewMatrix but since we don't transform the model this is fine
    uboData.normalMatrix = glm::transpose(glm::inverse(this->m_Projection->viewMatrix));
    uboData.offset = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    uboData.data = glm::vec4(1, 0.0f, 0.0f, 0.0f);
    uboData.ambiant = glm::vec4(0.0188235f, 0.0188235f, 0.0188235f, 1.0f);
    uboData.foamAmbiant = glm::vec4(0.0898039f, 0.0898039f, 0.0898039f, 1.0f);
    uboData.diffuse = glm::vec4(0.188235f, 0.380392f, 0.690196f, 1.0f);
    uboData.foamDiffuse = glm::vec4(0.898039f, 0.898039f, 0.898039f, 1.0f);
    uboData.lightDirViewSpace = this->m_Projection->viewMatrix * this->m_LightDir;
    uboData.specular = glm::vec4(1.0f, 1.0f, 1.0f, 6.f);
    uboData.foamSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 10.0f);

    glNamedBufferStorage(this->m_UBO, sizeof(UBOData), &uboData, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    this->m_UBOData = GL_CALL_REINTERPRET_CAST_RETURN_VALUE
    (
        UBOData*,
        glMapNamedBufferRange, 
        this->m_UBO, 0, 
        sizeof(UBOData),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
    );

    // Create textures
    // TODO Color in shader comes from foam, need to add it to create color or create simple color shader
    glCreateTextures(GL_TEXTURE_2D, 1, &this->m_HeightTexture);
    glTextureStorage2D(this->m_HeightTexture, 1, GL_R32F, this->m_bitmapWidth, this->m_bitmapHeight);
    glTextureSubImage2D(this->m_HeightTexture, 0, 0, 0, this->m_bitmapWidth, this->m_bitmapHeight, GL_RED, GL_FLOAT, &content);
    GL_CALL(glTextureParameteri, m_HeightTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL_CALL(glTextureParameteri, m_HeightTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL_CALL(glTextureParameteri, m_HeightTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    GL_CALL(glTextureParameteri, m_HeightTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Terrain::render() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->m_Projection->boundMatrixToShaders();
    this->m_Projection->handleCamera();

    // Bind buffers, textures, program, VAO
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->m_UBO);
    glBindTextureUnit(0, this->m_HeightTexture);
    glUseProgram(this->m_ShaderProgram);
    glBindVertexArray(this->m_VAO);

    // Draw IBO
    glDrawElements(GL_TRIANGLES, this->m_IndexCount, GL_UNSIGNED_INT, &this->m_IBO);

    // Unbind buffers
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
    glUseProgram(0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
}