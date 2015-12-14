/**************** (C) COPYRIGHT 2013 青岛金仕达电子科技有限公司 ****************
* 文 件 名: Query.C
* 创 建 者: Dean
* 描  述  : KB6120C 采样数据 记录/查询/打印
* 最后修改: 2013年6月5日
*********************************** 修订记录 ***********************************
* 版  本: 
* 修订人: 
*******************************************************************************/
#include "USBCopy.h"
#include <stdio.h>
/**
	文件打印
*/
static	BOOL	PromptTest( void ){	return	FALSE; }

extern	char	* const	TableSampler[];

static	void	Print_File_TSP( uint16_t FileNum, struct uFile_TSP_SHI_R24 const * pFile )
{
	CHAR	sbuffer[40];
	struct	tm	t_tm;
	time_t	now;

	while( ! PrinterInit( PromptTest ))
	{
		if( vbCancel == MsgBox( "打印机未联机 !", vbRetryCancel ))
		{
			return;
		}
	}
  sprintf( sbuffer, "\r\n%s型%s\r\n",szTypeIdent[Configure.InstrumentType],szNameIdent[Configure.InstrumentName] );
	PrinterPutString( sbuffer );
	PrinterPutString( "日均采样记录" );
	sprintf( sbuffer, "文件:%3u [第%2u次/共%2u次]", FileNum, pFile->run_loops, pFile->set_loops );	PrinterPutString( sbuffer );

	(void)_localtime_r ( &pFile->sample_begin, &t_tm );
	sprintf( sbuffer, "开始时间:%2d月%2d日 %02d:%02d", t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min );
	PrinterPutString( sbuffer );

	sprintf( sbuffer, "设置采样时间: %02u:%02u", pFile->set_time / 60u, pFile->set_time % 60u );	PrinterPutString( sbuffer );
	sprintf( sbuffer, "设置采样流量: %5.1f L/m", pFile->set_flow * 0.1f );							PrinterPutString( sbuffer );

	sprintf( sbuffer, "累计采样时间: %02u:%02u", pFile->sum_min / 60u, pFile->sum_min % 60u ); 		PrinterPutString( sbuffer );
	sprintf( sbuffer, "采样体积(工): %7.1f L", pFile->vd  );										PrinterPutString( sbuffer );
	sprintf( sbuffer, "采样体积(标): %7.1f L", pFile->vnd );										PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均流量(工): %6.2f L/m", (FP32)pFile->vd  / pFile->sum_min );				PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均流量(标): %6.2f L/m", (FP32)pFile->vnd / pFile->sum_min );				PrinterPutString( sbuffer );

	sprintf( sbuffer, "平均计前温度: %6.2f ℃ ", pFile->sum_tr / pFile->sum_min );					PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均计前压力: %6.2f kPa", pFile->sum_pr / pFile->sum_min );					PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均大气压力: %6.2f kPa", pFile->sum_Ba / pFile->sum_min );					PrinterPutString( sbuffer );

	now = get_Now();
	(void)_localtime_r ( &now, &t_tm );
	sprintf( sbuffer, "  == %4d/%2d/%2d %02d:%02d:%02d ==  ",
	t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec );
	PrinterPutString( sbuffer );

	PrinterPutString( " " );	PrinterPutString( " " );
}

