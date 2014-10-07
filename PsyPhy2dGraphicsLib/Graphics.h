/****************************************************************************/
/*                                                                          */
/*                             Graphics.h                                   */
/*                                                                          */
/****************************************************************************/

// Disable warnings about unsafe functions.
// We use the 'unsafe' versions to maintain source-code compatibility with Visual C++ 6
#define _CRT_SECURE_NO_WARNINGS

/*    (c) Copyright 1994, 1995 - PsyPhy Consulting. All Rights Reserved.    */

#ifndef	_Graphics_

#include "useful.h"

#ifdef __cplusplus 
extern "C" {
#endif

/*
    These macros allow us to write generic graphic programs, without
    regard for the actual device. This would be better accomplished
    in C++, but we don't have it for OS9.
*/

#define Point(obj, x, y)		      (*(obj->point))(obj, x, y)
#define	Moveto(obj, x, y)		      (*(obj->moveto))(obj, x, y)  	
#define	Lineto(obj, x, y)		      (*(obj->lineto))(obj, x, y)	      
#define Line(obj, x1, y1, x2, y2)	(*(obj->line))(obj, x1, y1, x2, y2)

#define Rectangle(obj, x1, y1, x2, y2)	      (*(obj->rectangle))(obj, x1,y1,x2,y2)
#define FilledRectangle(obj, x1, y1, x2, y2)	(*(obj->filled_rectangle))(obj, x1,y1,x2,y2)
#define Circle(obj, x, y, r)		              (*(obj->circle))(obj, x,y,r)
#define FilledCircle(obj, x, y, r)	          (*(obj->filled_circle))(obj, x,y,r)

#define	StartTrace(obj, x, y)		    (*(obj->start_trace))(obj, x, y)	      
#define	ContinueTrace(obj, x, y)		(*(obj->continue_trace))(obj, x, y)	      
#define	EndTrace(obj, x, y)		      (*(obj->end_trace))(obj, x, y)	      

#define StartPolygon(obj) 		  (*(obj->start_polygon))(obj)
#define AddVertex(obj, x, y) 		(*(obj->add_vertex))(obj, x, y)
#define OutlinePolygon(obj) 		(*(obj->outline_polygon))(obj)
#define FillPolygon(obj) 		    (*(obj->fill_polygon))(obj)

#define Erase(obj)		        (*(obj->erase))(obj)
#define EraseRectangle(obj,x1,y1,x2,y2)	(*(obj->erase_rectangle))(obj,x1,y1,x2,y2)
#define LineStyle(obj, sty)		(*(obj->set_style))(obj, sty)
#define	LinePattern(obj, pat)		(*(obj->set_pattern))(obj, pat)
#define Color(obj, col)			(*(obj->set_color))(obj, col)
#define ColorRGB(obj, r, g, b)			(*(obj->set_color_rgb))(obj, r, g, b)
#define Alu(obj, alu)			(*(obj->set_alu))(obj, alu)
#define Pen(obj, pen)			(*(obj->set_pen))(obj, pen)

#define Text(obj, str, x, y, dir)	(*(obj->text))(obj, str, x, y, dir)
#define	TextWidth(obj, str)		(*(obj->text_width))(obj, str)
#define	TextHeight(obj, str)		(*(obj->text_height))(obj, str)

#define Init(obj)			(*(obj->init))(obj)
#define InitDisplay(obj)		DisplayInit(obj)
#define Close(obj)			(*(obj->close))(obj)
#define Hardcopy(obj,filename)			(*(obj->hardcopy))(obj, filename)

/*
    Here we define the line style and ALU constants. Since this was developed
    originally on an HP9000, Starbase equivalents are used. Other devices
    should translate these values appropriately.
*/

/* Color Tables - matches gl/gl.h */

#define COLORS 16

#define BLACK			0
#define RED				1
#define GREEN			2
#define YELLOW			3
#define BLUE			4
#define MAGENTA			5
#define CYAN			6
#define WHITE			7

#define BACKGROUND  WHITE
#define FOREGROUND  BLACK

#define GREY_LEVELS 8

#define GREY1      8
#define GREY2      9
#define GREY3      10
#define GREY4      11
#define GREY5      12
#define GREY6      13
#define GREY7      14
#define GREY8      15

#define Grey(n)		(n + 8)

#define TRANSPARENT_COLOR	16
 
/* ALUs */
#define SET   3
#define XOR   6
#define CLEAR 0
#define ALUS  3

/* Line Patterns */
#define SOLID            0
#define DASH             1
#define DOT              2
#define DASH_DOT         3
#define DASH_DOT_DOT     4
#define LONG_DASH        5
#define CENTER_DASH      6
#define CENTER_DASH_DASH 7
#define PATTERNS 8



/* Symbols */
#define SYMBOL_POINT			0
#define SYMBOL_OPEN_SQUARE		1
#define SYMBOL_CROSS			2
#define SYMBOL_X			3
#define SYMBOL_OPEN_CIRCLE		4
#define SYMBOL_FILLED_CIRCLE		5
#define SYMBOL_FILLED_SQUARE		6
#define SYMBOL_STAR			7
#define SYMBOLS				8

#define STYLES 8

/*	Keyword locations for Label and Titles. */

#define	CENTER			-1
#define	INSIDE_LEFT		-2
#define	INSIDE_RIGHT	-3
#define	INSIDE_TOP		-4
#define	INSIDE_BOTTOM	-5
#define	OUTSIDE_LEFT	-6
#define	OUTSIDE_RIGHT	-7
#define	OUTSIDE_TOP		-8
#define	OUTSIDE_BOTTOM	-9

#define LEFT_JUSTIFY	-1
#define RIGHT_JUSTIFY	-2
#define TOP_JUSTIFY		-3
#define BOTTOM_JUSTIFY	-4
#define	CENTER_JUSTIFY	-5

typedef int ViewTxtAlign;

/*****************************************************************************/

/* 
	Map line styles to colors and patterns.  Style 0 should be used for grids,
	axes, etc.  Style 1 should be used for the main data.
*/

extern	int style_to_color[], style_to_pen[];

/* Map line styles to patterns. */

extern	int style_to_pattern[];

extern  int color_select_table[6];

#ifdef __cplusplus 
}
#endif

#define	_Graphics_

#endif

