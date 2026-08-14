/*
 *      turtleshape.h       logo turtle shape module
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

#ifndef _TURTLESHAPE_H
#define _TURTLESHAPE_H

#include "logo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TURTLE_SHAPE_POINTS 128

struct turtle_shape_point {
    int x, y;
};

/* Compute the outline of the turtle as a closed polygon.

     x, y                turtle coordinates of the turtle's origin
     heading             Logo degrees, clockwise from north
     xscale, yscale      pixels per turtle step
     x_center, y_center  screen position of the turtle's origin
     out_points          filled with the outline; needs room for
                         MAX_TURTLE_SHAPE_POINTS entries

   Returns the number of vertices written. */
extern int turtle_shape(FLONUM x, FLONUM y, FLONUM heading,
			FLONUM xscale, FLONUM yscale,
			int x_center, int y_center,
			struct turtle_shape_point *out_points);

/* The radius of the smallest circle centered on the turtle's origin that
   contains the whole shape, in turtle steps. */
extern FLONUM turtle_shape_extent(void);

/* Shapes are numbered from zero; shape zero is the default. */
extern int turtle_shape_count(void);
extern char *turtle_shape_name(int shape);	/* NULL if out of range */
extern int current_turtle_shape(void);
extern BOOLEAN set_turtle_shape(int shape);	/* FALSE if out of range */

#ifdef __cplusplus
}
#endif

#endif /* _TURTLESHAPE_H */