static	void	Print_File_R24_SHI( enum enumSamplerSelect SamplerSelect, uint16_t FileNum, struct uFile_TSP_SHI_R24 const * pFile )
{
	CHAR	sbuffer[40];
	struct	tm	t_tm;
	time_t	now;

	while( ! PrinterInit( PromptTest ))
	{
		if( vbCancel == MsgBox( "打印机未联机 !", vbRetryCancel ))
		{
			return;
		}
	}

	sprintf( sbuffer, "\r\n%s型 %s\r\n",szTypeIdent[Configure.InstrumentType],szNameIdent[Configure.InstrumentName] );
	PrinterPutString( sbuffer );
	PrinterPutString( TableSampler[SamplerSelect] );
	sprintf( sbuffer, "文件:%3u [第%2u次/共%2u次]", FileNum, pFile->run_loops, pFile->set_loops );	PrinterPutString( sbuffer );

	(void)_localtime_r ( &pFile->sample_begin, &t_tm );
	sprintf( sbuffer, "开始时间:%2d月%2d日 %02d:%02d", t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min );
	PrinterPutString( sbuffer );

	sprintf( sbuffer, "设置采样时间:  %02u:%02u",	pFile->set_time / 60u, pFile->set_time % 60u );	PrinterPutString( sbuffer );
	sprintf( sbuffer, "设置采样流量:  %5.3f L/m",	pFile->set_flow * 0.001f );								PrinterPutString( sbuffer );
	sprintf( sbuffer, "累计采样时间:  %02u:%02u",	pFile->sum_min / 60u, pFile->sum_min % 60u );		PrinterPutString( sbuffer );
	sprintf( sbuffer, "采样体积(标):  %7.1fL",		pFile->vnd );											PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均流量(标):  %6.3f L/m",	(FP32)pFile->vnd / pFile->sum_min );				PrinterPutString( sbuffer );
	sprintf( sbuffer, "采样体积(工):  %7.1fL",		pFile->vd );											PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均流量(工):  %6.3f L/m",	(FP32)pFile->vd / pFile->sum_min );				PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均计前温度:  %6.2f ℃",	pFile->sum_tr / pFile->sum_min );					PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均计前压力:  %6.2f kPa",	pFile->sum_pr / pFile->sum_min );					PrinterPutString( sbuffer );
	sprintf( sbuffer, "平均大气压力: %6.2f kPa",	pFile->sum_Ba / pFile->sum_min );						PrinterPutString( sbuffer );	

	now = get_Now();	(void)_localtime_r ( &now, &t_tm );
	sprintf( sbuffer, "  == %4d/%2d/%2d %02d:%02d:%02d ==  ",
		t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec );
	PrinterPutString( sbuffer );

	PrinterPutString( " " );	PrinterPutString( " " );
}


/***/
// static	void	Print_File_AIR( uint16_t FileNum, struct uFile_AIR const * pFile )
// {
// 	CHAR	sbuffer[40];
// 	struct	tm	t_tm;
// 	time_t	now;

// 	while( ! PrinterInit( PromptTest ))
// 	{
// 		if( vbCancel == MsgBox( "打印机未联机 !", vbRetryCancel ))
// 		{
// 			return;
// 		}
// 	}
// 	sprintf( sbuffer, "\r\n%s型%s\r\n",szTypeIdent[Configure.InstrumentType],szNameIdent[Configure.InstrumentName] );
// 	PrinterPutString( sbuffer );
// 	PrinterPutString( "大气采样记录" );
// 	sprintf( sbuffer, "文件:%3u [第%2u次/共%2u次]", FileNum, pFile->run_loops, pFile->set_loops );	PrinterPutString( sbuffer );

// 	(void)_localtime_r ( &pFile->sample_begin, &t_tm );
// 	sprintf( sbuffer, "开始时间:%2d月%2d日 %02d:%02d", t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min );
// 	PrinterPutString( sbuffer );

// 	sprintf( sbuffer, "设置采样时间: %02u:%02u", pFile->set_time / 60u, pFile->set_time % 60u );	PrinterPutString( sbuffer );
// 	sprintf( sbuffer, "设置采样流量: %5.1f L/m", pFile->set_flow * 0.1f );							PrinterPutString( sbuffer );

// 	sprintf( sbuffer, "累计采样时间: %02u:%02u", pFile->sum_min / 60u, pFile->sum_min % 60u ); 		PrinterPutString( sbuffer );
// 	now = get_Now();
// 	(void)_localtime_r ( &now, &t_tm );
// 	sprintf( sbuffer, "  == %4d/%2d/%2d %02d:%02d:%02d ==  ",
// 	t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec );
// 	PrinterPutString( sbuffer );

// 	PrinterPutString( " " );	PrinterPutString( " " );
// }


