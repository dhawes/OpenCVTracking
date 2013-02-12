 #include "opencv/cv.h" 
 #include "opencv/highgui.h" 
 #include <stdio.h>  
#include <unistd.h>
 // A Simple Camera Capture Framework 
 int main() {
   CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );
   //CvCapture* capture =
   //  cvCreateFileCapture("http://128.173.201.214/mjpg/video.mjpg");
   cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
   cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
   //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);
   //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
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
     //IplImage* frame = cvLoadImage("/home/dhawes/NetBeansProjects/Sample/VisionImages/First Choice Green Images/HybridLine_SmallGreen4.jpg");
     //IplImage* frame = cvLoadImage("/home/dhawes/NetBeansProjects/Sample/VisionImages/Other Images/OppLine_DoubleGreenBK2.jpg");
     if ( !frame ) {
       fprintf( stderr, "ERROR: frame is null...\n" );
       getchar();
       break;
     }

     printf("=====\n");

     IplImage* bin = cvCreateImage(cvGetSize(frame), 8, 1);
     IplImage* hsv = cvCreateImage(cvGetSize(frame), 8, 3);
     IplImage* hue = cvCreateImage(cvGetSize(frame), 8, 1);
     IplImage* sat = cvCreateImage(cvGetSize(frame), 8, 1);
     IplImage* val = cvCreateImage(cvGetSize(frame), 8, 1);

     cvCvtColor(frame, hsv, CV_BGR2HSV);
     cvSplit(hsv, hue, sat, val, NULL);

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
     CvMemStorage* storage = cvCreateMemStorage(0);

     cvFindContours(tempImage, storage, &contours, sizeof(CvContour),
        CV_RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS, cvPoint(0,0));

     while(contours)
     {
         CvScalar white = CV_RGB(255, 255, 255);
         CvSeq* convexContour =
           cvConvexHull2(contours, storage, CV_CLOCKWISE, 1);
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
         CvSeq* result = cvApproxPoly(contours, sizeof(CvContour), storage,
             CV_POLY_APPROX_DP, 10, 0);
         CvRect boundingRect = cvBoundingRect(result, 0);

         printf("width = %d, height = %d\n", boundingRect.width,
                boundingRect.height);
         printf("perimeter = %f\n", cvContourPerimeter(result));
         printf("area = %f\n", cvContourArea(result, CV_WHOLE_SEQ));

         if(result->total == 4 &&
            fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 20 &&
            cvCheckContourConvexity(result) &&
            boundingRect.width != boundingRect.height)
         {
           printf("Looks like a rect.\n");
            CvPoint *pt[4];
            for(int i=0; i < 4 ;i++)
                pt[i] = (CvPoint*)cvGetSeqElem(result, i);
 
            cvLine(frame, *pt[0], *pt[1], cvScalar(255), 2);
            cvLine(frame, *pt[1], *pt[2], cvScalar(255), 2);
            cvLine(frame, *pt[2], *pt[3], cvScalar(255), 2);
            cvLine(frame, *pt[3], *pt[0], cvScalar(255), 2);
            cvCircle(frame, cvPoint(boundingRect.x + boundingRect.width / 2,
                boundingRect.y + boundingRect.height / 2), 2,
                cvScalar(0, 255, 255), 2);
         }
         contours = contours->h_next;
         printf("=\n");
     }

     cvShowImage("tracking", frame);

     cvReleaseImage(&bin);
     cvReleaseImage(&hsv);
     cvReleaseImage(&hue);
     cvReleaseImage(&sat);
     cvReleaseImage(&val);
     cvReleaseImage(&tempImage);
     cvReleaseMemStorage(&storage);

     //If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
     //remove higher bits using AND operator
     if ( (cvWaitKey(10) & 255) == 27 ) break;

     printf("Done.\n");
   }

   cvReleaseCapture( &capture );
   cvDestroyWindow( "tracking" );
   cvDestroyWindow( "filled" );
   return 0;
 }
