#ifndef	_MACDISPLAY_

#include "Displays.h"
#include "Graphics.h"

#include <Windows.h>

#define MAC_MAX_POLY_POINTS 256

void	MacPoint(), MacLine(), MacMoveTo(), MacLineTo();
void	MacRectangle(), MacFilledRectangle();
void	MacCircle(), MacFilledCircle();
void	MacErase(), MacEraseRectangle();
void	MacStartPolygon(), MacAddVertex(), MacOutlinePolygon(), MacFillPolygon();
void	MacText();
int		MacTextWidth(), MacTextHeight();
void	MacInit(), MacClose(), MacHardcopy();
void	MacString(), MacColor(), MacAlu(), MacLineStyle(), MacLinePattern(), MacPenSize();

void	SIOUXInit();
 
typedef struct {

		char		*window_name;
		WindowPtr	window;
		
		int poly_n;
		int poly_x[MAC_MAX_POLY_POINTS];
		int poly_y[MAC_MAX_POLY_POINTS];
		
		} MacParams;


/* These values are set for the PowerBook screen. */

#define MAC_DISPLAY_TOP		40
#define MAC_DISPLAY_LEFT	30
#define	MAC_DISPLAY_WIDTH	488
#define MAC_DISPLAY_HEIGHT	190
#define	CONSOLE_OFFSET_TOP	22
#define	CONSOLE_OFFSET_LEFT	0
#define	CONSOLE_COLUMNS		80
#define	CONSOLE_ROWS		12
#define CONSOLE_TEXT_SIZE	9

#define	_MACDISPLAY_
#endif
