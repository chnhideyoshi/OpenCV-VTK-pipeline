#include <opencv/cv.h> 
#include <opencv/highgui.h>
#include <iostream>
#include <string>
using namespace cv;
using namespace std;
const char* directoryName="D:\\VTKproj\\imageCheck\\new\\";
const char *getFilePath(const char * fileName)
{
	string* dir= new string(directoryName);
	return dir->append(fileName).c_str();
}
void copyImage(IplImage *to,IplImage *from)
{
	cvCopy(from,to);
}
void equalizeHis(IplImage * src)
{
	cvEqualizeHist(src,src);
}
int bwareaopen(IplImage *image, int size)
{
	cvNot(image,image);
	/* OpenCV equivalent of Matlab's bwareaopen.
	image must be 8 bits, 1 channel, black and white
	(objects) with values 0 and 255 respectively */

	CvMemStorage *storage;
	CvSeq *contour = NULL;
	CvScalar white, black;
	IplImage *input = NULL; // cvFindContours changes the input
	double area;
	int foundCountours = 0;

	black = CV_RGB( 0, 0, 0);
	white = CV_RGB( 255, 255, 255 );

	if(image == NULL || size == 0)
		return(foundCountours);

	input = cvCloneImage(image);

	storage = cvCreateMemStorage(0); // pl.Ensure you will have enough room here.

		cvFindContours(input, storage, &contour, sizeof (CvContour),
		CV_RETR_LIST,
		CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

	while(contour)
	{
		area = cvContourArea(contour, CV_WHOLE_SEQ );
		if( -size <= area && area <= 0)
		{ // removes white dots
			cvDrawContours( image, contour, black, black, -1, CV_FILLED, 8 );
		}
		else
		{
			if( 0 < area && area <= size) // fills in black holes
				cvDrawContours( image, contour, white, white, -1, CV_FILLED, 8 );
		}
		contour = contour->h_next;
	}

	cvReleaseMemStorage( &storage ); // desallocate CvSeq as well.
	cvReleaseImage(&input);
	cvNot(image,image);
	return(foundCountours);
}
int imadjust(IplImage* src, double low=0.6667, double high=1, double bottom=0, double top=1,double gamma=1 )  
{  
	IplImage* dst=src;
	//IplImage* dst=cvCreateImage(cvGetSize(src),src->depth,src->nChannels);
	if(     low<0 && low>1 && high <0 && high>1&&  
		bottom<0 && bottom>1 && top<0 && top>1 && low>high)  
		return -1;  
	double low2 = low*255;  
	double high2 = high*255;  
	double bottom2 = bottom*255;  
	double top2 = top*255;  
	double err_in = high2 - low2;  
	double err_out = top2 - bottom2;  

	int x,y;  
	double val;  

	// intensity transform  
	for( y = 0;y<src->height;y++)  
	{  
		for (x = 0; x<src->width; x++)  
		{  
			val = ((uchar*)(src->imageData + src->widthStep*y))[x];   
			//val = pow((val - low2)/err_in, gamma) * err_out + bottom2;  
			val = ((val - low2)/err_in) * err_out + bottom2;  
			if(val>255) val=255; if(val<0) val=0; // Make sure src is in the range [low,high]  
			((uchar*)(dst->imageData + dst->widthStep*y))[x] = (uchar) val;  
		}  
	} 
	/*int step=src->widthStep;  
	uchar* data=(uchar *)src->imageData;  
	uchar* disdata=(uchar *)dst->imageData;  
	for(int i=0;i<src->height;i++)  
	{  
		for(int j=0;j<src->width;j++)  
		{  
			data[i*step+j] =disdata[i*step+j];
		}  
	}  */
	//cvReleaseImage(&dst);
	return 0;  
}  
void midFilter(IplImage* image,int size)
{
	cvSmooth(image, image, CV_MEDIAN,size);
}
void erodeImage(IplImage* image,int structSize)
{
	IplConvKernel* structEle=cvCreateStructuringElementEx(structSize,structSize,0,0, CV_SHAPE_RECT);
	//IplImage* dist=cvCreateImage(cvGetSize(image),image->depth,image->nChannels);
	cvErode(image,image,structEle);
	//copyImage(image,dist);
	cvReleaseStructuringElement(&structEle);
	//cvReleaseImage(&dist);
}
void dilateImage(IplImage* image,int structSize)
{
	IplConvKernel* structEle=cvCreateStructuringElementEx(structSize,structSize,0,0, CV_SHAPE_RECT);
	//IplImage* dist=cvCreateImage(cvGetSize(image),image->depth,image->nChannels);
	cvDilate(image,image,structEle);
	//copyImage(image,dist);
	cvReleaseStructuringElement(&structEle);
	//cvReleaseImage(&dist);
}
void averageImage(IplImage** images,int number,IplImage* imageDist)
{
	int step=imageDist->widthStep;
	uchar* data=(uchar *)imageDist->imageData;  
	for(int i=0;i<imageDist->height;i++)  
	{  
		for(int j=0;j<imageDist->width;j++)  
		{  
			int sum=0;
			for(int k=0;k<number;k++)
			{
				uchar* dataK=(uchar *)images[k]->imageData;  
				sum+=dataK[i*step+j];
			}
			int av=sum/number;
			data[i*step+j]=(uchar)(av);
		}  
	} 
}
IplImage* substract(IplImage* image1,IplImage* image2)
{
	IplImage* imageDist=cvCreateImage(cvGetSize(image1),image1->depth,image1->nChannels);
	cvSub(image1,image2,imageDist);
	return imageDist;
}
void subStractWhite(IplImage *image)
{
	cvNot(image,image);
}
void im2bw(IplImage* image,double thres)
{
	int step=image->widthStep;  
	uchar* data=(uchar *)image->imageData;  
	for(int i=0;i<image->height;i++)  
	{  
		for(int j=0;j<image->width;j++)  
		{  
			if(data[i*step+j]/255.0>thres)
			{
				data[i*step+j]=255;
			}
			else
			{
				data[i*step+j]=0;
			}
		}  
	} 
}
IplImage* processEroDia1(const IplImage* image,int size1,int size2)
{
	IplImage* copy1=cvCloneImage(image);
	dilateImage(copy1,size1);
	IplImage* copy2=cvCloneImage(image);
	erodeImage(copy2,size1);
	IplImage* sub=cvCreateImage(cvGetSize(image),image->depth,image->nChannels);
	cvSub(copy1,copy2,sub);
	erodeImage(sub,size2);
	cvReleaseImage(&copy1);
	cvReleaseImage(&copy2);
	return sub;
}
void processEroDia2(IplImage* image)
{
	const int times=2;
	IplImage* images[times];
	images[0]=processEroDia1(image,4,1);//cvSaveImage(getFilePath("cv_2.1.bmp"),images[0]);
	images[1]=processEroDia1(image,6,5);//cvSaveImage(getFilePath("cv_2.2.bmp"),images[1]);
	//images[2]=processEroDia1(image,4,3);//cvSaveImage(getFilePath("cv_2.3.bmp"),image3);
	//images[3]=processEroDia1(image,5,4);//cvSaveImage(getFilePath("cv_2.4.bmp"),image4);
	//images[4]=processEroDia1(image,6,5);//cvSaveImage(getFilePath("cv_2.5.bmp"),image5);
	//images[5]=processEroDia1(image,7,6);//cvSaveImage(getFilePath("cv_2.6.bmp"),image6);
	//images[6]=processEroDia1(image,8,7);//cvSaveImage(getFilePath("cv_2.7.bmp"),image7);
	averageImage(images,times,image);
	char buf[20];
    for(int i=0;i<times;i++)
	{
		sprintf(buf,"cv_2.%d.bmp",i+1);
		cvSaveImage(getFilePath(buf),images[i]);
		cvReleaseImage(&images[i]);
	}
}
void processAll(IplImage* image)
{
	cvSaveImage(getFilePath("cv_0.bmp"),image);
	processEroDia2(image);cvSaveImage(getFilePath("cv_2.fin.bmp"),image);
	subStractWhite(image);cvSaveImage(getFilePath("cv_3.1.bmp"),image);
	midFilter(image,25);cvSaveImage(getFilePath("cv_3.2.bmp"),image);
	imadjust(image);cvSaveImage(getFilePath("cv_3.3.bmp"),image);
	im2bw(image,0.3);cvSaveImage(getFilePath("cv_3.4.bmp"),image);
}
void processAllEx(IplImage* image)
{
	IplImage* copy1=cvCloneImage(image);
	dilateImage(copy1,5);
	cvSaveImage(getFilePath("cv_1dilate.bmp"),copy1);
	IplImage* copy2=cvCloneImage(image);
	erodeImage(copy2,5);
	cvSaveImage(getFilePath("cv_2erode.bmp"),copy2);
	cvSub(copy1,copy2,image);
	cvSaveImage(getFilePath("cv_3sub.fin.bmp"),image);
	dilateImage(image,3);
	cvSaveImage(getFilePath("cv_4subDia.fin.bmp"),image);
	midFilter(image,5);
	cvSaveImage(getFilePath("cv_5me.bmp"),image);
	im2bw(image,0.5);
	cvSaveImage(getFilePath("cv_6bw.bmp"),image);
	cvReleaseImage(&copy1);
	cvReleaseImage(&copy2);
}
void checkAll(IplImage* image)
{
	//IplImage* testEq=cvCloneImage(image);
	//equalizeHis(testEq);
	//IplImage* matEq=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_eq.bmp",0);
	//IplImage* subEq=substract(testEq,matEq);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_eq.bmp",subEq);

	//IplImage* testEr3=cvCloneImage(image);
	//erodeImage(testEr3,3);
	//IplImage* matEr3=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_er3.bmp",0);
	//IplImage* subEr3=substract(testEr3,matEr3);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_er3.bmp",subEr3);

	//IplImage* testEr5=cvCloneImage(image);
	//erodeImage(testEr5,5);
	//IplImage* matEr5=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_er5.bmp",0);
	//IplImage* subEr5=substract(testEr5,matEr5);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_er5.bmp",subEr5);

	//IplImage* testEr7=cvCloneImage(image);
	//erodeImage(testEr7,7);
	//IplImage* matEr7=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_er7.bmp",0);
	//IplImage* subEr7=substract(testEr7,matEr7);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_er7.bmp",subEr7);

	//IplImage* testDi3=cvCloneImage(image);
	//dilateImage(testDi3,3);
	//IplImage* matDi3=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_di3.bmp",0);
	//IplImage* subDi3=substract(testDi3,matDi3);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_di3.bmp",subDi3);

	//IplImage* testDi5=cvCloneImage(image);
	//dilateImage(testDi5,5);
	//IplImage* matDi5=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_di5.bmp",0);
	//IplImage* subDi5=substract(testDi5,matDi5);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_di5.bmp",subDi5);

	//IplImage* testDi7=cvCloneImage(image);
	//dilateImage(testDi7,7);
	//IplImage* matDi7=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_di7.bmp",0);
	//IplImage* subDi7=substract(testDi7,matDi7);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_di7.bmp",subDi7);

	//IplImage* testMe=cvCloneImage(image);
	//midFilter(testMe,25);
	//IplImage* matMe=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_me25.bmp",0);
	//IplImage* subMe=substract(testMe,matMe);
	//cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_me25.bmp",subMe);

	/*IplImage* testAdj=cvCloneImage(image);
	imadjust(testAdj);
	cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_cv_adj.bmp",testAdj);
	IplImage* matAdj=cvLoadImage("D:\\VTKproj\\imageCheck\\functions\\0_mat_adj.bmp",0);
	IplImage* subAdj=substract(testAdj,matAdj);
	cvSaveImage("D:\\VTKproj\\imageCheck\\functions\\0_sub_adj.bmp",subAdj);*/

}
IplImage* checkImage(IplImage* image,const char * fileName)
{
	IplImage* image2=cvLoadImage(fileName,0);
	return substract(image2,image);
}

int main()
{
	IplImage* image=cvLoadImage("D:\\VTKproj\\Data\\Gray-281\\Gray-281-209.bmp",0);
	processAllEx(image);
	cvNamedWindow("a",CV_WINDOW_NORMAL);
	cvShowImage("a",image);
	cvWaitKey(0);
	cvDestroyAllWindows();
	cvReleaseImage(&image);
	return 0;
}