// PsyPhyGraphicsTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/***************************************************************************/
/*                                                                         */
/*                             2dGraphicsTest.c                            */
/*                                                                         */
/***************************************************************************/

/* (c) Copyright 1994, 1995, 2006 - PsyPhy Consulting. All Rights Reserved.*/

/*
 *	Test the 2D Graphics Views and Displays systems.
 */
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include <2dMatrix.h>
//#include <Regression.h>

#include "..\Useful\Useful.h"
#include "..\PsyPhy2dGraphicsLib\OglDisplayInterface.h"
#include "..\PsyPhy2dGraphicsLib\OglDisplay.h"
#include "..\PsyPhy2dGraphicsLib\Graphics.h"
#include "..\PsyPhy2dGraphicsLib\Views.h"
#include "..\PsyPhy2dGraphicsLib\Layouts.h"

Display display; 
Layout  layout;
View	view1, view2;

local double	x[] = {.2, .3, .4, .1, .7, .9, .5};
local double	y[] = {.1, .4, .7, .3, .9, .5, .6};

local double ellipse[2][2] = { { 4.0, 0.0 }, { 0.0, 1.0 }};
local double scale = 1.0;
local double center[2] = { 0.25, 0.4 };

local double p[3] = { 0.2, 0.0, 1.0 };

local double ramp( void *ptr, double angle ) {

  while ( angle < - PI ) angle += 2.0 * PI;
  while ( angle > PI ) angle -= 2.0 * PI;
  return( angle / PI );

}

int _tmain(int argc, _TCHAR* argv[])
{

	float r;

	char    *string = "TESTTESTTESTTESTTestTestTest\nTestT\ne\ns\nt";

	display = DefaultDisplay();
	DisplayInit( display );
	Erase( display );


	Color( display, GREEN );
	glprintf( 300, 300, 12.0, "TESTAGAIN" );
	//  DisplayLabel( display, "TESTAGAIN", 600, 600, 0, LEFT_JUSTIFY, TOP_JUSTIFY );
	Color( display, RED );
	for ( r = 100; r < 500; r += 100 ) FilledCircle( display, r, r, 30.0 );
	Swap();
	RunOglWindow();

	Close( display );

	return 0;
}

