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

#define count_of(a) ((int)(sizeof(a)/sizeof((a)[0])))

/* Vertices in turtle steps: forward is along the turtle's heading, right
   is ninety degrees clockwise from it. */
struct vertex {
    FLONUM forward, right;
};

#define TRIANGLE_HALF_BASE 6.0
#define TRIANGLE_ALTITUDE 19.0

static const struct vertex triangle_vertices[] = {
    { 0.0,		  TRIANGLE_HALF_BASE },	/* right base corner */
    { 0.0,		 -TRIANGLE_HALF_BASE },	/* left base corner */
    { TRIANGLE_ALTITUDE,  0.0 }			/* nose */
};

/* Centered on the turtle's origin, head forward, going down the right side
   and back up the left.  The nose and the tail lie on the axis, so each is a
   single vertex rather than a mirrored pair. */
static const struct vertex turtle_vertices[] = {
    {   14.0,   0.0 },	/* nose */
    {  10.25,   2.5 },
    {    8.5,  1.75 },
    {   7.25,   5.0 },
    {    8.0,   9.0 },
    {    3.0,  13.5 },	/* right front flipper */
    {    4.0,   7.0 },
    {    0.0,  8.25 },	/* widest point of the shell */
    {   -7.0,   6.0 },
    {  -8.75,   8.0 },
    {  -12.5,  4.25 },	/* right rear flipper */
    {   -8.5,  4.25 },
    {  -10.0,   1.0 },
    { -13.25,   0.0 },	/* tail */
    {  -10.0,  -1.0 },
    {   -8.5, -4.25 },
    {  -12.5, -4.25 },	/* left rear flipper */
    {  -8.75,  -8.0 },
    {   -7.0,  -6.0 },
    {    0.0, -8.25 },	/* widest point of the shell */
    {    4.0,  -7.0 },
    {    3.0, -13.5 },	/* left front flipper */
    {    8.0,  -9.0 },
    {   7.25,  -5.0 },
    {    8.5, -1.75 },
    {  10.25,  -2.5 }
};

/* No shape may have more than MAX_TURTLE_SHAPE_POINTS vertices. */
static const struct {
    char *name;
    int count;
    const struct vertex *vertices;
} shapes[] = {
    { "triangle", count_of(triangle_vertices), triangle_vertices },
    { "turtle",   count_of(turtle_vertices),   turtle_vertices }
};

static int current_shape = 0;
static FLONUM current_extent = TRIANGLE_ALTITUDE;

static FLONUM extent_of(int shape) {
    FLONUM extent = 0.0;
    int i;

    for (i = 0; i < shapes[shape].count; i++) {
	FLONUM forward = shapes[shape].vertices[i].forward;
	FLONUM right = shapes[shape].vertices[i].right;
	FLONUM radius = sqrt(forward*forward + right*right);

	if (radius > extent) extent = radius;
    }

    return extent;
}

int turtle_shape(FLONUM x, FLONUM y, FLONUM heading,
		 FLONUM xscale, FLONUM yscale,
		 int x_center, int y_center,
		 struct turtle_shape_point *out_points) {
    const struct vertex *vertices = shapes[current_shape].vertices;
    int count = shapes[current_shape].count;
    FLONUM real_heading, cos_real_heading, sin_real_heading;
    int i;

    real_heading = -heading + 90.0;
    cos_real_heading = cos((FLONUM)(real_heading*degrad));
    sin_real_heading = sin((FLONUM)(real_heading*degrad));

    /* cos(90*degrad) is 6e-17 rather than zero, which is enough to tip a
       vertex sitting on a half step across g_round's tie, so the shape and
       its mirror land on different pixels.  Headings that are multiples of
       ninety must come out exact. */
    if (fabs(cos_real_heading) < 1e-12) cos_real_heading = 0.0;
    if (fabs(sin_real_heading) < 1e-12) sin_real_heading = 0.0;

    for (i = 0; i < count; i++) {
	FLONUM forward = vertices[i].forward;
	FLONUM right = vertices[i].right;
	FLONUM delta_x = xscale*(FLONUM)(forward*cos_real_heading +
					 right*sin_real_heading);
	FLONUM delta_y = yscale*(FLONUM)(forward*sin_real_heading -
					 right*cos_real_heading);

	out_points[i].x = x_center + g_round(x + delta_x);
	out_points[i].y = y_center - g_round(y + delta_y);
    }

    return count;
}

FLONUM turtle_shape_extent(void) {
    return current_extent;
}

int turtle_shape_count(void) {
    return count_of(shapes);
}

char *turtle_shape_name(int shape) {
    if (shape < 0 || shape >= count_of(shapes)) return NULL;
    return shapes[shape].name;
}

int current_turtle_shape(void) {
    return current_shape;
}

BOOLEAN set_turtle_shape(int shape) {
    if (shape < 0 || shape >= count_of(shapes)) return FALSE;

    current_shape = shape;
    current_extent = extent_of(shape);
    return TRUE;
}
