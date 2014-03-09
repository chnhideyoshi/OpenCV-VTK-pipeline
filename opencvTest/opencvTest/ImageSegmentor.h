#include <iostream>
#include <vector>
#include <queue>
#define byte unsigned char
namespace Segmentation
{
	static void Init3dArray(byte*** &pointer,int width,int height,int depth)
	{
		pointer = new byte**[width] ;
		for(int i = 0 ; i < width ; i ++ )
			pointer[i] = new byte*[height] ;
		for(int i = 0 ; i < width ; i ++ )
			for(int j = 0 ; j < height ; j ++ )
				pointer[i][j] = new byte[depth] ;
		for(int i=0;i<width;i++)
			for (int j=0;j<height;j++)
				for(int k=0;k<depth;k++)
					pointer[i][j][k]=0;
	}
	static void Release3dArray(byte*** &pointer,int width,int height,int depth)
	{
		for( int i = 0 ; i < width ; i ++ )
			for( int j = 0 ; j < height ; j ++ )
				delete []pointer[i][j] ;
		for( int i = 0 ; i < width ; i ++ )
			delete []pointer[i] ;
		delete []pointer ;
	}
	struct Point
	{
		int X;
		int Y;
	    int Z;
	    Point(int x,int y,int z)
		{
			X = x; Y = y; Z = z;
		}
		Point() {X=0;Y=0;Z=0; }
	};
	class Region
	{
		public:
			static const char* DIRPATH;
			Region(int x,int y,int z)
			{
				Id=0;
				startXIndex = x;
				endXIndex = x;
				startYIndex = y;
				endYIndex = y;
				startZIndex = z;
				endZIndex = z;
			}
			~Region()
			{
				dataList.clear();
			}
			int Id;
			int startXIndex;
			int startYIndex ;
			int startZIndex ;
			int endXIndex ;
			int endYIndex ;
			int endZIndex ;
		    inline int GetCount(){ return dataList.size(); }
			inline void Add(Point p){ dataList.push_back(p);RefreshRange(p.X,p.Y,p.Z); }
			inline void RefreshRange(int x, int y, int z)
			{
				if (x < startXIndex) { startXIndex = x; }
				if (x > endXIndex) { endXIndex = x; }

				if (y < startYIndex) { startYIndex = y; }
				if (y > endYIndex) { endYIndex = y; }

				if (z < startZIndex) { startZIndex = z; }
				if (z > endZIndex) { endZIndex = z; }
			}
			inline void PrintData(const char* fileName)
			{
				printf("region %d detected,point counts: %d ,box (%d,%d) (%d,%d) (%d,%d) \n"
					, this->Id, GetCount(), startXIndex, endXIndex, startYIndex, endYIndex, startZIndex, endZIndex);
			}
			void WriteXYZFile(const char* fileName);
		private:
			std::vector<Point> dataList;
	};
	void Region::WriteXYZFile(const char* fileName)
	{
		FILE *const file = fopen(fileName,"wb");	
		int count=this->GetCount();
		char a[30];
		fprintf(file,"%d\n",count);//write count to file
		for(int i=0;i<count;i++)
		{
			Point p=this->dataList[i];
			sprintf(a,"%d %d %d\n",p.X,p.Y,p.Z);
			fprintf(file, "%s", a);//write "a" to file
		}
		fclose(file);
	}
	class ImageSegmentor
	{
	public:
		ImageSegmentor(byte* &,int ,int ,int );
		~ImageSegmentor(void);
		static const byte WHITE = 255;
		static const byte BLACK = 0;
		int width;
		int height;
		int depth;
		std::queue<Point> bufferQueue;
		byte*** data;
		byte*** flags;
		void SetDemensions(byte* &buffer,int width,int height,int depth);
		void DoSegmentation(int minCount=0,int maxCount=999999999);
		void (*OnFindRegion)(Region* region);
		void (*OnModifyData)(byte*** &data,int width,int height,int depth);
	private:
		Region* FloodFill(int x, int y, int z, byte***& data, byte***& flags, std::queue<Point> &queue);
		bool CanEnqueue(int x, int y, int z);
		bool InRange(int x, int y, int z);
		void RefineData();
	};
	ImageSegmentor::ImageSegmentor(byte* &buffer,int width,int height,int depth)
	{
		OnFindRegion=0;
		OnModifyData=0;
		this->width=width;
		this->height=height;
		this->depth=depth;
		Init3dArray(data,width, height,depth);
		Init3dArray(flags,width, height,depth);
		SetDemensions(buffer,width,height,depth);
	}
	ImageSegmentor::~ImageSegmentor()
	{
		Release3dArray(data,width,height,depth);
		Release3dArray(flags,width,height,depth);
	}
    void ImageSegmentor:: SetDemensions(byte* &buffer,int width,int height,int depth)
	{
		int index1 = 0;
		for (int k = 0; k < depth; k++)
		{
			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i++)
				{
					data[i][j][k] = buffer[index1];
					index1++;
				}
			}
		}
	}
 	void ImageSegmentor::DoSegmentation(int minCount,int maxCount)
	{
		if(OnModifyData!=0)
			(*OnModifyData)(data,width,height,depth);
		int numberOfRegions = 0;
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				for (int k = 0; k < depth; k++)
				{
					byte pixel = data[i][j][k];
					if (pixel == WHITE && flags[i][j][k] == 0)
					{
						Region* r = FloodFill(i, j, k, data, flags, bufferQueue);
						if (r != 0 && r->GetCount() > minCount&&r->GetCount()<maxCount)
						{
							numberOfRegions++;
							r->Id = numberOfRegions;
							if(OnFindRegion!=0)
								(*OnFindRegion)(r);
						}
						delete r;
					}
				}
			}
		}
	}

	bool ImageSegmentor::CanEnqueue( int x, int y, int z )
	{
		return InRange(x, y, z) && data[x][y][z] == WHITE && flags[x][y][z] == 0;
	}

	bool ImageSegmentor::InRange( int x, int y, int z )
	{
		if (x < 0 || x > width - 1) return false;
		if (y < 0 || y > height- 1) return false;
		if (z < 0 || z > depth - 1) return false;
		return true;
	}

	void ImageSegmentor::RefineData()
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				for (int k = 0; k < depth; k++)
				{
					if (data[i][j][k] > 127) { data[i][j][k] = WHITE; }
					else { data[i][j][k] = BLACK; }
				}
			}
		}
	}

	Region* ImageSegmentor::FloodFill( int x, int y, int z, byte***& data, byte***& flags, std::queue<Point> &queue )
	{
		Region* r = new Region(x, y, z);
		Point startPoint(x,y,z);
		flags[x][y][z] = 1;
		queue.push(startPoint);
		while (!queue.empty())
		{
			Point p = queue.front();
			queue.pop();
			r->Add(p);
			if (CanEnqueue(p.X - 1, p.Y, p.Z))
			{
				flags[p.X - 1][p.Y][p.Z] = 1;
				Point pa(p.X - 1, p.Y, p.Z);
				queue.push(pa);
			}
			if (CanEnqueue(p.X + 1, p.Y, p.Z))
			{
				flags[p.X + 1][p.Y][p.Z] = 1;
				Point pa(p.X + 1, p.Y, p.Z);
				queue.push(pa);
			}
			if (CanEnqueue(p.X, p.Y - 1, p.Z))
			{
				flags[p.X][p.Y - 1][p.Z] = 1;
				Point pa(p.X, p.Y - 1, p.Z);
				queue.push(pa);
			}
			if (CanEnqueue(p.X, p.Y + 1, p.Z))
			{
				flags[p.X][p.Y + 1][p.Z] = 1;
				Point pa(p.X, p.Y + 1, p.Z);
				queue.push(pa);
			}
			if (CanEnqueue(p.X, p.Y, p.Z + 1))
			{
				flags[p.X][p.Y][p.Z + 1] = 1;
				Point pa(p.X, p.Y, p.Z + 1);
				queue.push(pa );
			}
			if (CanEnqueue(p.X, p.Y, p.Z - 1))
			{
				flags[p.X][p.Y][p.Z - 1] = 1;
				Point pa(p.X, p.Y, p.Z - 1);
				queue.push(pa);
			}
		}
		return r;
	}


}
