// Disable warnings about unsafe functions.
// We use the 'unsafe' versions to maintain source-code compatibility with Visual C++ 6
#define _CRT_SECURE_NO_WARNINGS

// Enable enhanced monitoring of memory leaks.
#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <mmsystem.h>
#include <winerror.h>

#include <crtdbg.h>
#include <stdlib.h>

#include <math.h>
#include <stdio.h>
#include <time.h>

#define true  1
#define false 0

#include <gl/gl.h>
#include "glut.h"

#include "OglDisplayInterface.h"
#include "fMessageBox.h"
#include "fOutputDebugString.h"

#define local static

local HWND      s_hWnd  = NULL;
local HGLRC     s_hRC   = NULL;
local HDC       s_hDC   = NULL;

local int _window_class_registered = 0;
local char _window_class_name[] = "OpenGL2Dgraphics";

local HWND _parent_window = NULL;
local HINSTANCE hInstance = 0;

/*********************************************************************************/

static void (*_refresh_callback)( HWND hwnd );
static void (*_print_callback)( void ) = 0;
#define WM_JOE 1238283

LONG WINAPI

WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{ 

	static PAINTSTRUCT ps;
	
	switch(uMsg) {
		
	case WM_SYSCOMMAND:					// Intercept System Commands
		
		switch (wParam)					  // Check System Calls
		{
		case SC_SCREENSAVE:			// Screensaver Trying To Start?
		case SC_MONITORPOWER:		// Monitor Trying To Enter Powersave?
			return 0;
			break;                  // Prevent From Happening
		}
		
		break;						        // Exit
		
		case WM_PAINT:
			
			if (_refresh_callback) (*_refresh_callback)( hWnd );
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			
			return 0;
			break;
			
		case WM_SIZE:
			
			// s_winwidth = LOWORD(lParam);
			// s_winheight = HIWORD(lParam);
			PostMessage(hWnd, WM_PAINT, 0, 0);
			/* 
			* For some reason, this has to be done twice when the window
			* is maximized, otherwise the whole window is not redraw.
			*/
			PostMessage(hWnd, WM_PAINT, 0, 0);
			
			return 0;
			break;
			
		case WM_CHAR:
			
			switch (wParam) {
				
			case 27:			/* ESC key */
				// PostMessage(hWnd, WM_QUIT, 0, 0);
				break;
				
			case ' ':
				PostMessage(hWnd, WM_PAINT, 0, 0);
				break;
				
			case 'p':    
				if (_print_callback) (*_print_callback)();
				break;
				
			default:
				PostMessage(hWnd, WM_PAINT, 0, 0);
				break;
				
			}     
			return 0;
			break;
			
			case WM_LBUTTONDOWN:
				
				PostMessage(hWnd, WM_PAINT, 0, 0);
				return 0;
				break;
				
			case WM_RBUTTONDOWN:
				
				/* click your heels three times...hang on toto. */
				
				return 0;
				break;
				
			case WM_MOUSEMOVE:
				
				//	PostMessage(hWnd, WM_PAINT, 0, 0);
				
				return 0;
				break;
				
			case WM_CLOSE:
				PostMessage(hWnd, WM_QUIT, 0, 0);
				return 0;
				break;
				
			case WM_QUIT:
				//   ShutdownWindow();
				return 0;
				break;
				
			case WM_DESTROY:
				
				//   PostQuitMessage(0);
				//   s_quit = true;
				
				return 0;
				break;
				
  }
  
  return DefWindowProc(hWnd, uMsg, wParam, lParam); 
  
} 

void RegisterOglWindowClass ( void ) {

	WNDCLASS    wc;

	if ( !_window_class_registered ) {
		
		/* only register the window class once - use hInstance as a flag. */
		
		wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc   = (WNDPROC) WindowProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = _window_class_name;

		hInstance = GetModuleHandle(NULL);	
		if (!RegisterClass(&wc)) {
			MessageBox(NULL, "RegisterClass() failed:\n Cannot register window class.", "Error", MB_OK);     
			return;
		}
		
		
	}

	_window_class_registered++;


}

