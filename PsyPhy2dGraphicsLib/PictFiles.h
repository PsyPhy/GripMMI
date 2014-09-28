/***************************************************************************/
/*                                                                         */
/*                               PictFiles.h                               */
/*                                                                         */
/***************************************************************************/

/* Support for reading and writing PICT files. */

#ifndef _PictFiles_
#define _PictFiles_

int PictOpenFile ( char *filename, double width, double height );
void PictCloseFile( int fp );

void PictWriteLine( int fp, int x1, int y1, int x2, int y2 );
void PictWritePolygon(int fp, int n, int x[], int y[] );
void PictWriteFilledPolygon(int fp, int n, int x[], int y[] );

void PictWriteFontSize( int fp, int size );
void PictWriteText( int fp, int x, int y, char *string );

void PictWriteCircle( int fp, int x, int y, int radius );
void PictWriteFilledCircle( int fp, int x, int y, int radius );
void PictWriteRectangle( int fp, int x1, int y1, int x2, int y2 );
void PictWriteFilledRectangle( int fp, int x1, int y1, int x2, int y2 );

void PictWriteRGBColor( int fp, int red, int green, int blue );
void PictWritePenSize( int fp, int size );
void PictWritePenPattern( int fp, unsigned char table[8] );

#endif


