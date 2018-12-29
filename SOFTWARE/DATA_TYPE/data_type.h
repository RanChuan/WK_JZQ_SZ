
#ifndef __DATA_TYPE_H
#define __DATA_TYPE_H
#include "includes.h"



/******�����ǻ���������ص���������**********/
 

typedef struct 
{
	float temperture;
	float Humidity;
	float tvoc;
} EnvirDef; //����ֵ����


typedef struct 
{
	float tempertureH;
	float tempertureL;
	float HumidityH;
	float HumidityL;
	float TVOCH;
	
} CtrllimitDef;//���������޶�





/******����������ص���������End**********/




/*******�豸���������������*************/

typedef struct
{
	u16 devId;					//�豸���
	u8 devType;					//�豸����
	u8 devPower;				//�豸��Դ
	u8 devState;				//�豸״̬
	u8 devCmdState;			//����״̬ ���Զ������еĶ�̬����
	u8 devHand;					//�Ƿ���ֶ�
	u8 channel;					//�豸�ŵ�
	u8 offline;					//����
	u8 devError;				//�Ƿ��д���
	u8 devData;					//�豸��չ����
} DeviceDef;

		//�����豸����
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
	devPowerOff=0,			//�豸��Դ��
	devPowerOn=1,				//�豸��Դ��
};

enum
{
	devStateNone=0,			//�豸��״̬
	devStateUp=1,				//�豸��
	devStateDown=2,			//�豸��
	devStateOther=3,		//����״̬
};



enum
{
	devHandOff=0,
	devHandOn=1,
};

enum
{
	offlineNo=0,			//����
	offlineYes=1,			//������
};



			//�����豸��������
enum
{
	devErrorLeaking=1,//©ˮ����
	devErrorDevice=2,//�豸����
	devErrorSleep=3,//�豸������
};




typedef struct
{
	u16 ImplementerIP;			//ִ����IP��ַ
	u8 cmdType;					//��������,�Զ����ƣ��ֶ����ƣ���̬�������Ƶ�
	u8 cmdSource;				//������Դ��������λ�������������ֶ�����
	u8 cmdPermissions;	//����Ȩ�� 0�����Ȩ��
	u8 cmdScopes ;			//����������
	u8 cmd[3];					//�������
} CtrlCmdDef;


enum
{
	cmdTypeAuto=0,								//���������Զ�
	cmdTypeHand=1,								//���������ֶ�
	cmdTypeAdjustment=2,					//�������Ͷ�̬����
};

enum
{
	cmdSourceJZQAuto=0,				//�������Զ�
	cmdSourceJZQHand=1,				//�������ֶ�
	cmdSourceServerAuto=2,				//�������Զ�
	cmdSourceServerHand=3,			//�������ֶ�
};

enum
{
	cmdPermissionsAdmin=0,			//����Ա�����Ȩ�޵�����ǿ��ִ��
	cmdPermissionsUser=1,				//�û������Ȩ�޿����ж϶�̬�����е��Զ��豸
	cmdPermissionsAuto=2,						//�Զ������Ȩ�޲��ܿ����Ѿ������Զ�ģʽ���豸
	cmdPermissionsVisitor=3,			//�οͣ����Ȩ��ֻ�ܿ��ƴ����ֶ�ģʽ���豸			
};


enum
{
	cmdScopesOnly=0,				//����ֻ������ִ����
	cmdScopesSame=1,				//������������ִ����ͬ���͵��豸
	cmdScopesJZQ=2,					//���������ڼ�����֮�������豸
};



/********�豸���������������End**********/














#endif


