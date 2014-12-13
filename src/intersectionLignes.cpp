#if CV_MAJOR_VERSION > 2 || CV_MINOR_VERSION >= 4
#define CV_2_4
#endif

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/time.h>
using namespace cv;
using namespace std;

void readme()
{ std::cout << " Usage: ./intersectionLignes <img> <img clip> " << std::endl; }

void issueOpeningVideo()
{ std::cout << "couldn't open video file" << std::endl; }

/**
 * Filtre les points singuliers inutiles car déjà compris dans un autre
 * rectangle.
 **/
bool unselectUselessKeypoints(vector<KeyPoint>& keypoints, uint index, vector<Rect>& rects) {
	if(!rects.empty()) {
		for(uint i=0;i<rects.size();i++) {
			if(rects[i].contains(keypoints[index].pt)) {
				return true;
			}
		}
	}
	return false;
}

/**
 * Test si on peut découper la zone autour du point
 **/
bool testKeypoint(Mat& img, KeyPoint& it, int ps) {
	return it.pt.x-ps>0 && it.pt.x+ps<img.cols  // Test Rows
			&& it.pt.y-ps>0 && it.pt.y+ps<img.rows; // Test Columns
}

void gradientPicHistogram(Mat& simg) {	
	Mat histo = Mat::zeros(Size(128,128), CV_8UC1);
	int h=simg.rows, w=simg.cols;
	double max_hist=0;
	int tmp=0;
	for(int i=0; i<w; i++) {
		for(int j=0; j<h; j++) {
			int x_grad=simg.at<uchar>(j,((i-1)<0)?i:i-1)-simg.at<uchar>(j,((i+1)>w)?i:i+1);
			int y_grad=simg.at<uchar>(((j-1)<0)?j:j-1,i)-simg.at<uchar>(((j+1)>w)?j:j+1,i);

			if(abs(x_grad)<128 && abs(y_grad)<128)
				tmp=(histo.at<uchar>(64+x_grad/2,64+y_grad/2)++);
			if(tmp>max_hist) {
				max_hist=(double)tmp;
			}
		}
	}
	histo*=(255./max_hist);

	Mat res_hist;
	resize(histo, res_hist, Size(256,256));
	imshow("hist", res_hist);
	return;
}

void gradientPicHistogram2(Mat& simg) {
	Mat dx, dy;
	Sobel(simg,dx,CV_8UC1,0,1,3);
	Sobel(simg,dy,CV_8UC1,1,0,3);
	int h=simg.rows, w=simg.cols;
	Mat edges=Mat::zeros(Size(h,w), CV_8UC1);
	for(int i=0; i<w; i++)
	{
		for(int j=0; j<h; j++) 
		{

			edges.at<uchar>(i,j) = (uchar) sqrt( pow(dx.at<uchar>(i,j),2) + pow(dy.at<uchar>(i,j),2) );

		}
	}
	imshow("hist", edges);
	return;
}

void filter_threshold(Mat& input, Mat& output, uchar low, uchar high) {
	Mat_<uchar>::iterator itIn = input.begin<uchar>(),
												itOut = output.begin<uchar>(),
												itInEnd=input.end<uchar>(),
												itOutEnd=output.end<uchar>();
	for (;itIn!=itInEnd && itOut!=itOutEnd ; itIn++, itOut++) {
		if(*itIn>low && *itIn<high) {
			*itOut=255;
		}
		else {
			*itOut=0;
		}
	}
}


void binariseAndSort(Mat& simg) {
	Mat hsv;
	cvtColor(simg,hsv,CV_BGR2HSV);
	// Met en evidence les éléments les plus blancs de l'image
	// TODO: Extraction des composantes les plus vertes ==> plus grande proba
	// d'avoir la ligne.
	Mat channels[3];
	split(hsv,channels);
	Mat dst, dst2(channels[0]), dst3;


	threshold(channels[2],dst,190, 255, THRESH_BINARY);
	filter_threshold(channels[0], dst2, 75, 100);
	bitwise_and(dst,dst2,dst3);



	//threshold(channels[0],dst,50, 255, THRESH_BINARY);
	Mat tmp1, tmp2;
	resize(dst3,tmp1,Size(200,200));
	resize(simg,tmp2,Size(200,200));
	gradientPicHistogram(tmp1);
	imshow("binarized", tmp1);
	imshow("real", tmp2);
	waitKey();
	return;

}

/**
 * Selectionne les sous image sur lesquelles effectuer le traitement
 **/
void selectSubPics(Mat& img, vector<KeyPoint>& keypoints, vector<Rect>& rects) {
	KeyPoint* it;
	vector<KeyPoint> new_keypoints;
	for(uint i=0; i<keypoints.size() ; i++) {
		it=&keypoints[i];
		// Setup a rectangle to define your region of interest
		int ps=25; // Picture Size
		if(unselectUselessKeypoints(keypoints,i,rects))
			continue;
		if(testKeypoint(img,(*it),ps)) {
			cv::Rect MagicCropstem((*it).pt.x - ps/2, (*it).pt.y - ps/2, ps, ps);
			Mat crop = img(MagicCropstem);
			rects.push_back(MagicCropstem);
			new_keypoints.push_back(keypoints[i]);
			//gradientPicHistogram2(crop);
			binariseAndSort(crop);
			//imshow("crop",crop);
			//waitKey();
		}
	}
	keypoints.clear();
	for(uint i=0; i<new_keypoints.size();i++) {
		keypoints.push_back(new_keypoints[i]);
	}
	return;
}

void detectFeatures(Mat& img, Mat& clip, std::vector<KeyPoint>& keypoints)
{
	int minHessian = 5;
	Mat grey_clip;
	cvtColor(clip,grey_clip,CV_BGR2GRAY);
	FastFeatureDetector detector(minHessian);
	detector.detect( img, keypoints, grey_clip);

	vector<Rect> rects;
	selectSubPics(clip,keypoints,rects);

	return;
}

int main(int argc , char** argv )
{
	if(argc!=3) {
		readme();
		return -1;
	}
	struct timeval t1, t2;
	/* VideoCapture cap((string)argv[1]);
		 if(!cap.isOpened())
		 {
		 issueOpeningVideo();
		 return -1;
		 }*/

	Mat img_keypoints;
	std::vector<KeyPoint> keypoints;
	Mat img = imread( argv[1]);
	Mat clip = imread( argv[2]);

	//for(int i=0; i<10; i++) {
		keypoints.clear();
		gettimeofday(&t1,NULL);

		detectFeatures(img,clip,keypoints);

		//gradientPicHistogram(img);

		gettimeofday(&t2,NULL);

		std::cout<<"Time elapsed "<<(t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec)<<"µs"<<std::endl;
	//}

	drawKeypoints( img, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
	imshow("frame",img_keypoints);
	waitKey();

	return 0;
}