/************************************************************/
static void FPrintf_TSP(uint16_t FileNum, struct uFile_TSP_SHI_R24  const * pFile)
{
	 static	struct uMenu  const  menu[] =
	{
		{ 0x0201u, "文件输出" },
		{ 0x0C06u, "打印机" },	
		{ 0x1806u, " U盘 " },
	};

	uint8_t	item = 1u;
  BOOL state = FALSE;
	do
	{
		cls();
		Menu_Redraw( menu );
		SamplerTypeShow( 0x010Fu );
     
		item = Menu_Select( menu, item, NULL );
		
		switch( item )
		{
		//	在主菜单下按取消键，显示大气压、恒温箱温度等环境参数
		case 1:
			Print_File_TSP( FileNum, pFile );
			state = TRUE;	
			break;
		case 2:		
			if( USB_PrintInit() )
			{
        USBPrint_TSP( FileNum, pFile );
				state = TRUE;			
			}
			break;
		case enumSelectESC: 
			state = TRUE;
			break;
		case enumSelectXCH: 
			break;
		}
	}while( state != TRUE );
}

/*********************************************************************/
static void FPrintf_R24_SHI( enum enumSamplerSelect SamplerSelect, uint16_t FileNum, struct uFile_TSP_SHI_R24  const * pFile )
{
	  static	struct uMenu  const  menu[] =
	{
		{ 0x0201u, "文件输出" },
		{ 0x0C06u, "打印机" },	
		{ 0x1806u, " U盘 " },
	};

	uint8_t	item = 1u;
  BOOL state = FALSE;
	do
	{
		cls();
		Menu_Redraw( menu );
		SamplerTypeShow( 0x010Fu );
     
		item = Menu_Select( menu, item, NULL );
		
		switch( item )
		{
		//	在主菜单下按取消键，显示大气压、恒温箱温度等环境参数
		case 1:
			Print_File_R24_SHI( SamplerSelect, FileNum, pFile );
			state = TRUE;	
			break;
		case 2:		
			if( USB_PrintInit() )
			{
        USBPrint_R24_SHI( SamplerSelect, FileNum, pFile );
				state = TRUE;			
			}
			break;
		case enumSelectESC: 
			state = TRUE;
			break;
		case enumSelectXCH: 
			break;
		}
	}while( state != TRUE );
}

