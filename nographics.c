
/* A dummy graphics implementation for Unix */

#ifdef X_DISPLAY_MISSING

int pw, ph, pc, pm, pv, px, py, bg;

#ifndef HAVE_WX
char *LogoPlatformName="Unix-Nographics";

void nop()
{
}
#endif
#endif
