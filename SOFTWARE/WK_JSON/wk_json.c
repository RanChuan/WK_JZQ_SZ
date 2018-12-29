
#include "my_rf.h"//包含这个文件用于获取本机地址
#include "my_lcd.h"//获取手动自动
#include "lcd.h"
#include "includes.h"
#include "my_debug.h"

#include "wk_json.h"
#include "w5500.h"
#include "enternet.h"
#include "dhcp.h"


//调试用，把S0的收发的数据复制到调试端口
u8 DBG_COPY_TO_S1CK=0;


				//数据发送到服务器接口函数
void server_send_data(u8 *databuff)
{
	extern u8 DBG_IP[4];
	extern u16 DBG_PORT;
	u16 size=strlen((const char *)databuff);
	Write_SOCK_Data_Buffer(0,databuff,size);
//	Write_SOCK_Data_Buffer(0,"_#_",3);
	Write_SOCK_Data_Buffer(0,"\r\n",3);//旧版协议加回车换行
	delay_us(500);//短暂延时保证数据正常发送
	if (DBG_COPY_TO_S1CK==1)
	{
		udp_send(1,DBG_IP,DBG_PORT,"发送：",6);
		udp_send(1,DBG_IP,DBG_PORT,databuff,strlen((const char *)databuff));
		udp_send(1,DBG_IP,DBG_PORT,"\r\n",2);
	}
	else if (DBG_COPY_TO_S1CK==2)
	{
		tcp_send(2,"发送：",6);		
		tcp_send(2,databuff,size);		
		tcp_send(2,"\r\n",2);		
	}
	delay_us(500);//短暂延时保证数据正常发送
}


			//读取服务器发来的数据接口函数
void server_read_data(u8 *databuff)
{
	extern u8 DBG_IP[4];
	extern u16 DBG_PORT;
	Read_SOCK_Data_Buffer(0,databuff);
	u16 size=strlen((const char *)databuff);
	if (DBG_COPY_TO_S1CK==1)
	{
		udp_send(1,DBG_IP,DBG_PORT,"接收：",6);
		udp_send(1,DBG_IP,DBG_PORT,databuff,strlen((const char *)databuff));
		udp_send(1,DBG_IP,DBG_PORT,"\r\n",2);
	}
	else if (DBG_COPY_TO_S1CK==2)
	{
		tcp_send(2,"接收：",6);		
		tcp_send(2,databuff,size);		
		tcp_send(2,"\r\n",2);		
	}
	delay_us(500);//短暂延时保证数据正常发送
	delay_us(500);//短暂延时保证数据正常发送
}




				//接收温控服务器的消息，用于检测是不是json数据，防止意外重启
