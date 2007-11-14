
/* A dummy graphics implementation for Unix */

#ifdef X_DISPLAY_MISSING

int pw, ph, pc, pm, pv, px, py, bg;

char *LogoPlatformName="Unix-Nographics";

#ifndef HAVE_WX
void nop()
{
}
#endif
#endif
