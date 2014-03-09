#include <opencv/cv.h> 
#include <opencv/highgui.h>
using namespace cv;
using namespace std;

const char *getFilePath(const char * fileName);
void copyImage(IplImage *to,IplImage *from);
void equalizeHis(IplImage * src);
int bwareaopen(IplImage *image, int size);
int imadjust(IplImage* src, double low=0.6667, double high=1, double bottom=0, double top=1,double gamma=1 );
void midFilter(IplImage* image,int size);
void erodeImage(IplImage* image,int structSize);
void dilateImage(IplImage* image,int structSize);
void averageImage(IplImage** images,int number,IplImage* imageDist);
IplImage* substract(IplImage* image1,IplImage* image2);
void subStractWhite(IplImage *image);
void im2bw(IplImage* image,double thres);
IplImage* processEroDia1(const IplImage* image,int size1,int size2);
void processEroDia2(IplImage* image);
void processAll(IplImage* image);
void processAllEx(IplImage* image);
void checkAll(IplImage* image);
IplImage* checkImage(IplImage* image,const char * fileName);