
#include "delay.h"
#include "my_w5500.h"
#include "my_messeg.h"
#include "malloc.h"
#include "cJSON.h"

#include "my_rf.h"//��������ļ����ڻ�ȡ������ַ
#include "my_lcd.h"//��ȡ�ֶ��Զ�
#include "lcd.h"
#include "includes.h"
#include "my_debug.h"
#include "dhcp.h"
#include "enternet.h"
#include "wk_json.h"

/*******************************************

			msgλ���壺
			msg[0],1,���жϣ�2��������״̬��Ϣ����.3�������ڣ�4�����豸������Ϣ��5��ִ�н�����أ�6�����ͻ��������ޣ�7�����ͱ���
			msg[1],������Ϣʱ�����͵��豸����	��
													����ִ�н��ʱ����8λ
													
			msg[2���豸״̬��0�����ߣ�1�����ߣ�
													����ִ�н��ʱ����8λ
													
			�����豸����״̬��Ϣʱ
			������ʱ��
			msg3,id,��8λ
			msg4,id,��8λ
			msg5���豸״̬������״̬��1����
			msg6������״̬��1������2����
			�ɼ���ʱ��
			msg3~6���ɼ������ݻ�������ַ���ɸߵ���,
			msg7-8,����ʱ�ĵ�ַ
			

*********************************************/


//����״̬��־λ,ip��ַ��ͻ������û�����ӵ�
u8 NET_STATE=0;
u8 NET_S0_STATE=0;
u8 NET_S1_STATE=0;
u8 NET_S2_STATE=0;

u8 WK_FUCOS=TASK_MAX_NUM;//���ڽ���


//��������״̬�������ã�0��δ���ӣ�1�������������أ�2���������Ϸ�����
u8 DBG_INTER_STATE=0;



void my_w5500 (void * t)
{
	cJSON_Hooks a;
	W5500_Initialization();	
	net_test();
	net_sysboot_init();
	a.malloc_fn=mymalloc;
	a.free_fn=myfree;
	cJSON_InitHooks(&a);

	
	while(1)
	{
		delay_ms(100);
		if (net_get_phycstate()==0) continue;//����û�����ӣ�ִ����һ��ѭ��
		if (net_check_parameters()==0)//����ip��ַ���Ϸ�
		{
			DBG_INTER_STATE=dhcp_retry();//�Զ���ȡIP��ַ
			
		}
		if (NET_STATE&CONFLICT)//ip��ͻ
		{
			DBG_INTER_STATE=dhcp_retry();//�Զ���ȡIP��ַ
		}
		NET_STATE=0;
		my_debug ( );		//������Ϣ���
		wk_client();
		
	}
}



		//�¿ؿͻ����¼�����
void wk_client(void)
{
	u8 m_send[MESSEG_DATA]={0};
	u8 m_recv[MESSEG_DATA]={0};
	static u8 so_state_old=0;
	
	/*�ж���Ϣ����*/
	so_state_old|=NET_S0_STATE;
	if (so_state_old&IR_CON)//���ӳɹ�����������Ϣ
	{
		so_state_old&=~IR_CON;
	}
	if (so_state_old&IR_RECV)//��������ݵȴ�����
	{
		so_state_old&=~IR_RECV;
		server_data ();
	}
	NET_S0_STATE=0;
	
	
			/*�˿�0������ϴ���*/
	if ((net_get_comstate(0)!=SOCK_ESTABLISHED))//������ӶϿ�
	{		
		if (tcp_connect(0,4545,SERVER_IP,SERVER_PORT)==FALSE)
		{
			if (net_check_gateway()==TRUE)
			{
				DBG_INTER_STATE=1;//����������
				//serch_server();
			}
			else
			{
				DBG_INTER_STATE=0;
				DBG_INTER_STATE=dhcp_retry();//�Զ���ȡIP��ַ
			}
		}
		else
		{
			DBG_INTER_STATE=2;//�����Ϸ�����
		}
	}



	//ϵͳ����Ϣ����
	if (get_messeg(WK_MESSEG,m_recv)==0)//�������Ϣ
	{
		if (DBG_INTER_STATE==2) 
		{
			switch (m_recv[0])
			{
				case 2:
					send_json (m_recv);
					break;
				case 3:
//					W5500_Initialization();	
					break;
				case 4:
					send_json_adddevice(m_recv);
					break;
				case 6:
					send_json_limit (m_recv);
					break;
				case 7:
					send_json_warn(m_recv); 
					break;
				default:
					break;
			}
		}
	}

}











					//���������жϵ�������
