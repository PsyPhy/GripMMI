#ifndef _OglDisplayInterfaceH_
#define _OglDisplayInterfaceH_

#ifdef __cplusplus 
extern "C" {
#endif

  
void glprintf(unsigned int x, unsigned int y, double scale, char* format, ...);

typedef struct {
	HWND	hWnd;
	HGLRC	hRC;
	HDC		hDC;
} OglWindow;

extern void SetOglWindowParent( HWND parent );
extern void GetOglWindow( OglWindow *params );
extern void SetOglWindow( OglWindow *params );

extern int StartOglWindow( char *name, int x, int y, int width, int height, BOOL fullscreen,
                 void refresh_callback( HWND hwnd ),
                 void print_callback( void ) );
extern BOOL InitOglWindow( HWND parent, char *name, int x, int y, int width, int height, BOOL fullscreen );
extern void PrepOglWindow( void );
extern void ActivateOglWindow( void );
extern void ShowOglWindow( void );
extern void HideOglWindow( void );
extern void RunOglWindow( void );
extern int  RunOglWindowOnce( void );
extern void Swap( void );
extern void ShutdownOglWindow( void );
extern void ShutdownOglWindowFromHandle ( OglWindow *params );


#ifdef __cplusplus 
}
#endif

#endif