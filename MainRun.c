#include <os.h>
#include "graph.h"
#include "RTC.h"
#include "Chinese.h"
#include "menu.h"
#include "config.h"

#define PAGEMAX 20000
#define MAXNOTDIV 100000

#define FIRST_KEY_WAIT 300

long filesize = 0;
FILE *TXT ;
extern char fname[];
char txtname[50];
int cl_fg = 0 , cl_bg = 15;
int pagepos[PAGEMAX];
char * VRAM;

static unsigned int* p_contrast = (unsigned int*) 0x900F0020;

void load_mark( int * page , int total_page , int type )
{
	int i , j  ;
	int selected;
	char markpath[50];
	char temp[100];
	char ttemp[500];
	char preview[ 5 ][ 45 ];
	int marks[6] = { 1 , 1 , 1 , 1 , 1 , 1 };
	
	for( i = 0 ; i < strlen(txtname) - 8 ; i ++ )
		markpath[i] = txtname[i] ;
	markpath[ i ] = 0;
	strcat( markpath , ".sav" );
	
	FILE *MARK = fopen( markpath , "rb+" );
	if( MARK == NULL )
	{
		MARK = fopen( markpath , "wb+" );
		fseek( MARK , 0 , SEEK_SET );
		fwrite( marks , sizeof(int) , 6 , MARK );
	}
	fseek( MARK , 0 , SEEK_SET );
	fread( marks , sizeof(int) , 6 , MARK );
	if( type == 1) //Auto load
	{
		*page = marks[ 5 ] - 1 ;
		fclose(MARK);
		return;
	}

	for( i = 0 ; i < 5 ; i ++)
	{
		fseek( TXT , pagepos[marks[i]-1] , SEEK_SET );
		fread( preview[i] , 40 , 1 , TXT );
		for( j = 0 ; j < 40 ; j++ )
			if( preview[i][j] == '\n' || preview[i][j] == '\r' ) preview[i][j] = ' ';
		preview[i][40] = 0;
	}
	sprintf(ttemp , "\n%s\n%s\n%s\n%s\n%s\n" , preview[0],preview[1],preview[2],preview[3],preview[4]);

	sprintf( temp , "选择书签来读取\n1.   %4d/%4d\n2.   %4d/%4d\n3.   %4d/%4d\n4.   %4d/%4d\n5.   %4d/%4d\n" ,marks[0],total_page,marks[1],total_page,marks[2],total_page,marks[3],total_page,marks[4],total_page);

	selected = menu( SCREEN_BASE_ADDRESS , 30 , 20 , temp , ttemp );
	if( selected == 0 )
	{
		fclose(MARK);
		return ;
	}
	else
	{
		*page = marks[ selected-1 ] - 1 ;
		fclose(MARK);
		return;
	}
}
void save_mark( int page , int total_page ,int type )
{
	int marks[6] = { 1 , 1 , 1 , 1 , 1 , 1 } , i , j ;
	char markpath[50];
	for( i = 0 ; i < strlen(txtname) - 8 ; i ++ )
		markpath[i] = txtname[i] ;
	markpath[ i ] = 0;
	strcat( markpath , ".sav." );

	
	FILE *MARK = fopen( markpath , "rb+" );
	if( MARK == NULL )
	{
		MARK = fopen( markpath , "wb+" );
		fseek( MARK , 0 , SEEK_SET );
		fwrite( marks , sizeof(int) , 6 , MARK );
		fclose(MARK);
		return;
	}
	
	int selected;
	char temp[100];
	char ttemp[500];
	char preview[ 5 ][ 45 ];
	
	fseek( MARK , 0 , SEEK_SET );
	fread( marks , sizeof(int) , 6 , MARK );
	if( type == 1 )//Auto Save Mode
	{
		marks[ 5 ] = page + 1;
		fseek( MARK , 0 , SEEK_SET );
		fwrite( marks , sizeof(int) , 6 , MARK );
		fclose(MARK);
		return;
	}

	for( i = 0 ; i < 5 ; i ++)
	{
		fseek( TXT , pagepos[marks[i]-1] , SEEK_SET );
		fread( preview[i] , 40 , 1 , TXT );
		for( j = 0 ; j < 40 ; j++ )
			if( preview[i][j] == '\n' || preview[i][j] == '\r' ) preview[i][j] = ' ';
		preview[i][40] = 0;
	}
	sprintf(ttemp , "\n%s\n%s\n%s\n%s\n%s\n" , preview[0],preview[1],preview[2],preview[3],preview[4]);

	sprintf( temp , "选择书签来保存\n1.   %4d/%4d\n2.   %4d/%4d\n3.   %4d/%4d\n4.   %4d/%4d\n5.   %4d/%4d\n" ,marks[0],total_page,marks[1],total_page,marks[2],total_page,marks[3],total_page,marks[4],total_page);

	selected = menu( SCREEN_BASE_ADDRESS, 30 , 20 , temp , ttemp );
	if( selected == 0 )
	{
		fclose(MARK);
		return ;
	}
	else
	{
		marks[ selected-1 ] = page + 1 ;
		fseek( MARK , 0 , SEEK_SET );
		fwrite( marks , sizeof(int) , 6 , MARK );
		fclose(MARK);
		return;
	}
}

