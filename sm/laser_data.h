#ifndef H_LASER_DATA
#define H_LASER_DATA

#include "sm.h"

#define LDP   struct laser_data*

void ld_compute_cartesian(LDP);
void ld_create_jump_tables(LDP);
int  ld_valid_ray(LDP, int i);
// -1 if not found
int ld_next_valid(LDP ld, int i, int dir);

int ld_valid_corr(LDP, int i);
void ld_set_correspondence(LDP, int i, int j1, int j2);
void ld_set_null_correspondence(LDP, int i);
/** -1 if not found */
int ld_next_valid_up(LDP, int i);
int ld_next_valid_down(LDP, int i);

void ld_simple_clustering(LDP ld, double threshold);
void ld_compute_orientation(LDP ld, int size_neighbourhood);

#endif