void UnregisterOglWindowClass ( void ) {

	if ( !_window_class_registered || !hInstance ) {
		fOutputDebugString( "Skipping UnregisterClass(). Link count = %d\n", _window_class_registered );
		return;
	}
	_window_class_registered--;
	if ( !_window_class_registered ) {
		if (!UnregisterClass(_window_class_name, hInstance))			// Are We Able To Unregister Class
		{
			MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			hInstance=NULL;
		}
		else fOutputDebugString( "UnregisterClass() succeeded\n" );

	}
	else fOutputDebugString( "Skipping UnregisterClass(). Link count = %d\n", _window_class_registered );

}

/*********************************************************************************/

HWND CreateOglWindow( HWND parent, char* title, 
                   int x, int y, int width, int height, 
                   BOOL fullscreen, BYTE type, DWORD flags )
                   
{
	
	int         pf;
	HDC         hDC;
	HWND        hWnd;
	PIXELFORMATDESCRIPTOR pfd;

	
	DWORD		dwExStyle;		            // Window Extended Style
	DWORD		dwStyle;		              // Window Style
	RECT		WindowRect;		            // Grabs Rectangle Upper Left / Lower Right Values
	
	WindowRect.left = (long) x;		          // Set Left Value To x
	WindowRect.right = (long) x + width;	  // Set Right Value To Achieve Requested Width
	WindowRect.top = (long) y;			        // Set Top Value To y
	WindowRect.bottom = (long) y + height;	// Set Bottom Value To Achieve Requested Height
	

	RegisterOglWindowClass();
	
	if (fullscreen)						// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;		// Window Extended Style
		dwStyle=WS_POPUP;				// Windows Style
	}
	else if ( parent != NULL )
	{
		dwExStyle=WS_EX_APPWINDOW;			// Window Extended Style
		dwStyle=WS_CHILD;					// Windows Style
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW;			// Window Extended Style
		dwStyle=WS_POPUPWINDOW | 
			WS_CAPTION |    
			//		WS_MAXIMIZEBOX |
			//		WS_MAXIMIZE | 
			WS_MINIMIZEBOX;					// Windows Style
	}
	
	AdjustWindowRectEx( &WindowRect, dwStyle, false, dwExStyle );		// Adjust Window To True Requested Size
	
	// Create the window.
	hWnd = CreateWindowEx(
		dwExStyle,
		_window_class_name, title, 
		dwStyle | 
		WS_CLIPSIBLINGS | 
		WS_CLIPCHILDREN,
		x, y,  
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		parent, NULL, hInstance, NULL);
	
	if (hWnd == NULL) {
		DWORD err;
		err = GetLastError( );
		MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.", "Error", MB_OK);
		return NULL;
	}
	

	// Set up the pixel format.
	hDC = GetDC( hWnd );

	memset(&pfd, 0, sizeof(pfd));		// Make sure to initialize the bits to zero.
	pfd.nSize        = sizeof(pfd);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
	pfd.iPixelType   = type;
	pfd.cColorBits   = 32;
	pfd.cDepthBits   = 16;
	pfd.iLayerType   = PFD_MAIN_PLANE;
	
	pf = ChoosePixelFormat(hDC, &pfd);
	if (pf == 0) {
		MessageBox(NULL, "ChoosePixelFormat() failed:  "
			"Cannot find a suitable pixel format.", "Error", MB_OK); 
		return 0;
	} 
	
	if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
		MessageBox(NULL, "SetPixelFormat() failed:  "
			"Cannot set format specified.", "Error", MB_OK);
		return 0;
	} 
	
	DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	ReleaseDC(hWnd, hDC);

	return hWnd;
	
}    

/*********************************************************************************/