void my_w5500interrupt (void * t)
{
	while (1)
	{
		delay_ms(4);
	}
}

 





























					//��λ�����û���������
void json_setting (cJSON *root)
{
	u16 wdup;
	u16 sdup;
	u16 tvocup;
	u16 wddown;
	u16 sddown;
	cJSON *js_controldata = cJSON_GetObjectItem(root, "data");
	wdup= cJSON_GetObjectItem(js_controldata, "wdup")->valueint;
	sdup= cJSON_GetObjectItem(js_controldata, "sdup")->valueint;
	tvocup= cJSON_GetObjectItem(js_controldata, "tvocup")->valueint;
	wddown= cJSON_GetObjectItem(js_controldata, "wddown")->valueint;
	sddown= cJSON_GetObjectItem(js_controldata, "sddown")->valueint;

	Lcd_SetLimitData(0,wdup);
	Lcd_SetLimitData(1,wddown);
	Lcd_SetLimitData(2,sdup);
	Lcd_SetLimitData(3,sddown);
	Lcd_SetLimitData(4,tvocup);
	Save_LCD();
	json_return (Get_MyAddr(),cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
}
				//���õ�ָ��ֵ
void json_setto (cJSON *root)
{
}

			//�����豸
void json_start (cJSON *root)
{
	
	
	
	u16 devId;
	devId= cJSON_GetObjectItem(root, "devId")->valueint;
	if (Lcd_GetHandstate()==0)//�Զ�״̬��ִ���ֶ�����
	{
		json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NotHandMode");
		return ;
	}
	u8 m_send[MESSEG_DATA]={0};
	
	m_send[0]=0xff;
	m_send[1]=0x02;
	m_send[2]=devId>>8;
	m_send[3]=devId;
	m_send[4]=1;
	m_send[5]=0;
	send_messeg(RF_MESSEG,m_send);
	while (1)
	{
		delay_ms(200);//�ȴ��������
		if (get_messeg(WK_MESSEG,m_send)==0)
		{
			if (m_send[0]==5)
			{
				u16 err=(m_send[1]<<8)|m_send[2];
				switch (err)
				{
					case ERR_SUCCESS:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
						break;
					case ERR_TIMEOUT:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"TimeOut");
						break;
					case ERR_OFFLINE:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OffLine");
						break;
					case ERR_NONEADDR:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NoAddr");
						break;
					default:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OtherErr");
						break;
				}
				break;
			}
		}
	}
}


