#include <string.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include "devSignal.h"
#include "sysglobal.h"
#include "updateService.h"
#include "param.h"
#include "sysparam.h"
#include "gsm.h"
#include "modules.h"
#include "printer.h"
#include "sand_network.h"

static unsigned short ucOSCurrAppID;


#define DRIVER_USED       _IOW('K',6,int)


/* 设置应用程序id，在多应用管理系统中每次调度前都对其进行设置 */
int  set_app_id(unsigned short appid)
{
    ucOSCurrAppID = appid;
}


SPARAM Os_readParam(void)
{
	SYS_SETTING setting;
	SPARAM _p;
	
	memset(&_p,0,sizeof(SPARAM));
	
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_SETTING");
 	strcpy(setting.key,"MACHINE_SERIAL_NO");
 	PARAM_setting_get(&setting);
	strncpy(_p.aucSerialNo,setting.value,sizeof(_p.aucSerialNo)-1);
	
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_SETTING");
 	strcpy(setting.key,"PINPAD_SERIAL_NO");
 	PARAM_setting_get(&setting);
	strncpy(_p.aucPADSerialNo,setting.value,sizeof(_p.aucPADSerialNo)-1);
	
 	return _p;
}

void SYSGLOBAL_sys_poweron(void)
{
	SYS_SETTING setting;
	int b=0;
	
	//判断wifi类型
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"WIFI_SETTING");
 	strcpy(setting.key,"WIFITYPE");
	//有文件WIFI_DEVICE则表示是老机器,带HUB
	if(access(WIFI_DEVICE,F_OK) == 0)
		b = 2;
	else
		b = 1;
	memset(setting.value,0,sizeof(setting.value));
	sprintf(setting.value,"%d",b);
	PARAM_setting_set(&setting);

	//初始化屏幕亮度
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"SCREEN_SETTING");
 	strcpy(setting.key,"BRIGHTNESS");
 	PARAM_setting_get(&setting);	
 	b=atoi(setting.value);
 	Os__lcd_bright(b);
 	
 	//初始化wifi拨号状态
 	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_ACTIVE");
 	strcpy(setting.key,"WIFI_OPEN");
 	PARAM_setting_get(&setting);
 	b=atoi(setting.value);
 	modules_wifi_active(b);
 	
 	
 	//初始化gps拨号状态
 	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_ACTIVE");
 	strcpy(setting.key,"GPS_OPEN");
 	PARAM_setting_get(&setting);
 	b=atoi(setting.value);
 	modules_gps_active(b);
 	
 	//初始化打印机打印深浅
 	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"PRINTER_SETTING");
 	strcpy(setting.key,"PRINT_LEVEL");
 	PARAM_setting_get(&setting);
 	b=atoi(setting.value);
 	PRINTER_set_printlevel(b);
 	
 	//每次开机时设置SMT状态为关闭(0)
 	modules_smt_active(0);
//	timer_init();
//	lcd_init();
	

}

