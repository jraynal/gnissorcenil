#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <sys/time.h>

using namespace cv;
using namespace std;

#define BAD -1

Point* intersection(Point p1, Point p2, Point p3, Point p4) {
  // Store the values for fast access and easy
  // equations-to-code conversion
  float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
  float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
 
  float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  // If d is zero, there is no intersection
  if (d < 0.001) return NULL;
 
  // Get the x and y
  float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
  float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
  float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
 
  // Check if the x and y coordinates are within both lines
  if ( x < min(x1, x2) || x > max(x1, x2) ||
       x < min(x3, x4) || x > max(x3, x4) ) return NULL;
  if ( y < min(y1, y2) || y > max(y1, y2) ||
       y < min(y3, y4) || y > max(y3, y4) ) return NULL;
 
  // Return the point of intersection
  Point* ret = new Point();
  ret->x = x;
  ret->y = y;
  return ret;
}

int main(int argc, char** argv) {
  if(argc != 2) {
    cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
    return -1;
  }

  Mat image;
  image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

  if (!image.data) {                              // Check for invalid input
    cout <<  "Could not open or find the image" << std::endl ;
    return -1;
  }
  imshow("source", image);

  timeval curTime;
  Mat dst, cdst;
  gettimeofday(&curTime, NULL);

  // Benchmark : starting time
  long int ms = curTime.tv_usec;

  // Line detection
  Canny(image, dst, 50, 200, 3);
  cvtColor(dst, cdst, CV_GRAY2BGR);
  imshow("detected edges", cdst);
  imwrite("edges.jpg",cdst);

  vector<Vec4i> lines;
  vector<Vec4i> new_lines;
  HoughLinesP(dst, lines, 1, CV_PI/180, 10, 50, 10 );

  // Benchmark : ending time
  gettimeofday(&curTime, NULL);
  long int t = curTime.tv_usec - ms;
  cout << t << endl;

  // Show the lines
  for (size_t i = 0; i < lines.size(); i++) {
    Vec4i l = lines[i];
    line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 2, CV_AA);
  }
  imshow("detected lines", cdst);
  imwrite("lines.jpg",cdst);

  Mat image_hsv;
  cvtColor(image, image_hsv, CV_RGB2HSV);
  //imshow("image_hsv", image_hsv);
  Mat final = image.clone();
  RNG rng(12345);
  for (size_t i = 0; i < lines.size(); i++) {
    Mat cdst2 = image_hsv.clone();
    Vec4i l = lines[i];

    // Find and display the line center point
    Point center((l[0] + l[2]) / 2, (l[1] + l[3]) / 2);
    for (size_t j = 0; j < lines.size(); j++) {
      Vec4i l2 = lines[j];
      line(cdst2, Point(l2[0], l2[1]), Point(l2[2], l2[3]), Scalar((i==j?0:255),0,(i!=j?0:255)), 1, CV_AA);
    }
    circle(cdst2, center, 10, Scalar(0, 0, 255), 1);
    //imshow("tmp", cdst2);


    // Compute the local (ROI) histogram
    int roiSize = 10;
    MatND hist;
    int channels[] = {0};
    int histSize[] = {300};
    float range[] = {0, 180};
    const float* ranges[] = {range};
    Mat image_hsv_roi(image_hsv, Rect(center.x - roiSize, center.y - roiSize, roiSize * 2, roiSize * 2));
    imshow("roi", image_hsv_roi);
    calcHist(&image_hsv_roi, 1, channels, Mat(), hist, 1, histSize, ranges, true, false);

    // Calculate the highest value
    int histMax = 0;
    for (int k = 1; k < histSize[0]; k++) {
      float binVal = hist.at<float>(k);
      if (binVal > histMax) {
	histMax = binVal;
      }
    }

    // Draw the histogram 
    int scaleX = 2;
    int scaleY = 300;
    Mat histImg = Mat::zeros(scaleY, histSize[0] * scaleX, CV_8UC3);
    
    int tot = 0;
    int tot2 = 0;
    for (int t = 0; t < histSize[0]; t++) {
      float binVal = hist.at<float>(t);
      tot += binVal;
      if (t  >= 25 && t <= 45) {
	tot2 += binVal;
      }
      rectangle(histImg, Point(t * scaleX, scaleY), Point((t + 1) * scaleX, scaleY - binVal * scaleY / histMax), Scalar::all(255), CV_FILLED);
    }
    //imshow("roi histogram", histImg);

    // Decide wether this line is to be selected
    if (tot2 > tot * 0.9) {
      // Draw this line on the final image
      line(final, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)), 2, CV_AA);
      new_lines.push_back(lines[i]);
    }
  }

  cout<<"LOL"<<endl;

  if(new_lines.size()>1){
    for (size_t i = 0; i < new_lines.size()-1; i++) {
      for (size_t j = i+1; j < new_lines.size(); j++) {
	Point* p=intersection(Point(lines[i][0],lines[i][1]), Point(lines[i][2],lines[i][3]),
			      Point(lines[j][0],lines[j][1]), Point(lines[i][2],lines[i][3]));
	if(p!=NULL) {
	  cout << p->x << endl;
	  cout << p->y << endl;

	  if(p->x<0 || p->x>final.cols || p->y<0 || p->y>final.rows)
	    continue;

	  //Showing in red the point supposed to be on an intersection
	  for(int y1=0; y1<9; y1++){
	    for(int y2=0; y2<9; y2++){
	      final.at<cv::Vec3b>(p->x-4+y1,p->y-4+y2)={0,0,255};
	    }
	  }	
	}
      }
    }
  }

  imshow("final", final);
    
  waitKey(0);

  return 0;
}
