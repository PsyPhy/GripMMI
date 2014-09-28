#define AA_LINE_WIDTH      16
#define AA_TEXTURE_SIZE		256


#define AA_WHITE		0
#define AA_RED		  1
#define AA_GREEN 	  2
#define AA_BLUE     3
#define AA_YELLOW   4
#define AA_MAGENTA  5
#define AA_CYAN     6
#define AA_COLORS   7


void InitAntialiasing( void );
void DrawAALine(GLfloat from_x, GLfloat from_y, GLfloat to_x, GLfloat to_y, GLuint color);
void DrawAACircle( float x, float y, float radius, GLuint color );

void EnableAntialiasing( void );