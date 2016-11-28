#include "inc/scenegraph.h"
#include <inc/gl.h>
#include "inc/mesh.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flag
#include "stdio.h"
#include <inc/camera.h>
#include <inc/light.h>
using namespace std;
using namespace glm;


extern int default_mesh_prog;
extern int mesh_lidr_prog;

SceneGraph::SceneGraph()
{
  glGenVertexArrays(1, &lightvolume_vao);
  glGenBuffers(1, &lightvolume_vertex_buffer);
}

SceneGraph::~SceneGraph()
{
  glDeleteBuffers(1, &lightvolume_vertex_buffer);
  glDeleteVertexArrays(1, &lightvolume_vao);
}

void SceneGraph::printGraph()
{
  for (int i=0; i<nodes.size(); ++i)
  {
    fprintf(stderr, "node %s\r\n", nodes[i]->getName());
    fprintf(stderr, "\tchildren: ");
    vector<Node *> children = nodes[i]->getChildren();
    for (int j=0; j<children.size(); ++j)
    {
      fprintf(stderr, "%s ", children[j]->getName());
    }
  }
}


//bake world transforms into the meshes
void SceneGraph::bake()
{
  // do DFS with a while loop so its faster
  struct state_variables
  {
    Node *N;
    mat4 M;
  };
  vector <struct state_variables> Nstack;
  Nstack.push_back((struct state_variables){root, root->getTransform()});
  while (Nstack.size()>0)
  {
    struct state_variables cur_depth = Nstack.back();
    Nstack.pop_back();
    Node *curN = cur_depth.N;
    mat4 M = cur_depth.M;
    vector<Mesh*> meshes = curN->getMeshes();
    vector<Light*> Nlights = curN->getLights();
    for (int i=0; i<Nlights.size(); ++i)
    {
     // fprintf(stderr, "\r\n");
     // for (int i=0; i<4; ++i)
     // {
     //   vec4 col = M[i];
     //   fprintf(stderr, "%f %f %f %f", col.x, col.y, col.z, col.w);
     // }
     // fprintf(stderr, "\r\n");
      mat4 fixed_M = transpose(M);
      vec4 trs = fixed_M[3];
      fprintf(stderr, "bake: found light %s\r\n", Nlights[i]->getName());
      fprintf(stderr, "\t pos %f %f %f %f\r\n", trs.x, trs.y, trs.z, trs.w);
      Nlights[i]->updatePos(&fixed_M);
    }
    for (unsigned i=0; i<meshes.size(); ++i)
    {
      meshes[i]->setWorldPos(M);
    }

    vector<Node *> children = curN->getChildren();
    for (unsigned i=0; i<children.size(); ++i)
    {
      Nstack.push_back((struct state_variables){children[i], M * children[i]->getTransform()});
    }
  }
}

void SceneGraph::drawBaked(Camera *camera, bool wireframe)
{
  for (int lnum=0; lnum<lights.size(); ++lnum)
  {
    for (int i=0; i<meshes.size(); ++i)
    {
      int program = meshes[i]->getProgram();
      glUseProgram(program);
      camera->updateUniforms(program);
      lights[lnum]->updateUniforms(program);
      mat4 M = meshes[i]->getWorldPos();
      mat3 N = transpose(inverse(glm::mat3(M)));
      meshes[i]->draw(wireframe, &M[0][0], &N[0][0]);
      glUseProgram(0);
    }
  }
}


void SceneGraph::zPreBaked(int program)
{
  for (int i=0; i<meshes.size(); ++i)
  {
    mat4 M = meshes[i]->getWorldPos();
    meshes[i]->quickdraw(&M[0][0], program);
  }
}

//cast shadows for the first 2 lights in the lights list
//Ill do more for the final project
void SceneGraph::zPre()
{
  // do DFS with a while loop so its faster
  struct state_variables
  {
    Node *N;
    mat4 M;
  };
  for (unsigned lnum=0; lnum<lights.size(); ++lnum)
  {
    Light *dislight = lights[lnum];
    int program = dislight->shadowMap();
    vector <struct state_variables> Nstack;
    Nstack.push_back((struct state_variables){root, root->getTransform()});
    while (Nstack.size()>0)
    {
      struct state_variables cur_depth = Nstack.back();
      Nstack.pop_back();
      Node *curN = cur_depth.N;
      mat4 M = cur_depth.M;
      for (unsigned i=0; i<meshes.size(); ++i)
      {
        meshes[i]->quickdraw(&M[0][0], program);
        //int program = meshes[i]->getProgram();
        //glUseProgram(program);
        //meshes[i]->draw(false, &M[0][0], &M[0][0]);
        //glUseProgram(0);
      }

      vector<Node *> children = curN->getChildren();
      for (unsigned i=0; i<children.size(); ++i)
      {
        Nstack.push_back((struct state_variables){children[i], M * children[i]->getTransform()});
      }
    }
    dislight->restore();
  }
}

