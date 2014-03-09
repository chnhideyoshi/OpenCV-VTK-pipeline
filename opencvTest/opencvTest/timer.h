#include <windows.h>
#include <iostream>
using namespace std;
class MyTimerCounter
{
public:
	MyTimerCounter(void);//构造函数
	~MyTimerCounter(void);//析构函数

private:
	LARGE_INTEGER startCount;//记录开始时间

	LARGE_INTEGER endCount;//记录结束时间

	LARGE_INTEGER freq;//本机CPU时钟频率

public:
	double dbTime;//程序运行的时间保存在这里
public:
	inline double GetTick(){return dbTime*1000;}
	inline void ShowMessage(const char* title)
	{
		cout<<title<<" : "<<GetTick()<<"ms."<<endl;
	}
	void Start();//被测程序开始点处开始计时
	void Stop();//被测程序结束点处结束计时
	void Stop(const char* title);
	void StopStart(const char* title);
};

MyTimerCounter::MyTimerCounter(void)
{
	QueryPerformanceFrequency(&freq);//获取主机CPU时钟频率
}
MyTimerCounter::~MyTimerCounter(void)
{
}
void MyTimerCounter::Start()
{
	QueryPerformanceCounter(&startCount);//开始计时
}
void MyTimerCounter::Stop()
{
	QueryPerformanceCounter(&endCount);//停止计时

	dbTime=((double)endCount.QuadPart-(double)startCount.QuadPart)/(double)freq.QuadPart;//获取时间差
}
void MyTimerCounter::Stop(const char* title)
{
	Stop();
	ShowMessage(title);
}
void MyTimerCounter::StopStart(const char* title)
{
	Stop();
	ShowMessage(title);
	Start();
}