BOOL InitOglWindow( HWND parent, char *name, int x, int y, int width, int height, BOOL fullscreen ) {
	
	s_hWnd = CreateOglWindow( parent, name, x, y, width, height, fullscreen, PFD_TYPE_RGBA, PFD_DOUBLEBUFFER );
	
	if ( s_hWnd == NULL ) exit(1);
	
	s_hDC = GetDC( s_hWnd );
	s_hRC = wglCreateContext( s_hDC );
	wglMakeCurrent( s_hDC, s_hRC );
	
	SetForegroundWindow( s_hWnd );		// Slightly Higher Priority
	SetFocus( s_hWnd );			     	// Sets Keyboard Focus To The Window
	ShowWindow( s_hWnd, SW_SHOW );

	if ( s_hWnd != NULL ) return TRUE;
	return FALSE;
	
}

/*********************************************************************************/

// Call this if you want subsequent OglWindows to be children of another window.
void SetOglWindowParent( HWND parent ) {
	_parent_window = parent;
}

// This will get an OglWindow up and running.
int StartOglWindow( char *name, int x, int y, int width, int height, BOOL fullscreen,
                void refresh_callback( HWND hwnd ),
                void print_callback( void ) ) {
	
	if ( InitOglWindow( _parent_window, name, x, y, width, height, fullscreen ) ) {	
		_refresh_callback = refresh_callback;
		_print_callback = print_callback;
		return( true );	
	}
	else return( false );
	
}

// Fill a handle to the current OglWindow so that you can use more than one.
void GetOglWindow( OglWindow *params ) {
	params->hDC = s_hDC;
	params->hRC = s_hRC;
	params->hWnd = s_hWnd;
}

// Set the current window from a handle.
void SetOglWindow( OglWindow *params ) {
	s_hDC = params->hDC;
	s_hRC = params->hRC;
	s_hWnd = params->hWnd;
	wglMakeCurrent( s_hDC, s_hRC );	
	SetForegroundWindow( s_hWnd );		// Slightly Higher Priority
	SetFocus( s_hWnd );			     	// Sets Keyboard Focus To The Window
	//ShowWindow( s_hWnd, SW_SHOW );
}

/*********************************************************************************/

void PrepOglWindow( void ) {
	
	GLfloat viewport_dimensions[4];
	glGetFloatv( GL_VIEWPORT, viewport_dimensions );

//	glViewport(0, 0, s_winwidth, s_winheight );
	glViewport( viewport_dimensions[0], viewport_dimensions[1], viewport_dimensions[2], viewport_dimensions[3]);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluOrtho2D(0, s_winwidth, 0, s_winheight);
	gluOrtho2D(viewport_dimensions[0], viewport_dimensions[2], viewport_dimensions[1], viewport_dimensions[3]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor( 1.0F, 1.0F, 1.0F, 0.0F );
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0F, 0.0F, 0.0F );
	
}

/*********************************************************************************/

void Swap ( void ) {
	glFlush();
	SwapBuffers( s_hDC );
}
void SwapWindowFromHandle ( OglWindow *params ) {
	glFlush();
	SwapBuffers( params->hDC );
}


/*********************************************************************************/


// Shutdown the currently selected 0glWindow;
void ShutdownOglWindow( void ) {
	
	if ( s_hRC )											    // Do We Have A Rendering Context?
	{
		// Make sure that the rendering context is not in use by deselecting it.
		if (!wglMakeCurrent(NULL,NULL))	
		{
			// If it is in use, not much we can do about it. So just signal the information.
			// Besides, it could be because we already released it when destroying another window.
			fOutputDebugString( "WARNING: wglMakeCurrent(NULL,NULL) Failed.\n" );
		}
		// For debuggin puposes I signal when this call is successful.
		else fOutputDebugString( "INFO:   wglMakeCurrent(NULL,NULL) Succeeded.\n" );
		
		// Now actually delete the rendering context. Here we want everyone to know if it fails.
		if (!wglDeleteContext( s_hRC ))			
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		s_hRC=NULL;	
	}
	
	// Release the device context.
	if ( s_hDC && s_hWnd ) {
		if ( !ReleaseDC( s_hWnd, s_hDC ))
		{
			MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			s_hDC=NULL;	
		}
	}
	
	// Destroy thet actual window.
	if ( s_hWnd ) {
		if ( !DestroyWindow( s_hWnd ) )	
		{
			MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			s_hWnd=NULL;
		}
		
	}

	UnregisterOglWindowClass();
}

