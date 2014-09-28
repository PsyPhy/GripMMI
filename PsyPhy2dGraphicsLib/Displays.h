/*****************************************************************************/
/*                                                                           */
/*                           Displays.h                                      */
/*                                                                           */
/*****************************************************************************/

/*    (c) Copyright 1994, 1995 - PsyPhy Consulting. All Rights Reserved.    */


#ifndef	_Displays_
#include "Graphics.h"
	
typedef enum { null_token, point_token, line_token, moveto_token, lineto_token, 
	start_token, continue_token, end_token,
	rectangle_token, filled_rectangle_token,
	circle_token, filled_circle_token, 
	start_polygon_token, add_vertex_token, 
	outline_polygon_token, fill_polygon_token,
	erase_token, erase_rectangle_token, 
	text_token, 
	style_token, pattern_token, color_token, alu_token, pen_token, rgb_token 
} Token;

typedef struct _mallocItem {
	void *buffer;
	void *next;
} DisplayMemoryItem;

typedef struct _cacheItem {
	
	Token token;
	struct _cacheItem  *next, *previous;
	
	union {
		struct {
			float x, y;
		} point;
		struct {
			float x1, y1, x2, y2;
		} line;
		struct {
			float left, top, right, bottom;
		} rectangle;
		struct {
			float x, y, radius;
		} circle;
		struct {
			float x, y;
			double dir;
			char *string;
		} text;
		struct {
			float r, g, b;
		} rgb;
		int style, pattern, color, alu, pen;
		
	} param;
	
} DisplayCacheItem;

struct _display {
	
	char	name[256];
	float	left;
	float	top;
	float	right;
	float	bottom;
	
	void	(*point)( struct _display *dsp, float x, float y );
	void	(*line)( struct _display *dsp, float x1, float y1, float x2, float y2 );
	void	(*moveto)( struct _display *dsp, float x, float y );
	void	(*lineto)( struct _display *dsp, float x, float y );
	
	void	(*start_trace ) ( struct _display *dsp, float x, float y );
	void	(*continue_trace ) ( struct _display *dsp, float x, float y );
	void	(*end_trace ) ( struct _display *dsp, float x, float y );
	
	void	(*text)( struct _display *dsp, char *string, float x, float y, double dir );
	float	(*text_width)( struct _display *dsp, char *string );
	float 	(*text_height)( struct _display *dsp, char *string );
	
	void	(*rectangle)( struct _display *dsp, float x1, float y1, float x2, float y2 );
	void	(*filled_rectangle)( struct _display *dsp, float x1, float y1, float x2, float y2 );
	void	(*circle)( struct _display *dsp, float x1, float y1, float radius );
	void	(*filled_circle)( struct _display *dsp, float x1, float y1, float radius );
	
	void	(*start_polygon)( struct _display *dsp );
	void	(*add_vertex)( struct _display *dsp, float x, float y );
	void	(*outline_polygon)( struct _display *dsp );
	void	(*fill_polygon)( struct _display *dsp );
	
	void	(*erase)( struct _display *dsp );
	void	(*erase_rectangle)( struct _display *dsp, float x1, float y1, float x2, float y2 );
	
	void	(*set_style)( struct _display *dsp, int style );
	void	(*set_pattern)( struct _display *dsp, int pattern );
	void	(*set_color)( struct _display *dsp, int color );
	void    (*set_color_rgb)( struct _display *dsp, float r, float g, float b );
	void	(*set_alu)( struct _display *dsp, int alu );
	void	(*set_pen)( struct _display *dsp, float pen );
	
	void	(*init)( struct _display *dsp );
	void	(*activate)( struct _display *dsp );
	
	void	(*swap)( struct _display *dsp );
	void	(*close)( struct _display *dsp );
	void	(*hardcopy)( struct _display *dsp, char *filename );
	int		(*input)( struct _display *dsp, float *x, float *y );
	
	int	pattern;
	int	alu;
	int	color;
	int   bw;
	float symbol_radius;
	
	double desired_width;
	double desired_height;
	
	double desired_left;
	double desired_top;
	
	DisplayCacheItem  *cache; 	
	DisplayCacheItem  *last_cache;
	int	cache_active;

	void 	*next;					// Next display an linked list.
	void	*parameters;			// Device dependent parameters.
	
};

typedef struct _display *Display;

#ifdef __cplusplus 
extern "C" {
#endif
	

/* These should be assigned at the beginning of each program. */

extern	Display	ScreenDisplay, HardcopyDisplay;
extern	Display	NullDisplay;
	
#define	_Displays_	1
	
#define	StyleToColor(style)	style_to_color[style % STYLES]
#define	StyleToPattern(style) style_to_pattern[style % STYLES]

Display DefaultDisplay ( void );
extern	Display PictDisplay;
void DisplaySetName( Display display, char *name );
	
	
#ifdef MACINTOSH
	extern	Display SIOUXDisplay;
#endif 
	
	
/* These are called before initializing the display. */

void DisplaySetSizeInches (Display display, double width, double height );
void DisplaySetSizeCM (Display display, double width, double height );
void DisplaySetSizePixels(Display display, int width, int height );
void DisplaySetScreenPosition (Display display, int left, int top );
void DisplaySetName( Display display, char *name );

/* These are called after DisplayInit(). */

void DisplaySetDefaults( Display display );
void DisplayDescribe (Display display);
void DisplayTitle (Display display, char *string, float x, float y, double dir);
void DisplayLabel (Display display, char *string, float x, float y, double dir, int x_justify, int y_justify);
void DisplaySymbol( Display display, float x, float y, int symbol );
void DisplayBox (Display display);

void DisplayArrow (Display display, float from_x, float from_y, float to_x, float to_y );


/* Pointer (mouse) input. */
	
#define DISPLAY_LEFT_MOUSE 0x01
#define DISPLAY_MIDDLE_MOUSE 0x02
#define DISPLAY_RIGHT_MOUSE 0x04
#define DISPLAY_ESCAPE 0x08

DisplayCacheItem *DisplayInsertCacheItem( Display display );
void DisplayInitCache( Display display );
void DisplayWalkCache ( Display input, Display output );
void DisplayFreeCache ( Display display );
void DisplayEnableCache( Display display );
void DisplayDisableCache( Display display );

Display CreateDisplay( Display model );
void	DisplayInit( Display display );
void	DisplayActivate( Display display );
int		DisplayInput( Display display, float *x, float *y );
void	DisplaySwap( Display display );
void	DestroyDisplays ( void );
	
#ifdef __cplusplus 
}
#endif

#endif

