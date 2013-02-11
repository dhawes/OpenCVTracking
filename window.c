 #include "opencv/cv.h" 
 #include "opencv/highgui.h" 
 #include <stdio.h>  
 // A Simple Camera Capture Framework 
 int main() {
   // Create a window in which the captured images will be presented
   cvNamedWindow( "mywindow", CV_WINDOW_AUTOSIZE );
   cvWaitKey(10);
   sleep(10);
   // Release the capture device housekeeping
   cvDestroyWindow( "mywindow" );
   return 0;
 }
