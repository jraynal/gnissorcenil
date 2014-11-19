#if CV_MAJOR_VERSION > 2 || CV_MINOR_VERSION >= 4
#define CV_2_4
#endif

#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <sys/time.h>
using namespace cv;

void readme()
{ std::cout << " Usage: ./intersectionLignes <img1> " << std::endl; }

void issueOpeningVideo()
{ std::cout << "couldn't open video file" << std::endl; }



void detectFeatures(Mat& img, std::vector<KeyPoint>& keypoints)
{
	int minHessian = 500;
	SurfFeatureDetector detector(minHessian);
	detector.detect( img, keypoints);
}

//void detectFeaturesMaison(Mat& im, std::vector<KeyPoint>& keypoints)
//{
//	
//}


int main(int argc , char** argv )
{

	if(argc<2)
	{
	readme();
	return -1;
	}
	struct timeval t1, t2;
	// 
       /* VideoCapture cap((string)argv[1]);
        if(!cap.isOpened())
	{
	 issueOpeningVideo();
         return -1;
	}*/

	Mat img_keypoints;
	std::vector<KeyPoint> keypoints;
	Mat img = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	for(int i=0; i<10; i++) {
	keypoints.clear();
	gettimeofday(&t1,NULL);

  detectFeatures(img, keypoints);

	gettimeofday(&t2,NULL);

	std::cout<<"Time elapsed "<<(t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec)<<"µs"<<std::endl;
	}
	drawKeypoints( img, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
	namedWindow("frame",1);
	imshow("frame",img);
	imshow("frame",img_keypoints);
	waitKey();


	/*while(1)
        {
        if(!cap.read(frame))
        break;	
	waitKey();
	if(waitKey(30)>=0)
	break;
	}*/
	return -1;

}