void json_alarm (cJSON *root)
{
	
	u8 m_send[MESSEG_DATA]={0};
	u8 *phone;
	u8 sms=0;
	u8 call=0;
	cJSON *js_controldata = cJSON_GetObjectItem(root, "data");
	phone=cJSON_GetObjectItem(js_controldata, "phone")->valuestring;
	sms=cJSON_GetObjectItem(js_controldata, "sms")->valueint;
	call=cJSON_GetObjectItem(js_controldata, "call")->valueint;
	
						//���͸������������
	m_send[0]=3;
	m_send[2]=(u32)phone>>24;
	m_send[3]=(u32)phone>>16;
	m_send[4]=(u32)phone>>8;
	m_send[5]=(u32)phone;
//	send_messeg(DX_MESSEG,m_send);��ʹ�ö���ģ��
	while (1)
	{
		delay_ms(200);//�ȴ��������
//		if (get_messeg(DX_MESSEG,m_send)==0)
		{
			if (m_send[0]==5)
			{
				u16 err=(m_send[1]<<8)|m_send[2];
				switch (err)
				{
					case ERR_SUCCESS:
						json_return (Get_MyAddr(),cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
						break;
					case ERR_FAIL:
						json_return (Get_MyAddr(),cJSON_GetObjectItem(root, "cmdNum")->valuestring,"Fail");
						break;
					default:
						json_return (Get_MyAddr(),cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NoneDevice");
						break;
				}
				break;
			}
		}
	}
}


void json_close (cJSON *root)
{
	
	
	u16 devId;
	devId= cJSON_GetObjectItem(root, "devId")->valueint;
	if (Lcd_GetHandstate()==0)//�Զ�״̬��ִ���ֶ�����
	{
		json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NotHandMode");
		return ;
	}
	u8 m_send[MESSEG_DATA]={0};
	
	m_send[0]=0xff;
	m_send[1]=0x02;
	m_send[2]=devId>>8;
	m_send[3]=devId;
	m_send[4]=0;
	m_send[5]=0;
	send_messeg(RF_MESSEG,m_send);
	while (1)
	{
		delay_ms(200);//�ȴ��������
		if (get_messeg(WK_MESSEG,m_send)==0)
		{
			if (m_send[0]==5)
			{
				u16 err=(m_send[1]<<8)|m_send[2];
				switch (err)
				{
					case ERR_SUCCESS:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
						break;
					case ERR_TIMEOUT:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"TimeOut");
						break;
					case ERR_OFFLINE:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OffLine");
						break;
					case ERR_NONEADDR:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NoAddr");
						break;
					default:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OtherErr");
						break;
				}
				break;
			}
		}
	}

}

void json_mode0 (cJSON *root)
{
	Lcd_SetHandstate(0);
	{
		json_return (Get_MyAddr(),cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
	}

}

void json_mode1 (cJSON *root)
{
	Lcd_SetHandstate(1);
	{
		json_return (Get_MyAddr(),cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
	}
}


void json_version (cJSON *root)
{	
	
	char *out1;
  cJSON *root1,*js_collect1;
  root1 = cJSON_CreateObject();
	
	cJSON_AddNumberToObject(root1,"centerId",Get_MyAddr());
	cJSON_AddStringToObject(root1,"cmdNum",cJSON_GetObjectItem(root, "cmdNum")->valuestring);
	cJSON_AddStringToObject(root1,"errorType","0");
	cJSON_AddNumberToObject(root1,"type",5);
	
	cJSON_AddItemToObject(root1,"data", js_collect1 = cJSON_CreateObject());
	cJSON_AddStringToObject(js_collect1,"version",__DATE__);//�������ʱ��
	cJSON_AddStringToObject(js_collect1,"versiontime",__TIME__);//�������ʱ��
	
							//ȡ��json�ַ���
	out1=cJSON_PrintUnformatted(root1);	
	cJSON_Delete(root1);
	server_send_data((u8*)out1);//�������ݣ��Զ�д��س�����
	myfree(out1);
	
}




void json_startup (cJSON *root)
{
	
	
	u16 devId;
	devId= cJSON_GetObjectItem(root, "devId")->valueint;
	if (Lcd_GetHandstate()==0)//�Զ�״̬��ִ���ֶ�����
	{
		json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NotHandMode");
		return ;
	}
	u8 m_send[MESSEG_DATA]={0};
	
	m_send[0]=0xff;
	m_send[1]=0x02;
	m_send[2]=devId>>8;
	m_send[3]=devId;
	m_send[4]=1;
	m_send[5]=1;
	send_messeg(RF_MESSEG,m_send);
	while (1)
	{
		delay_ms(200);//�ȴ��������
		if (get_messeg(WK_MESSEG,m_send)==0)
		{
			if (m_send[0]==5)
			{
				u16 err=(m_send[1]<<8)|m_send[2];
				switch (err)
				{
					case ERR_SUCCESS:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
						break;
					case ERR_TIMEOUT:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"TimeOut");
						break;
					case ERR_OFFLINE:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OffLine");
						break;
					case ERR_NONEADDR:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NoAddr");
						break;
					default:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OtherErr");
						break;
				}
				break;//�˳�ѭ��
			}
		}
	}

}


void json_startdown (cJSON *root)
{
	
	
	u16 devId;
	devId= cJSON_GetObjectItem(root, "devId")->valueint;
	if (Lcd_GetHandstate()==0)//�Զ�״̬��ִ���ֶ�����
	{
		json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NotHandMode");
		return ;
	}
	u8 m_send[MESSEG_DATA]={0};
	
	m_send[0]=0xff;
	m_send[1]=0x02;
	m_send[2]=devId>>8;
	m_send[3]=devId;
	m_send[4]=1;
	m_send[5]=2;
	send_messeg(RF_MESSEG,m_send);
	while (1)
	{
		delay_ms(200);//�ȴ��������
		if (get_messeg(WK_MESSEG,m_send)==0)
		{
			if (m_send[0]==5)
			{
				u16 err=(m_send[1]<<8)|m_send[2];
				switch (err)
				{
					case ERR_SUCCESS:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"0");
						break;
					case ERR_TIMEOUT:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"TimeOut");
						break;
					case ERR_OFFLINE:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OffLine");
						break;
					case ERR_NONEADDR:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"NoAddr");
						break;
					default:
						json_return (devId,cJSON_GetObjectItem(root, "cmdNum")->valuestring,"OtherErr");
						break;
				}
				break;
			}
		}
	}
	
}
























				//���Ͳɼ�������
