
#include "my_rf.h"//��������ļ����ڻ�ȡ������ַ
#include "my_lcd.h"//��ȡ�ֶ��Զ�
#include "lcd.h"
#include "includes.h"
#include "my_debug.h"

#include "wk_json.h"
#include "w5500.h"
#include "enternet.h"
#include "dhcp.h"


//�����ã���S0���շ������ݸ��Ƶ����Զ˿�
u8 DBG_COPY_TO_S1CK=0;


				//���ݷ��͵��������ӿں���
void server_send_data(u8 *databuff)
{
	extern u8 DBG_IP[4];
	extern u16 DBG_PORT;
	u16 size=strlen((const char *)databuff);
	Write_SOCK_Data_Buffer(0,databuff,size);
//	Write_SOCK_Data_Buffer(0,"_#_",3);
	Write_SOCK_Data_Buffer(0,"\r\n",3);//�ɰ�Э��ӻس�����
	delay_us(500);//������ʱ��֤������������
	if (DBG_COPY_TO_S1CK==1)
	{
		udp_send(1,DBG_IP,DBG_PORT,"���ͣ�",6);
		udp_send(1,DBG_IP,DBG_PORT,databuff,strlen((const char *)databuff));
		udp_send(1,DBG_IP,DBG_PORT,"\r\n",2);
	}
	else if (DBG_COPY_TO_S1CK==2)
	{
		tcp_send(2,"���ͣ�",6);		
		tcp_send(2,databuff,size);		
		tcp_send(2,"\r\n",2);		
	}
	delay_us(500);//������ʱ��֤������������
}


			//��ȡ���������������ݽӿں���
void server_read_data(u8 *databuff)
{
	extern u8 DBG_IP[4];
	extern u16 DBG_PORT;
	Read_SOCK_Data_Buffer(0,databuff);
	u16 size=strlen((const char *)databuff);
	if (DBG_COPY_TO_S1CK==1)
	{
		udp_send(1,DBG_IP,DBG_PORT,"���գ�",6);
		udp_send(1,DBG_IP,DBG_PORT,databuff,strlen((const char *)databuff));
		udp_send(1,DBG_IP,DBG_PORT,"\r\n",2);
	}
	else if (DBG_COPY_TO_S1CK==2)
	{
		tcp_send(2,"���գ�",6);		
		tcp_send(2,databuff,size);		
		tcp_send(2,"\r\n",2);		
	}
	delay_us(500);//������ʱ��֤������������
	delay_us(500);//������ʱ��֤������������
}




				//�����¿ط���������Ϣ�����ڼ���ǲ���json���ݣ���ֹ��������
u8 server_data(void)
{
	u8 *data;
	data=mymalloc(2048);//����2k�ڴ�
	server_read_data( data);
	//udp������8���ֽڰ�ͷ��tcp����û��
	if (data[0]=='{')//json����
		recv_json(data);
	else if (data[0]=='-')
	{
		myfree(data);
		return 1;
	}
	
	 
	myfree(data);
	return 0;
}



			//���ҷ�����
u8 serch_server(void)
{
	static u8 serip=254;
	extern u8 Gateway_IP[4];
	extern u8 SERVER_IP[4];
	extern u16 SERVER_PORT;
	extern u8 NET_S0_STATE;
	u8 ret=FALSE;
	u8 udpip[4]={255,255,255,255};
	while (serip>Gateway_IP[3]+1)
	{
		SERVER_IP[3]=serip;
		if (udp_init(0,4545)==FALSE)
		{
			return FALSE;
		}
		else
		{
			udp_send(0,udpip,6001,"{\"cmd\":\"search\"}",16);
			delay_ms(1000);
			u8 *data=0;
			if (NET_S0_STATE&IR_RECV)//����յ�����Ϣ
			{
				NET_S0_STATE&=~IR_RECV;
				data=mymalloc(2048);//����2k�ڴ�
				server_read_data( data);
				if (data[8]=='{')
				{
					cJSON *root = cJSON_Parse((const char *)(data+8));
					if (strcmp(cJSON_GetObjectItem(root, "cmd")->valuestring,"serverconf")==0)
					{
						//�������õ�������Ŀ���ַ
						mymemcpy(SERVER_IP,data,4);
						cJSON *js_controldata = cJSON_GetObjectItem(root, "data");
						SERVER_PORT=cJSON_GetObjectItem(js_controldata, "serverport")->valueint;
					
						cJSON_Delete(root);
						ret= TRUE;
					}
					else
					{
						cJSON_Delete(root);
						ret= FALSE;
					}
				}
				myfree(data);
				if (ret==TRUE) return ret;
			}
			
		}
	}
	return FALSE;
}


