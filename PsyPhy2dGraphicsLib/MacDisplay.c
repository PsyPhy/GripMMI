#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <sioux.h>

#include "MacDisplay.h"

/***************************************************************************/

MacParams	_mac_params = {"MacDisplay", 0};
struct _display	_MacDisplay = {
				"Mac Display",
				0, 0, 0, 0,
				MacPoint, MacLine, MacMoveTo, MacLineTo,
				MacText, MacTextWidth, MacTextHeight,
				MacRectangle, MacFilledRectangle,
				MacCircle, MacFilledCircle,
				MacStartPolygon, MacAddVertex, MacOutlinePolygon, MacFillPolygon,
				MacErase, MacEraseRectangle,
				MacLineStyle, MacLinePattern,
				MacColor, MacAlu, 
				MacPenSize,
				MacInit, MacClose, MacHardcopy,
				SOLID,		/* Line Pattern */
				SET, 		/* ALU */
				FOREGROUND, /* Color */
				2, 			/* Symbol Size (radius) */
				-1, -1, 	/* Desired Width and Height */
				FALSE,		/* Black and White */
				NULL,		/* Redraw cache */
				&_mac_params
				};

Display		MacDisplay = &_MacDisplay;

struct _display	_SIOUXDisplay = {
				"Mac SOIUX Display",
				0, 0, 0, 0,
				MacPoint, MacLine, MacMoveTo, MacLineTo,
				MacText, MacTextWidth, MacTextHeight,
				MacRectangle, MacFilledRectangle,
				MacCircle, MacFilledCircle,
				MacStartPolygon, MacAddVertex, MacOutlinePolygon, MacFillPolygon,
				MacErase, MacEraseRectangle,
				MacLineStyle, MacLinePattern,
				MacColor, MacAlu, 
				MacPenSize,
				SIOUXInit, MacClose, MacHardcopy,
				SOLID,		/* Line Pattern */
				SET, 		/* ALU */
				FOREGROUND, /* Color */
				2, 			/* Symbol Size (radius) */
				-1, -1, 	/* Desired Width and Height */
				FALSE,		/* Black and White */
				NULL,		/* Redraw cache */
				&_mac_params
				};

Display		SIOUXDisplay = &_SIOUXDisplay;