/*
#define  GPRS_PWR	(1<<0)
#define  GPS_PWR	(1<<1)
#define  BAR_PWR	(1<<2)
#define  WIFI_PWR	(1<<3)
#define  LED1_PWR	(1<<4)
#define  LED2_PWR	(1<<5)
#define  LED3_PWR	(1<<6)
#define  LED4_PWR	(1<<7)
*/
void SYSGLOBAL_sys_suspend(void)
{
	int fd_m,ret=0;
	int wifi_open=0, gps_open=0, wifi_suspend_model = 0;
	unsigned int c;
	int brightValue = 0;
	SYS_SETTING setting;
	
	Os__lcd_bright(0);
	
	//get bright value
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"SCREEN_SETTING");
 	strcpy(setting.key,"BRIGHTNESS");
 	PARAM_setting_get(&setting);
 	brightValue = atoi(setting.value);

	//wifi
	ret = Os__get_device_signal(DEV_WIFI);
	if(ret==0)
		wifi_open=0;
	else
	{
		wifi_open=1;
		
		memset(&setting,0,sizeof(SYS_SETTING));
	 	strcpy(setting.section,"WIFI_SETTING");
	 	strcpy(setting.key,"WIFISUSPENDTYPE");
	 	PARAM_setting_get(&setting);
		wifi_suspend_model = atoi(setting.value);
	}
	
	
	//gps	
	ret = Os__get_device_signal(DEV_GPS);
	if(ret==0)		
		gps_open=0;
	else
		gps_open=1;	
  
  	c=(gps_open<<1)|(1<<0);
  	fd_m = open(MODULE_DEVICE, O_RDWR);
    ioctl(fd_m,DRIVER_USED,&c);
    
    if(fd_m >=0)
   		close(fd_m);

   	//gsm
	while(1)
    {
    	ret=access("/var/lock/LCK..ttymxc1",F_OK);
    	if(ret==0)
    	{
    		if (!GSM__dial_check(1)) 
    		{
    			system("/etc/ppp/ppp-off");
    			Os__xdelay(100);
			}    		
    	}
    	else
    		break;   	
    	
    	Os__xdelay(50);
    }
    
    
    if(wifi_open == 1 && wifi_suspend_model == 1)
    {
    	system("/etc/usbwifi/stop.sh");
	    while(1)
	    {
	    	ret=Os__get_device_signal(DEV_WIFI);
	    	
	    	if(ret==0)
	    	{
	    		break;	
	    	}
	    		
	    	Os__xdelay(50);		
	    }
	}

	
    printf("API SUSPEND\n");
 
    //suspend system
    system("echo mem > /sys/power/state");
   
     
    /**********************************/
    /*	休眠起来后					  */ 
    /**********************************/ 
    
    printf("API AWAKE\n"); 

 	Os__lcd_bright(brightValue);
	Os__beep();

    //初始化摄像头
	Os__camera_init(1, 1);
    Os__camera_close();
}

void SYSGLOBAL_sys_awake(void)
{
	int wifi_open = 0, wifi_suspend_model = 0, ret = 0;
	SYS_SETTING setting;
	
	// 获取wifi状态
	memset(&setting,0,sizeof(SYS_SETTING));
 	strcpy(setting.section,"MODULE_ACTIVE");
 	strcpy(setting.key,"WIFI_OPEN");
 	PARAM_setting_get(&setting);
	
	wifi_open = atoi(setting.value);

	if(wifi_open == 1)
	{
		memset(&setting,0,sizeof(SYS_SETTING));
	 	strcpy(setting.section,"WIFI_SETTING");
	 	strcpy(setting.key,"WIFISUSPENDTYPE");
	 	PARAM_setting_get(&setting);
		wifi_suspend_model = atoi(setting.value);
		
		if(wifi_suspend_model == 1)
    		system("/etc/usbwifi/start.sh &");
    	else
    		system("/etc/usbwifi/awake.sh &");
    	//WIFI休眠唤醒后添加pppd路由,防止wifi唤醒后路由添加不正确
    	ret = Os__get_device_signal(DEV_GPRS_SIGNALL);
    	if(ret >= 2 && ret != 9)
    		activeDev(DEV_COMM_GSM);
	}
	
}

void Os__sys_suspend(void)
{
	SYSGLOBAL_sys_suspend();
}

void Os__sys_awake(void)
{
	SYSGLOBAL_sys_awake();
}

/*
 * Add by xiangliu for setting language 20130617
 */
int SYSGLOBAL_set_language()
{
	int lang = 0;
	SYS_SETTING setting;
	
	memset(&setting, 0, sizeof(SYS_SETTING));
 	strcpy(setting.section, "LANGUAGE_SETTING");
 	strcpy(setting.key, "LANGUAGE");
 	PARAM_setting_get(&setting);
	
	lang = atoi(setting.value);
	switch (lang)
	{
		case 1: //1-CHINESE
			system("cp -rf /daemon/qss/lang/zh/*.qm /daemon/qss/");
			break;

		case 2: //2-ENGLISH
			system("rm -rf /daemon/qss/*.qm");
			break;

		default:
			return -1;
	}
	return 0;
}	