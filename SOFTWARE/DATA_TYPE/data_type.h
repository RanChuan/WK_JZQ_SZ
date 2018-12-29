
#ifndef __DATA_TYPE_H
#define __DATA_TYPE_H
#include "includes.h"



/******以下是环境控制相关的数据类型**********/
 

typedef struct 
{
	float temperture;
	float Humidity;
	float tvoc;
} EnvirDef; //环境值变量


typedef struct 
{
	float tempertureH;
	float tempertureL;
	float HumidityH;
	float HumidityL;
	float TVOCH;
	
} CtrllimitDef;//环境控制限度





/******环境控制相关的数据类型End**********/




/*******设备类型相关数据类型*************/

typedef struct
{
	u16 devId;					//设备编号
	u8 devType;					//设备类型
	u8 devPower;				//设备电源
	u8 devState;				//设备状态
	u8 devCmdState;			//控制状态 ，自动控制中的动态调整
	u8 devHand;					//是否打开手动
	u8 channel;					//设备信道
	u8 offline;					//离线
	u8 devError;				//是否有错误
	u8 devData;					//设备扩展数据
} DeviceDef;

		//定义设备类型
enum
{
	devTypeJZQ=0,
	devTypeCJQ=1,
	devTypeKT=2,
	devTypeCS=3,
	devTypeJH=4,
	devTypeJS=5,
	devTypeYT=6,
};


enum
{
	devPowerOff=0,			//设备电源关
	devPowerOn=1,				//设备电源开
};

enum
{
	devStateNone=0,			//设备无状态
	devStateUp=1,				//设备升
	devStateDown=2,			//设备降
	devStateOther=3,		//其他状态
};



enum
{
	devHandOff=0,
	devHandOn=1,
};

enum
{
	offlineNo=0,			//在线
	offlineYes=1,			//不在线
};



			//定义设备错误类型
enum
{
	devErrorLeaking=1,//漏水报警
	devErrorDevice=2,//设备错误
	devErrorSleep=3,//设备休眠中
};




typedef struct
{
	u16 ImplementerIP;			//执行者IP地址
	u8 cmdType;					//命令类型,自动控制，手动控制，动态调整控制等
	u8 cmdSource;				//命令来源，来自上位机，集中器，手动控制
	u8 cmdPermissions;	//命令权限 0，最高权限
	u8 cmdScopes ;			//命令作用域
	u8 cmd[3];					//命令参数
} CtrlCmdDef;


enum
{
	cmdTypeAuto=0,								//命令类型自动
	cmdTypeHand=1,								//命令类型手动
	cmdTypeAdjustment=2,					//命令类型动态调整
};

enum
{
	cmdSourceJZQAuto=0,				//集中器自动
	cmdSourceJZQHand=1,				//集中器手动
	cmdSourceServerAuto=2,				//服务器自动
	cmdSourceServerHand=3,			//服务器手动
};

enum
{
	cmdPermissionsAdmin=0,			//管理员，这个权限的命令强制执行
	cmdPermissionsUser=1,				//用户，这个权限可以中断动态调整中的自动设备
	cmdPermissionsAuto=2,						//自动，这个权限不能控制已经处于自动模式的设备
	cmdPermissionsVisitor=3,			//游客，这个权限只能控制处于手动模式的设备			
};


enum
{
	cmdScopesOnly=0,				//命令只作用于执行者
	cmdScopesSame=1,				//命令作用于与执行者同类型的设备
	cmdScopesJZQ=2,					//命令作用于集中器之下所有设备
};



/********设备类型相关数据类型End**********/














#endif


