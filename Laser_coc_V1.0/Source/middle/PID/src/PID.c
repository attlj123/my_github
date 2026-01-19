#include <stdlib.h>
#include <math.h>

#include "PID.h"


void (*Func_Opt)(double) = NULL;				//执行函数指针
double (*Func_Samp)(void) = NULL;				//采样函数指针


//**************************************************
//函数：PID_Initial
//功能：PID参数初始化函数
//说明：
//**************************************************
void PID_Initial(double p,double i,double d,PID_Ratio *pid)
{
	pid->SetValue = 0.0;				//设置值
	pid->ActualValue = 0.0;			//实际值			
	pid->ErrValue = 0.0;				//偏差值
	pid->LastErrValue = 0.0;		//上次偏差值
	pid->OpationValue = 0.0;		//执行参数值
	pid->Integral = 0.0;				//积分参数
	pid->Kp = p;								//比例系数
	pid->Ki = i;								//积分系数
	pid->Kd = d;								//微分系数
}

//**************************************************
//函数：PID_Function
//功能：PID控制执行函数
//说明：
//**************************************************
void PID_Function(void (*f_Opt)(double val),double (f_Samp)(void),PID_Ratio *pid,double set)
{
	double index = 0;				//变积分
	Func_Opt = f_Opt;			//执行函数指针指向当前执行函数
	Func_Samp = f_Samp;			//采样函数指针指向当前采样函数

	pid->SetValue = set;								//PID设置值
	pid->ErrValue = pid->SetValue - pid->ActualValue;	//当前偏差值
	
	if(fabs(pid->ErrValue)>pid->SetValue)
	{
		index = 0.0;
	}
	else if(fabs(pid->ErrValue)<((pid->SetValue)*0.5))
	{
		index = 1.0;
		pid->Integral += pid->ErrValue;						//当前积分值
	}
	else
	{
//		index = (pid->SetValue-fabs(pid->ErrValue))/fabs(pid->ErrValue);
//		pid->Integral += pid->ErrValue;						//当前积分值
		index = 1.0;
		pid->Integral += pid->ErrValue;						//当前积分值
	}
	
	pid->OpationValue = pid->Kp*pid->ErrValue + index*pid->Ki*pid->Integral + pid->Kd*(pid->ErrValue-pid->LastErrValue);		//U(x)执行控制表达式
	pid->LastErrValue = pid->ErrValue;					//上次偏差值
	
	if(Func_Opt != NULL)		//当执行函数为非空，则运行执行函数
		(*Func_Opt)(pid->OpationValue);
	
	if(Func_Samp !=NULL)		//当采样函数为非空，则运行采样函数
		pid->ActualValue = (*Func_Samp)();
}
