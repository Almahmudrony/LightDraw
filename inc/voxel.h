#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <inc/gl.h>
using namespace glm;

class Voxel
{
  public:
    Voxel(mat4 transform);
    vec3 *getVerts();
    GLuint getnVerts();
    GLuint *getIndices();
    GLuint getnIndices();
    mat4 getTransform();
    bool getVisible();
    void setVisible();
    void setInvisible();
  private:
    bool visible=true;
    mat4 m_transform;

    GLuint m_nverts;
    glm::vec3 *m_position;


    GLuint m_nindices;
    GLuint *m_indices;
};
