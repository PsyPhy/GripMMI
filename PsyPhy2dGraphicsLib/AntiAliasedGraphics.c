#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>
#include <winerror.h>

#define true  1
#define false 0

#include <gl/gl.h>
#include <gl/glut.h>

#include <useful.h>
#include "OglDisplayInterface.h"
#include "AntialiasedGraphics.h"

int wire = false;
int textures = true;
int gaussian = true;

float aa_colors_rgb[ AA_COLORS ][3] =
		{ {1.0, 1.0, 1.0}, 
      {0.75, 0.0, 0.25}, 
      {0.0, 1.0, 0.0}, 
      {0.0, 0.0, 1.0}, 
      {1.0, 1.0, 0.0},
      {1.0, 0.0, 1.0},
      {0.0, 1.0, 1.0},

 };


// Texture
static GLuint Texture[ AA_COLORS ];

/**************************************************************************************/

int round( double x )
{
    if(x > 0.0) return((int) (x + 0.5));
    else if(x < 0.0) return((int) (x - 0.5));
    else return( 0 );
}

/**************************************************************************************/

// Setup 1D textures for antialisasing.
void SetupAATextures( void )
{
	int i, j, k;
  double delta, distance;
	GLubyte bits[ AA_COLORS ][ 3 * AA_TEXTURE_SIZE ], c;

	// Crée la texture LinesTexture
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
	for(i = 0; i < AA_COLORS; i++) glGenTextures(1, &(Texture[i]));

	// Copie les pixels du bitmap formatés dans bits
	for(i = 0; i < AA_COLORS; i++) {
		for(j = 0; j < AA_TEXTURE_SIZE; j++) {
      // Intensity falls off as a gaussian around the center.
			if ( gaussian ) {
        delta = (double)( j - AA_TEXTURE_SIZE / 2 ) / (double)( AA_TEXTURE_SIZE / 2 );
        distance = delta * delta;
        c = (GLubyte) round( 255.0 * exp( - 5.0 * distance ) );
      }
      // Intensity is a sin around the center.
      // This was the original way, but it's not good because the gradiant
      //  is too steep as it goes to zero.
			else c = (GLubyte) round(255.0 * sin( PI * (double) j / (double) ( AA_TEXTURE_SIZE - 1)));
			for(k = 0; k < 3; k++) {
				bits[i][3*j + k] = round( c * aa_colors_rgb[i][k]);
      }
		}
	}

	// Construit et charge chaque niveau de MipMap
	for(i = 0; i < AA_COLORS; i++) {
      glBindTexture(GL_TEXTURE_1D, Texture[i]);
	    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      gluBuild1DMipmaps(GL_TEXTURE_1D, 3, AA_TEXTURE_SIZE, GL_RGB, GL_UNSIGNED_BYTE, bits[i]);
    }

	// Textures MODULATE au lieu de DECAL pour avoir les effets de lumière
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

/**************************************************************************************/

void InitAntialiasing( void ) {

	// Initializations
	if ( wire ) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor3d( 1.0, 1.0, 1.0 );
  glEnable(GL_BLEND);
  // glBlendEquation(GL_MAX);
  glBlendFunc(GL_ONE, GL_ONE);

	// Textures
	SetupAATextures();
  glEnable(GL_TEXTURE_1D);

}

/**************************************************************************************/

void DrawAALine(GLfloat from_x, GLfloat from_y, GLfloat to_x, GLfloat to_y, GLuint color)
{
  float angle;
  GLfloat R1[3][2], R2[3][2];

  glBindTexture( GL_TEXTURE_1D, Texture[color] );
  glEnable( GL_TEXTURE_1D );
  angle = atan2( to_y - from_y, to_x - from_x ) - PI / 2.0;

  R1[0][0] = from_x;
  R1[0][1] = from_y;
  R2[0][0] = to_x;
  R2[0][1] = to_y;

	R1[1][0] = from_x - 0.5 * ROOT2 * AA_LINE_WIDTH * cos( PI/4 + angle);
	R1[1][1] = from_y - 0.5 * ROOT2 * AA_LINE_WIDTH * sin( PI/4 + angle);
	R1[2][0] = from_x + 0.5 * ROOT2 * AA_LINE_WIDTH * sin( PI/4 + angle);
	R1[2][1] = from_y - 0.5 * ROOT2 * AA_LINE_WIDTH * cos( PI/4 + angle);

	R2[1][0] = to_x   + 0.5 * ROOT2 * AA_LINE_WIDTH * cos( PI/4 + angle);
	R2[1][1] = to_y   + 0.5 * ROOT2 * AA_LINE_WIDTH * sin( PI/4 + angle);
	R2[2][0] = to_x   - 0.5 * ROOT2 * AA_LINE_WIDTH * sin( PI/4 + angle);
	R2[2][1] = to_y   + 0.5 * ROOT2 * AA_LINE_WIDTH * cos( PI/4 + angle);

	glBegin(GL_TRIANGLES);
	  glTexCoord1f(0.5);
	  glVertex2fv(R1[0]);
	  glTexCoord1i(0);
	  glVertex2fv(R1[1]);
	  glVertex2fv(R1[2]);
	glEnd();

	glBegin(GL_QUADS);
	  glTexCoord1f(0.5);
	  glVertex2fv(R1[0]);
	  glTexCoord1i(0);
	  glVertex2fv(R1[2]);
	  glVertex2fv(R2[1]);
	  glTexCoord1f(0.5);
	  glVertex2fv(R2[0]);
	glEnd();

	glBegin(GL_TRIANGLES);
	  glTexCoord1f(0.5);
	  glVertex2fv(R2[0]);
	  glTexCoord1i(0);
	  glVertex2fv(R2[1]);
	  glVertex2fv(R2[2]);
	glEnd();

	glBegin(GL_QUADS);
	  glTexCoord1f(0.5);
	  glVertex2fv(R2[0]);
	  glTexCoord1i(0);
	  glVertex2fv(R2[2]);
	  glVertex2fv(R1[1]);
	  glTexCoord1f(0.5);
	  glVertex2fv(R1[0]);
	glEnd();
}


void DrawAACircle( float x, float y, float radius, GLuint color )

{

  double angle;
  double sin_angle;
  double cos_angle;

  float x1, y1, x2, y2;

  static double angle_step = PI / 255;
  static double angle_limit = 2.0 * PI;

	glBindTexture(GL_TEXTURE_1D, Texture[color]);
  glEnable( GL_TEXTURE_1D );
	glBegin(GL_QUAD_STRIP);
	for( angle = 0.0; angle <= angle_limit; angle += angle_step ) {

    sin_angle = sin( angle );
    cos_angle = cos( angle );

    x1 = x + radius * cos_angle;
    y1 = y + radius * sin_angle;
    x2 = x + (radius + AA_LINE_WIDTH) * cos_angle;
    y2 = y + (radius + AA_LINE_WIDTH) * sin_angle;
  
    glTexCoord1i(0);
		glVertex2f( x1, y1 );
		glTexCoord1i(1);
		glVertex2f( x2, y2 );
	}
	glEnd();
}