/***********************************************************************/
/**/
/**
	文件查询
*/
void	Query_File_TSP( void )
{
	uint16_t	FileNum;
	struct	uFile_TSP_SHI_R24 File;
	enum 
	{
		opt_exit,
		opt_tsp_1,   opt_tsp_2,  
		opt_max, opt_min = opt_tsp_1
	};
	uint8_t	option = opt_min;
	uint16_t gray  = Configure.DisplayGray;
	BOOL	graychanged = FALSE;
	FileNum = SampleSet[SP_TSP].FileNum;

	if (( FileNum < 1 ) || ( FileNum > FileNum_Max ))
	{	//	文件全空！主要指 FileNum 是 0 的情况。
		beep();
		MsgBox( " 粉尘文件:[空]", vbOKOnly );
		return;	//	中止 查询
	}
  
	File_Load_TSP_SHI_R24 (SamplerSelect, FileNum, &File );
	
	do {
		cls();	
		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);

		if ( 0u == File.sample_begin )
		{
			Lputs( 0x0102u, "粉尘文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, "[空]" );
		}
		else
		{
			Lputs( 0x0102u, "粉尘文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, NULL );
			switch ( option )
			{
			default:
			case opt_tsp_1:
				Lputs( 0x011Bu, "[1/2]" );
				Lputs( 0x0602u, "开始时间:" );	  ShowDATE( 0x0610u, File.sample_begin );
				Lputs( 0x0A02u, "采样次数:" );		ShowI16U( 0x0A16u, File.run_loops,  						0x0200u, NULL );
				Lputs( 0x0E02u, "设置次数:" );		ShowI16U( 0x0E16u, File.set_loops,   						0x0200u, NULL );
				Lputs( 0x1202u, "温   度:" );		ShowFP32( 0x1215u, File.sum_tr / File.sum_min, 	0x0402u, "℃" );
				Lputs( 0x1602u, "压   力:" );		ShowFP32( 0x1617u, File.sum_pr / File.sum_min, 	0x0402u, "kPa" );
				Lputs( 0x1A02u, "大气压力:" );	  ShowFP32( 0x1A15u, File.sum_Ba / File.sum_min,	0x0402u, "kPa" );
			//	Lputs( 0x0800u, "环境温度:" );	ShowFP32( 0x0811u, File.sum_tr / File.sum_min, 0x0602u, "℃" );
				break;
			case opt_tsp_2:
				Lputs( 0x011Bu, "[2/2]" );
				Lputs( 0x0602u, "工   况:" );		ShowFP32( 0x0617u, File.vd     / File.sum_min, 0x0401u, "L/m" );
				Lputs( 0x0A02u, "标   况:" );		ShowFP32( 0x0A17u, File.vnd    / File.sum_min, 0x0401u, "L/m" );
				Lputs( 0x0E02u, "设   置:" );		ShowI16U( 0x0E15u, File.set_flow,              0x0401u, "L/m" );
				Lputs( 0x1202u, "工况体积:" );		ShowFP32( 0x1213u, File.vd,                    0x0600u, "L" );
				Lputs( 0x1602u, "标况体积:" );		ShowFP32( 0x1613u, File.vnd,                   0x0600u, "L" );
				Lputs( 0x1A02u, "采样时间:" );		ShowTIME( 0x1A16u, File.sum_min  );
			//	Lputs( 0x1800u, "设置时间:" );	ShowTIME( 0x1816u, File.set_time );
				break;
			}
		}		
		switch ( getKey() )
		{
		case K_UP:		//	向前（时间较早的文件）查找
		{
			uint16_t	fname;
			
			if ( FileNum == 1u )
			{
				fname = 1;
				beep();
				MsgBox( "无更多文件!", vbOKOnly );
			}
			else
			{
				fname = FileNum - 1u;			
			}

			File_Load_TSP_SHI_R24 ( SamplerSelect, fname, &File );
			FileNum = fname;
			if ( 0u == File.sample_begin )
			{
				beep();	//	此文件可能无效！
			}
		}	break;

		case K_DOWN:	//	向后（时间较新的文件）查找
		{
			uint16_t	fname;
			
			if ( FileNum == FileNum_Max )
			{
				fname = FileNum_Max;
				beep();
				MsgBox( "文件满！", vbOKOnly );	
				return;
			}
			else
			{
				fname = FileNum + 1u;
			}

			File_Load_TSP_SHI_R24 ( SamplerSelect, fname, &File );			
			if( fname > SampleSet[SP_TSP].FileNum )
			{
				beep();//	到达最后，"后"面无有效数据
				File_Load_TSP_SHI_R24 ( SamplerSelect, FileNum, &File );
			}
			else	
			{
				if ( 0u == File.sample_begin )
					beep();
				FileNum = fname;
			}
		}	break;
  
		case K_OK:
			if ( 0u != File.sample_begin )
			{
				FPrintf_TSP( FileNum, &File );
			}
			
			break;

		case K_RIGHT:
			++option;
			if ( option >= opt_max )
			{
				option = opt_min;
			}
			break;

		case K_LEFT:
			if ( option <= opt_min )
			{
				option = opt_max;
			}
			--option;
			break;

		case K_ESC:
			option = opt_exit;
			break;

		case K_OK_UP:	
			if ( gray < 2200u )
			{
				++gray;
			}
			if( ! releaseKey( K_OK_UP,100 ))
			{
				while( ! releaseKey( K_OK_UP, 1 ))
				{
					++gray;
					DisplaySetGrayVolt( gray * 0.01f );
				}
			}
			graychanged = true;		
			break;
		case K_OK_DOWN:
			if ( gray >  200u )
			{
				--gray;
			}
			if( ! releaseKey( K_OK_DOWN, 100 ))
			{
				while( ! releaseKey( K_OK_DOWN, 1 ))
				{
					--gray;
					DisplaySetGrayVolt( gray * 0.01f );
				}			
			}
			graychanged = true;
			break;

		case K_OK_RIGHT:
			if ( gray < ( 2000u - 50u ))
			{ 
				gray += 100u;
			}
			graychanged = true;
			break;
		case K_OK_LEFT:	
			if ( gray > ( 200 + 20u ))
			{
				gray -= 20u;
			}
			graychanged = true;
			break;
		default:
			break;
		}
		if( graychanged == true )
		{
			DisplaySetGrayVolt( gray * 0.01f );
			Configure.DisplayGray = gray;
			ConfigureSave();
			graychanged = FALSE;
		}		

		
		} while ( opt_exit != option );
}
/************************************************************/
/************************************************************/
/************************************************************/
void	Query_File_SHI_R24( enum enumSamplerSelect SamplerSelect )
{
	struct	uFile_TSP_SHI_R24 File;
	uint16_t	FileNum;
	enum 
	{
		opt_exit,
		opt_A_1,
		opt_A_A1, 
		opt_max, opt_min = 1u
	};
	uint8_t	option = opt_min;
	uint16_t gray  = Configure.DisplayGray;
	BOOL	graychanged = FALSE;

	FileNum = SampleSet[SamplerSelect].FileNum;

	if (( FileNum < 1 ) || ( FileNum > FileNum_Max ))
	{
		beep();
		switch ( SamplerSelect )
		{
// 			case SP_TSP:
// 				MsgBox( " 粉尘文件:[空]", vbOKOnly );
// 				break;
			case SP_R24_A:
				MsgBox( " 日均A文件:[空]", vbOKOnly );
				break;
			case SP_R24_B:
				MsgBox( " 日均B文件:[空]", vbOKOnly );
				break;
			case SP_SHI_C:
				MsgBox( " 时均C文件:[空]", vbOKOnly );
				break;
			case SP_SHI_D:
				MsgBox( " 时均D文件:[空]", vbOKOnly );
				break;
			default:
				break;
		}
		return;	//	中止 查询
	}

	File_Load_TSP_SHI_R24 ( SamplerSelect, FileNum, &File );
	
	do {
		cls();
		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);

		if ( 0u == File.sample_begin )
		{
			
			switch ( SamplerSelect )
			{
// 				case SP_TSP:
// 					Lputs( 0x0102u, "粉尘文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, "[空]" );
// 					break;
				case SP_R24_A:
					Lputs( 0x0102u, "日均A文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, "[空]" );
					break;
				case SP_R24_B:
					Lputs( 0x0102u, "日均B文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, "[空]" );
					break;
				case SP_SHI_C:
					Lputs( 0x0102u, "时均C文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, "[空]" );
					break;
				case SP_SHI_D:
					Lputs( 0x0102u, "时均D文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, "[空]" );
					break;
				default:
					break;
			}
		} 
		else
		{
			switch ( SamplerSelect )
			{
// 				case SP_TSP:
// 					Lputs( 0x0102u, "粉尘文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, NULL );
// 					break;
				case SP_R24_A:
					Lputs( 0x0102u, "日均A文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, NULL );
					break;
				case SP_R24_B:
					Lputs( 0x0102u, "日均B文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, NULL );
					break;
				case SP_SHI_C:
					Lputs( 0x0102u, "时均C文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, NULL );
					break;
				case SP_SHI_D:
					Lputs( 0x0102u, "时均D文件" );	ShowI16U( 0x0114u, FileNum, 0x0300u, NULL );
					break;
				default:
					break;
			}
			switch ( option )
			{
			default:
			case opt_A_1:
				Lputs( 0x011Bu, "[1/2]" );
				Lputs( 0x0602u, "开始时间:" );	ShowDATE( 0x0610u, File.sample_begin );
				Lputs( 0x0C02u, "大气压力:" );	ShowFP32( 0x0C11u, File.sum_Ba / File.sum_min, 0x0602u, "kPa" );
				Lputs( 0x1202u, "采样次数:" );	ShowI16U( 0x1216u, File.run_loops,   0x0200u, NULL );
				Lputs( 0x1802u, "设置次数:" );	ShowI16U( 0x1816u, File.set_loops,   0x0200u, NULL );
				break;		
			case opt_A_A1:
				Lputs( 0x011Bu, "[2/2]" );
				Lputs( 0x0602u, "温 度:" );		ShowFP32( 0x0615u, File.sum_tr/ File.sum_min, 	0x0402u, "℃" );
				Lputs( 0x0A02u, "压 力:" );		ShowFP32( 0x0A15u, File.sum_pr / File.sum_min, 	0x0402u, "kPa" );
				Lputs( 0x0E02u, "标况流量:" );	ShowFP32( 0x0E15u, File.vnd / File.sum_min, 		0x0403u, "L/m" );	
				Lputs( 0x1202u, "标况体积:" );	ShowFP32( 0x1213u, File.vnd,  									0x0602u, "L " );
				Lputs( 0x1602u, "采样时间:" );	ShowTIME( 0x1615u, File.sum_min  );
				Lputs( 0x1A02u, "设置时间:" );	ShowTIME( 0x1A15u, File.set_time );
				break;			
			}
		}
		switch ( getKey())
		{
		case K_UP:		//	向前（时间较早的文件）查找
		{
			uint16_t	fname;
			
			if ( FileNum == 1u )
			{
				fname = 1;
				beep();
				MsgBox( "无更多文件!", vbOKOnly );
			}
			else
			{
				fname = FileNum - 1u;			
			}

			File_Load_TSP_SHI_R24 ( SamplerSelect, fname, &File );
			FileNum = fname;
			if ( 0u == File.sample_begin )
			{
				beep();	//	此文件可能无效！
			}
		}	break;

		case K_DOWN:	//	向后（时间较新的文件）查找
		{
			uint16_t	fname;
			
			if ( FileNum == FileNum_Max )
			{
				fname = FileNum_Max;
				beep();
				MsgBox( "文件满！", vbOKOnly );
				return;				
			}
			else
			{
				fname = FileNum + 1u;
			}

			File_Load_TSP_SHI_R24 ( SamplerSelect, fname, &File );
			if( fname > SampleSet[SamplerSelect].FileNum )
			{
				beep();//	到达最后，"后"面无有效数据
				File_Load_TSP_SHI_R24 ( SamplerSelect, FileNum, &File );
			}
			else	
			{
				if ( 0u == File.sample_begin )
					beep();
				FileNum = fname;
			}
		}	break;
		case K_OK:
			if ( 0u != File.sample_begin )
			{
				FPrintf_R24_SHI( SamplerSelect, FileNum, &File );
			}
			break;

		case K_RIGHT:
			++option;
			if ( option >= opt_max )
			{
				option = opt_min;
			}
			break;

		case K_LEFT:
			if ( option <= opt_min )
			{
				option = opt_max;
			}
			--option;
			break;

		case K_ESC:
			option = opt_exit;
			break;

		case K_OK_UP:	
			if ( gray < 2200u )
			{
				++gray;
			}
			if( ! releaseKey( K_OK_UP,100 ))
			{
				while( ! releaseKey( K_OK_UP, 1 ))
				{
					++gray;
					DisplaySetGrayVolt( gray * 0.01f );
				}
			}
			graychanged = true;		
			break;
		case K_OK_DOWN:
			if ( gray >  200u )
			{
				--gray;
			}
			if( ! releaseKey( K_OK_DOWN, 100 ))
			{
				while( ! releaseKey( K_OK_DOWN, 1 ))
				{
					--gray;
					DisplaySetGrayVolt( gray * 0.01f );
				}			
			}
			graychanged = true;
			break;

		case K_OK_RIGHT:
			if ( gray < ( 2000u - 50u ))
			{ 
				gray += 100u;
			}
			graychanged = true;
			break;
		case K_OK_LEFT:	
			if ( gray > ( 200 + 20u ))
			{
				gray -= 20u;
			}
			graychanged = true;
			break;
		default:
			break;
		}
		if( graychanged == true )
		{
			DisplaySetGrayVolt( gray * 0.01f );
			Configure.DisplayGray = gray;
			ConfigureSave();
			graychanged = FALSE;
		}		
	
		
	} while ( opt_exit != option );
	

}

/********************************** 功能说明 ***********************************
: 开关机日志记录的查询
*******************************************************************************/
extern volatile	uint16_t	PowerLogIndex ;

void	PowerLog_Query( void )
{
	enum
	{
		opt_exit, opt_standard,
		
	};
	uint8_t	option = opt_standard;
	uint16_t	PLindex = 0u;
	uint16_t gray  = Configure.DisplayGray;
	BOOL	graychanged = FALSE;	
	struct	uPowerLog Plog;

	CHAR sbuffer[20] = {0};
	do {
		cls();
		Lputs( 0x0102u, "开关机记录" );
		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);		
		sprintf( sbuffer, "编号:   [%1u/%1u]",PLindex + 1 ,PowerLogIndex + 1 ); 
		Lputs( 0x0802u, sbuffer );
		PowerLogLoad( PLindex, &Plog );	
		Lputs( 0x1002u, "开机:" );	ShowDATE( 0x100Cu, Plog.boot );
		Lputs( 0x1802u, "关机:" );	if ( PLindex ){  ShowDATE( 0x180Cu, Plog.shut ); }
		if( PowerLogIndex < PowerLogIndex_Max )
		{
			switch ( getKey())
			{
			case K_UP:
				if ( PLindex <  PowerLogIndex )
				{
					++PLindex;
				}
				else
				{
					beep();
					MsgBox( "无更多记录!", vbOKOnly );
				}
				break;

			case K_DOWN:
				if ( PLindex > 0u )
				{
					--PLindex;
				}
				break;

			case K_OK:
				//$TODO:	RecPower_Print( Log, index );
			
				if ( ! releaseKey( K_OK, 200u ))
				{
					beep();
					
					if ( vbYes == MsgBox( "清空开关机记录?", vbYesNo | vbDefaultButton2 ))
					{
						PowerLog_Clean();						
						PLindex = 0u;
					}
				}
				break;

			case K_ESC:
				option = opt_exit;
				break;

		case K_OK_UP:	
			if ( gray < 2200u )
			{
				++gray;
			}
			if( ! releaseKey( K_OK_UP,100 ))
			{
				while( ! releaseKey( K_OK_UP, 1 ))
				{
					++gray;
					DisplaySetGrayVolt( gray * 0.01f );
				}
			}
			graychanged = true;		
			break;
		case K_OK_DOWN:
			if ( gray >  200u )
			{
				--gray;
			}
			if( ! releaseKey( K_OK_DOWN, 100 ))
			{
				while( ! releaseKey( K_OK_DOWN, 1 ))
				{
					--gray;
					DisplaySetGrayVolt( gray * 0.01f );
				}			
			}
			graychanged = true;
			break;

		case K_OK_RIGHT:
			if ( gray < ( 2000u - 50u ))
			{ 
				gray += 100u;
			}
			graychanged = true;
			break;
		case K_OK_LEFT:	
			if ( gray > ( 200 + 20u ))
			{
				gray -= 20u;
			}
			graychanged = true;
			break;
		default:
			break;
		}
		if( graychanged == true )
		{
			DisplaySetGrayVolt( gray * 0.01f );
			Configure.DisplayGray = gray;
			ConfigureSave();
			graychanged = FALSE;
		}		

		}
		else
		{
			beep();
			MsgBox("文件满！",vbOKOnly);
			return;
		}
	} while( opt_exit != option );
}

void	menu_SampleQuery( void )	//enum enumSamplerSelect SamplerSelect )
{
	switch ( SamplerSelect )
	{
		case SP_TSP:		Query_File_TSP();	break;
		case SP_R24_A:	Query_File_SHI_R24(SamplerSelect );break;
		case SP_R24_B:	Query_File_SHI_R24(SamplerSelect );break;
		case SP_SHI_C:	Query_File_SHI_R24(SamplerSelect );break;
		case SP_SHI_D:	Query_File_SHI_R24(SamplerSelect );break;
		default:
			break;
	}
}

/********  (C) COPYRIGHT 2013 青岛金仕达电子科技有限公司  **** End Of File ****/