u8 send_json_cj (u8 *msg)
{
				//jsonʹ�õı���
	char *out;
  cJSON *root,*js_collect;
  root = cJSON_CreateObject();
	
				//��ʹ�õı���
	u8 * data;//�ɼ������ݻ����ַ
	u16 id;
	u8 dd;
	
	data=(u8 *)((msg[3]<<24)|(msg[4]<<16)|(msg[5]<<8)|(msg[6]));//ȡ�õ�ַ
	
	id=(data[2]<<8)|data[3];
	
	

		cJSON_AddNumberToObject(root,"dataType",2);
	  cJSON_AddNumberToObject(root,"deviceId",id);
    cJSON_AddNumberToObject(root,"pcbType",11);
		cJSON_AddNumberToObject(root,"centerId",Get_MyAddr());
		

    cJSON_AddItemToObject(root,"data", js_collect = cJSON_CreateObject());
	
    cJSON_AddNumberToObject(js_collect,"temp",data[17]+data[18]/10.0);
		cJSON_AddNumberToObject(js_collect,"humi",data[19]+data[20]/10.0);
		cJSON_AddNumberToObject(js_collect,"pm25",0);
		cJSON_AddNumberToObject(js_collect,"tvoc",data[21]+data[22]/10.0);


								
								//ȡ��json�ַ���
		out=cJSON_PrintUnformatted(root);	
		dd=mem_perused();
		cJSON_Delete(root);
		server_send_data((u8*)out);//�������ݣ��Զ�д��س�����
		myfree(out);
		dd=mem_perused();

	return 0;
}





