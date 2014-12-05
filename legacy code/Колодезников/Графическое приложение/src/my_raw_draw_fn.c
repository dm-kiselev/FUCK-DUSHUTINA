/* Y o u r   D e s c r i p t i o n                       */
/*                            AppBuilder Photon Code Lib */
/*                                         Version 2.03  */

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Local headers */
#include "ablibs.h"
#include "abimport.h"
#include "proto.h"
#include "data.h"

sem_t sem;
double_t px[POINT_MAX];
double_t py[POINT_MAX];
void my_raw_draw_fn( PtWidget_t *widget,
                     PhTile_t *damage )
{
   PhRect_t     raw_canvas;
   PhPoint_t points, pointe;
   size_t i;
   double x_step, y_step;

   PtSuperClassDraw( PtBasic, widget, damage );
   PtCalcCanvas(widget, &raw_canvas);
   PtClipAdd ( widget, &raw_canvas);

   PgSetStrokeWidth(2);
   PgSetStrokeColor(Pg_RED);

   x_step = (raw_canvas.lr.x - raw_canvas.ul.x)/X_MAX;
   y_step = (raw_canvas.lr.y - raw_canvas.ul.y)/Y_MAX;
   points.y = -1;
   i=0;
   sem_wait(&sem);
   while(i<POINT_MAX) {
	   if(px[i] >= 0) {
		   pointe.x = (short)(px[i] * x_step) + raw_canvas.ul.x;
		   pointe.y = (short)(py[i] * y_step) + raw_canvas.ul.y;
	   }
	   if(points.y >= raw_canvas.ul.y) {
		   PgDrawLine(&points, &pointe);
	   }
	   if(px[i] >= 0) {
		   points = pointe;
	   }
	   ++i;
   }
   sem_post(&sem);
   PtClipRemove ();
}

