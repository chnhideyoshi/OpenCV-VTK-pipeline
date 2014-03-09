#include <opencv/cv.h> 
#include <opencv/highgui.h>
#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageData.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkSTLWriter.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkCamera.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageDilateErode3D.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkUniformGrid.h>
#include <vtkRendererCollection.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkImageToStructuredGrid.h>
#include "imageProcess.h"
#include "ImageSegmentor.h"
#include "CImgInvoke.h"
#define byte unsigned char
#define main2 main
using namespace cv;
using namespace std;

IplImage* GetSliceFrom281ByIndex(byte* data,int index,int width=281,int height=481,int depth=2501)
{
	IplImage* image=cvCreateImage(cvSize(height,depth),IPL_DEPTH_8U,1);
	int step=image->widthStep;
	byte* imagedata=(byte *)image->imageData;
	for(int i=0;i<image->width;i++)
	{
		for(int j=0;j<image->height;j++)
		{
			imagedata[j*step+i]=data[index+i*width+j*width*height];//(*vol)(index,i,j);
		}
	}
	return image;
}
IplImage* GerSliceFrom2501ByIndex(byte* data,int index,int width=281,int height=481,int depth=2501)
{
	IplImage* image=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	int step=image->widthStep;
	byte* imagedata=(byte *)image->imageData;
	for(int i=0;i<image->width;i++)
	{
		for(int j=0;j<image->height;j++)
		{
			imagedata[j*step+i]=data[i+j*width+index*width*height];//(*vol)(index,i,j);
		}
	}
	return image;
}

