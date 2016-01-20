#include <os.h>
#include "graph.h"
#include "Chinese.h"
#include "menu.h"

#define CONFIG_PATH "/documents/ndless/nNovel.cfg.tns"

typedef struct
{
	int show_time;
	int show_second;
	int show_page_slide;
	int auto_save;
	int auto_save_time;
	int auto_save_exit;
	char Font_path[50];	
} CONFIG;
CONFIG config;

void load_config()
{
	FILE * cfgdata = fopen( CONFIG_PATH , "rb+" );
	
	if( cfgdata == NULL )
	{
		cfgdata = fopen( CONFIG_PATH , "wb+" );
		fseek( cfgdata , 0 , SEEK_SET );
		config.show_time		= 1 ;
		config.show_second		= 1 ; 
		config.show_page_slide	= 1 ;
		config.auto_save		= 1 ;
		config.auto_save_time	= 20 ;
		config.auto_save_exit	= 1 ;
		strcpy( config.Font_path , "/documents/Fonts/HZK16.nft.tns" );
		fwrite( &config , sizeof(int)*6 + sizeof(char)*50 , 1 , cfgdata );
		fclose( cfgdata );
	}
	else
	{
		fseek( cfgdata , 0 , SEEK_SET );
		fread( &config , sizeof(int)*6 + sizeof(char)*50 , 1 , cfgdata );
		fclose( cfgdata );
	}
}

void setup_config()
{
	FILE * cfgdata = fopen( CONFIG_PATH , "rb+" );
	char menu_title[200],temp[50];
	menu_title[0] = 0 ;
	
	strcat( menu_title , "    ����\n��������          ");

	strcat( menu_title , "\nʱ����ʾ          ");
	if(config.show_time == 1)
		strcat( menu_title , "��" );
	else 
		strcat( menu_title , "��" );
	
	
	strcat( menu_title , "\n������ʾ          ");
	if(config.show_second == 1)
		strcat( menu_title , "��" );
	else 
		strcat( menu_title , "��" );
	
	
	strcat( menu_title , "\n���ط�ҳЧ��      ");
	if(config.show_page_slide == 1)
		strcat( menu_title , "��" );
	else 
		strcat( menu_title , "��" );
	
	
	strcat( menu_title , "\n�Զ�����          ");
	if(config.auto_save == 1)
		strcat( menu_title , "��" );
	else 
		strcat( menu_title , "��" );
	
	
	strcat( menu_title , "\n�Զ�����ѡ��      ");	
	strcat( menu_title , "\n�����ļ�����      ");	
	
	strcat( menu_title , "\n�˳�ʱ�Զ�����    ");
	if(config.auto_save_exit == 1)
		strcat( menu_title , "��\n" );
	else 
		strcat( menu_title , "��\n" );	
	
	int i ;
	i = menu( SCREEN_BASE_ADDRESS , 30 , 30 , menu_title , "\nѡ��һ�������ļ�\n�Ƿ�����Ļ���·���ʾʱ��\n��ʾ��ʱ���Ƿ��������\n��Ч������ʱ���Թر�\n�Ƿ����Զ�����\n�Զ������Ƶ��\n�������ֱ�Ӵ�txt�ļ��Ķ�\n�����˳�ʱ�Ƿ񱣴����\n" );
	switch(i)
	{
		case 1 :
		{
			if(file_browse("/documents/Fonts",".nft.tns",temp))
			{	
				CloseHZK();
				sprintf(config.Font_path,"/documents/Fonts/%s",temp);
				OpenHZK();
			}
			
			break ;
		}
		case 2 : if( config.show_time == 1 )  				config.show_time = 0 ;			else config.show_time = 1 ;			break;
		case 3 : if( config.show_second == 1 )  			config.show_second = 0 ;		else config.show_second = 1 ;		break;
		case 4 : if( config.show_page_slide == 1 )  		config.show_page_slide = 0 ;	else config.show_page_slide = 1 ;	break;
		case 5 : if( config.auto_save == 1 )  				config.auto_save = 0 ;			else config.auto_save = 1 ;			break;
		case 6 :     config.auto_save_time = num_input( SCREEN_BASE_ADDRESS , 50 , 50 , "��ҳ�뱻����ʱ" );			break;
		case 7 : 
		{
			FILE *fp = NULL;
			fp = fopen( "/documents/ndless/ndless.cfg.tns" , "rb+" );
			if( fp == NULL )
			{
				wait_no_key_pressed();
				if(show_msgbox_2b("提示","未找到ndless.cfg.tns\n是否要建立该文件？","是","不，谢谢")==1)
				{
					fp = fopen( "/documents/ndless/ndless.cfg.tns" , "wb+" );
				}
				else
					break;
			}
			int cfgsize = 0;
			fseek( fp , 0 , SEEK_END );
			cfgsize = ftell(fp);
			char *pncfg = NULL;
			pncfg = malloc( cfgsize + 50 );
			if(pncfg==NULL)
			{
				wait_no_key_pressed();
				show_msgbox("错误","内存严重不足，无法继续！");
				fclose( fp );
				break;
			}
			fseek(fp,0,SEEK_SET);
			fread(pncfg,cfgsize,1,fp);
			if( strstr(pncfg,"ext.txt=nNovel") )
			{
				wait_no_key_pressed();
				show_msgbox("提示","文件关联已建立，无需重复操作。");
				fclose( fp );
				free(pncfg);
				break;
			}
			else
			{
				strcat(pncfg,"\r\next.txt=nNovel");
				fseek(fp,0,SEEK_SET);
				fwrite(pncfg,strlen(pncfg),1,fp);
				wait_no_key_pressed();
				show_msgbox("提示","文件关联已建立!");
				fclose( fp );
				free(pncfg);
				break;
			}
		}
		case 8 : if( config.auto_save_exit == 1 )  			config.auto_save_exit = 0 ;		else config.auto_save_exit = 1 ;	break;
		default : break ;
	}
	fseek( cfgdata , 0 , SEEK_SET );
	fwrite( &config , sizeof(int)*6 + sizeof(char)*50 , 1 , cfgdata );
	fclose( cfgdata );
}