u8 send_json_kz (u8 *msg)
{
	char *out;
	cJSON *root,*js_control;
	root = cJSON_CreateObject();
	
	u16 id;
	id=(msg[3]<<8)|msg[4];


	cJSON_AddNumberToObject(root,"dataType",2);
	cJSON_AddNumberToObject(root,"deviceId",id);
	cJSON_AddNumberToObject(root,"pcbType",12);
	cJSON_AddNumberToObject(root,"centerId",Get_MyAddr());
	
	cJSON_AddItemToObject(root,"data", js_control = cJSON_CreateObject());
	
	if (msg[1]==2)			//�յ�
	{
		cJSON_AddNumberToObject(js_control,"conditionswitch",msg[5]);
		cJSON_AddNumberToObject(js_control,"conditionstate",msg[6]);
		cJSON_AddNumberToObject(js_control,"humiswitch",0);
		cJSON_AddNumberToObject(js_control,"airswitch",0);
		cJSON_AddNumberToObject(js_control,"humidifier",0);
		cJSON_AddNumberToObject(js_control,"humidity",0);
		cJSON_AddNumberToObject(js_control,"humiset",0);
	}
	else if (msg[1]==3)			//��ʪ��
	{
		cJSON_AddNumberToObject(js_control,"humiswitch",msg[5]);
		cJSON_AddNumberToObject(js_control,"conditionstate",0);
		cJSON_AddNumberToObject(js_control,"conditionswitch",0);
		cJSON_AddNumberToObject(js_control,"airswitch",0);
		cJSON_AddNumberToObject(js_control,"humidifier",0);
		cJSON_AddNumberToObject(js_control,"humidity",0);
		cJSON_AddNumberToObject(js_control,"humiset",0);
	}
	else if (msg[1]==4)			//����������
	{
		cJSON_AddNumberToObject(js_control,"airswitch",msg[5]);
		cJSON_AddNumberToObject(js_control,"humiswitch",0);
		cJSON_AddNumberToObject(js_control,"conditionstate",0);
		cJSON_AddNumberToObject(js_control,"conditionswitch",0);
		cJSON_AddNumberToObject(js_control,"humidifier",0);
		cJSON_AddNumberToObject(js_control,"humidity",0);
		cJSON_AddNumberToObject(js_control,"humiset",0);
	}
	else if (msg[1]==5)			//��ʪ��
	{
		cJSON_AddNumberToObject(js_control,"humidifier",msg[5]);
		cJSON_AddNumberToObject(js_control,"humidity",0);
		cJSON_AddNumberToObject(js_control,"airswitch",0);
		cJSON_AddNumberToObject(js_control,"humiswitch",0);
		cJSON_AddNumberToObject(js_control,"conditionstate",0);
		cJSON_AddNumberToObject(js_control,"conditionswitch",0);
		cJSON_AddNumberToObject(js_control,"humiset",0);
	}
	else if (msg[1]==6)			//��ʪ��
	{
		cJSON_AddNumberToObject(js_control,"humidity",msg[5]);
		cJSON_AddNumberToObject(js_control,"humiset",msg[6]);
		cJSON_AddNumberToObject(js_control,"humidifier",0);
		cJSON_AddNumberToObject(js_control,"airswitch",0);
		cJSON_AddNumberToObject(js_control,"humiswitch",0);
		cJSON_AddNumberToObject(js_control,"conditionstate",0);
		cJSON_AddNumberToObject(js_control,"conditionswitch",0);
	}
	cJSON_AddNumberToObject(js_control,"cmd",Lcd_GetHandstate());//�ֶ�
	



							//ȡ��json�ַ���
	out=cJSON_PrintUnformatted(root);	cJSON_Delete(root);
	server_send_data((u8*)out );//ָ��Socket(0~7)�������ݴ���,�˿�0����23�ֽ�����
	myfree(out);

	return 0;
}

/*
		��Ϣ��ʽ��
		msg0=7��������Ϣ
		msg1=�������ͣ�1���¶ȹ��ߣ�2���¶ȹ��ͣ�3ʪ�ȹ��ߣ�4ʪ�ȹ��ͣ�5tvoc���꣬6©ˮ������7���豸����
		msg2=�豸����
		msg3��msg4���豸��ַ
		msg5������ֵ��ʵ�ʵĻ���ֵ
*/


const char warn_char[][8]={"wdup","wddown","sdup","sddown","tvoc","loushui","deverr"};