void AddToReVol_281(IplImage* imageSlice,byte* reData,int index)
{
	byte* data=(byte *)imageSlice->imageData;
	int step=imageSlice->widthStep;
	int length=imageSlice->width*imageSlice->height;
	int offset=index*length;
	for(int i=0;i<imageSlice->width;i++)
	{
		for(int j=0;j<imageSlice->height;j++)
		{
			reData[i+j*imageSlice->width+index*imageSlice->width*imageSlice->height] = data[j*step+i];
		}
	}
}
void AddToReVol_2501(IplImage* imageSlice,byte* reData,int index)
{
	byte* data=(byte *)imageSlice->imageData;
	int step=imageSlice->widthStep;
	int length=imageSlice->width*imageSlice->height;
	int offset=index*length;
	for(int i=0;i<imageSlice->width;i++)
	{
		for(int j=0;j<imageSlice->height;j++)
		{
			reData[i+j*imageSlice->width+index*imageSlice->width*imageSlice->height] = data[j*step+i];
		}
	}
}
void AddToReVol_2501Ex(IplImage* imageSlice,byte* reData,int index)
{
	byte* data=(byte *)imageSlice->imageData;
	int step=imageSlice->widthStep;
	int length=imageSlice->width*imageSlice->height;
	int offset=index*length;
	for(int i=0;i<imageSlice->width;i++)
	{
		for(int j=0;j<imageSlice->height;j++)
		{
			byte value=data[j*step+i];
			if(value==0)
			{
				reData[i+j*imageSlice->width+index*imageSlice->width*imageSlice->height] = data[j*step+i];
			}
		}
	}
}
void SaveBytesToFile(const char * fileName,byte* data,int length=481*2501*281)
{
	FILE *const nfile = fopen(fileName,"wb");
	fwrite(data,sizeof(byte),length,nfile);
	fclose(nfile);
}
void SaveBytesToFileSmall(const char * fileName,byte* data,int width,int height, int depth)
{
	makeSmall(fileName,data,width,height,depth,width/2+1,height/2+1,depth/2+1);
}
void ShowImage(IplImage* imageSlice)
{
	cvNamedWindow("a",CV_WINDOW_NORMAL);
	cvShowImage("a",imageSlice);
	cvWaitKey(0);
}
void InitArrayFromVolFile(byte * &data,const char* fileName,int length) 
{
	long dataSize = length;
	FILE* file = fopen(fileName,"rb");//"D:\\VTKproj\\3result_re.raw" ,"rb" );
	if( file == NULL )
	{
		printf("open the file failed\n");
	}
	fread( data,sizeof( unsigned char),dataSize,file );
	fclose(file);
}
void FindRegion(Segmentation::Region* r)
{
	char a[40];
	sprintf(a,"D:\\VTKproj\\SegmentData\\region%d.xyz",r->Id);
	r->PrintData(a);
	if(r->GetCount()>3000)
	{
		r->WriteXYZFile(a);
	}
}
void ModifyData(byte*** &data,int width=481,int height=2501,int depth=281)
{
	for(int i=0;i<width;i++)
	{
		for(int j=0;j<220;j++)
		{
			for(int k=0;k<depth;k++)
			{
				data[i][j][k]=0;
			}
		}
	}
}
void ModifyDataEx(byte*& data,int width=281,int height=481,int depth=2501)
{
	for(int i=0;i<width;i++)
	{
		for(int j=0;j<height;j++)
		{
			for(int k=0;k<depth*200/2501;k++)
			{
				data[i+j*width+k*width*height] = 0;
			}
		}
	}
}
void ModifyData(byte*& data,int width=481,int height=2501,int depth=281)
{
	for(int i=0;i<width;i++)
	{
		for(int j=0;j<height/8;j++)
		{
			for(int k=0;k<depth;k++)
			{
				data[i+j*width+k*width*height] = 0;
			}
		}
	}
	for(int i=width-width*10/481;i<width;i++)
	{
		for(int j=0;j<height;j++)
		{
			for(int k=0;k<depth;k++)
			{
				data[i+j*width+k*width*height] = 0;
			}
		}
	}
}
void ModifyDataBw(byte* &data,int length)
{
	for(int i=0;i<length;i++)
	{
		if(data[i]>128){data[i]=255;}
		else{data[i]=0;}
	}
}
void show_progress(char* message,double progressRate)
{
	printf("\r%s:%d%%",message,(int)(progressRate*100));
}
void KeypressCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
	int pos=30;
	double dis=0.7;
	vtkRenderWindowInteractor *iren =  static_cast<vtkRenderWindowInteractor*>(caller);
	vtkRendererCollection *rens = iren->GetRenderWindow()->GetRenderers();
	vtkRenderer* ren = rens->GetFirstRenderer();
	vtkCamera * aCamera=ren->GetActiveCamera();
	double normal[3];    
	double focal[3];  
	double position[3]; 
	double view[3]; 
	aCamera->GetViewPlaneNormal (normal); 
	aCamera->GetFocalPoint(focal);       
	aCamera->GetPosition(position);
	char* key=iren->GetKeySym();
	if(strcmp(key,"Up")==0)
	{
		position[0]=position[0]-pos*normal[0];
		position[1]=position[1]-pos*normal[1];
		//position[2]=position[2]-pos*normal[2];

		focal[0]=focal[0]-pos*normal[0];
		focal[1]=focal[1]-pos*normal[1];
		//focal[2]=focal[2]-pos*normal[2];

		aCamera->SetPosition(position);
		aCamera->SetFocalPoint(focal);
	}
	if(strcmp(key,"Down")==0)
	{
		position[0]=position[0]+pos*normal[0];
		position[1]=position[1]+pos*normal[1];
		//position[2]=position[2]+pos*normal[2];

		focal[0]=focal[0]+pos*normal[0];
		focal[1]=focal[1]+pos*normal[1];
		//focal[2]=focal[2]+pos*normal[2];

		aCamera->SetPosition(position);
		aCamera->SetFocalPoint(focal);
	}
	if(strcmp(key,"Left")==0)
	{
		aCamera->Yaw (-pos/10);
	}
	if(strcmp(key,"Right")==0)
	{
		aCamera->Yaw (pos/10);
	}	
	iren->GetRenderWindow()->Render();
}
void ProgressFunction ( vtkObject* caller,long unsigned int vtkNotUsed(eventId),void* vtkNotUsed(clientData),void* vtkNotUsed(callData) )
{
	vtkMarchingCubes* testFilter = static_cast<vtkMarchingCubes*>(caller);
	//std::cout << "Progress: " << testFilter->GetProgress() << std::endl;
	double progress=testFilter->GetProgress();
	show_progress("caculating ",progress);
}