//�����ݷ�װ��json�ٷ��͵�������
//0,�ѷ��ͣ���=0��ʧ��
u8	send_json (u8 *msg)
{
	u8 ret=0;
	if (msg[0]!=2) return 1;//���Ƿ���ָ�
	if (msg[2]==1) return 1;//���߲�������Ϣ
	switch (msg[1])
	{
		case 1:
			ret=send_json_cj(msg);
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
				ret=send_json_kz(msg);
			break;
		default:
			return 3;//��֧�ֵ��豸
	}
	return ret;
}




				//json��ʽ�����ݵ��������������
void recv_json (u8 *data) 
{
	u8 pcbtype;
	cJSON *root = cJSON_Parse((const char *)data);
	pcbtype=cJSON_GetObjectItem(root, "pcbType")->valueint;
	
	void recv_json_kz (cJSON *root);

	
	
	switch(pcbtype)
	{
		case 11://���»���������

			break;
		case 12://�����豸
			recv_json_kz(root);
			break;
		case 13://�绰����
			break;
		default :
			break;

	}
	cJSON_Delete(root);
}

					//���������Ϳ��ư�ָ��
			//�ɰ�Э��
void recv_json_kz (cJSON *root)
{
	u16 deviceid=0x0000;
	u8 pcbtype;
	u8 conditionswitch;
	u8 conditionstate;
	u8 humiswitch;
	u8 airswitch;
	u8 humidifier;
	u8 humidity;
	u8 humiset;
	u8 cmd;
	u8 m_send[MESSEG_DATA]={0};
							
										//����json����
	cJSON *js_controldata = cJSON_GetObjectItem(root, "data");
	deviceid = cJSON_GetObjectItem(root, "deviceId")->valueint;
	pcbtype = cJSON_GetObjectItem(root, "pcbType")->valueint;

	conditionswitch = cJSON_GetObjectItem(js_controldata, "conditionswitch")->valueint;
	conditionstate = cJSON_GetObjectItem(js_controldata, "conditionstate")->valueint;
	humiswitch = cJSON_GetObjectItem(js_controldata, "humiswitch")->valueint;
	airswitch = cJSON_GetObjectItem(js_controldata, "airswitch")->valueint;
	humidifier=	cJSON_GetObjectItem(js_controldata,"humidifier")->valueint;
	humidity=	cJSON_GetObjectItem(js_controldata,"humidity")->valueint;
	humiset=	cJSON_GetObjectItem(js_controldata,"humiset")->valueint;
	cmd = cJSON_GetObjectItem(js_controldata, "cmd")->valueint;

	if (deviceid)
	{
		m_send[0]=0xff;//����ָ����ַ���豸
	}
	m_send[1]=2;//�ֶ������豸
	m_send[2]=deviceid>>8;
	m_send[3]=deviceid;
	if (conditionswitch==1)
	{
		m_send[0]=2;//�յ�
		m_send[4]=1;
		m_send[5]=conditionstate;
	}
	else if (humiswitch==1)
	{
		m_send[0]=3;//
		m_send[4]=1;
		m_send[5]=2;
	}
	else if (airswitch==1)
	{
		m_send[0]=4;//
		m_send[4]=1;
		m_send[5]=2;
	}
	else if (humidifier==1)
	{
		m_send[0]=5;//
		m_send[4]=1;
		m_send[5]=1;
	}
	else if (humidity==1)//��ʪһ���
	{
		m_send[0]=6;//
		m_send[4]=1;
		m_send[5]=humiset;//�趨��ʪ��ֵ
	}
	if (cmd==2)//��Ϊ�Զ�����
	{
		m_send[0]=0;
		Lcd_SetHandstate(0);//�ı��Զ�״̬
	}
	else if (cmd==1)//�ֶ�
	{
		Lcd_SetHandstate(1);//�ı��ֶ��Զ�״̬
	}
									//���͸�����
	send_messeg(RF_MESSEG,m_send);	
}



						//����λ������豸���Ѹ���2018.12.8
u8 send_json_adddevice (u8 * msg)

