#include "vector.h"
#include "draw.h" // g_colors
#include "objects.h"
#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <limits.h> // INT_MIN
#include <unistd.h>
#include <stdlib.h> // exit
#include <stdbool.h> // true/false 

// colors for each face of the cube
color_t g_colors[6] = {'~', '.', '=', '@', '%', '|'};

// rows, columns and aspect ratio of the terminal
int g_rows;
int g_cols;
int g_min_rows;
int g_max_rows;
int g_min_cols;
int g_max_cols;
// columns over rows for the terminal 
float g_cols_over_rows;
// screen resolution (pixels over pixels) 
float g_screen_res;

/**
 * @brief Checks whether a null-terminated array of characters represents
 *        a positive decimal number, e.g. 1.8999 or 1,002
 *
 * @param string A null-terminated array of chars
 *
 * @return true if the given string is numerical
 */
static bool is_decimal(char* string) {
    bool ret = false;
    for (char* s = string; *s != '\0'; ++s) {
        if (((*s >= '0') && (*s <= '9')) ||
            (*s == '.') || (*s == ',') ||
            (*s == '\n'))
            ret = true;
        else
            return false;
    }
    return ret;
}

/**
 * Adapted from
 * https://stackoverflow.com/a/646254
 */
static float get_screen_res() {
  FILE *fp;
  char path[1035];
  // what screen resolution to return
  // initialise with a common value in case xrand fails
  float ret = 1.7777;

  // Open the command for reading
  fp = popen("echo `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f1` / `xrandr --current | grep \'*\' | uniq | awk '{print $1}\' | cut -d \'x\' -f2` | bc -l", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }
  // parse the output - it should only be the resolution
  while (fgets(path, sizeof(path), fp) != NULL) {
    if (is_decimal(path)) {
        ret = atof(path);
        break;
    }
  }
  // close file
  pclose(fp);
  return ret;
}


void draw_init() {
    // start the curses mode
    initscr();
    curs_set(0);
    // get the number of rows and columns
    getmaxyx(stdscr, g_rows, g_cols);
    g_min_rows = -g_rows;
    g_max_rows = g_rows + 1;
    g_min_cols = -g_cols/2 + 1;
    g_max_cols = g_cols/2;
    // find terminal window's aspect ratio
    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    g_cols_over_rows = (float)wsize.ws_col/wsize.ws_row;
    g_screen_res = get_screen_res();
}

/* Uses the following coordinate system:
 *
 *      ^ y
 *      |
 *      |
 *      |
 *      |
 *      o---------> x
 *       \
 *        \
 *         v z
 */
void draw_pixel(int x, int y, color_t c) {
    int y_scaled = y/(g_cols_over_rows/g_screen_res);
    mvaddch(y_scaled+g_rows/2, x+g_cols/2, c);
}

void draw_end() {
    getch();
    endwin();
}


void draw_cube(cube_t* cube) {
/*
 * This function renders the given cube by the basic ray tracing principle.
 *
 * A ray is shot from the origin to every pixel on the screen row by row.
 * For each screen coordinate, there can zero to two intersections with the cube.
 * If there is one, render the (x, y) of the intersection (not the x,y of the screen!).
 * If there are two, render the (x, y) of the closer intersection. In the figure below,
 * z_hit are the z of the two intersections and z_rend is the closer one.
 *
 * The ray below intersects faces (p0, p1, p2, p3) and  (p4, p5, p6, p7)
 * 
 *                      O camera origin  
 *                       \
 *                        \
 *                         V ray
 *                    p3    \            p2                      o cube's centre 
 *                    +-------------------+                      + cube's vertices
 *                    | \     \           | \                    # ray-cube intersections
 *                    |    \   # z_rend   |    \                   (z_hit)
 *                    |      \  p7        |       \
 *                    |         +-------------------+ p6         ^ y
 *                    |         | \       .         |            |
 *                    |         |  \      .         |            |
 *                    |         |   \     .         |            o-------> x
 *                    |         |    \    .         |             \
 *                    |         |     \   .         |              \
 *                 p0 +---------|......\..+ p1      |               V z
 *                     \        |       \    .      |
 *                       \      |        #     .    |
 *                          \   |         \      .  |
 *                             \+----------\--------+
 *                              p4          \        p5
 *                                           \
 *                                            V
 */
    // aliases for cube's vertices
    vec3i_t* p0 = cube->vertices[0];
    vec3i_t* p1 = cube->vertices[1];
    vec3i_t* p2 = cube->vertices[2];
    vec3i_t* p3 = cube->vertices[3];
    vec3i_t* p4 = cube->vertices[4];
    vec3i_t* p5 = cube->vertices[5];
    vec3i_t* p6 = cube->vertices[6];
    vec3i_t* p7 = cube->vertices[7];
    ray_t* ray = obj_ray_new(0, 0, 0);
    //plane_t* plane = obj_plane_new(pt1, pt2, pt3);
    for (int i = g_min_rows; i <= g_max_rows; ++i) {
        for (int j = g_min_cols; j <= g_max_cols; ++j) {
            // we test whether the ray has hit the following surafaces:
            // (p0, p1, p2, p3), (p0, p4, p7, p3)
            // (p4, p5, p6, p7), (p5, p1, p2, p6)
            // (p7, p6, p2, p3), (p0, p4, p5, p1)

            // the final pixel to render
            // we keep the z to find the furthest one from the origin and we draw its x and y
            vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MIN};
            // the color of the rendered pixel
            color_t rendered_color;
            // which z the ray currently hits the plane - can be up to two hits
            int z_hit;
            // through (p0, p1, p2)
            plane_t* plane = obj_plane_new(p0, p1, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p1, p2, p3) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[0];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p0, p4, p7);
            obj_plane_set(plane, p0, p4, p7);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p4, p7, p3) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[1];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p4, p5, p6);
            obj_plane_set(plane, p4, p5, p6);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p4, p5, p6, p7) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[2];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p5, p1, p2);
            obj_plane_set(plane, p5, p1, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p5, p1, p2, p6) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[3];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p7, p6, p2);
            obj_plane_set(plane, p7, p6, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p7, p6, p2, p3) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[4];
                rendered_point = (vec3i_t) {j, i, z_hit};
            } 
            // through (p0, p4, p5);
            obj_plane_set(plane, p0, p4, p5);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p4, p5, p1) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[5];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // if it's valid, i.e. at least one intersection, rendered it
            if (rendered_point.z != INT_MIN)
                draw_pixel(rendered_point.x, rendered_point.y, rendered_color);
            obj_plane_free(plane);
        } /* for columns */
    } /* for rows */
    obj_ray_free(ray);
    // render with with ncurse's `refresh`
    refresh();
}
