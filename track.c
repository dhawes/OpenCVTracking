 #include "opencv/cv.h" 
 #include "opencv/highgui.h" 
 #include <stdio.h>  
#include <unistd.h>
 // A Simple Camera Capture Framework 
 int main() {
   CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );
   cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
   cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
   if ( !capture ) {
     fprintf( stderr, "ERROR: capture is NULL \n" );
     getchar();
     return -1;
   }
   // Create a window in which the captured images will be presented
   cvNamedWindow( "tracking", CV_WINDOW_AUTOSIZE );
   cvNamedWindow( "filled", CV_WINDOW_AUTOSIZE );

   IplConvKernel* morphKernel =
       cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);
   // Show the image captured from the camera in the window and repeat
   while ( 1 ) {
     // Get one frame
     IplImage* frame = cvQueryFrame( capture );
     //IplImage* frame = cvLoadImage("/home/dhawes/NetBeansProjects/Sample/VisionImages/First Choice Green Images/HybridLine_SmallGreen2.jpg");
     if ( !frame ) {
       fprintf( stderr, "ERROR: frame is null...\n" );
       getchar();
       break;
     }

     IplImage* bin = cvCreateImage(cvGetSize(frame), 8, 1);
     IplImage* hsv = cvCreateImage(cvGetSize(frame), 8, 3);
     IplImage* hue = cvCreateImage(cvGetSize(frame), 8, 1);
     IplImage* sat = cvCreateImage(cvGetSize(frame), 8, 1);
     IplImage* val = cvCreateImage(cvGetSize(frame), 8, 1);

     cvCvtColor(frame, hsv, CV_BGR2HSV);
     cvSplit(hsv, hue, sat, val, NULL);

/*
     cvInRangeS(hsv, cvScalar(60, 90, 20), cvScalar(100, 255, 255),
                bin);
*/
     cvThreshold(hue, bin, 60, 100, CV_THRESH_BINARY);
     cvThreshold(hue, hue, 60+15, 100, CV_THRESH_BINARY_INV);
     cvThreshold(sat, sat, 90, 255, CV_THRESH_BINARY);
     cvThreshold(val, val, 20, 255, CV_THRESH_BINARY);
     cvAnd(hue, bin, bin, NULL);
     cvAnd(bin, sat, bin, NULL);
     cvAnd(bin, val, bin, NULL);

     cvMorphologyEx(bin, bin, NULL, morphKernel, CV_MOP_CLOSE, 2);

     IplImage* tempImage = cvCreateImage(cvGetSize(bin), 8, 1);
     cvCopy(bin, tempImage);

     CvSeq* contours; 
     CvSeq* result;
     CvMemStorage* storage = cvCreateMemStorage(0);

     cvFindContours(tempImage, storage, &contours, sizeof(CvContour),
        CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0));

     while(contours)
     {
         CvScalar white = CV_RGB(255, 255, 255);
         CvSeq* convexContour = cvConvexHull2(contours, storage, CV_CLOCKWISE,
            1);
         cvDrawContours(
           tempImage, convexContour, white, white, -1, CV_FILLED, 8);
         contours = contours->h_next;
     }

     cvShowImage("filled", tempImage);

     contours = NULL;
     cvFindContours(tempImage, storage, &contours, sizeof(CvContour),
         CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0));

     while(contours)
     {
         CvSeq* convexContour =
           cvConvexHull2(contours, storage, CV_CLOCKWISE, 1);
/*
         result = cvApproxPoly(convexContour, sizeof(CvContour), storage,
           CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);
*/
         result = cvApproxPoly(contours, sizeof(CvContour), storage,
           CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);
         CvRect boundingRect = cvBoundingRect(result, 0);
         if(result->total == 4 &&
            fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 20 &&
            cvCheckContourConvexity(result) &&
            boundingRect.width != boundingRect.height)
         {
           printf("Looks like a rect.\n");
           printf("width = %d, height = %d\n", boundingRect.width,
                  boundingRect.height);
            CvPoint *pt[4];
            for(int i=0; i < 4 ;i++)
                pt[i] = (CvPoint*)cvGetSeqElem(result, i);
 
            cvLine(frame, *pt[0], *pt[1], cvScalar(255), 2);
            cvLine(frame, *pt[1], *pt[2], cvScalar(255), 2);
            cvLine(frame, *pt[2], *pt[3], cvScalar(255), 2);
            cvLine(frame, *pt[3], *pt[0], cvScalar(255), 2);
	   //usleep(1000);
         }
         contours = contours->h_next;
     }

     cvShowImage("tracking", frame);

     cvReleaseImage(&bin);
     cvReleaseImage(&hsv);
     cvReleaseImage(&hue);
     cvReleaseImage(&sat);
     cvReleaseImage(&val);
     cvReleaseImage(&tempImage);
     //cvReleaseImage(&frame);
     cvReleaseMemStorage(&storage);

     // Do not release the frame!
     //If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
     //remove higher bits using AND operator
     if ( (cvWaitKey(10) & 255) == 27 ) break;
     printf("Done.\n");
   }
   // Release the capture device housekeeping
   cvReleaseCapture( &capture );
   cvDestroyWindow( "tracking" );
   cvDestroyWindow( "filled" );
   return 0;
 }
