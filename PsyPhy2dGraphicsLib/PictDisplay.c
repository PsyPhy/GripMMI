#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "PictFiles.h"
#include "PictDisplay.h"

/***************************************************************************/

PictParams	_pict_params = {"PictFile", 0, 0, 0};
struct _display	_PictDisplay = {
				"Pict Display",
				0, 0, 0, 0,
				PictPoint, PictLine, PictMoveTo, PictLineTo,
				PictText, PictTextWidth, PictTextHeight,
				PictRectangle, PictFilledRectangle,
				PictCircle, PictFilledCircle,
				PictStartPolygon, PictAddVertex, PictOutlinePolygon, PictFillPolygon,
				PictErase, PictEraseRectangle,
				PictLineStyle, PictLinePattern,
				PictColor, PictAlu, 
				PictPenSize,
				PictInit, PictClose, PictHardcopy,
				SOLID,		/* Line Pattern */
				SET, 		/* ALU */
				FOREGROUND, /* Color */
				2, 			/* Symbol Size (radius) */
				-1, -1, 	/* Desired Width and Height */
				&_pict_params
				};

Display		PictDisplay = &_PictDisplay;

RGBColor	PictColorTable[] = {

				{ 0x0000, 0x0000, 0x0000 },	/* Black	*/
				{ 0xffff, 0xffff, 0xffff },	/* White	*/
				{ 0xffff, 0x0000, 0x0000 },	/* Red		*/
				{ 0xffff, 0xcccc, 0x0000 },	/* Yellow (orange)	*/
				{ 0x0000, 0xffff, 0x0000 },	/* Green	*/
				{ 0x0000, 0xffff, 0xffff },	/* Cyan		*/
				{ 0x0000, 0x0000, 0xffff },	/* Blue		*/
				{ 0xffff, 0x0000, 0xffff },	/* Magenta	*/

				{ 0x1111, 0x1111, 0x1111 },	/* Grey1	*/
				{ 0x3333, 0x3333, 0x3333 },	/* Grey2	*/
				{ 0x5555, 0x5555, 0x5555 },	/* Grey3	*/
				{ 0x7777, 0x7777, 0x7777 },	/* Grey4	*/
				{ 0x9999, 0x9999, 0x9999 },	/* Grey5	*/
				{ 0xbbbb, 0xbbbb, 0xbbbb },	/* Grey6	*/
				{ 0xdddd, 0xdddd, 0xdddd },	/* Grey7	*/
				{ 0xffff, 0xffff, 0xffff },	/* Grey8	*/

			};
				