RGBColor	MacColorTable[] = {

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
				

local Pattern line_pattern[] = {
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

#undef CONSOLE_ROWS
#define CONSOLE_ROWS 5

void	MacInit ( Display display ) {

	register MacParams	*params = display->parameters;

	Rect		windowBounds;
	double		width, height, fx;
	int			i;

	/* 
		Create a window for graphics.
	*/
	windowBounds = qd.screenBits.bounds;

	InsetRect(&windowBounds, 5, 5 );
	windowBounds.top += 20;

	if ( display->desired_width > 0.0 ) {

		width = windowBounds.right - windowBounds.left;
		fx = display->desired_height / display->desired_width;
		height = fx * width;
		if ( height > windowBounds.bottom - windowBounds.top ) {
			height = windowBounds.bottom - windowBounds.top;
			fx = display->desired_width / display->desired_height;
			width = fx * height;
		}
			
		windowBounds.right = windowBounds.left + width;
		windowBounds.bottom = windowBounds.top + height;
		
	}
 
	params->window = NewCWindow(nil, &windowBounds, "\pMacDisplay", true, noGrowDocProc, 
						(WindowPtr) -1, true, 0);
	SetPort(params->window);
	
	display->left = 0;
	display->top = 0;
	display->bottom = windowBounds.bottom - windowBounds.top - 1;
	display->right = windowBounds.right - windowBounds.left - 1;

	MacColor(display, display->color);
	MacLinePattern(display, display->pattern);
	MacAlu(display, display->alu);

	}

void	SIOUXInit ( Display display ) {

	register MacParams	*params = display->parameters;

	Rect		windowBounds;
	double		width, height, fx;
	int			i;


	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	/* Don¹t exit the program after it runs or ask whether to save the window when the program exit */ 
	SIOUXSettings.autocloseonquit = FALSE; 
	SIOUXSettings.asktosaveonclose = FALSE;
	
	/* Make the window large enough to fit n lines of text that contain 80 characters. */
	SIOUXSettings.columns = 80; SIOUXSettings.rows = CONSOLE_ROWS;
	
	/* Place the window in the lower part of the screen. */	
	windowBounds = qd.screenBits.bounds;
	SIOUXSettings.toppixel = windowBounds.bottom - CONSOLE_ROWS * 15; 
	SIOUXSettings.leftpixel = 10;
	
	/* Move cursor to bottom of the screen so that we can see the output as it comes. */
	
	for ( i = 0; i < CONSOLE_ROWS; i++ ) fprintf( stderr, "\n" );

	/* 
		Create a window for graphics.
	*/
	windowBounds = qd.screenBits.bounds;
	InsetRect(&windowBounds, 10, 0 );
	windowBounds.top += 40;
	windowBounds.bottom -= 20 + CONSOLE_ROWS * 15;

	if ( display->desired_width > 0.0 ) {

		width = windowBounds.right - windowBounds.left;
		fx = display->desired_height / display->desired_width;
		height = fx * width;
		if ( height > windowBounds.bottom - windowBounds.top ) {
			height = windowBounds.bottom - windowBounds.top;
			fx = display->desired_width / display->desired_height;
			width = fx * height;
		}
			
		windowBounds.right = windowBounds.left + width;
		windowBounds.bottom = windowBounds.top + height;
		
	}

	params->window = NewCWindow(nil, &windowBounds, "\pMacDisplay", true, noGrowDocProc, 
						(WindowPtr) -1, true, 0);
	SetPort(params->window);
	
	display->top = 0;
	display->left = 0;
	display->bottom = windowBounds.bottom - windowBounds.top - 1;
	display->right = windowBounds.right - windowBounds.left - 1;

	MacColor(display, display->color);
	MacLinePattern(display, display->pattern);
	MacAlu(display, display->alu);	

}


void	MacClose ( Display display ) {

	register MacParams	*params = display->parameters;
	
}

void	MacHardcopy ( Display display ) {

	register MacParams	*params = display->parameters;
	
}	

/***************************************************************************/

/* 
	All operations are carried out in device  or pixel coordinates.
*/

#define PEN_RADIUS	1

void	MacPoint ( Display display, int x, int y) {

	Rect	rect;
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

/*
	rect.left = x;
	rect.right = x + PEN_RADIUS;
	rect.top = y;
	rect.bottom = y + PEN_RADIUS;
	MoveTo( x, y );
	PaintOval( &rect );
	

*/
	MacLine(display, x, y, x, y);

	

	}

void	MacLine	( Display display, int x1, int y1, int x2, int y2) {

	register MacParams	*params = (MacParams *)display->parameters;

	SetPort(params->window);
	MoveTo(x1, y1);
	LineTo(x2, y2);

	
	}

void	MacMoveTo ( Display display, int x1, int y1) {

	register MacParams	*params = (MacParams *)display->parameters;

	SetPort(params->window);
	MoveTo(x1, y1);

	
	}

void	MacLineTo	( Display display, int x2, int y2 ) {

	register MacParams	*params = (MacParams *)display->parameters;

	SetPort(params->window);
	LineTo(x2, y2);

	
	}

void	MacText	( Display display, char *string, int x, int y, double dir ) {
	
	register MacParams	*params = display->parameters;
	unsigned char	 	text[256];
	

	SetPort(params->window);
	
	memcpy( text + 1, string, strlen( string ) + 1 );
	*text = strlen( string );

	MoveTo( x, y );
	DrawString( text );

}

int	MacTextWidth ( Display display, char *string ) {
	
	return (7 * strlen(string));
	
	}
	
int	MacTextHeight ( Display display, char *string ) {
	
	return (10);
	
	}
	
	
void	MacRectangle	( Display display, int x1,int y1, int x2, int y2) {
	
	Rect	rect;
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

	rect.left = x1;
	rect.right = x2;
	rect.top = y1;
	rect.bottom = y2;
	MoveTo( x2, y2 );
	FrameRect( &rect );

	}

void	MacFilledRectangle	( Display display, int x1,int y1, int x2, int y2) {
	
	Rect	rect;
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

	rect.left = x1;
	rect.right = x2;
	rect.top = y1;
	rect.bottom = y2;
	MoveTo( x2, y2 );
	PaintRect( &rect );

	}

void	MacCircle	(Display display, int x, int y, int radius )

{

	Rect	rect;
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

	rect.left = x - radius;
	rect.right = x + radius;
	rect.top = y - radius;
	rect.bottom = y + radius;
	MoveTo( x, y );
	FrameOval( &rect );
		
}

void	MacFilledCircle	(Display display, int x, int y, int radius )

{

	Rect	rect;
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

	rect.left = x - radius;
	rect.right = x + radius;
	rect.top = y - radius;
	rect.bottom = y + radius;
	MoveTo( x, y );
	FrameOval( &rect );
	PaintOval( &rect );
	fprintf( stderr, "%d %d %d\n", x, y, radius );
		
}

void	MacEraseRectangle	( Display display, int x1, int y1, int x2, int y2) {

	Rect	rect;
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

	rect.left = x1;
	rect.right = x2;
	rect.top = y1;
	rect.bottom = y2;
	MoveTo( x2, y2 );
	EraseRect( &rect );

	}

void	MacErase ( Display display) {

	register MacParams	*params = display->parameters;
	Rect	rect;

	SetPort(params->window);

	rect.left = display->left;
	rect.right = display->right;
	rect.top = display->top;
	rect.bottom = display->bottom;
	MoveTo( display->left, display->top );
	EraseRect( &rect );
	}

void	MacAlu	( Display display, int alu) {

	register MacParams	*params = display->parameters;

	display->alu = alu;

	}

void	MacLineStyle ( Display display, int style ) {
	
	register MacParams	*params = display->parameters;

	if ( display->bw ) MacLinePattern( display, StyleToPattern( style ) ); 
	else MacColor(display, StyleToColor( style ) );

	}

void	MacLinePattern	( Display display, int pattern ) {
	
	register MacParams	*params = display->parameters;
	SetPort(params->window);

	PenPat( &line_pattern[ pattern ] );

	}

void	MacColor ( Display display, int color) {
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);

	if ( display->bw ) RGBForeColor( &MacColorTable[ BLACK ] );
	else RGBForeColor( &MacColorTable[ color ] );

	}

void	MacPenSize ( Display display, int size) {
	
	register MacParams	*params = (MacParams *)display->parameters;
	SetPort(params->window);
	if ( size ) PenSize( size, size );
	else PenNormal();

	}

void	MacStartPolygon ( Display display ) {

	register MacParams	*params = display->parameters;
	
	params->poly_n = 0;

	}
void	MacAddVertex ( Display display, int x, int y ) {

	register MacParams	*params = display->parameters;
	
	params->poly_x[ params->poly_n ] = x;
	params->poly_y[ params->poly_n ] = y;
	if ( params->poly_n < MAC_MAX_POLY_POINTS - 1 ) params->poly_n++;
	

	}
void	MacOutlinePolygon ( Display display ) {

	register MacParams	*params = display->parameters;
	
	int i;
	
	Point( display, params->poly_x[0], params->poly_y[0] );
	for ( i = 0; i < params->poly_n; i++ ) Lineto( display, params->poly_x[i], params->poly_y[i] );

	}
void	MacFillPolygon ( Display display ) {

	register MacParams	*params = display->parameters;

	}

