C_DIR=.moc
OBJECT_DIR=.obj
TEMPLATE=app
CONFIG+=qt warn_on relax thread


#poss de definir des hearders
SOURCES=intersectionLignes.cpp
TARGET=intersectionLignes
CONFIG-=app_bundle

INCLUDEPATH=/usr/include/opencv2

LIBS +=\
             -lopencv_core\
             -lopencv_highgui\
             -lopencv_imgproc\
             -lopencv_features2d\
             -lopencv_nonfree\
             -lopencv_calib3d\
             -lopencv_video

