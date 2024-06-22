/* AUTHOR: muly / morryaland
 * See file LICENSE for full license details.*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/quad_tree.h"

PARTICLE *particles;
int particle_c;
QTREE_NODE *qtree;
static int upper_i;
int upper_c;
QTREE_NODE **upper;

static void quad_tree(QTREE_NODE *parent, QTREE_NODE *node, int depth, int nx, int ny, int ex, int ey);
static void quad_tree_check(QTREE_NODE *node, int depth);
static void upper_start();
static QTREE_NODE *upper_next();
static void upper_add(QTREE_NODE *node);
static void upper_del(QTREE_NODE *node);

static void quad_tree(QTREE_NODE *parent, QTREE_NODE *node, int depth, int nx, int ny, int ex, int ey)
{
  if (!depth)
    return;
  if (!node)
    node = tzalloc(sizeof(QTREE_NODE), parent);
  node->p = parent;
  node->nx = nx; node->ny = ny; node->ex = ex; node->ey = ey;
  node->contpart = talloc(sizeof(PARTICLE*), node);
  for (int i = 0; i < parent->mass; i++) {
    if (parent->contpart[i]->x < parent->ex && parent->contpart[i]->y < parent->ey &&
        parent->contpart[i]->x > parent->nx && parent->contpart[i]->y > parent->ny) {
      node->contpart = trealloc(node->contpart, sizeof(PARTICLE*) * (node->mass + 1));
      node->contpart[node->mass] = parent->contpart[i];
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
    upper_add(node);
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

static void quad_tree_check(QTREE_NODE *node, int depth)
{
  for (int i = 0; i < node->mass; i++) {
    if (!(node->contpart[i]->x < node->ex && node->contpart[i]->y < node->ey &&
          node->contpart[i]->x > node->nx && node->contpart[i]->y > node->ny)) {
      if (!depth)
        upper_del(node);
      quad_tree_check(node->p, depth + 1);
    }
  }
  if (depth && node->p) {
    tfree(node);
    quad_tree(node->p, NULL, depth, node->nx, node->ny, node->ex, node->ey);
  }
}

static void upper_start()
{
  upper_i = 0;
}

static QTREE_NODE *upper_next()
{
  if (upper_i >= upper_c)
    return NULL;
  QTREE_NODE *node = upper[upper_i];
  upper_i++;
  return node;
}

static void upper_add(QTREE_NODE *node)
{
  upper = realloc(upper, sizeof(QTREE_NODE*) * (upper_c + 1));
  upper[upper_c] = node;
  upper_c++;
}

static void upper_del(QTREE_NODE *node)
{
  for (int i = 0; i < upper_c; i++) {
    if (upper[i]->nx == node->nx && upper[i]->ny == node->ny && upper[i]->ex == node->ex && upper[i]->ey == node->ey) {
      upper_c--;
      for (int j = i; j < upper_c; j++) {
        upper[j] = upper[j + 1];
      }
      upper = realloc(upper, sizeof(QTREE_NODE*) * (upper_c));
      break;
    }
  }
}

void quad_tree_update()
{
  upper_start();
  QTREE_NODE *node = NULL;
  while ((node = upper_next())) {
    quad_tree_check(node, 0);
  }
}

int quad_tree_init()
{
  if (!qtree)
    qtree = tzalloc(sizeof(QTREE_NODE), NULL);
  if (!upper)
    upper = malloc(sizeof(QTREE_NODE*));
  upper_c = 0;
  if (!particles || !particle_c) {
    puts("ERROR: particles == NULL");
    return -1;
  }
  int depth = particle_ln();
  if (depth < 0) {
    puts("ERROR: depth ln");
    return -1;
  }
  int bord = (int)(powf(2, depth) + 0.5f);
  qtree->mass = particle_c;
  qtree->contpart = talloc(sizeof(PARTICLE*) * particle_c, qtree);
  for (int i = 0; i < particle_c; i++) {
    qtree->contpart[i] = particles + i;
  }
  qtree->nx = -bord; qtree->ny = -bord; qtree->ex = bord; qtree->ey = bord;
  quad_tree(qtree, NULL, depth, -bord, -bord, bord, bord);
  return 0;
}

void quad_tree_free()
{
  if (qtree) {
    tfree(qtree);
    qtree = NULL;
  }
  if (upper) {
    free(upper);
    upper = NULL;
    upper_c = 0;
  }
}
