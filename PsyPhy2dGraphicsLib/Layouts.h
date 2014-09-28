/*****************************************************************************/
/*                                                                           */
/*                                  Layouts                                  */
/*                                                                           */
/*****************************************************************************/

#ifndef _Layouts_

#include "Views.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _layout {

  View *views;
  int  rows;
  int  columns;
  int  n_views;

  double x_space;
  double y_space;

  double left, right, top, bottom;

  int    display_left, display_right, display_top, display_bottom;

  Display display;
  void *next;

} *Layout;

Layout CreateLayout( Display display, int rows, int columns );
void DestroyLayouts ( void );
View LayoutView( Layout layout, int row, int column );
View LayoutViewN( Layout layout, int n );
View LayoutViewNT( Layout layout, int n );

#define  LayoutViews(l) l->n_views

void LayoutConfigure( Layout layout, int rows, int columns );
void LayoutSetSpacing( Layout layout, double x_space, double y_space );
void LayoutSetDisplayEdgesRelative ( Layout layout, 
				     double left, double bottom, 
				     double right, double top );
void LayoutDescribe ( Layout layout );

int LayoutInput( Layout layout, View *view, int *row, int *column );
int LayoutProcessInput( Layout layout, int display_x, int display_y,
			int *row, int *column );

void LayoutTitle (Layout layout, char *string, int x, int y, double dir);

void LayoutBox (Layout layout );

#ifdef __cplusplus
}
#endif

#define _Layouts_

#endif














