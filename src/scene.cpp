#include "inc/scene.h"
#include "inc/chunk.h"
#include "inc/glt.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "inc/recorder.h"
#include "inc/camera.h"
#include "inc/teapot.h"
#include "inc/cube.h"
#include "inc/voxel.h"
#include "stdio.h"
#include "inc/scenegraph.h"

void drawTeapot();

Camera *camera;// = Camera(vec3(0.0f,0.0f,10.0f),vec2(0.0f,0.0f));
VoxelGrid *vxg;
VoxelGrid *gnd;
BinVox *garg;
AssimpGraph *sgr;
glm::mat4 floormat = glm::translate(glm::mat4(), vec3(-20.0,-1.0, -20.0));
glm::mat4 ident;

unsigned voxelprog;
unsigned default_mesh_prog;

void initScene(mat4 Projection)
{
  camera = new Camera(vec3(0.0f,0.0f,10.0f), vec2(0.0f,0.0f), Projection);
  camera->enableInput();
  voxelprog = compileProgram("../shaders/voxel.vs", "../shaders/voxel.fs");
  default_mesh_prog = compileProgram("../shaders/flat.vs", "../shaders/flat.fs");
  vxg = new VoxelGrid(10,10,10);
  vxg->setProgram(voxelprog);
  gnd = new VoxelGrid(100,1,100);
  gnd->setProgram(voxelprog);
  //garg = new BinVox("../data/garg.binvox");
  //garg->setProgram(voxelprog);
  sgr = new AssimpGraph("../data/sponza/sponza_norm.obj");
  //sgr->printGraph();
}

void drawScene()
{
  int loc = glGetUniformLocation(voxelprog, "M");
  camera->updateUniforms(voxelprog);
  glUseProgram(voxelprog);
  glUniformMatrix4fv(loc, 1, false, &floormat[0][0]);
  gnd->draw();

  sgr->drawScene(camera, false);
//
//  glUseProgram(voxelprog);
//  if (loc<0)
//  {
//    fprintf(stderr, "M not found\n");
//  }
//  glUniformMatrix4fv(loc, 1, false, &ident[0][0]);
//  //vxg->draw();
//  //garg->draw();
}

void cleanupScene()
{
  delete vxg;
  delete gnd;
  //delete garg;
  delete camera;
  delete sgr;
}


//debug
void drawTeapot()
{
    // set the required buffer size exactly.
    GeometryRecorder rec(teapot_num_faces * 3);
    for (int idx : teapot_indices) {
        vec3 position(teapot_positions[idx * 3 + 0],
            teapot_positions[idx * 3 + 1],
            teapot_positions[idx * 3 + 2]);

        vec3 normal(teapot_normals[idx * 3 + 0],
            teapot_normals[idx * 3 + 1],
            teapot_normals[idx * 3 + 2]);

        rec.record(position, normal);
    }
    rec.draw();
}
