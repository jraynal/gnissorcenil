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

/**
 * Filtre passe bande
 **/
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
/**
 * Retourne le point d'intersection de l'image coupee (ou sous image)
 * Renvoie NULL si aucune intersection n'est trouvée
 **/
bool intersection(Mat subpic, vector<Vec4i> lines, Point& new_point){
	(void) subpic;
	(void) lines;
	for (size_t i = 0; i < lines.size()-1; i++) {
		//Vec4i l1 = lines[i];
		//Calcul du vecteur directeur

		for (size_t j = i+1; j < lines.size(); j++) {
			//Vec4i l2 = lines[j];

			//Calcul du vecteur directeur

			//Estimation de la distance entre vecteurs: s'ils sont tres similaires, on arrete la.

		}
	} 
	return false; 
}


/**
 * Detecte les lignes dans l'image coupee
 * puis observe les lignes pour en extraire une intersection.
 **/
bool detectIntersection(Mat subpic, Point& new_point) {
	vector<Vec4i> lines;
	Mat cdst;
	cvtColor(subpic, cdst, CV_GRAY2BGR);

	HoughLinesP(subpic, lines, 1, CV_PI/180, 10, 9, 3 );

	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 2, CV_AA);
	}

	imshow("lines", cdst);

	if(lines.size() >1)
		return intersection(cdst, lines, new_point);
	else
		return false;
}

/**
 * Binarisation d'une image de manière à isoler le blanc et diminuer le bruit
 **/
bool binariseAndSort(Mat& simg, Point& new_point) {
	// Pour une meilleure séparation, on travaille sur le HSV
	Mat hsv;
	cvtColor(simg,hsv,CV_BGR2HSV);

	Mat channels[3];
	split(hsv,channels);
	Mat white_selection, green_selection(channels[0]), binar_result;

	// Met en evidence les éléments les plus blancs de l'image
	threshold(channels[2], white_selection, 190, 255, THRESH_BINARY);
	filter_threshold(channels[0], green_selection, 75, 150); // May need some tuning
	bitwise_and(white_selection, green_selection, binar_result);
	
	// Recherche le croisement
	if(detectIntersection(binar_result, new_point))
			return false;
	else
			return true;
	}

/**
 * Selectionne les sous image sur lesquelles effectuer le traitement
 **/
void selectSubPics(Mat& img, vector<KeyPoint>& keypoints) {
	// Liste de memorisation rectangles retenus
	vector<Rect> rects;
	// Liste de sauvegarde des points retenus
	vector<KeyPoint> new_keypoints;
	KeyPoint* it;
	for(uint i=0; i<keypoints.size() ; i++) {
		it=&keypoints[i];

		// Setup a rectangle to define your region of interest
	 	// Picture Size
		int ps=25;
		// Ne traite pas les points présent dans les rectangles déjà retenus
		if(!unselectUselessKeypoints(keypoints,i,rects)) {
			// Vérifie que la zone ne sort pas de l'image
			if(testKeypoint(img,(*it),ps)) {
				// Découpe la zone d'intéret
				cv::Rect MagicCropstem((*it).pt.x - ps/2, (*it).pt.y - ps/2, ps, ps);
				Mat crop = img(MagicCropstem);
				
				// Sauvegarde le rectangle
				rects.push_back(MagicCropstem);
				
				Point intersection_point;
				// Ne garde pas le point s'il n'y a pas de croisement (potentiel)
				if(binariseAndSort(crop, intersection_point)) {
					// Modifie la position du point singulier en celle de l'intersection
					keypoints[i].pt=intersection_point;
					new_keypoints.push_back(keypoints[i]);
				}
			}
		}
	}
	keypoints.clear();
	for(uint i=0; i<new_keypoints.size();i++) {
		keypoints.push_back(new_keypoints[i]);
	}
	return;
}

/**
 * Pipeline begin
 **/
void detectFeatures(Mat& img, Mat& clip, std::vector<KeyPoint>& keypoints){
	// Seuil de la détction de points singuliers
	int minHessian = 5;

	// Transforme le clip en niveau de gris
	Mat grey_clip;
	cvtColor(clip,grey_clip,CV_BGR2GRAY);
	// Détecte les points singuliers
	FastFeatureDetector detector(minHessian);
	detector.detect( img, keypoints, grey_clip);

	// Lance le filtrage des points
	selectSubPics(clip,keypoints);
	return;
}

int main(int argc , char** argv )
{
	if(argc!=3) {
		readme();
		return -1;
	}
	struct timeval t1, t2;

	Mat img_keypoints;
	std::vector<KeyPoint> keypoints;
	Mat img = imread( argv[1]);
	Mat clip = imread( argv[2]);

	keypoints.clear();
	gettimeofday(&t1,NULL);

	detectFeatures(img,clip,keypoints);

	gettimeofday(&t2,NULL);

	std::cout<<"Time elapsed "<<(t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec)<<"µs"<<std::endl;
	drawKeypoints( img, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
	imshow("frame",img_keypoints);
	waitKey();

	return 0;
}

