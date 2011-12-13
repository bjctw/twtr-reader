
MOC_DIR = tmp
OBJECTS_DIR = tmp
RCC_DIR = tmp
UI_DIR = tmp
DESTDIR = bin

TARGET = twtrReader
#CONFIG += qt
CONFIG += qt
QT += network

TEMPLATE = app

INCLUDEPATH += .

LIBS += -Lbin
LIBS +=	-ljsoncpp

SOURCES += twtrReader.cpp\


HEADERS += twtrReader.h\

    
RESOURCES += 


