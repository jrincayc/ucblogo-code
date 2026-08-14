/*
 *      turtleshape.c       logo turtle shape module
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "logo.h"
#include "globals.h"
#include "turtleshape.h"

#define TRIANGLE_HALF_BASE 6.0
#define TRIANGLE_ALTITUDE 19.0

/* Vertices in turtle steps: forward is along the turtle's heading, right
   is ninety degrees clockwise from it. */
static struct {
    FLONUM forward, right;
} local_vertices[MAX_TURTLE_SHAPE_POINTS] = {
    { 0.0,		  TRIANGLE_HALF_BASE },	/* right base corner */
    { 0.0,		 -TRIANGLE_HALF_BASE },	/* left base corner */
    { TRIANGLE_ALTITUDE,  0.0 }			/* nose */
};

static int local_vertex_count = 3;

int turtle_shape(FLONUM x, FLONUM y, FLONUM heading,
		 FLONUM xscale, FLONUM yscale,
		 int x_center, int y_center,
		 struct turtle_shape_point *out_points) {
    FLONUM real_heading, cos_real_heading, sin_real_heading;
    int i;

    real_heading = -heading + 90.0;
    cos_real_heading = cos((FLONUM)(real_heading*degrad));
    sin_real_heading = sin((FLONUM)(real_heading*degrad));

    for (i = 0; i < local_vertex_count; i++) {
	FLONUM forward = local_vertices[i].forward;
	FLONUM right = local_vertices[i].right;
	FLONUM delta_x = xscale*(FLONUM)(forward*cos_real_heading +
					 right*sin_real_heading);
	FLONUM delta_y = yscale*(FLONUM)(forward*sin_real_heading -
					 right*cos_real_heading);

	out_points[i].x = x_center + g_round(x + delta_x);
	out_points[i].y = y_center - g_round(y + delta_y);
    }

    return local_vertex_count;
}

FLONUM turtle_shape_extent(void) {
    FLONUM extent = 0.0;
    int i;

    for (i = 0; i < local_vertex_count; i++) {
	FLONUM forward = local_vertices[i].forward;
	FLONUM right = local_vertices[i].right;
	FLONUM radius = sqrt(forward*forward + right*right);

	if (radius > extent) extent = radius;
    }

    return extent;
}
