#ifndef _PID_
#define _PID_

//PID参数结构体
typedef struct
{
	double SetValue;		//设置值
	double ActualValue;	//实际值
	double ErrValue;		//偏差值
	double LastErrValue;	//上次的偏差值
	double Kp;				//比例系数
	double Ki;				//积分系数
	double Kd;				//微分系数
	double OpationValue;	//执行器值（控制目标变化）
	double Integral;		//积分系数
}PID_Ratio;


//初始化PID参数值，同时设置比例系数、积分系数、微分系数
void PID_Initial(double p,double i,double d,PID_Ratio *pid);

//PID算法执行函数，指针函数指向执行和采样功能
void PID_Function(void (*f_Opt)(double val),double (f_Samp)(void),PID_Ratio *pid,double set);

#endif


