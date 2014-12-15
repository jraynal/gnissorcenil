#if CV_MAJOR_VERSION > 2 || CV_MINOR_VERSION >= 4
#define CV_2_4
#endif

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/time.h>
#include <math.h>
using namespace cv;
using namespace std;

//definition de l'angle minimal entre deux lignes pour declarer une intersection, en radians
#define MIN_ANGLE 0.25

// Picture Size (cropped rectangle)
#define PS 25

int global_width = 0;
int global_height = 0;



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
bool testKeypoint(Mat& img, KeyPoint& it) {
  return it.pt.x-PS>0 && it.pt.x+PS<img.cols  // Test Rows
				    && it.pt.y-PS>0 && it.pt.y+PS<img.rows; // Test Columns
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

bool computeIntersection(Point p1, Point p2, Point p3, Point p4, Point& new_point) {
  // Store the values for fast access and easy
  // equations-to-code conversion
  float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
  float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
 
  float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  // If d is zero, there is no intersection
  if (d < 0.001) return false;
 
  // Get the x and y
  float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
  float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
  float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
 
  // Return the point of intersection
  new_point.x += x;
  new_point.y += y;

  // Check if the x and y coordinates are within the image limits
  if ( new_point.x < 0 || new_point.x > global_width-1 ) return false;
  if ( new_point.y < 0 || new_point.y > global_height-1 ) return false;

  return true;
}

/**
 * Retourne le point d'intersection de l'image coupee (ou sous image)
 * Renvoie NULL si aucune intersection n'est trouvée
 **/
bool intersection(Mat subpic, vector<Vec4i> lines, Point& new_point){
  (void) subpic;
  (void) lines;
  int i_max=-1;
  int j_max=-1;
  float max_angle=0.;

  for (size_t i = 0; i < lines.size()-1; i++) {
    Vec4i l1 = lines[i];

    //Calcul du vecteur directeur
    float vec_x = l1[0] - l1[2];
    float vec_y = l1[1] - l1[3];

    for (size_t j = i+1; j < lines.size(); j++) {
      Vec4i l2 = lines[j];

      //Calcul du vecteur directeur
      float vec2_x = l2[0] - l2[2];
      float vec2_y = l2[1] - l2[3];

      // Angle entre les vecteurs. Si tres faible, on arrete la.
      // Si fort, on trouve l'intersection.
      float angle = acos((vec_x*vec2_x+vec_y*vec2_y)/
			 (sqrt(vec_x*vec_x+vec_y*vec_y)*
			  sqrt(vec2_x*vec2_x+vec2_y*vec2_y)));
      
      //Si angle trop petit, j'aimerais retirer la droite du lot
      if(angle>MIN_ANGLE && angle>max_angle){
	max_angle=angle;
	i_max=i;
	j_max=j;
      }
    }
  }
  
  if(i_max!=-1) {
    if(computeIntersection(Point(lines[i_max][0],lines[i_max][1]), Point(lines[i_max][2],lines[i_max][3]),
			   Point(lines[j_max][0],lines[j_max][1]), Point(lines[j_max][2],lines[j_max][3]),
			   new_point))
      return true;
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
    return true;
  else
    return false;
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

    //Construit un rectangle autour de la zone d'interet
    // Ne traite pas les points présent dans les rectangles déjà retenus
    if(!unselectUselessKeypoints(keypoints,i,rects)) {
      // Vérifie que la zone ne sort pas de l'image
      if(testKeypoint(img,(*it))) {
	// Découpe la zone d'intéret
	cv::Rect MagicCropstem((*it).pt.x - PS/2, (*it).pt.y - PS/2, PS, PS);
	Mat crop = img(MagicCropstem);
				
	// Sauvegarde le rectangle
	rects.push_back(MagicCropstem);
				
	Point intersection_point(keypoints[i].pt.x-PS/2,keypoints[i].pt.y-PS/2);
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

  global_width=img.cols;
  global_height=img.rows;

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