{
	extern u8 NET_S0_STATE;
	if (msg[0]!=4) return 1;//������Ƿ����豸������Ϣ��ʲôҲ����
				//jsonʹ�õı���
	
	u16 *cfg=GetCfgData(); 
	u16 i=0;//���������豸����
	u16 j=0;//�ȴ���Ϣ��Ӧ��ʱ����
	u16 k=0;//����������Ϣ���Դ���
	for (i=0;cfg[i];)
	{
		char *out;
		cJSON *root,*js_collect;
		root = cJSON_CreateObject();
		u8 *strbuff=mymalloc(64);
		cJSON_AddNumberToObject(root,"devNum",cfg[i]);//�豸���
		cJSON_AddNumberToObject(root,"centerNum",Get_MyAddr());//���������
		cJSON_AddStringToObject(root,"type","wk");//��Ϣ�����¿�
		cJSON_AddStringToObject(root,"devType",json_get_devicname(cfg[i+1]&0x00ff));//�豸����
		cJSON_AddStringToObject(root,"cmd","online");//���͵���������Ϣ

		sprintf (strbuff,"%d",(cfg[i]<<6)+i);//���ü�����id��Ψһ�Բ���Ψһ��ȷ�ϱ���
		cJSON_AddStringToObject(root,"cmdNum",strbuff);//������Ϣ���ȷ�ϱ���

		if (cfg[i+1]&0x0800)//����
		{
			cJSON_AddStringToObject(root,"devState","offline");//		
		}
		else
		{
			if (cfg[i+1]&0x0100)//�豸����״̬
			{
				cJSON_AddStringToObject(root,"devState","on");//		
			}
			else
			{
				cJSON_AddStringToObject(root,"devState","off");//		
			}			
		}
		
		if (Lcd_GetHandstate())
		{
			cJSON_AddStringToObject(root,"mode","hand");//		
		}
		else
		{
			cJSON_AddStringToObject(root,"mode","auto");//		
		}
		
		out=cJSON_PrintUnformatted(root);	
		cJSON_Delete(root);
		server_send_data((u8*)out);//�������ݣ��Զ�д��س�����
		myfree(out);
		for (j=0;j<100;j++)
		{
			delay_ms(5);
			if (NET_S0_STATE&IR_RECV)//������յ�����Ϣ
			{
				NET_S0_STATE&=~IR_RECV;
				u8 *data;
				data=mymalloc(2048);//����2k�ڴ�
				server_read_data( data);
				cJSON *root = cJSON_Parse((const char *)data);
				if (strcmp(cJSON_GetObjectItem(root, "cmdNum")->valuestring,strbuff)==0)
				{
					if (strcmp(cJSON_GetObjectItem(root, "type")->valuestring,"wk")==0)
					{
						i+=2;//���ͳɹ���������һ��
						k=0;
					}
				}
				cJSON_Delete(root);
				myfree(data);

			}
			else
			{
			}
		}
		myfree(strbuff);//������û�յ���λ�����أ����ͷ��ڴ�
		k++;
		if (k>10)
		{
			break ;
		}
	}
	if (k>10) return -1;
	return 0;
	
}



		//�������ͻ�ȡjson�����ַ���
u8 * json_get_devicname(u8 devtype)
{
	switch (devtype)
	{
		case 0:
			return "jzq";
			break;
		case 1:
			return "cjq";
			break;
		case 2:
			return "znkt";
			break;
		case 3:
			return "csj";
			break;
		case 4:
			return "kqjhq";
			break;
		case 5:
			return "jiasj";
			break;
		case 6:
			return "hsj";
			break;
		default:
			return "nuknown";
			break;
	}
}


					//ִ����λ��������֮��������
void json_return (u8 deviceid,const char * cmdNum,const char *errType)
{
	char *out;
  cJSON *root,*js_collect;
  root = cJSON_CreateObject();
	
	cJSON_AddNumberToObject(root,"devNum",deviceid);//�豸���
	cJSON_AddNumberToObject(root,"centerNum",Get_MyAddr());//���������
	cJSON_AddStringToObject(root,"type","wk");
	cJSON_AddStringToObject(root,"devtype","jzq");
	cJSON_AddStringToObject(root,"cmd","cmdresult");
	cJSON_AddStringToObject(root,"cmdNum",cmdNum);
	cJSON_AddStringToObject(root,"errorType",errType);
	

							//ȡ��json�ַ���
	out=cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);
	server_send_data((u8*)out);//�������ݣ��Զ�д��س�����
	myfree(out);
	
}




