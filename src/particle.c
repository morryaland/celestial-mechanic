/* AUTHOR: muly / morryaland
 * See file LICENSE for full license details.*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "include/quad_tree.h"
#include "include/particle.h"

PARTICLE *particles;
int particle_c;

static int count;
static float *distance;
static QTREE_NODE **out;
static PARTICLE **particle;

static void distancef(QTREE_NODE *node);

static void distancef(QTREE_NODE *node)
{
  float x = (node->massx - (**particle).x) * (node->massx - (**particle).x);
  float y = (node->massy - (**particle).y) * (node->massy - (**particle).y);
  float d = sqrt(x + y);
  if (isnan(d))
    return;
  if (d < 10)
    d = 10;
  if(abs(node->ex - node->nx) / d > theta &&
         node->a && node->b && node->c && node->d) {
      distancef(node->a);
      distancef(node->b);
      distancef(node->c);
      distancef(node->d);
  }
  else {
    distance = realloc(distance, sizeof(float) * (count + 1));
    distance[count] = d;
    out = realloc(out, sizeof(QTREE_NODE*) * (count + 1));
    out[count] = node;
    count++;
  }
}

int particle_add(int x, int y)
{
  particles = realloc(particles, sizeof(PARTICLE) * (particle_c + 1));
  if (!particles)
    return -1;
  particles[particle_c].x = x;
  particles[particle_c].y = y;
  particles[particle_c].speedx = 0;
  particles[particle_c].speedy = 0;
  particle_c++;
  quad_tree_rebuild();
  return 0;
}

void particle_move()
{
  if (particle_c < 2)
    return;
  for (int i = 0; i < particle_c; i++) {
    count = 0;
    particle = malloc(sizeof(PARTICLE*));
    *particle = particles + i;
    distancef(qtree);
    float acx = 0;
    float acy = 0;
    for (int j = 0; j < count; j++) {
      float aot = (out[j]->mass) / (distance[j] * distance[j]);
      acx += aot * ((out[j]->massx - (**particle).x) / distance[j]);
      acy += aot * ((out[j]->massy - (**particle).y) / distance[j]);
    }
    (**particle).speedx += acx;
    (**particle).speedy += acy;
    (**particle).x += (**particle).speedx;
    (**particle).y += (**particle).speedy;
    free(particle);
    free(distance);
    free(out);
    distance = NULL;
    out = NULL;
  }
}

void particle_clean()
{
  free(particles);
  particles = NULL;
  particle_c = 0;
  quad_tree_rebuild();
}