void ShutdownOglWindowFromHandle ( OglWindow *params ) {

	if ( params->hRC )	{										    // Do We Have A Rendering Context?

		if (!wglMakeCurrent(NULL,NULL))	// Are We Able To Release The DC And RC Contexts?
		{
			fOutputDebugString( "WARNING - wglMakeCurrent(NULL,NULL) Failed.\n" );
		}
		else fOutputDebugString( "wglMakeCurrent(NULL,NULL) Succeeded.\n" );
		
		if (!wglDeleteContext( params->hRC ))			// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
	}
	
	if ( params->hDC && params->hWnd ) {
		if ( !ReleaseDC( params->hWnd, params->hDC ))	// Are We Able To Release The DC
		{
			MessageBox(NULL,"ReleaseDC( params->hWnd, params->hDC ).","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		else fOutputDebugString( "ReleaseDC( params->hWnd, params->hDC ) Succeeded. %08xl %08xl\n", params->hWnd, params->hDC );
	}
	
	if ( params->hWnd ) {
		if ( !DestroyWindow( params->hWnd ) )	// Are We Able To Destroy The Window?
		{
			MessageBox(NULL,"DestroyWindow( params->hWnd ) failed","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		else fOutputDebugString( "DestroyWindow( params->hWnd ) Succeeded. %08xl\n", params->hWnd );
		
	}

	UnregisterOglWindowClass();
			
}

/*********************************************************************************/

void RunOglWindow( void ) {
	
	MSG msg;				/* message */
	int cont;
	
	do {
		cont = GetMessage( &msg, s_hWnd, 0, 0 );
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	} while( cont );
	
}

void ActivateOglWindow( void ) {
	// ShowWindow( s_hWnd, SW_SHOW);
	wglMakeCurrent( s_hDC, s_hRC );
}

void ShowOglWindow( void ) {
	ShowWindow( s_hWnd, SW_SHOW);
}

void HideOglWindow( void ) {
	ShowWindow( s_hWnd, SW_HIDE);
}

int RunOglWindowOnce( void ) {
	
	MSG msg;				/* message */
	int cont;
	
	while ( PeekMessage( &msg, s_hWnd, 0, 0, 0 ) ) {
		cont = GetMessage( &msg, s_hWnd, 0, 0 );    
		if ( cont == 0 ) {
			ShutdownOglWindow();
			return( 1 );
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return( 0 );
	
}

/*************************************************************************************/

/* 

 general purpose text routine.  draws a string according to the
 format in a stroke font at x, y after scaling it by the scale
 specified.  x, y and scale are all in window-space [i.e., pixels]
 with origin at the lower-left.
 
*/

void glprintf(GLuint x, GLuint y, double scale, char* format, ...) {
	
	va_list args;
	char buffer[1024], *p;
	GLfloat font_scale = 119.05F + 33.33F;


	GLfloat viewport_dimensions[4];
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	
	glGetFloatv( GL_VIEWPORT, viewport_dimensions );
	// glViewport( 0, 0, s_winwidth, s_winheight);
	glViewport( viewport_dimensions[0], viewport_dimensions[1], viewport_dimensions[2], viewport_dimensions[3]);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	// gluOrtho2D(0, s_winwidth, 0, s_winheight);
	gluOrtho2D(viewport_dimensions[0], viewport_dimensions[2], viewport_dimensions[1], viewport_dimensions[3]);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	//    glPolygonMode(GL_FRONT, GL_LINE);
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	
	glTranslatef((GLfloat) x, (GLfloat) y, 0.0F);
	glScalef(scale/font_scale, scale/font_scale, scale/font_scale);
	
	for(p = buffer; *p; p++) {
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *p);
		//      glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, *p );
		//        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, *p );
	}
	
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
}