u8 server_data(void)
{
	u8 *data;
	data=mymalloc(2048);//申请2k内存
	server_read_data( data);
	//udp数据有8个字节包头，tcp数据没有
	if (data[0]=='{')//json数据
		recv_json(data);
	else if (data[0]=='-')
	{
		myfree(data);
		return 1;
	}
	
	 
	myfree(data);
	return 0;
}



			//查找服务器
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
			if (NET_S0_STATE&IR_RECV)//如果收到了消息
			{
				NET_S0_STATE&=~IR_RECV;
				data=mymalloc(2048);//申请2k内存
				server_read_data( data);
				if (data[8]=='{')
				{
					cJSON *root = cJSON_Parse((const char *)(data+8));
					if (strcmp(cJSON_GetObjectItem(root, "cmd")->valuestring,"serverconf")==0)
					{
						//重新设置调试主机目标地址
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


//将数据封装成json再发送到服务器
//0,已发送，！=0，失败
u8	send_json (u8 *msg)
{
	u8 ret=0;
	if (msg[0]!=2) return 1;//不是发送指令，
	if (msg[2]==1) return 1;//离线不发送消息
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
			return 3;//不支持的设备
	}
	return ret;
}




				//json格式的数据调用这个函数解析
void recv_json (u8 *data) 
{
	u8 pcbtype;
	cJSON *root = cJSON_Parse((const char *)data);
	pcbtype=cJSON_GetObjectItem(root, "pcbType")->valueint;
	
	void recv_json_kz (cJSON *root);

	
	
	switch(pcbtype)
	{
		case 11://更新环境上下限

			break;
		case 12://控制设备
			recv_json_kz(root);
			break;
		case 13://电话号码
			break;
		default :
			break;

	}
	cJSON_Delete(root);
}

					//解析并发送控制板指令
			//旧版协议
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
							
										//解析json数据
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
		m_send[0]=0xff;//控制指定地址的设备
	}
	m_send[1]=2;//手动控制设备
	m_send[2]=deviceid>>8;
	m_send[3]=deviceid;
	if (conditionswitch==1)
	{
		m_send[0]=2;//空调
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
	else if (humidity==1)//恒湿一体机
	{
		m_send[0]=6;//
		m_send[4]=1;
		m_send[5]=humiset;//设定的湿度值
	}
	if (cmd==2)//改为自动控制
	{
		m_send[0]=0;
		Lcd_SetHandstate(0);//改变自动状态
	}
	else if (cmd==1)//手动
	{
		Lcd_SetHandstate(1);//改变手动自动状态
	}
									//发送给无线
	send_messeg(RF_MESSEG,m_send);	
}



						//向上位机添加设备，已更改2018.12.8
u8 send_json_adddevice (u8 * msg)

{
	extern u8 NET_S0_STATE;
	if (msg[0]!=4) return 1;//如果不是发送设备上线消息就什么也不做
				//json使用的变量
	
	u16 *cfg=GetCfgData(); 
	u16 i=0;//发送上线设备计数
	u16 j=0;//等待消息回应超时计数
	u16 k=0;//发送上线消息重试次数
	for (i=0;cfg[i];)
	{
		char *out;
		cJSON *root,*js_collect;
		root = cJSON_CreateObject();
		u8 *strbuff=mymalloc(64);
		cJSON_AddNumberToObject(root,"devNum",cfg[i]);//设备编号
		cJSON_AddNumberToObject(root,"centerNum",Get_MyAddr());//集中器编号
		cJSON_AddStringToObject(root,"type","wk");//消息类型温控
		cJSON_AddStringToObject(root,"devType",json_get_devicname(cfg[i+1]&0x00ff));//设备类型
		cJSON_AddStringToObject(root,"cmd","online");//发送的是上线消息

		sprintf (strbuff,"%d",(cfg[i]<<6)+i);//利用集中器id的唯一性产生唯一的确认编码
		cJSON_AddStringToObject(root,"cmdNum",strbuff);//发送消息身份确认编码

		if (cfg[i+1]&0x0800)//离线
		{
			cJSON_AddStringToObject(root,"devState","offline");//		
		}
		else
		{
			if (cfg[i+1]&0x0100)//设备上线状态
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
		server_send_data((u8*)out);//发送数据，自动写入回车换行
		myfree(out);
		for (j=0;j<100;j++)
		{
			delay_ms(5);
			if (NET_S0_STATE&IR_RECV)//如果接收到了消息
			{
				NET_S0_STATE&=~IR_RECV;
				u8 *data;
				data=mymalloc(2048);//申请2k内存
				server_read_data( data);
				cJSON *root = cJSON_Parse((const char *)data);
				if (strcmp(cJSON_GetObjectItem(root, "cmdNum")->valuestring,strbuff)==0)
				{
					if (strcmp(cJSON_GetObjectItem(root, "type")->valuestring,"wk")==0)
					{
						i+=2;//发送成功，发送下一个
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
		myfree(strbuff);//无论收没收到上位机返回，都释放内存
		k++;
		if (k>10)
		{
			break ;
		}
	}
	if (k>10) return -1;
	return 0;
	
}



		//根据类型获取json类型字符串
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


					//执行上位机的命令之后结果返回
void json_return (u8 deviceid,const char * cmdNum,const char *errType)
{
	char *out;
  cJSON *root,*js_collect;
  root = cJSON_CreateObject();
	
	cJSON_AddNumberToObject(root,"devNum",deviceid);//设备编号
	cJSON_AddNumberToObject(root,"centerNum",Get_MyAddr());//集中器编号
	cJSON_AddStringToObject(root,"type","wk");
	cJSON_AddStringToObject(root,"devtype","jzq");
	cJSON_AddStringToObject(root,"cmd","cmdresult");
	cJSON_AddStringToObject(root,"cmdNum",cmdNum);
	cJSON_AddStringToObject(root,"errorType",errType);
	

							//取得json字符串
	out=cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);
	server_send_data((u8*)out);//发送数据，自动写入回车换行
	myfree(out);
	
}