u8 send_json_warn (u8 *msg)
{
	char *out;
	cJSON *root,*js_control;
	root = cJSON_CreateObject();
	
	u16 id;
	id=(msg[3]<<8)|msg[4];


	cJSON_AddNumberToObject(root,"dataType",2);
	cJSON_AddNumberToObject(root,"deviceId",id);
	cJSON_AddNumberToObject(root,"pcbType",12);
	cJSON_AddNumberToObject(root,"centerId",Get_MyAddr());
	
	cJSON_AddItemToObject(root,"data", js_control = cJSON_CreateObject());
	
	if (msg[1]==6)			//��ʪ��
	{
		cJSON_AddNumberToObject(js_control,"humidity",msg[5]);
		cJSON_AddNumberToObject(js_control,"humiset",msg[6]);
		cJSON_AddNumberToObject(js_control,"loushui",1);
		cJSON_AddNumberToObject(js_control,"humidifier",0);
		cJSON_AddNumberToObject(js_control,"airswitch",0);
		cJSON_AddNumberToObject(js_control,"humiswitch",0);
		cJSON_AddNumberToObject(js_control,"conditionstate",0);
		cJSON_AddNumberToObject(js_control,"conditionswitch",0);
	}
	cJSON_AddNumberToObject(js_control,"cmd",Lcd_GetHandstate());//�ֶ�



							//ȡ��json�ַ���
	out=cJSON_PrintUnformatted(root);	cJSON_Delete(root);
	server_send_data((u8*)out);//ָ��Socket(0~7)�������ݴ���,�˿�0����23�ֽ�����
	myfree(out);

	return 0;
}
























u8  send_json_limit (u8 *msg)
{
				//jsonʹ�õı���
	char *out;
  cJSON *root,*js_collect;
  root = cJSON_CreateObject();

	cJSON_AddNumberToObject(root,"devId",Get_MyAddr());
	cJSON_AddNumberToObject(root,"centerId",Get_MyAddr());
	cJSON_AddNumberToObject(root,"mode",Lcd_GetHandstate());
	cJSON_AddNumberToObject(root,"type",3);
	cJSON_AddStringToObject(root,"swType","wk");//�ϴ��������������¿�
	
	cJSON_AddItemToObject(root,"data", js_collect = cJSON_CreateObject());
	cJSON_AddNumberToObject(js_collect,"wdup",Lcd_GetLimitData(0));
	cJSON_AddNumberToObject(js_collect,"wddown",Lcd_GetLimitData(1));
	cJSON_AddNumberToObject(js_collect,"sdup",Lcd_GetLimitData(2));
	cJSON_AddNumberToObject(js_collect,"sddown",Lcd_GetLimitData(3));
	cJSON_AddNumberToObject(js_collect,"tvocup",Lcd_GetLimitData(4));


								//ȡ��json�ַ���
		out=cJSON_PrintUnformatted(root);	
		cJSON_Delete(root);
		server_send_data((u8*)out);//�������ݣ��Զ�д��س�����
		myfree(out);

	return 0;
	
}
	


								//���жϾ͸�����������Ϣ
void EXTI4_IRQHandler(void)
{
	u8 temp=0;u8 sntemp=0;
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line4);


 
	do
	{
		temp = Read_W5500_1Byte(IR);//��ȡ�жϱ�־�Ĵ���
		Write_W5500_1Byte(IR, (temp&0xf0));//��д����жϱ�־
		NET_STATE=temp;


		temp=Read_W5500_1Byte(SIR);//��ȡ�˿��жϱ�־�Ĵ���	
		if((temp & S0_INT) == S0_INT)//Socket0�¼����� 
		{
			sntemp=Read_W5500_SOCK_1Byte(0,Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
			Write_W5500_SOCK_1Byte(0,Sn_IR,sntemp);
			NET_S0_STATE=sntemp;
		}
		if((temp & S1_INT) == S1_INT)//Socket1�¼����� 
		{
			sntemp=Read_W5500_SOCK_1Byte(1,Sn_IR);//��ȡSocket1�жϱ�־�Ĵ���
			Write_W5500_SOCK_1Byte(1,Sn_IR,sntemp);
			NET_S1_STATE=sntemp;
		}
		if((temp & S2_INT) == S2_INT)//Socket1�¼����� 
		{
			sntemp=Read_W5500_SOCK_1Byte(2,Sn_IR);//��ȡSocket1�жϱ�־�Ĵ���
			Write_W5500_SOCK_1Byte(2,Sn_IR,sntemp);
			NET_S2_STATE=sntemp;
		}

	}while(Read_W5500_1Byte(SIR) != 0) ;
	TaskIntSendMsg(7,1);




	}
}








