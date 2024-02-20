#include "objects.h"
#include "renderer.h"
#include "arg_parser.h"
#include "utils.h" // UT_MAX
#include <math.h> // sin, cos
#include <unistd.h> // for usleep
#include <stdlib.h> // exit
#include <time.h> // time
#include <signal.h> // signal

#include "getbno055.h"

/* Callback that clears the screen and makes the cursor visible when the user hits Ctr+C */
static void interrupt_handler(int int_num) {
    if (int_num == SIGINT) {
        render_end();
        exit(SIGINT);
    }
}

int main(int argc, char** argv) {
    arg_parse(argc, argv);

    get_i2cbus(i2c_bus, senaddr);

    set_mode(ndof);

    struct bnoeul bnod;

    // make sure we end gracefully if the user hits Ctr+C
    signal(SIGINT, interrupt_handler);

    render_init();

    // mesh_t* shape = obj_mesh_from_file(g_mesh_file, g_cx, g_cy, g_cz, g_width, g_height, g_depth);
    mesh_t* shape = obj_mesh_from_file(object_file, g_cx, g_cy, g_cz, g_cube_size, 1.2*g_cube_size, g_cube_size);

	obj_mesh_translate_by(shape, g_move_x, g_move_y, g_move_z);
	
	do{    
	get_eul(&bnod);
	
    	obj_mesh_rotate_to(shape,bnod.eul_pitc*M_PI/180,bnod.eul_head*M_PI/180,bnod.eul_roll*M_PI/180);
    	render_write_shape(shape);
    	render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
#endif
    
    }while(1);

    obj_mesh_free(shape);
    render_end();

    return 0;
}