int divid()
{
	char divpath[50];
	int x = 0 , y = 0 , page = 1 , pos = 0 , i ;
	pagepos[0] = 0 ;
	char c;
	
	for( i = 0 ; i < strlen(txtname) - 8 ; i ++ )
		divpath[i] = txtname[i] ;
	divpath[ i ] = 0;
	strcat( divpath , ".div" );

	
	FILE *DIV = fopen( divpath , "rb" );
	if( filesize > MAXNOTDIV )
	{
		if( DIV == NULL )
			DIV = fopen( divpath , "wb+" );
		else
		{
			fread( pagepos , sizeof(int) , PAGEMAX , DIV );
			page = pagepos[ 0 ];
			pagepos[ 0 ] = 0;
			fclose( DIV );
			return page -1 ;
		}
		clrscr();
		AllFill_VRAM(VRAM,cl_bg);
		PrintChStr(VRAM,0,0,"首次打开文本文件会进行分页，若文件较大可能需要较长时间。分页完成后将生成一个被隐藏的分页结果。你可以在连接软件上删除它，也可以在阅读时使用菜单中的功能来删除它。",cl_fg , cl_bg);
		PutDisp_DDVRAM( VRAM );
	}
	else
	{
		clrscr();
		AllFill_VRAM(VRAM,cl_bg);
		PrintChStr(VRAM,0,0,"请稍等......",cl_fg , cl_bg);
		PutDisp_DDVRAM( VRAM );
	}
/*	while( ( !feof( TXT ) ) || page >= PAGEMAX -1 )
	{
		if( x > 39 )
		{
			x = 0 ; y ++ ;
		}
		if( y > 13 )
		{
			x = y = 0 ;
			pagepos[ page ++ ] = pos;
		}
		fread(&c , 1 , 1 , TXT );pos++;
		if( c == '\r' )
		{	y ++ ; x = 0 ; fread(&c , 1 , 1 , TXT );pos++; continue; }
		if( !( c&0x80 ) )
		{
			if(x>19)
			{	x = 0 ; y ++ ;}
			x ++ ;
			continue;
		}
		fread(&c , 1 , 1 , TXT );pos++;
		x+=2;
	}*/
	fseek( TXT , 0 , SEEK_SET );
	while( ( !feof( TXT ) ) || page >= PAGEMAX -1 )
	{
		if(x>40)
		{
			x = 0 ; y++ ;
		}
		if( y > 13 )
		{
			x = y = 0 ;
			pagepos[ page ++ ] = pos;
			continue;
		}
		fread(&c , 1 , 1 , TXT );pos++;
		if( !(c&0x80) )
		{
			if( c == '\r' || c == '\n'  )
			{
				y ++ ; x = 0 ; fread(&c , 1 , 1 , TXT );pos++;
				continue;
			}
			if(x > 40)
			{
				y ++ ; x = 0 ;
			}
			x++;
			continue;
		}
		if(x + 1 >= 40)
		{
			if( y == 13 )
			{
				fseek(TXT , -1 , SEEK_CUR);
				pos--;
				y ++;
				continue;
			}
			x = 0 ;
			y ++;
		}
		fread(&c , 1 , 1 , TXT );pos++;
		x+=2;
	}
	
	
	pagepos[ page ] = pos ;
	pagepos[ 0 ] = page;
	if(filesize > MAXNOTDIV)
	{
		fwrite( pagepos , sizeof(int) , PAGEMAX , DIV );
		fclose( DIV );
		PrintChStr(VRAM,0,6*16 , "分页顺利完成，任意键以继续" ,cl_fg , cl_bg);
		PutDisp_DDVRAM( VRAM );
		wait_key_pressed();
	}

	pagepos[ 0 ] = 0;
	return page -1;
}