void SceneGraph::drawSceneShadowed(Camera *camera, bool wireframe)
{
  // do DFS with a while loop so its faster
  struct state_variables
  {
    Node *N;
    mat4 M;
  };
  for (unsigned lnum=0; lnum<lights.size(); ++lnum)
  {
    Light *dislight = lights[lnum];
    vector <struct state_variables> Nstack;
    Nstack.push_back((struct state_variables){root, root->getTransform()});
    while (Nstack.size()>0)
    {
      struct state_variables cur_depth = Nstack.back();
      Nstack.pop_back();
      Node *curN = cur_depth.N;
      mat4 M = cur_depth.M;
      vector<Mesh*> meshes = curN->getMeshes();
      int shadowprog = dislight->shadowMap();
      for (unsigned i=0; i<meshes.size(); ++i)
      {
        meshes[i]->quickdraw(&M[0][0]);
      }
      dislight->restore();
      for (unsigned i=0; i<meshes.size(); ++i)
      {
        int program = meshes[i]->getProgram();
        glUseProgram(program);
        camera->updateUniforms(program);
        lights[lnum]->updateUniforms(program);
        mat3 N = transpose(inverse(glm::mat3(M)));
        meshes[i]->draw(wireframe, &M[0][0], &N[0][0]);
        glUseProgram(0);
      }

      vector<Node *> children = curN->getChildren();
      for (unsigned i=0; i<children.size(); ++i)
      {
        Nstack.push_back((struct state_variables){children[i], M * children[i]->getTransform()});
      }
    }
  }
}

void SceneGraph::drawScene(Camera *camera, bool wireframe)
{
  // do DFS with a while loop so its faster
  struct state_variables
  {
    Node *N;
    mat4 M;
  };
  for (unsigned lnum=0; lnum<lights.size(); ++lnum)
  {
    vector <struct state_variables> Nstack;
    Nstack.push_back((struct state_variables){root, root->getTransform()});
    while (Nstack.size()>0)
    {
      struct state_variables cur_depth = Nstack.back();
      Nstack.pop_back();
      Node *curN = cur_depth.N;
      mat4 M = cur_depth.M;
      vector<Mesh*> meshes = curN->getMeshes();
      int oldProgram = -1;
      vector<Light*> Nlights = curN->getLights();
      for (int i=0; i<Nlights.size(); ++i)
      {
        vec4 t = M*vec4(0,0,0,1);
        Nlights[i]->updatePos(&M);
      }
      for (unsigned i=0; i<meshes.size(); ++i)
      {
        int program = meshes[i]->getProgram();
        glUseProgram(program);
        camera->updateUniforms(program);
        lights[lnum]->updateUniforms(program);
        mat3 N = transpose(inverse(glm::mat3(M)));
        meshes[i]->draw(wireframe, &M[0][0], &N[0][0]);
        glUseProgram(0);
      }

      vector<Node *> children = curN->getChildren();
      for (unsigned i=0; i<children.size(); ++i)
      {
        Nstack.push_back((struct state_variables){children[i], M * children[i]->getTransform()});
      }
    }
  }
}

void SceneGraph::drawLightVolumes(int lightvolume_program, Camera *camera)
{
  //camera->updateUniforms(lightvolume_program);
  glBindVertexArray(lightvolume_vao);
  int lightposition_loc = glGetUniformLocation(lightvolume_program, "light_position_radius");
  if (lightposition_loc < 0)
  {
    perror("cant find light position loc\r\n");
  }
  int lightindex_loc = glGetUniformLocation(lightvolume_program, "light_index");
  if (lightindex_loc < 0)
  {
    perror("cant find light index loc\r\n");
  }
  int PV_inverse_loc = glGetUniformLocation(lightvolume_program, "PV_inverse");
  if (PV_inverse_loc < 0)
  {
    perror("cant find PV inverse loc\r\n");
  }
  mat4 pv_inverse = camera->getProjectionViewInverse();
  glUniformMatrix4fv(PV_inverse_loc, 1, false, &pv_inverse[0][0]);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_CONSTANT_COLOR);
  glBlendColor(0.25f, 0.25f, 0.25f, 0.25f);
  //fprintf(stderr, "about to draw %d lights\r\n", lights.size());
  for (int i=0; i<lights.size() && i<256; ++i)
  //for (int i=0; i<3 && i<256; ++i)
  {
    vec4 lightprop = vec4(lights[i]->getWorldPos(), lights[i]->getRadius());
    //fprintf(stderr, "\t light at %f %f %f\r\n", lightprop.x, lightprop.y, lightprop.z);
    glUniform4fv(lightposition_loc, 1, &lightprop[0]);
    int adj = i+1;
    //vec4 index = vec4((adj&0x3) << 6, (adj&0xC) << 4, (adj&0x30) << 2, (adj&0xC0) << 0)/256.0f;
    vec4 index = vec4((adj&0x3) << 6, (adj&0xC) << 4, (adj&0x30) << 2, (adj&0xC0) << 0)/255.0f;
    //fprintf(stderr, "i -> %d, index -> %f %f %f %f\r\n", adj, index.x, index.y, index.z, index.w);
    //vec4 index = vec4((adj&0x3) << 6, (adj&0xC) << 6, (adj&0x30) << 6, (adj&0xC0) << 6)/255.0f;
    //fprintf(stderr, "light index %f %f %f %f\r\n", index.x, index.y, index.z, index.w);
    glUniform4fv(lightindex_loc, 1, &index[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
  glDisable(GL_BLEND);
}

std::vector<Light *> SceneGraph::getLights()
{
  return lights;
}

Node *SceneGraph::allocNode()
{
  Node *newnode = new Node();
  nodes.push_back(newnode);
  return newnode;
}


void SceneGraph::doMouseInput(double xpos, double ypos)
{
}

void SceneGraph::doKeyboardInput(int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) { // only handle PRESS and REPEAT
        return;
    }

    //printf("camera key input\n");
    // Special keys (arrows, CTRL, ...) are documented
    // here: http://www.glfw.org/docs/latest/group__keys.html
    switch (key)
    {
      case 'F':
        for (int i=0; i<meshes.size(); ++i)
        {
          meshes[i]->setProgram(default_mesh_prog);
        }
        bake();
        break;
      case 'L':
        for (int i=0; i<meshes.size(); ++i)
        {
          meshes[i]->setProgram(mesh_lidr_prog);
        }
        bake();
        break;
    }
}
