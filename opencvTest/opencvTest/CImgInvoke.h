#include <CImg.h>
#include <opencv/cv.h> 
#include <opencv/highgui.h>
#define byte unsigned char
using namespace cimg_library;
void InitArrayFromVolFile(byte * &data,const char* fileName,int length);
IplImage* GetSliceFrom281ByIndex(CImg<byte> *vol,int index)
{
	IplImage* image=cvCreateImage(cvSize(vol->height(),vol->depth()),IPL_DEPTH_8U,1);
	int step=image->widthStep;
	byte* data=(byte *)image->imageData;
	byte* volData=vol->data();
	int X=vol->width();
	int Y=vol->height();
	int Z=vol->depth();
	for(int i=0;i<image->width;i++)
	{
		for(int j=0;j<image->height;j++)
		{
			data[j*step+i]=volData[index+i*X+j*X*Y];//(*vol)(index,i,j);
		}
	}
	return image;
}
void AddToReVol( IplImage* imageSlice, CImg<byte> * vol,int index) 
{
	uchar* volData=vol->data();
	int X=vol->width();
	int Y=vol->height();
	int Z=vol->depth();
	int step=imageSlice->widthStep;  
	uchar* data=(byte *)imageSlice->imageData;  
	for(int i=0;i<imageSlice->width;i++)
	{
		for(int j=0;j<imageSlice->height;j++)
		{
			volData[i+j*X+index*X*Y] = data[j*step+i];
		}
	}
}
int main1()
{
	//slice process
	const int lineNumber =281;// 281;
	const int cdpNumber = 481;//;
	const int timeNumber = 2501;//2501;
	
	byte* pointer=new byte[lineNumber*cdpNumber*timeNumber];
	InitArrayFromVolFile(pointer,"D:\\VTKproj\\mig.raw",lineNumber*cdpNumber*timeNumber);
	cout<<"read image complete"<<endl<<endl;
	CImg<byte>* imageVol=new CImg<byte>(pointer,lineNumber,cdpNumber,timeNumber,1,true);
	CImg<byte> *imageResSpace=new CImg<byte>(cdpNumber,timeNumber,lineNumber,1,255);
	for(int i=0;i<lineNumber;i++)
	{
		IplImage* imageSlice = GetSliceFrom281ByIndex(imageVol,i);
		processAll(imageSlice);
		AddToReVol(imageSlice,imageResSpace,i);
		cvReleaseImage(&imageSlice);
		cout<<"process image "<<i+1<<"/281 complete."<<endl<<endl;
	}
	delete pointer;
	imageResSpace->save_raw("D:\\VTKproj\\new_result.raw");
	/*Segmentation::ImageSegmentor *segmentor 
	=new Segmentation::ImageSegmentor(imageResSpace->_data,cdpNumber,timeNumber,lineNumber);
	segmentor->OnModifyData=ModifyData;
	segmentor->OnFindRegion=FindRegion;
	segmentor->DoSegmentation(50);
	delete segmentor;*/
	getchar();
	return 0;
}
int main4()
{
	int lineNumber =281;// 281;
	int cdpNumber = 481;//;
	int timeNumber = 2501;//2501;
	CImg<byte> *imageResSpace=new CImg<byte>();
	imageResSpace->load_raw("D:\\VTKproj\\new_result_test2.raw",lineNumber,cdpNumber,timeNumber,1);
	printf("load complete!\n");
	imageResSpace->resize(141,241,1251);
	imageResSpace->save_raw("D:\\VTKproj\\new_result_test2_small.raw");
	printf("save complete!\n");
	return 0;
}
void makeSmall(const char * fileName,byte* data,int width,int height, int depth,int smw,int smh,int smd)
{
	CImg<byte> *imageResSpace=new CImg<byte>(data,width,height,depth,1);
	imageResSpace->resize(smw,smh,smd);
	imageResSpace->save_raw(fileName);
}