int Read()
{
	VRAM   = init_VRAM();			AllClr_VRAM( VRAM   );
	char * VRAM_A = init_VRAM();	AllClr_VRAM( VRAM_A );
	char * VRAM_B = init_VRAM();	AllClr_VRAM( VRAM_B );
	int total_page , page = 0 , i ,last_key_pressed=0;
	char buff[1000] , statue_bar[100];
	int auto_down = 0 , _auto = 0 , mark = 1 ;
	int user_contrast = *p_contrast;
	long auto_time = 0 ;
	int msg_utf8 = 0;
	auto_down = _auto = 0 ;
	strcpy( txtname , fname );
	TXT = fopen( txtname , "rb" ) ;
	fseek(TXT , 0 , SEEK_END);
	filesize = ftell( TXT );
	
	total_page = divid();
	
	memset( buff , 0x0 , 1000 );
	
	load_mark( &page , total_page , 1 );

	fseek( TXT , pagepos[page] , SEEK_SET );
	fread( buff , pagepos[ page+1 ] - pagepos[ page ] , 1 , TXT );
	AllFill_VRAM(VRAM_A,cl_bg);
	PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );

	PutDisp_DDVRAM( VRAM_A );
	auto_down = 0 ;
	while( 1 )
	{
		if(isKeyPressed(KEY_NSPIRE_SCRATCHPAD))
		{
			wait_no_key_pressed();
			if(msg_utf8==0)
				i = show_msgbox_3b( "鎻愮ず" , "璋冪敤API蹇呴』淇濊瘉鏂囨湰涓篣TF-8缂栫爜锛屼篃灏辨槸姝ゆ椂鐢╪Novel鏃犳硶姝ｅ父闃呰锛岃缁х画鍚楋紵" , "纭畾" , "鍙栨秷" ,"纭畾涓斾笉鍐嶆彁绀�" );
			else
				i = 1;
			if( i == 3 )
			{
				i = 1;
				msg_utf8 = 1;
			}
			if( i == 1 )
			{
				wait_no_key_pressed();
				show_msgbox( "鏈〉鍐呭锛�" , buff );
			}
		}
		if(isKeyPressed(KEY_NSPIRE_MULTIPLY))
		{
			if( is_cx && (*p_contrast)< 225)
				(*p_contrast)++;
			else if( (!is_cx) && (*p_contrast) < 0xc0 )
				(*p_contrast)++;
		}
		else if(isKeyPressed(KEY_NSPIRE_DIVIDE))
		{
			if( is_cx && (*p_contrast)> 1 )
				(*p_contrast)--;
			else if( (!is_cx) && (*p_contrast) >50 )
				(*p_contrast)--;
		}
		else if(isKeyPressed(KEY_NSPIRE_TAB))
		{
			i = num_input(VRAM , 30 , 30 , "  输入页码" );
			if( i == -1) continue;
			else if( i == 0 ){ page = 0;}
			else if( i > total_page){ page = total_page;}
			else page = i - 1;
			memset( buff , 0x0 , 1000 );
			fseek( TXT , pagepos[page] , SEEK_SET );
			fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
			AllFill_VRAM(VRAM_A,cl_bg);
			PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
			PutDisp_DDVRAM( VRAM_A );
			wait_no_key_pressed();
		}
		else if(isKeyPressed(KEY_NSPIRE_DOC))
		{
			save_mark( page , total_page , 0 );
			wait_no_key_pressed();
		}
		else if(isKeyPressed(KEY_NSPIRE_MENU))
		{
			load_mark( &page , total_page , 0 );
			memset( buff , 0x0 , 1000 );
			fseek( TXT , pagepos[page] , SEEK_SET );
			fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
			AllFill_VRAM(VRAM_B,cl_bg);
			PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
			PutDisp_DDVRAM( VRAM_A );
			wait_no_key_pressed();
		}
		else if(isKeyPressed(KEY_NSPIRE_SHIFT))
		{
			if(cl_bg == 15)
			{
				cl_fg = 15;
				cl_bg = 0;
			}
			else
			{
				cl_fg = 0;
				cl_bg = 15;
			}
			memset( buff , 0x0 , 1000 );
			fseek( TXT , pagepos[page] , SEEK_SET );
			fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
			AllFill_VRAM(VRAM_A,cl_bg);
			PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
			PutDisp_DDVRAM( VRAM_A );
			wait_no_key_pressed();
		}
		
		if((*p_RTC - auto_time)%(_auto) != 0)
			mark = 0;
		if( ( isKeyPressed(KEY_NSPIRE_DOWN ) || isKeyPressed(KEY_NSPIRE_MINUS )|| auto_down == 1 )&& page < total_page )
		{
			auto_down = 0;
			page++;
			memset( buff , 0x0 , 1000 );
			fseek( TXT , pagepos[page] , SEEK_SET );
			fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
			if( cl_bg != 15 )
				AllFill_VRAM(VRAM_B,cl_bg);
			else
				AllClr_VRAM(VRAM_B);
			PrintChStr( VRAM_B , 0 , 0 , buff , cl_fg , cl_bg );
			if( config.show_page_slide == 1 )
				slide_down(statue_bar , VRAM_A , VRAM_B , 1 );
			else
			{
				Fill_Rect_VRAM( VRAM_B , 0 , 224 , 320 , 240 , 0 );
				PrintChStr( VRAM_B , 0 , 224 , statue_bar , 15 , 0 );
				PutDisp_DDVRAM(VRAM_B);
				for( i=0 ; i<=FIRST_KEY_WAIT ; i++ )
				{
					if((!isKeyPressed(KEY_NSPIRE_DOWN )|| isKeyPressed(KEY_NSPIRE_MINUS )))
						break;
					if(last_key_pressed)
						break;
					sleep(1);
					if( i == FIRST_KEY_WAIT )
						last_key_pressed = 1;
				}
			}
			memcpy(VRAM_A,VRAM_B,SCREEN_BYTES_SIZE);
		}
		else if( isKeyPressed(KEY_NSPIRE_UP )|| isKeyPressed(KEY_NSPIRE_PLUS ) )
		{
			if( page == 0 )
				continue;
			page --;
			memset( buff , 0x0 , 1000 );
			fseek( TXT , pagepos[page] , SEEK_SET );
			fread( buff , pagepos[ page +1 ] - pagepos[ page ], 1 , TXT );
			AllFill_VRAM(VRAM_B,cl_bg);
			PrintChStr( VRAM_B , 0 , 0 , buff , cl_fg , cl_bg );
			if( config.show_page_slide == 1 )
				slide_up( statue_bar , VRAM_B , VRAM_A , 1 );
			else
			{
				Fill_Rect_VRAM( VRAM_B , 0 , 224 , 320 , 240 , 0 );
				PrintChStr( VRAM_B , 0 , 224 , statue_bar , 15 , 0 );
				PutDisp_DDVRAM(VRAM_B);
				for( i=0 ; i<=FIRST_KEY_WAIT ; i++ )
				{
					if((!isKeyPressed(KEY_NSPIRE_UP )|| isKeyPressed(KEY_NSPIRE_PLUS )))
						break;
					if(last_key_pressed)
						break;
					sleep(1);
					if( i == FIRST_KEY_WAIT )
						last_key_pressed = 1;
				}
			}
			memcpy(VRAM_A,VRAM_B,SCREEN_BYTES_SIZE);
		}
		else
			last_key_pressed = 0;
		memcpy(VRAM,VRAM_A,SCREEN_BYTES_SIZE);
		Fill_Rect_VRAM( VRAM , 0 , 224 , 320 , 240 , 0 );
		if( config.show_time )
		{
			update_time();
			if( config.show_second )
				sprintf(statue_bar , " %d%d:%d%d:%d%d     %5d/%5d   %4s", time.hour/10 , time.hour%10 , time.minute/10, time.minute%10 , time.second/10 , time.second%10 , page +1 , total_page +1 , _auto == 0?"":"Auto");
			else
				sprintf(statue_bar , " %d%d:%d%d        %5d/%5d   %4s", time.hour/10 , time.hour%10 , time.minute/10, time.minute%10 , page +1 , total_page +1 ,_auto == 0?"":"Auto");
		}
		else 
		{
			sprintf(statue_bar , "              %5d/%5d", page +1 , total_page +1 );
		}
		PrintChStr( VRAM , 0 , 224 , statue_bar , 15 , 0 );
//		DrawMiniString_VRAM(VRAM , 0 , 228 , statue_bar , 15 , 0 );
		PutDisp_DDVRAM(VRAM);
			
		if( ! config.show_time )
		{
			if( !(isKeyPressed(KEY_NSPIRE_UP)||isKeyPressed(KEY_NSPIRE_DOWN)) )
				wait_key_pressed();
		}
		
		if( isKeyPressed(KEY_NSPIRE_ESC))
		{
			if(isKeyPressed(KEY_NSPIRE_CTRL))
				goto exit;
			switch(menu(VRAM,10,10,"    菜单\n页码跳转(Tab)\n自动翻页\n读取书签(menu)\n保存书签(doc)\n设置\n夜间模式(shift)\n清空书签分页并退出\n调用API查看本页(便签本)\n退出(ctrl+esc)\n","\n从现在位置跳转到输入位置\n启动/关闭自动翻页\n从书签文件中读取进度\n向书签文件中写入进度\n查看和修改设置\n进入夜间模式(反色)\n可清空生成的所有书签和分页的结果文件\n调用MsgBox来显示本页内容\n退出程序\n") )
			{
				case 1://页码跳转
				{
					i = num_input(SCREEN_BASE_ADDRESS , 30 , 30 , "  输入页码" );
					if( i == -1) break;
					else if( i == 0 ){ page = 0;}
					else if( i > total_page){ page = total_page;}
					else page = i - 1;
					memset( buff , 0x0 , 1000 );
					fseek( TXT , pagepos[page] , SEEK_SET );
					fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
					AllFill_VRAM(VRAM_A,cl_bg);
					PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
					PutDisp_DDVRAM( VRAM_A );
					break;
				}
				case 2://自动翻页
				{
					if( _auto != 0 )
					{
						_auto = 0 ;
						mark = 1;
						break;
					}
					_auto = num_input( SCREEN_BASE_ADDRESS , 30 , 30 , "输入秒数" );
					if(_auto == -1)
						_auto = 0;
					auto_time = *p_RTC;
					
					break;
				}
				case 3: //读取书签
				{
					load_mark( &page , total_page , 0 );
					memset( buff , 0x0 , 1000 );
					fseek( TXT , pagepos[page] , SEEK_SET );
					fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
					AllFill_VRAM(VRAM_B,cl_bg);
					PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
					PutDisp_DDVRAM( VRAM_A );
					break;
				}
				case 4: save_mark( page , total_page , 0 );break;//保存书签
				case 5:	//设置
				{
					setup_config();
					memset( buff , 0x0 , 1000 );
					fseek( TXT , pagepos[page] , SEEK_SET );
					fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
					AllFill_VRAM(VRAM_A,cl_bg);
					PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
					PutDisp_DDVRAM( VRAM_A );
					break;
				}
				case 6: //反色
				{
					if(cl_bg == 15)
					{
						cl_fg = 15;
						cl_bg = 0;
					}
					else
					{
						cl_fg = 0;
						cl_bg = 15;
					}
					memset( buff , 0x0 , 1000 );
					fseek( TXT , pagepos[page] , SEEK_SET );
					fread( buff , pagepos[ page +1 ] - pagepos[ page ] , 1 , TXT );
					AllFill_VRAM(VRAM_A,cl_bg);
					PrintChStr( VRAM_A , 0 , 0 , buff , cl_fg , cl_bg );
					PutDisp_DDVRAM( VRAM_A );
					break;
				}
				case 7: //清空书签
				{
					char path[ 50 ];
					for( i = 0 ; i < strlen(txtname) - 8 ; i ++ )
						path[i] = txtname[i] ;
					path[ i ] = 0;
					strcat( path , ".div" );
					remove(path);
					for( i = 0 ; i < strlen(txtname) - 8 ; i ++ )
						path[i] = txtname[i] ;
					path[ i ] = 0;
					strcat( path , ".sav" );
					remove(path);
					goto exit2;
				}
				case 8://API
				{
					wait_no_key_pressed();
					if(msg_utf8==0)
						i = show_msgbox_3b( "鎻愮ず" , "璋冪敤API蹇呴』淇濊瘉鏂囨湰涓篣TF-8缂栫爜锛屼篃灏辨槸姝ゆ椂鐢╪Novel鏃犳硶姝ｅ父闃呰锛岃缁х画鍚楋紵" , "纭畾" , "鍙栨秷" ,"纭畾涓斾笉鍐嶆彁绀�" );
					else
						i = 1;
					if( i == 3 )
					{
						i = 1;
						msg_utf8 = 1;
					}
					if( i == 1 )
					{
						wait_no_key_pressed();
						show_msgbox( "鏈〉鍐呭锛�" , buff );
						break;
					}
					else
						break;
				}
				case 9: goto exit;//退出
				default :wait_no_key_pressed();break;
			}
			wait_no_key_pressed();
		}
		if( config.auto_save )
			if( page%config.auto_save_time==0 )
				save_mark( page , total_page , 1 );
		if( (*p_RTC - auto_time)%(_auto) == 0 && mark == 0)
		{
			mark = 1 ;
			auto_down = 1 ;
			auto_time = *p_RTC;
		}
	}
	exit:
	save_mark( page , total_page , 1 );
	exit2:
	* p_contrast = user_contrast;
	fclose( TXT );
	close_VRAM( VRAM_A );close_VRAM( VRAM_B );
	close_VRAM( VRAM );
	return 0;
}
