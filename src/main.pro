# emplacement pour les fichiers temporaire de Qt
MOC_DIR = .moc
OBJECT_DIR = .obj
TEMPLATE = app
CONFIG += qt warn_on release thread

# possibilité de définir des header : HEADER
SOURCES = main.cpp
TARGET = main
CONFIG -= app_bundle
INCLUDEPATH = /usr/include/opencv2
LIBS += \
-lopencv_core\
-lopencv_highgui\
-lopencv_imgproc\
