#ifndef	_PICTDISPLAY_

#include "Displays.h"
#include "Graphics.h"

#include <Windows.h>

void	PictPoint(), PictLine(), PictMoveTo(), PictLineTo();
void	PictRectangle(), PictFilledRectangle();
void	PictCircle(), PictFilledCircle();
void	PictErase(), PictEraseRectangle();
void	PictStartPolygon(), PictAddVertex(), PictOutlinePolygon(), PictFillPolygon();
void	PictText();
int		PictTextWidth(), PictTextHeight();
void	PictInit(), PictClose(), PictHardcopy();
void	PictString(), PictColor(), PictAlu(), PictLineStyle(), PictLinePattern(), PictPenSize();

#define PICT_MAX_POLY_POINTS 256
 
typedef struct {

		char		*filename;
		int			fp;
		
		int last_x;
		int last_y;
		
		int poly_n;
		int poly_x[PICT_MAX_POLY_POINTS];
		int poly_y[PICT_MAX_POLY_POINTS];
		
} PictParams;


#define	_PICTDISPLAY_
#endif
