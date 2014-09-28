/*****************************************************************************/
/*                                                                           */
/*                               useful.h                                    */
/*                                                                           */
/*****************************************************************************/

/* Stop warnings about double to float. */
#pragma warning( disable : 4244 )  
#pragma warning( disable : 4305 )  

#ifndef _useful_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define X 0
#define Y 1
#define Z 2
#define M 3

#ifndef	Pi
# ifdef M_PI
#  define Pi M_PI
# else
#  define Pi	3.141592653589796
# endif
#endif
#ifndef	PI

#define PI Pi

#endif

#define ROOT2 1.4142135623730951455
#define ROOT3 1.73205080756887729352744634150587

#define SUCCESS 0
#define FAILURE -1

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define YES 1
#define NO 0

#define PATHLENGTH 256

#define local static
#define global extern

#define degrees(r) ((r)*180.0/PI)
#define radians(d) ((d)*PI/180.0)

#define datan2(y,x)	degrees( atan2((y),(x)) )
#define upper_half(d)	((d) < 0.0 ? ((d) + 180.0) : (d))

#ifndef HUGE
#define HUGE HUGE_VAL
#endif

#define NaN (*((double *) &__nan))
#define ESC 0x1B

#define max(x,y) (((x) > (y)) ? (x) : (y))
#define min(x,y) (((x) < (y)) ? (x) : (y))

#define	PATH_LENGTH	128	/* Maximum length of a full file specification. */

#define random() rand()
#define MAX_RANDOM RAND_MAX

/* Prototypes for lppa.c */

#ifdef  __cplusplus

extern "C"

   {

#endif

void *ealloc( size_t size );
local unsigned long __nan = 0x7ff7ffff;

int open_pc ( char *filename, int flags );
FILE *fopen_pc ( char *filename, char *flags );
void redirect_argv ( int *argc, char **argv[] );

char *fgetsNonblank ( char *buffer, int n, FILE *fp );

#ifdef  __cplusplus

}

#endif

#define _useful_
#endif