void FilterData( byte* ptr, int lineNumber, int cdpNumber, int timeNumber ) 
{
	/*for(int i=0;i<lineNumber;i++)
	{
		for(int j=0;j<cdpNumber;j++)
		{
			for(int k=0;k<timeNumber;k++)
			{
				data[i+j*lineNumber+k*lineNumber*cdpNumber] = 0;
			}
		}
	}*/
	
}

const char* migFile="D:\\VTKproj\\mig.raw";
const char* migSmallFile="D:\\VTKproj\\mig_small.raw";
const char* resultFile2501="D:\\VTKproj\\new_result_2501.raw";
const char* resultFile2501Small="D:\\VTKproj\\new_result_2501_small.raw";
const char* resultFile281="D:\\VTKproj\\new_result_281.raw";
const char* resultFile281Small="D:\\VTKproj\\new_result_281_small.raw";
const char* resultFileMerge="D:\\VTKproj\\new_result_merge.raw";
const char* resultFileMergeSmall="D:\\VTKproj\\new_result_merge_small.raw";
int main1Ex281()
{
	const int lineNumber =281;// 281;
	const int cdpNumber = 481;//;
	const int timeNumber = 2501;//2501;
	byte* pointer=new byte[lineNumber*cdpNumber*timeNumber];
	InitArrayFromVolFile(pointer,migFile,lineNumber*cdpNumber*timeNumber);
	cout<<"read image"<<migFile<<" complete"<<endl;
	byte* imageResSpace=new byte[lineNumber*cdpNumber*timeNumber];
	for(int i=0;i<lineNumber;i++)
	{
		IplImage* imageSlice = GetSliceFrom281ByIndex(pointer,i,lineNumber,cdpNumber,timeNumber);
		processAllEx(imageSlice);
		AddToReVol_281(imageSlice,imageResSpace,i);
		cvReleaseImage(&imageSlice);
		show_progress("process image ",(i+1)/281.0);
	}
	for(int i=0;i<lineNumber;i++)
	{
		for(int j=0;j<cdpNumber;j++)
		{
			for(int k=0;k<timeNumber;k++)
			{
				byte value2=imageResSpace[j+k*cdpNumber+i*cdpNumber*timeNumber];
				if(value2==0)
				{
					pointer[i+j*lineNumber+k*lineNumber*cdpNumber]=value2;
				}
			}
		}
	}
	SaveBytesToFile(resultFile281,pointer,lineNumber*cdpNumber*timeNumber);
	SaveBytesToFileSmall(resultFile281Small,pointer,lineNumber,cdpNumber,timeNumber);
	cout<<"\nsave complete."<<endl;
	delete imageResSpace;
	delete pointer;
	getchar();
	return 0;
}//Process
int main1Ex2501()
{
	const int lineNumber =281;// 281;
	const int cdpNumber = 481;//;
	const int timeNumber = 2501;//2501;

	byte* pointer=new byte[lineNumber*cdpNumber*timeNumber];
	InitArrayFromVolFile(pointer,migFile,lineNumber*cdpNumber*timeNumber);
	cout<<"read image "<<migFile<<" complete!"<<endl;
	for(int i=0;i<timeNumber;i++)
	{
		IplImage* imageSlice = GerSliceFrom2501ByIndex(pointer,i,lineNumber,cdpNumber,timeNumber);
		processAllEx(imageSlice);
		AddToReVol_2501Ex(imageSlice,pointer,i);
		cvReleaseImage(&imageSlice);
		show_progress("process image ",(i+1)/2501.0);
		//cout<<"process image "<<i+1<<"/2501 complete."<<endl;
	}
	SaveBytesToFile(resultFile2501,pointer,lineNumber*cdpNumber*timeNumber);
	SaveBytesToFileSmall(resultFile2501Small,pointer,lineNumber,cdpNumber,timeNumber);
	cout<<"\nsave complete."<<endl;
	getchar();
	delete pointer;
	return 0;
}//Process
int main1Merge()
{
	const int lineNumber =281;// 281;
	const int cdpNumber = 481;//;
	const int timeNumber = 2501;//2501;
	byte* pointer=new byte[lineNumber*cdpNumber*timeNumber];
	byte* pointer2=new byte[lineNumber*cdpNumber*timeNumber];
	byte* imageResSpace=new byte[lineNumber*cdpNumber*timeNumber];
	InitArrayFromVolFile(pointer,migFile,lineNumber*cdpNumber*timeNumber);
	cout<<"read image"<<migFile<<" complete"<<endl;
	for(int i=0;i<lineNumber;i++)
	{
		IplImage* imageSlice = GetSliceFrom281ByIndex(pointer,i,lineNumber,cdpNumber,timeNumber);
		processAllEx(imageSlice);
		AddToReVol_281(imageSlice,imageResSpace,i);
		cvReleaseImage(&imageSlice);
		show_progress("process image ",(i+1)/(281.0*2));
	}
	for(int i=0;i<timeNumber;i++)
	{
		IplImage* imageSlice = GerSliceFrom2501ByIndex(pointer,i,lineNumber,cdpNumber,timeNumber);
		processAllEx(imageSlice);
		AddToReVol_2501(imageSlice,pointer2,i);
		cvReleaseImage(&imageSlice);
		show_progress("process image ",0.5+(i+1)/(2501.0*2));
	}
	printf("\nmerge volume...\n");
	for(int i=0;i<lineNumber;i++)
	{
		for(int j=0;j<cdpNumber;j++)
		{
			for(int k=0;k<timeNumber;k++)
			{
				byte value=pointer2[i+j*lineNumber+k*lineNumber*cdpNumber];
				byte value2=imageResSpace[j+k*cdpNumber+i*cdpNumber*timeNumber];
				pointer2[i+j*lineNumber+k*lineNumber*cdpNumber]=value2|value;
			}
		}
	}
	for(int i=0;i<lineNumber;i++)
	{
		for(int j=0;j<cdpNumber;j++)
		{
			for(int k=0;k<timeNumber;k++)
			{
				byte value2=pointer2[i+j*lineNumber+k*lineNumber*cdpNumber];
				if(value2==0)
				{
					pointer[i+j*lineNumber+k*lineNumber*cdpNumber]=value2;
				}
			}
		}
	}
	printf("save volume data...\n");
	SaveBytesToFile(resultFileMerge,pointer,lineNumber*cdpNumber*timeNumber);
	SaveBytesToFileSmall(resultFileMergeSmall,pointer,lineNumber,cdpNumber,timeNumber);
	cout<<"save complete."<<endl;
	delete pointer;
	delete pointer2;
	delete imageResSpace;
	return 0;
}
int mainSeg()
{
	const int lineNumber =281;// 281;
	const int cdpNumber = 481;//;
	const int timeNumber = 2501;//2501;
	byte* pointer=new byte[lineNumber*cdpNumber*timeNumber];
	InitArrayFromVolFile(pointer,resultFile2501,lineNumber*cdpNumber*timeNumber);
	cout<<"read image "<<resultFile2501<<" complete"<<endl;
	Segmentation::ImageSegmentor *segmentor 
		=new Segmentation::ImageSegmentor(pointer,lineNumber,cdpNumber,timeNumber);
	segmentor->OnFindRegion=FindRegion;
	segmentor->DoSegmentation(300);
	delete segmentor;
	cout<<"segmentation completed!\n";
	getchar();
	return 0;
}
int mainMc(int width,int height,int depth,const char* fileName)
{
	vtkSmartPointer<vtkImageData> image=vtkImageData::New();
	image->SetDimensions(width, height, depth);  
	image->SetScalarTypeToUnsignedChar();
	image->SetSpacing(1, 1,1);  
	image->SetOrigin(0, 0, 0); 
	image->SetNumberOfScalarComponents(1);  
	image->AllocateScalars();  
	byte*ptr = (byte *)image->GetScalarPointer();
	InitArrayFromVolFile(ptr,fileName,width*height*depth);
	cout<<"load image "<<fileName<<" complete!\n";

	vtkSmartPointer<vtkCallbackCommand> progressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	progressCallback->SetCallback(ProgressFunction);

	vtkSmartPointer<vtkMarchingCubes> iso = vtkMarchingCubes::New(); 
	iso->SetInput(image);
	iso->AddObserver(vtkCommand::ProgressEvent, progressCallback);
	iso->SetNumberOfContours(1);  
	iso->SetValue(0,0.5);
	iso->ComputeNormalsOn();
	iso->ComputeScalarsOff();
	iso->Update();
	cout<<"\nisosurface complete\n";

	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkPolyDataMapper::New();  
	cubeMapper->SetInput(iso->GetOutput());  
	cubeMapper->Update();
	vtkSmartPointer<vtkActor> cubeActor = vtkActor::New();  
	cubeActor->GetProperty()->SetColor(1,1,0.5);
	cubeActor->SetMapper(cubeMapper);  
	vtkSmartPointer<vtkRenderer> ren =vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin =vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren =vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);
	iren->Initialize();
	renWin->SetSize(1024, 768);
	ren->AddActor(cubeActor);  
	vtkSmartPointer<vtkCallbackCommand> keypressCallback =  vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback ( KeypressCallbackFunction );
	iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );
	iren->Start();
	return 0;
}
int mainShowSeg()
{
	int number=259;
	printf("input *.xyz\n in directory 'SegmentData'\n");
	scanf("%d",&number);
	char fileName[40];
	sprintf(fileName,"D:\\VTKproj\\SegmentData\\region%d.xyz",number);
	FILE *const file = fopen(fileName,"rb");
	int lineNum=0;
	fscanf(file, "%d", &lineNum);
	if(lineNum==0) { return 0;}
	Segmentation::Point *points=new Segmentation::Point[lineNum];
	int maxX=-1,minX=99999,maxY=-1,minY=99999,maxZ=-1,minZ=99999;
	for(int i=0;i<lineNum;i++)
	{
		int x,y,z;
		fscanf(file, "%d%d%d", &x, &y, &z);
		Segmentation::Point p;
		p.X=x;p.Y=y;p.Z=z;
		if(maxX<x){maxX=x;}
		if(maxY<y){maxY=y;}
		if(maxZ<z){maxZ=z;}
		if(minX>x){minX=x;}
		if(minY>y){minY=y;}
		if(minZ>z){minZ=z;}
		points[i]=p;
	}
	fclose(file);
	if(minX==99999||maxX==-1||maxY==-1,minY==99999,maxZ==-1,minZ==99999){return 1;}
	int xLength=maxX-minX+1;
	int yLength=maxY-minY+1;
	int zLength=maxZ-minZ+1;
	vtkSmartPointer<vtkImageData> image=vtkImageData::New();
	image->SetDimensions(xLength,yLength,zLength);  
	image->SetScalarTypeToUnsignedChar();
	image->SetSpacing(1, 1,1);  
	image->SetOrigin(0, 0, 0); 
	image->SetNumberOfScalarComponents(1);  
	image->AllocateScalars();  
	byte*ptr = (byte *)image->GetScalarPointer();
	memset(ptr,0,xLength*yLength*zLength);
	for(int i=0;i<lineNum;i++)
	{
		Segmentation::Point p=points[i];
		int xIndex=p.X-minX;
		int yIndex=p.Y-minY;
		int zIndex=p.Z-minZ;
		ptr[xIndex+yIndex*xLength+zIndex*xLength*yLength]=(byte)255;
	}

	vtkSmartPointer<vtkMarchingCubes> iso = vtkMarchingCubes::New(); 
    iso->SetInput(image);
	iso->SetNumberOfContours(1);  
	iso->SetValue(0,0.5);
	iso->ComputeNormalsOn();
	iso->ComputeScalarsOff();
	iso->Update();
	cout<<"isosurface complete\n";

	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkPolyDataMapper::New();  
	cubeMapper->SetInput(iso->GetOutput());  
	cubeMapper->Update();
	vtkSmartPointer<vtkActor> cubeActor = vtkActor::New();  
	cubeActor->GetProperty()->SetColor(1,1,0.5);
	cubeActor->SetMapper(cubeMapper);  
	vtkSmartPointer<vtkRenderer> ren =vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin =vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren =vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);
	iren->Initialize();
	renWin->SetSize(1024, 768);
	ren->AddActor(cubeActor);  
	vtkSmartPointer<vtkCallbackCommand> keypressCallback =  vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback ( KeypressCallbackFunction );
	iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );
	iren->Start();
	return 0;
}
int mainVolRender(int width,int height,int depth,const char* fileName)
{
	printf("enable shading? put in 1.(yes) or 2.(no) \n");
	int num=2;
	scanf("%d",&num);
	vtkSmartPointer<vtkImageData> image=vtkImageData::New();
	image->SetDimensions(width, height,depth);  
	image->SetScalarTypeToUnsignedChar();
	image->SetSpacing(1, 1,1);  
	image->SetOrigin(0, 0, 0); 
	image->SetNumberOfScalarComponents(1);  
	image->AllocateScalars();  
	byte*ptr = (byte *)image->GetScalarPointer();
	InitArrayFromVolFile(ptr,fileName,width*height*depth);
	//if(width<200){ModifyDataBw(ptr,width*height*depth);}
	cout<<"load image "<<fileName<<" cmp\n";

	vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rayCastFunction = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkColorTransferFunction::New();
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity= vtkPiecewiseFunction::New();
	volumeColor->SetColorSpaceToDiverging();
	volumeColor->AddRGBSegment(
		0,
		255/255.0,
		255/255.0,
		255/255.0,
		255,
		255/255.0,
		0/255.0,
		0/255.0);  
	volumeScalarOpacity->AddPoint(0,0);
	volumeScalarOpacity->AddPoint(255,1);
	vtkSmartPointer<vtkVolumeProperty> volumePro=vtkVolumeProperty::New();
	volumePro->SetColor(volumeColor);
	volumePro->SetScalarOpacity(volumeScalarOpacity);
	if(num==1)
	{
		volumePro->ShadeOn();
	}
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();//81 86 108
	renWin->AddRenderer(ren);
	ren->SetBackground(81/255.0,86/255.0,108/255.0);
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);
	iren->Initialize();
	vtkSmartPointer<vtkVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
	volumeMapper->SetInput(image);
	volumeMapper->SetVolumeRayCastFunction(rayCastFunction);
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetProperty(volumePro);
	volume->SetMapper(volumeMapper);
	renWin->SetSize(1024, 768);

	ren->AddViewProp(volume);
	vtkSmartPointer<vtkCamera> aCamera = ren->GetActiveCamera();
	aCamera->SetPosition(6000,0,0);
	aCamera->SetFocalPoint(width/2,height/2,depth/2);
	aCamera->SetViewUp(0,0,1);
	vtkSmartPointer<vtkCallbackCommand> keypressCallback =  vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback ( KeypressCallbackFunction );
	iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );

	iren->Start();
	cout<<"over"<<endl;
	return 0;
}
int mainShowOr(int para)
{
	int lineNumber =281;// 281;
	int cdpNumber = 481;//;
	int timeNumber = 2501;//2501;
	//CImg<byte> *imageResSpace=new CImg<byte>();
	//imageResSpace->load_raw("D:\\VTKproj\\new_result_small.raw",cdpNumber,timeNumber,lineNumber,1);
	vtkSmartPointer<vtkImageData> image=vtkImageData::New();
	image->SetDimensions(lineNumber, cdpNumber, timeNumber);  
	image->SetScalarTypeToUnsignedChar();
	image->SetSpacing(1, 1,1);  
	image->SetOrigin(0, 0, 0); 
	image->SetNumberOfScalarComponents(1);  
	image->AllocateScalars();  
	byte*ptr = (byte *)image->GetScalarPointer();
	InitArrayFromVolFile(ptr,migFile,lineNumber*cdpNumber*timeNumber);
	//memcpy(ptr,imageResSpace->_data,cdpNumber*timeNumber*lineNumber);
	//ModifyData(ptr,cdpNumber,timeNumber,lineNumber);
	/*delete imageResSpace;*/
	cout<<"load image "<<migFile<<" complete\n";

	vtkSmartPointer<vtkRenderer> ren =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin =
		vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);
	vtkSmartPointer<vtkRenderWindowInteractor> iren =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);
	iren->Initialize();
	vtkSmartPointer<vtkVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
	volumeMapper->SetInput(image);
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	renWin->SetSize(1024, 768);
	vtkCamera *camera = ren->GetActiveCamera();
	double *c = volume->GetCenter();
	camera->SetFocalPoint(c[0], c[1], c[2]);
	camera->SetPosition(c[0] + 3400, c[1], c[2]);
	camera->SetViewUp(0, 0, -1);
	ren->AddViewProp(volume);

	vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rayCastFunction = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
	volumeMapper->SetVolumeRayCastFunction(rayCastFunction);

	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkColorTransferFunction::New();
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity= vtkPiecewiseFunction::New();
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity=vtkPiecewiseFunction::New();

	volumeColor->AddRGBSegment(
		0,
		0/255.0,
		0/255.0,
		0/255.0,
		255,
		255/255.0,
		255/255.0,
		255/255.0);  //64 255 16

	volumeScalarOpacity->AddPoint(0,0);
	volumeScalarOpacity->AddPoint(para,0);
	volumeScalarOpacity->AddPoint(255,1);


	vtkSmartPointer<vtkVolumeProperty> volumePro=vtkVolumeProperty::New();
	volumePro->SetColor(volumeColor);
	volumePro->SetScalarOpacity(volumeScalarOpacity);

	volume->SetProperty(volumePro);
	renWin->Render();
	iren->Start();
	cout<<"over"<<endl;
	return 0;
}
int main2()
{
	int number=3;
	printf("1.show original data.\n");
	printf("2.process data.\n");
	printf("3.show result mesh data.\n");
	printf("4.show result volume data.\n");
	printf("5.segment data.\n");
	printf("6.show segment result.\n");
	printf("put in number : ");
	scanf("%d",&number);
	#pragma region ShowOr
	if(number==1)
	{
		while(1)
		{
			printf("input threshold:\n");
			int thres=180;
			scanf("%d",&thres);
			mainShowOr(thres);
		}
	}
	#pragma endregion ShowOr
	#pragma region Process
	
	if(number==2)
	{
		int number2=1;
		printf("1.use 2501 slices\n");
		printf("2.use 281 slices\n");
		printf("3.use merged slices\n");
		printf("put in number : ");
		scanf("%d",&number2);
		if(number2==1){return main1Ex2501();}
		if(number2==2){return main1Ex281();}
		if(number2==3){return main1Merge();}
	}
	#pragma endregion Process 
	#pragma region MC 
	if(number==3)
		{
			printf("choose volume 1.volume2501s 2. volume281s 3. volumeMergeds\n");
			int number2=1;
			scanf("%d",&number2);
			printf("use small size raw? 1.no 2.yes\n");
			int number3=1;
			scanf("%d",&number3);
			if(number2==1)
			{
				if(number3==1){return mainMc(281,481,2501,resultFile2501);}
				if(number3==2){return mainMc(141,241,1251,resultFile2501Small);}
			}
			if(number2==2)
			{
				if(number3==1){return mainMc(281,481,2501,resultFile281);}
				if(number3==2){return mainMc(141,241,1251,resultFile281Small);}
			}
			if(number2==3)
			{
				if(number3==1){return mainMc(281,481,2501,resultFileMerge);}
				if(number3==2){return mainMc(141,241,1251,resultFileMergeSmall);}
			}
		}
	#pragma endregion MC
	#pragma region VR 
	if(number==4)
	{
		printf("choose volume 1.volume2501s 2. volume281s 3. volumeMergeds\n");
		int number2=1;
		scanf("%d",&number2);
		printf("use small size raw? 1.no 2.yes\n");
		int number3=1;
		scanf("%d",&number3);
		if(number2==1)
		{
			if(number3==1){return mainVolRender(281,481,2501,resultFile2501);}
			if(number3==2){return mainVolRender(141,241,1251,resultFile2501Small);}
		}
		if(number2==2)
		{
			if(number3==1){return mainVolRender(281,481,2501,resultFile281);}
			if(number3==2){return mainVolRender(141,241,1251,resultFile281Small);}
		}
		if(number2==3)
		{
			if(number3==1){return mainVolRender(281,481,2501,resultFileMerge);}
			if(number3==2){return mainVolRender(141,241,1251,resultFileMergeSmall);}
		}
	}
	#pragma endregion VR
	#pragma region Seg
	if(number==5)
	{
		mainSeg();
	}
	#pragma endregion Seg
    #pragma region SSeg 
	if(number==6)
	{
		while(1)
		{
			mainShowSeg();
		}
	}
    #pragma endregion SSeg
	return 0;
}
//int mainTest()
//{
//	int lineNumber =141;// 281;
//	int cdpNumber = 241;//;
//	int timeNumber = 1251;//2501;
//	/*vtkSmartPointer<vtkUniformGrid> image=vtkUniformGrid::New();
//	image->SetDimensions(lineNumber, cdpNumber, timeNumber);  
//	image->SetScalarTypeToUnsignedChar();
//	image->SetSpacing(1, 1,1);  
//	image->SetOrigin(0, 0, 0); 
//	image->SetNumberOfScalarComponents(1);  
//	image->AllocateScalars();  
//	byte*ptr = (byte *)image->GetScalarPointer();*/
//	byte*ptr=new byte[lineNumber*cdpNumber*timeNumber];
//	InitArrayFromVolFile(ptr,migSmallFile,lineNumber*cdpNumber*timeNumber);
//	//for(int i=0;i<lineNumber*cdpNumber*timeNumber;i++)
//	//{
//	//	byte value = ptr[i];
//	//	if(value<180)
//	//		ptr[i]=0;
//	//	else
//	//		ptr[i]=255;
//	//	
//	//		//image->BlankPoint(i);
//	//}
//	//SaveBytesToFile("D:\\VTKproj\\mig_small_180b.raw",ptr,lineNumber*cdpNumber*timeNumber);
//	cout<<"load image "<<migFile<<" complete\n";
//
//	vtkSmartPointer<vtkImageToStructuredGrid> itos=vtkImageToStructuredGrid::New();
//	itos->SetInput(image);
//	itos->Update();
//	for(int i=0;i<lineNumber*cdpNumber*timeNumber;i++)
//	{
//		byte value = ptr[i];
//		if(value<180)
//			itos->BlankPoint(i);
//	}
//
//
//	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkPolyDataMapper::New();  
//	cubeMapper->SetInput(polydata);  
//	cubeMapper->Update();
//	vtkSmartPointer<vtkActor> cubeActor = vtkActor::New();  
//	cubeActor->GetProperty()->SetColor(1,1,0.5);
//	cubeActor->SetMapper(cubeMapper);  
//	vtkSmartPointer<vtkRenderer> ren =vtkSmartPointer<vtkRenderer>::New();
//	vtkSmartPointer<vtkRenderWindow> renWin =vtkSmartPointer<vtkRenderWindow>::New();
//	renWin->AddRenderer(ren);
//	vtkSmartPointer<vtkRenderWindowInteractor> iren =vtkSmartPointer<vtkRenderWindowInteractor>::New();
//	iren->SetRenderWindow(renWin);
//	iren->Initialize();
//	renWin->SetSize(1024, 768);
//	ren->AddActor(cubeActor);  
//	vtkSmartPointer<vtkCallbackCommand> keypressCallback =  vtkSmartPointer<vtkCallbackCommand>::New();
//	keypressCallback->SetCallback ( KeypressCallbackFunction );
//	iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );
//	iren->Start();
//
//	return 0;
//}