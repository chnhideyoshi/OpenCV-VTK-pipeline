#include <windows.h>
#include <iostream>
using namespace std;
class MyTimerCounter
{
public:
	MyTimerCounter(void);//���캯��
	~MyTimerCounter(void);//��������

private:
	LARGE_INTEGER startCount;//��¼��ʼʱ��

	LARGE_INTEGER endCount;//��¼����ʱ��

	LARGE_INTEGER freq;//����CPUʱ��Ƶ��

public:
	double dbTime;//�������е�ʱ�䱣��������
public:
	inline double GetTick(){return dbTime*1000;}
	inline void ShowMessage(const char* title)
	{
		cout<<title<<" : "<<GetTick()<<"ms."<<endl;
	}
	void Start();//�������ʼ�㴦��ʼ��ʱ
	void Stop();//�����������㴦������ʱ
	void Stop(const char* title);
	void StopStart(const char* title);
};

MyTimerCounter::MyTimerCounter(void)
{
	QueryPerformanceFrequency(&freq);//��ȡ����CPUʱ��Ƶ��
}
MyTimerCounter::~MyTimerCounter(void)
{
}
void MyTimerCounter::Start()
{
	QueryPerformanceCounter(&startCount);//��ʼ��ʱ
}
void MyTimerCounter::Stop()
{
	QueryPerformanceCounter(&endCount);//ֹͣ��ʱ

	dbTime=((double)endCount.QuadPart-(double)startCount.QuadPart)/(double)freq.QuadPart;//��ȡʱ���
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