local unsigned char line_pattern[][8] = {
							{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, /* SOLID */
							{ 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00 }, /* DASH */
							{ 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 }, /* DOT */
							{ 0xff, 0x0f, 0x0f, 0xf0, 0xf0, 0xff, 0x0f, 0x0f }, /* DASH_DOT */
							{ 0xff, 0x0f, 0x00, 0xf0, 0xff, 0x0f, 0x00, 0xf0 }, /* DASH_DOT_DOT */
							{ 0xff, 0xf0, 0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf0 }, /* LONG_DASH */
							{ 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0 }, /* CENTER_DASH */
							{ 0x0f, 0xf0, 0xff, 0x0f, 0xf0, 0xff, 0x0f, 0xf0 }
						};
							
							
/***************************************************************************/

void	PictInit ( Display display ) {

	register PictParams	*params = display->parameters;

#ifdef LANDSCAPE
	/*
	 * An A4 landscape picture, at 72 dpi, with 1 cm borders. 
	 * ClarisDraw adds the border. 
	 */
	int width = floor( 27.4 * 72.0 / 2.54 );
	int height = floor( 18.9 * 72.0 / 2.54 );
#else
	/*
	 * Top half of an A4 sheet, which is a convenient size for jounals. 
	 */
	int width = floor( 18.9 * 72.0 / 2.54 );
	int height = floor( (double) width * 0.75 );
#endif
	 
	if ( display->desired_width > 0.0 ) width = floor( display->desired_width * 72.0 );
	if ( display->desired_height > 0.0 ) height = floor( display->desired_height * 72.0 );
	
	params->fp = PictOpenFile( params->filename, width, height );

 	display->left = 0;
	display->right = display->left + width;
	display->top = 0;
	display->bottom = display->top + height;

	PictColor(display, display->color);
	PictLinePattern(display, display->pattern);
	PictAlu(display, display->alu);

	}

void	PictClose ( Display display ) {

	register PictParams	*params = display->parameters;
	
	PictCloseFile( params->fp );

}

void	PictHardcopy ( Display display ) {

	register PictParams	*params = display->parameters;
	
	
}	

/***************************************************************************/

/* 
	All operations are carried out in device  or pixel coordinates.
*/

#define PEN_RADIUS	1

void	PictPoint ( Display display, int x, int y) {

	register PictParams	*params = (PictParams *)display->parameters;

	PictWriteLine( params->fp, x, y, x + 1, y + 1);
	params->last_x = x;
	params->last_y = y;

	}

void	PictLine	( Display display, int x1, int y1, int x2, int y2) {

	register PictParams	*params = (PictParams *)display->parameters;

	PictWriteLine( params->fp, x1, y1, x2, y2 );
	params->last_x = x2;
	params->last_y = y2;
	
	}

void	PictMoveTo ( Display display, int x1, int y1) {

	register PictParams	*params = (PictParams *)display->parameters;

	params->last_x = x1;
	params->last_y = y1;

	
	}

void	PictLineTo	( Display display, int x2, int y2 ) {

	register PictParams	*params = (PictParams *)display->parameters;

	PictLine( display, params->last_x, params->last_y, x2, y2 );
	
	}

void	PictText	( Display display, char *string, int x, int y, double dir ) {
	
	register PictParams	*params = display->parameters;

	PictWriteText( params->fp, x, y, string );
}

int	PictTextWidth ( Display display, char *string ) {
	
	return (8 * strlen(string));
	
	}
	
int	PictTextHeight ( Display display, char *string ) {
	
	return (12);
	
	}
	
	
void	PictRectangle	( Display display, int x1,int y1, int x2, int y2) {
	
	register PictParams	*params = (PictParams *)display->parameters;
	PictWriteRectangle( params->fp, x1, y1, x2, y2 );
	params->last_x = x2;
	params->last_y = y2;

	}

void	PictFilledRectangle	( Display display, int x1,int y1, int x2, int y2) {

	register PictParams	*params = (PictParams *)display->parameters;
	PictWriteFilledRectangle( params->fp, x1, y1, x2, y2 );
	params->last_x = x2;
	params->last_y = y2;
	
	
	}

void	PictCircle	(Display display, int x, int y, int radius )

{
	register PictParams	*params = (PictParams *)display->parameters;

	PictWriteCircle( params->fp, x, y, radius );
	params->last_x = x;
	params->last_y = y;
	
}

void	PictFilledCircle	(Display display, int x, int y, int radius )

{
	register PictParams	*params = (PictParams *)display->parameters;

	PictWriteFilledCircle( params->fp, x, y, radius );
	params->last_x = x;
	params->last_y = y;
		
}

void	PictEraseRectangle	( Display display, int x1, int y1, int x2, int y2) {

	}

void	PictErase ( Display display) {

	}

void	PictAlu	( Display display, int alu) {

	register PictParams	*params = display->parameters;

	display->alu = alu;

	}

void	PictLineStyle ( Display display, int style ) {
	
	register PictParams	*params = display->parameters;
	PictColor(display, style_to_color[ style % STYLES ] );

	}

void	PictLinePattern	( Display display, int pattern ) {
	
	register PictParams	*params = display->parameters;
	
	PictWritePenPattern( params->fp, line_pattern[ pattern % PATTERNS ] );
	
	}

void	PictColor ( Display display, int color) {
	
	register PictParams	*params = (PictParams *)display->parameters;
	
	PictWriteRGBColor( params->fp, 
		PictColorTable[ color % COLORS ].red,
		PictColorTable[ color % COLORS ].green,
		PictColorTable[ color % COLORS ].blue );

	}

void	PictPenSize ( Display display, int size) {
	
	register PictParams	*params = (PictParams *)display->parameters;
	PictWritePenSize( params->fp, size );
	
	}

void	PictStartPolygon ( Display display ) {

	register PictParams	*params = display->parameters;

	params->poly_n = 0;

	}
void	PictAddVertex ( Display display, int x, int y ) {

	register PictParams	*params = display->parameters;
	
	params->poly_x[ params->poly_n ] = x;
	params->poly_y[ params->poly_n ] = y;
	if ( params->poly_n < PICT_MAX_POLY_POINTS - 1 ) params->poly_n++;
	else fprintf( stderr, "PictAddVertex: vertex overflow.\n" );
	

	}
void	PictOutlinePolygon ( Display display ) {

	register PictParams	*params = display->parameters;
	
	PictWritePolygon( params->fp, params->poly_n, params->poly_x, params->poly_y );
	

	}
void	PictFillPolygon ( Display display ) {

	register PictParams	*params = display->parameters;

	PictWriteFilledPolygon( params->fp, params->poly_n, params->poly_x, params->poly_y );

	}

