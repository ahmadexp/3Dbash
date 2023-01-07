#include "draw.h"
#include "objects.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//#define DEBUG


int main() {
        draw__init();
#ifndef DEBUG
        printf("starting\n");
        int x = 0, y = 20, z = 20;
        cube_t* cube = obj__cube_new(x, y, z, 10);
        ray_t* ray = obj__ray_new(0, 0, 0);
        plane_t* plane = obj__plane_new(cube->vertices[0], cube->vertices[1], cube->vertices[2]);
        obj__cube_rotate(cube, 9.9, 8.6, 7.3);
        draw__pixel(g_min_cols, g_min_rows, '*');
        draw__pixel(g_max_cols, g_max_rows-1, '*');
        draw__pixel(0, 0, 'O');
        //draw__pixel(-10, -10, '*');
#if 1
        for (int i = g_min_rows; i <= g_max_rows; ++i) {
            for (int j = g_min_cols; j <= g_max_cols; ++j) {
                z = obj__plane_z_at_xy(plane, j, i);
                obj__ray_send(ray, j, i, z);
                if (obj__ray_hits_rectangle(ray, cube->vertices[0], cube->vertices[1], cube->vertices[2], cube->vertices[3])) {
                    draw__pixel(j, i, '#');
                }
            }
        }
#endif
#else
    printf("r = %d, c = %d\n", g_min_rows, g_min_cols);
        cube_t* cube = obj__cube_new(0, 0, 20, 10);
        obj__cube_rotate(cube, 0.1, 0.1, 0.05); 
        ray_t* ray = obj__ray_new(1, 50, 20);
        vec3i_t* p0 = vec__vec3i_new(-1, 1, 2);
        vec3i_t* p1 = vec__vec3i_new(-4, 2, 2);
        vec3i_t* p2 = vec__vec3i_new(-2, 1, 5);
        plane_t* pl = obj__plane_new(p0, p1, p2);
        printf("%d, %d, %d, %d\n", pl->normal->x, pl->normal->y, pl->normal->z, pl->offset);
        vec3i_t inters = obj__ray_plane_intersection(pl, ray);
        printf("%d, %d, %d\n", inters.x, inters.y, inters.z);
        printf("(Debugging mode)\n");
#endif
#ifndef DEBUG
        refresh();
        draw__end();
#endif
        return 0;
}
