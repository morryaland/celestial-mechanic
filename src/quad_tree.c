/* AUTHOR: muly / morryaland
 * See file LICENSE for full license details.*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/quad_tree.h"

PARTICLE *particles;
int particle_c;
QTREE_NODE *qtree;

static void quad_tree(QTREE_NODE *parent, QTREE_NODE *node, int depth, int nx, int ny, int ex, int ey)
{
  if (!depth)
    return;
  if (!node)
    node = tzalloc(sizeof(QTREE_NODE), parent);
  node->p = parent;
  node->nx = nx;
  node->ny = ny;
  node->ex = ex;
  node->ey = ey;
  for (int i = 0; i < particle_c; i++) {
    if (particles[i].x < ex && particles[i].y < ey &&
        particles[i].x > nx && particles[i].y > ny) {
      node->massx += particles[i].x;
      node->massy += particles[i].y;
      node->mass++;
    }
  }
  if (!(node->mass)) {
    node->massx /= node->mass;
    node->massy /= node->mass;
  }
  if(node->mass == 1){
    //TODO
    //printf("%d %d %d %d\n", nx, ny, ex, ey);
    return;
  }
  int midx = (nx + ex) >> 1;
  int midy = (ny + ey) >> 1;
  depth--;
  quad_tree(node, node->a, depth, nx, ny, midx, midy);
  quad_tree(node, node->b, depth, midx, ny, ex, midy);
  quad_tree(node, node->c, depth, midx, midy, ex, ey);
  quad_tree(node, node->d, depth, nx, midy, midx, ey);
}

static int particle_ln()
{
  int res = -1;
  double max = fabs(particles[0].x);
  for (int i = 0; i < particle_c; i++) {
    if (fabs(particles[i].x) > max) {
      max = fabs(particles[i].x);
    }
    if (fabs(particles[i].y) > max) {
      max = fabs(particles[i].y);
    }
  }
  if (!(int)max)
    return -1;
  res = (int)log2(max) + 1;
  return res;
}

int quad_tree_update()
{
  quad_tree_free();
  if (quad_tree_init() < 0)
    return -1;
  return 0;
}

int quad_tree_init()
{
  if (!qtree)
    qtree = tzalloc(sizeof(QTREE_NODE), NULL);
  if (!particles || !particle_c)
    return -1;
  int depth = particle_ln();
  if (depth < 0)
    return -1;
  int bord = (int)(powf(2, depth) + 0.5f);
  quad_tree(NULL, qtree, depth, -bord, -bord, bord, bord);
  return 0;
}

void quad_tree_free()
{
  if (qtree)
    tfree(qtree);
}
