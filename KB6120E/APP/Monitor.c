/**************** (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司 ****************
* 文 件 名: Monitor.C
* 创 建 者: KB-6120E
* 描  述  : 监控主程序，显示采样过程中的采样状态
* 最后修改: 2013年12月17日
*********************************** 修订记录 ***********************************
* 版  本:
* 修订人:
*******************************************************************************/
#include "AppDEF.H"
#include "BMP.H"
/*
采样过程中需要显示的状态分成以下几部分
0.对应采样器名称
1.时间控制状态，包括：次数，运行时间、剩余时间、当前工作状态是采样还是暂停还是故障等等
2.工况状态：包括温度、压力、大气压力、流量（工、标），电机输出信息等
3.统计信息：包括累计时间、累积体积（工、标）等
4.其他信息：如 恒温箱温度及运行状态、环境温度、大气压力 等

原则上，监控程序只显示、不控制。所显示的数据，经全局变量传递，由相应控制程序主动更新。
*/





/********************************** 功能说明 ***********************************
*  显示电池状态
*******************************************************************************/
FP32	get_Bat_Voltage( void );
FP32	get_Bat_Current( void );
bool	get_Bat_Charging( void );


void	show_battery( uint16_t yx )
{
	static const char * BatteryIdentify[]={"㈠⑴","㈠⑴","㈡⑵","㈢⑶","㈣⑷","㈤⑸"};
	
	#define	BatVolStateFULL	5u
	static	uint8_t	BatVolState = BatVolStateFULL;

	uint8_t i;
	bool isCharging;

	FP32 inVoltage = get_Bat_Voltage();
	if( Configure.Battery_SW == TRUE )
	{
    if ( inVoltage < 1.0f )
		{
			return;	//	电池电压异常，认为电池没装，或者没有电池。
		}
		
		isCharging = get_Bat_Charging();

		if ( ! isCharging )
		{	//	充满状态 或 放电状态，显示电池分档
			FP32	voltage = inVoltage * 0.2f; // 5 节电池
			uint8_t NewState;

			if     ( voltage >= 3.20f )  { NewState = 5; }
			else if( voltage >= 3.10f )  { NewState = 4; }
			else if( voltage >= 3.00f )  { NewState = 3; }
			else if( voltage >= 2.95f )  { NewState = 2; }
			else if( voltage >= 2.85f )  { NewState = 1; }
			else						             { NewState = 0; } // Battery Empty.

			if( NewState < BatVolState )
			{
				BatVolState = NewState;
			}
			i = BatVolState;
		}
		else
		{	//	充电状态，（闪烁）显示充电符号。

			BatVolState = BatVolStateFULL;
			
			i = get_Now() % BatVolStateFULL + 1;
			}
		
		Lputs( yx, BatteryIdentify[i] );
		}
	
}

/********************************** 功能说明 ***********************************
*  显示环境参数：大气压、恒温箱 或 加热器 状态
*******************************************************************************/
void	show_env_state( void )
{		switch ( Configure.Mothed_Ba )
	{
	case enumUserInput:	Lputs( 0x0910u, "大气压=" );	break;
	case enumMeasureBa:	Lputs( 0x0910u, "大气压:" );	break;
	}
	ShowFP32( 0x0C14u, get_Ba(), 0x0602u, "kPa" );

	//	如果配有恒温箱
	switch ( Configure.HeaterType )
	{
	case enumHCBoxOnly :
		{
			FP32	OutValue = get_HCBoxOutput();
			FP32	RunTemp  = get_HCBoxTemp();
			Lputs( 0x0F10u, "恒温箱:" );	ShowFP32( 0x1214u, RunTemp, 0x0501u, "℃" );
			if      ( OutValue > 0.0f )
			{
				Lputs( 0x1510u, "加 热: " );	ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
			}
			else if ( OutValue < 0.0f )
			{
				Lputs( 0x1510u, "制 冷: " );	ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
			}
			else	//	OutValue == 0.0f
			{			
				Lputs( 0x1510u, "输 出:" );Lputs( 0x1814u," [关闭]    " );
			}
		}
		break;
	case enumHeaterOnly:
		{
			FP32	OutValue = get_HeaterOutput();
			FP32	RunTemp  = get_HeaterTemp();
			uint8_t state = Configure.Heater_SW;
			Lputs( 0x0F10u, "加热器:" );	ShowFP32( 0x1214u, RunTemp, 0x0501u, "℃" );
      if( state )
			{
       	if      ( OutValue > 0.0f )
				{
					Lputs( 0x1510u, "输 出:   " );	ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
				}
				else
				{			
					Lputs( 0x1510u, "输 出:");	Lputs( 0x1814u," [关闭]    " );
				}
			}
			else
			{
				Lputs( 0x1510u, "输 出:");	Lputs( 0x1814u," [停用]    " );
			}
		
		}
		break;
	}
}
/********************************** 功能说明 ***********************************
*  显示 泵工作状态
*******************************************************************************/

void PumpWorkFlag( void )
{
	static uint8_t i;
	i = get_Now() % 4;
	if( i == 0 )
	WBMP(0x1818,0x010A, PUMP1 );
	if( i == 1 )
	WBMP(0x1818,0x010A, PUMP2 );
	if( i == 2 )
	WBMP(0x1818,0x010A, PUMP3 );
	if( i == 3 )
	WBMP(0x1818,0x010A, PUMP4 );
}
static	FP32	fstdx[4][PP_Max];
static	FP32	fstd ;
void Sampler_TdMonitor( enum enumSamplerSelect SamplerSelect )
{	
  switch(  SamplerSelect )
  {	
    case Q_TSP:
      fstdx[0][PP_TSP] = fstdx[1][PP_TSP];
      fstdx[1][PP_TSP] = fstdx[2][PP_TSP];
      fstdx[2][PP_TSP] = fstdx[3][PP_TSP];
      fstdx[3][PP_TSP] = get_fstd( PP_TSP );    
      break;
    case Q_SHI:
      fstdx[0][PP_SHI_C] = fstdx[1][PP_SHI_C];
      fstdx[1][PP_SHI_C] = fstdx[2][PP_SHI_C];
      fstdx[2][PP_SHI_C] = fstdx[3][PP_SHI_C];
      fstdx[3][PP_SHI_C] = get_fstd( PP_SHI_C );
      fstdx[0][PP_SHI_D] = fstdx[1][PP_SHI_D];
      fstdx[1][PP_SHI_D] = fstdx[2][PP_SHI_D];
      fstdx[2][PP_SHI_D] = fstdx[3][PP_SHI_D];
      fstdx[3][PP_SHI_D] = get_fstd( PP_SHI_D );
      break;
    case Q_R24:
      fstdx[0][PP_R24_A] = fstdx[1][PP_R24_A];
      fstdx[1][PP_R24_A] = fstdx[2][PP_R24_A];
      fstdx[2][PP_R24_A] = fstdx[3][PP_R24_A];
      fstdx[3][PP_R24_A] = get_fstd( PP_R24_A );
      fstdx[0][PP_R24_B] = fstdx[1][PP_R24_B];
      fstdx[1][PP_R24_B] = fstdx[2][PP_R24_B];
      fstdx[2][PP_R24_B] = fstdx[3][PP_R24_B];
      fstdx[3][PP_R24_B] = get_fstd( PP_R24_B );
      break;
    default:  
      break;
  }
    
    
 
}
/********************************** 功能说明 ***********************************
*  显示指定采样器的时间状态
*******************************************************************************/
static	void	ShowTimeState ( enum enumSamplerSelect SamplerSelect, enum enumPumpSelect PumpSelect )
{
	struct uSamplerQuery * p = & Q_Sampler[SamplerSelect];
 	struct	uPumpQuery * pT = &Q_Pump[PumpSelect];
  Sampler_TdMonitor( SamplerSelect );
	Lputs ( 0x0102u, "工况" );		//	ShowQueryType ( 0x000Au );
// 	if( PumpSelect != PP_AIR )
	{
		switch ( p->state )
		{
		default:
		case state_ERROR:		Lputs ( 0x0705u, "   !!故障!! " ); break;
		case state_FINISH:	Lputs ( 0x0705u, "   完成采样 " );	break;
		case state_SAMPLE:	Lputs ( 0x0705u, "   正在采样 " );	break;
		case state_SUSPEND:	Lputs ( 0x0705u, "   等待采样 " );	break;
		case state_PAUSE:		Lputs ( 0x0705u, "   暂停采样 " );	break;
		}
		//TODO:	2路同开不同关的时间不一样！！！
		Lputs ( 0x0B02u, "温   度:" );	ShowFP32 ( 0x0B11u, get_Tr( PumpSelect ), 0x0602u, "℃" );
		Lputs ( 0x0E02u, "压   力:" );	ShowFP32 ( 0x0E11u, get_Pr( PumpSelect ), 0x0602u, "kPa" );
		Lputs ( 0x1102u, "大气压力:" );	ShowFP32 ( 0x1111u, get_Ba(),             0x0602u, "kPa" );
		Lputs ( 0x1402u, "剩余时间:" );	ShowTIME ( 0x1416u, p->timer );	
		Lputs ( 0x1702u, "已采时间:" );	ShowTIME ( 0x1716u, pT->sum_time );
		Lputs ( 0x1A02u, "当前次数:" );	ShowI16U ( 0x1A11u, p->loops, 0x0500u, NULL );
	}
// 	else
// 	{
//     
// 		switch ( p->state )
// 		{
// 		default:
// 		case state_ERROR:		Lputs ( 0x0605u, "    !!故障!!    " ); break;
// 		case state_FINISH:	Lputs ( 0x0605u, "    完成采样    " );	break;
// 		case state_SAMPLE:	Lputs ( 0x0605u, "    正在采样    " );	break;
// 		case state_SUSPEND:	Lputs ( 0x0605u, "    等待采样    " );	break;
// 		case state_PAUSE:		Lputs ( 0x0605u, "    暂停采样    " );	break;
// 		}
// 		Lputs ( 0x0902u, "温   度:" );	ShowFP32 ( 0x0911u, get_Tr( PumpSelect ), 0x0602u, "℃" );
// 		Lputs ( 0x0C02u, "压   力:" );	ShowFP32 ( 0x0C11u, get_Pr( PumpSelect ), 0x0602u, "kPa" );
// 		Lputs ( 0x0F02u, "大气压力:" );	ShowFP32 ( 0x0F11u, get_Ba(),             0x0602u, "kPa" );
// 		Lputs ( 0x1202u, "剩余时间:" );	ShowTIME ( 0x1216u, p->timer );	
// 		Lputs ( 0x1502u, "当前次数:" );	ShowI16U ( 0x1511u, p->loops, 0x0500u, NULL );
// 		Lputs ( 0x1802u, "累积时间:" );	ShowTIME ( 0x1816u, pT->sum_time );
// 	}		
		
}

/********************************** 功能说明 ***********************************
*  显示指定采样泵的计前工况状态
*******************************************************************************/
static	void	ShowPumpBefore( enum enumPumpSelect PumpSelect )
{
	struct	uPumpQuery * p = &Q_Pump[PumpSelect];
	Lputs ( 0x0102u, "流量" );

	if ( ! Q_Pump[PumpSelect].xp_state )
	{
		switch (PumpSelect )
		{
		case PP_TSP: // TSP_KB120F
			Lputs ( 0x0602u, 		"工况体积: "				 );	ShowFP32 ( 0x0613u, p->vd,       0x0600u, "L" );
			Lputs ( 0x0902u, 		"标况体积: "				 );	ShowFP32 ( 0x0913u, p->vnd,      0x0600u, "L" );
			Lputs( 0x0C02u, "                   " );
			Lputs( 0x0F02u, "                   " );
			Lputs( 0x1202u, "    泵已关闭        " );
			Lputs( 0x1502u, "                   " );
			Lputs( 0x1802u, "                   " );
			Lputs( 0x1B02u, "                   " );
			return;
		case PP_R24_A:
		case PP_R24_B:
		case PP_SHI_C:
		case PP_SHI_D:
			Lputs ( 0x0602u, 		"标况体积: " 			);	ShowFP32 ( 0x0611u, p->vnd,  0x0702u, "L" );
			Lputs( 0x0A02u, "                   " );
			Lputs( 0x0E02u, "                   " );
			Lputs( 0x1202u, "    泵已关闭        " );
			Lputs( 0x1602u, "                   " );
			Lputs( 0x1A02u, "                   " );
			return;
		case PP_AIR:
			break;
		}
	}
	else 
	{
		
		FP32	OutValue = Pump_GetOutput( PumpSelect );
		Sampler_TdMonitor( SamplerSelect );
    fstd = ( fstdx[0][PumpSelect] + fstdx[1][PumpSelect] + fstdx[2][PumpSelect] + fstdx[3][PumpSelect] ) / 4; 
		switch (PumpSelect )
		{
		case PP_TSP: // TSP_KB120F
			{
				FP32	Te   = get_Te();
				FP32	Ba   = get_Ba();
				FP32	flow;
				flow = Calc_flow( fstd, Te, 0.0f, Ba, Q_TSP );
				Lputs ( 0x0602u, "工况体积: " );		ShowFP32 ( 0x0613u, p->vd,       0x0600u, "L" );
				Lputs ( 0x0A02u, "标况体积: " );		ShowFP32 ( 0x0A13u, p->vnd,      0x0600u, "L" );
				Lputs ( 0x0E02u, "工   况: " );		ShowFP32    ( 0x0E11u, flow, 0x0701u, "L/m" );
				Lputs ( 0x1202u, "标   况: " );		ShowFP32    ( 0x1211u, fstd, 0x0701u, "L/m" );	
				Lputs ( 0x1602u, "输   出:   " );	ShowPercent ( 0x1615u, OutValue );

			}
			break;
		case PP_R24_A:
		case PP_R24_B:
		case PP_SHI_C:
		case PP_SHI_D:
			Lputs ( 0x0602u, "标况体积: " );		ShowFP32 ( 0x0611u, p->vnd,  0x0702u, "L" );
			Lputs ( 0x0A02u, "标   况: " );		ShowFP32    ( 0x0A13u, fstd, 0x0703u, "L/m" );
			Lputs ( 0x0E02u, "输   出: " );		ShowPercent ( 0x0E15u, OutValue );
		switch ( Configure.HeaterType )
		{
		default:
		case enumHeaterNone:	break;	//	MsgBox( "未安装恒温箱", vbOKOnly );	break;
		case enumHCBoxOnly://			空格->防止花屏
			Lputs ( 0x1202u, "恒温箱温度:  " );	ShowFP32 ( 0x1215u, get_HCBoxTemp(),     0x0602u, "℃" );
			Lputs ( 0x1602u, "恒温箱输出:" );		ShowFP32 ( 0x1615u, get_HCBoxOutput(),   0x0501u, "% " );
			Lputs ( 0x1A02u, "恒温箱风扇:" );		ShowI16U ( 0x1A15u, get_HCBoxFanSpeed(), 0x0500u, "RPM" );
			break;	
		case enumHeaterOnly:
			Lputs ( 0x1202u, "加热器            " );	
			Lputs ( 0x1602u, "加热器温度:" );		ShowFP32 ( 0x1615u, get_HeaterTemp(),     0x0602u, "℃" );
			Lputs ( 0x1A02u, "加热器输出:" );		ShowFP32 ( 0x1A15u, get_HeaterOutput(),   0x0501u, "%" );
			break;
		case enumHCBoxHeater:
// 			set_HCBoxTemp( Configure.HCBox_SetTemp * 0.1f, Configure.HCBox_SetMode );
// 			set_HeaterTemp( Configure.Heater_SetTemp*0.1f);
			Lputs ( 0x1602u, "硬件不支持!" );	
			break;
		}
			break;
		case PP_AIR:
			break;
		}
	}	
	
}


/************** 功能说明 ****************
        停止暂停状态选择菜单 
 ****************************************/
void Samplestate_Select( BOOL state )
{
       static	struct  uMenu	menu1[] =
    {
        { 0x0103u, "采样状态控制" },
        { 0x0C02u, "暂停" },{ 0x0C0Eu, "停止" },{ 0x0C19u, "取消" }
		
    };
		   static	struct  uMenu	menu2[] =
    {
        { 0x0103u, "采样状态控制" },
        { 0x0C02u, "恢复" },{ 0x0C0Eu, "停止" },{ 0x0C1Bu, "取消" }
		
    };
		static	struct  uMenu	* menu[] = 
		{
			menu1,
			menu2,
		};
    uint8_t	item = 2u;
    BOOL	need_redraw = TRUE;
    do
    {   
        if ( need_redraw )
        {
          cls();
					Menu_Redraw( menu[state] );
          need_redraw = FALSE;
        }
				item = Menu_Select( menu[state], item + 1u, NULL );
        switch( item )
        {
        case 1:
            Sampler_PauseKey( SamplerSelect );
						item = enumSelectESC;
            break;
        case 2:
            Sampler_Terminate( SamplerSelect );
						item = enumSelectESC;
            break;
        case 3:
						item = enumSelectESC;					
            break;
        default:
            break;
        }
	}while( enumSelectESC != item );
}
/********************************** 功能说明 ***********************************
*  按键处理，集中统一处理各种情况下的按键
*******************************************************************************/
void	disposeKey( const enum enumSamplerSelect SamplerSelect, uint8_t * pOption, const uint8_t opt_max, enum enumPumpSelect * pPumpSelect )
{
	enum {
		opt_exit,
		opt_min = 1u
	};
	static	uint16_t gray  = 1900u;
	BOOL graychanged = FALSE;	

	uint8_t option  = * pOption;
	enum enumPumpSelect PumpSelect;

	if ( NULL != pPumpSelect )
	{
		PumpSelect = *pPumpSelect;
	}
	else
	{
		PumpSelect = PP_Max;
	}
	
	if ( hitKey ( 100u ) )
	{
		switch ( getKey() )
		{
		case K_UP:
			switch ( SamplerSelect )
			{
			case Q_R24:	PumpSelect = PP_R24_A;	break;
			case Q_SHI:	PumpSelect = PP_SHI_C;	break;
			default:	break;
			}
			break;
		case K_DOWN:
			switch ( SamplerSelect )
			{
			case Q_R24:	PumpSelect = PP_R24_B;	break;
			case Q_SHI:	PumpSelect = PP_SHI_D;	break;
			default:	break;
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

		case K_SHIFT:
			{
				SamplerTypeSwitch();
				option = opt_exit;
			}
				
			break;

		case K_ESC:
		   Samplestate_Select( StateRead(SamplerSelect) );
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
			graychanged = FALSE;;
		}		

		cls();
		
		*pOption = option;
		if ( NULL != pPumpSelect )
		{
			*pPumpSelect = PumpSelect;
		}
	}

}


void	monitor_TSP( void )
{
	enum
	{
	    opt_exit,
	    opt_qt_1, opt_qt_2,
	    opt_max, opt_min = 1u
	};
	uint8_t option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	const	enum	enumPumpSelect  PumpSelect = PP_TSP;
	
	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();	option = opt_min;
			}
		}	
		
		SamplerTypeShow( 0x010Eu );
		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);
		Show_std_clock();
		PumpWorkFlag();
		
		switch ( option )
		{
		case opt_qt_1:	ShowTimeState( SamplerSelect, PumpSelect );	break;
		case opt_qt_2:	ShowPumpBefore( PumpSelect );break;
// 		case opt_qt_3:	ShowPumpRunFlow( PumpSelect );	break;
// 		case opt_qt_4:	ShowSumCubage( PumpSelect );	break;
		default:	break;
		}
		
		disposeKey( SamplerSelect, &option, opt_max, NULL );
	}
}

static	void	monitor_R24 ( void )
{
	enum
	{
	    opt_exit,
	    opt_qt_1, opt_qt_2, 
	    opt_max, opt_min = 1u
	};
	uint8_t option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	enum	enumPumpSelect  PumpSelect = PP_R24_A;
	
	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();	option = opt_min;
			}
		}
		
		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);
		
		PumpWorkFlag();
		
		switch ( PumpSelect )
		{
		case PP_R24_A:		Lputs ( 0x010Eu, "日均A" );		Show_std_clock();	break;
		case PP_R24_B:		Lputs ( 0x010Eu, "日均B" );		Show_std_clock();	break;
		}

		switch ( option )
		{
		case opt_qt_1:	ShowTimeState( SamplerSelect, PumpSelect );	
						show_battery( 0x0020u );
						break;
		case opt_qt_2:	ShowPumpBefore( PumpSelect );	break;
		default:	break;
		}

		disposeKey( SamplerSelect, &option, opt_max, &PumpSelect );
	}

}

static	void	monitor_SHI ( void )
{
	enum
	{
	    opt_exit,
	    opt_qt_1, opt_qt_2,  
	    opt_max, opt_min = 1u
	};

	uint8_t	option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	enum	enumPumpSelect  PumpSelect = PP_SHI_C;

	while (( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();	option = opt_min;
			}
		}
		

		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);
		

		PumpWorkFlag();
		

		switch ( PumpSelect )
		{
		case PP_SHI_C:		Lputs ( 0x010Eu, "时均C" );		Show_std_clock();break;
		case PP_SHI_D:		Lputs ( 0x010Eu, "时均D" );		Show_std_clock();	break;
		}

		switch ( option )
		{
		case opt_qt_1:	ShowTimeState( SamplerSelect, PumpSelect );	break;
		case opt_qt_2:	ShowPumpBefore( PumpSelect );	break;
		default:	break;
		}

		disposeKey( SamplerSelect, &option, opt_max, &PumpSelect );
	}
}

static	void	monitor_AIR ( void )
{
	enum
	{
	    opt_exit,
	    opt_qt_1,
	    opt_max, opt_min = 1u
	};
	uint8_t	option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	const enum	enumPumpSelect  PumpSelect = PP_AIR;
	
	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();	option = opt_min;
			}
		}
		
		PumpWorkFlag();
		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);
		ShowTimeState( SamplerSelect, PumpSelect );
		show_battery( 0x0020u );
		SamplerTypeShow( 0x010Eu );
		
		disposeKey( SamplerSelect, &option, opt_max, NULL );
	}
}

/********************************** 功能说明 ***********************************
*  采样过程中显示各种状态
*******************************************************************************/
void	monitor ( void )
{

	while ( Sampler_isRunning( SamplerSelect ) )
	{
		cls();
		SampleFinishFState[SamplerSelect] = TRUE;
		switch ( SamplerSelect )
		{
		default:	
		case Q_TSP:	monitor_TSP();	break;
		case Q_R24:	monitor_R24();	break;
		case Q_SHI:	monitor_SHI();	break;
		case Q_AIR:	monitor_AIR();	break;
		}
	
	}	
}

/********************************** 功能说明 ***********************************
*  根据仪器型号，显示、切换当前的采样器
*******************************************************************************/
enum  enumSamplerSelect  SamplerSelect; 	//	= Q_TSP;

void	SamplerTypeShow( uint16_t yx )
{
		switch ( SamplerSelect )
		{
		case Q_TSP: Lputs( yx, "<粉尘>" );	break;
		case Q_R24: Lputs( yx, "<日均>" );	break;
		case Q_SHI:	Lputs( yx, "<时均>" );	break;
		case Q_AIR:	Lputs( yx, "<大气>" );	break;
		default:	Lputs( yx, "<未知>" );	break;
		}
}

void	SamplerTypeSwitch( void )
{
	BOOL	hasPumpTSP =  ( enumPumpNone != Configure.PumpType[PP_TSP] );
	BOOL	hasPumpR24 = (( enumPumpNone != Configure.PumpType[PP_R24_A] ) || ( enumPumpNone != Configure.PumpType[PP_R24_B] ));
	BOOL	hasPumpSHI = (( enumPumpNone != Configure.PumpType[PP_SHI_C] ) || ( enumPumpNone != Configure.PumpType[PP_SHI_D] ));
	BOOL	hasPumpAIR =  ( enumPumpNone != Configure.PumpType[PP_AIR] );
	switch ( SamplerSelect )
	{
	case Q_ALL:	if ( hasPumpTSP ) 
								SamplerSelect = Q_TSP; 
							else if ( hasPumpR24 ) 
								SamplerSelect = Q_R24;
							else if( hasPumpSHI) 
								SamplerSelect = Q_SHI;
							else if( hasPumpAIR ) 
								SamplerSelect = Q_AIR;
		break;
		
	case Q_TSP:	if ( hasPumpR24 ) 
								SamplerSelect = Q_R24;
							else if( hasPumpSHI) 
								SamplerSelect = Q_SHI;
							else if( hasPumpAIR ) 
								SamplerSelect = Q_AIR;
		break;	
	case Q_R24:	if( hasPumpSHI) 
								SamplerSelect = Q_SHI;
							else if( hasPumpAIR ) 
								SamplerSelect = Q_AIR;
							else if( hasPumpTSP ) 
								SamplerSelect = Q_TSP;					
		break;
	case Q_SHI:	if( hasPumpAIR ) 
								SamplerSelect = Q_AIR;
							else if( hasPumpTSP ) 
								SamplerSelect = Q_TSP;	
							else if ( hasPumpR24 ) 
								SamplerSelect = Q_R24;			
		break;
	case Q_AIR:	if( hasPumpTSP ) 
								SamplerSelect = Q_TSP;	
							else if ( hasPumpR24 ) 
								SamplerSelect = Q_R24;
							else if( hasPumpSHI) 
								SamplerSelect = Q_SHI;							
		break;
	default:	break;
	}
}

// void	DisplaySetContrast( uint8_t SetContrast );

// /********************************** 功能说明 ***********************************
// *  设置显示屏参数（有可能在看不到显示的情况下进入）
// *******************************************************************************/
// void	ModifyLCD( void )
// {
// 	uint16_t gray  = Configure.DisplayGray;
// 	BOOL	graychanged = FALSE;

// 	BOOL	changed = false;
// 	
// 	cls();
// 	Lputs( 0x0000u, "配置 液晶 灰度" );
// 	Lputs( 0x1800u, "用方向键调整电压" );
// 	for(;;)
// 	{
// 		DisplaySetGrayVolt( gray * 0.01f );

// 		Lputs( 0x0C00u, "灰度" );	ShowI16U( 0x0C0Cu, gray,  0x0502u, " V " );


// 		switch( getKey())
// 		{
// 		case K_UP:	
// 			if ( gray < 2000u )
// 			{
// 				++gray;
// 			}
// 			changed = true;
// 			break;
// 		case K_DOWN:
// 			if ( gray >  200u )
// 			{
// 				--gray;
// 			}
// 			changed = true;
// 			break;

// 		case K_RIGHT:
// 			if ( gray < ( 2000u - 100u ))
// 			{ 
// 				gray += 100u;
// 			}
// 			changed = true;
// 			break;
// 		case K_LEFT:	
// 			if ( gray > ( 200 + 20u ))
// 			{
// 				gray -= 20u;
// 			}
// 			changed = true;
// 			break;

// 		case K_ESC:
// 		case K_OK:
// 			if ( changed )
// 			{
// 				Configure.DisplayGray  = gray;
// 				ConfigureSave();
// 			}
// 			return;
// 		case K_OK_UP:	
// 			if ( gray < 2200u )
// 			{
// 				++gray;
// 			}
// 			if( ! releaseKey( K_OK_UP,100 ))
// 			{
// 				while( ! releaseKey( K_OK_UP, 1 ))
// 				{
// 					++gray;
// 					DisplaySetGrayVolt( gray * 0.01f );
// 				}
// 			}
// 			graychanged = true;		
// 			break;
// 		case K_OK_DOWN:
// 			if ( gray >  200u )
// 			{
// 				--gray;
// 			}
// 			if( ! releaseKey( K_OK_DOWN, 100 ))
// 			{
// 				while( ! releaseKey( K_OK_DOWN, 1 ))
// 				{
// 					--gray;
// 					DisplaySetGrayVolt( gray * 0.01f );
// 				}			
// 			}
// 			graychanged = true;
// 			break;

// 		case K_OK_RIGHT:
// 			if ( gray < ( 2000u - 50u ))
// 			{ 
// 				gray += 100u;
// 			}
// 			graychanged = true;
// 			break;
// 		case K_OK_LEFT:	
// 			if ( gray > ( 200 + 20u ))
// 			{
// 				gray -= 20u;
// 			}
// 			graychanged = true;
// 			break;
// 		default:
// 			break;
// 		}
// 		if( graychanged == true )
// 		{
// 			DisplaySetGrayVolt( gray * 0.01f );
// 			Configure.DisplayGray = gray;
// 			ConfigureSave();
// 			graychanged = FALSE;;
// 		}		

// 	}
// }
// uint8_t LCD_Gray( uint8_t item )
// {
// 	if( ( item == 1 ) || ( item == 0xff ) )
//   {
//     if ( ! releaseKey( K_OK, 300 ))
//     {
//       item = enumSelectESC;	
//       beep();
//       delay( 100u );
//       beep();
//       cls();	
//       ModifyLCD();
//     }
//     else
//     if ( ! releaseKey( K_SHIFT, 300 ))
//     {
//       item = enumSelectESC;
//       beep();
//       delay( 100u );
//       beep();
//       cls();
//       Lputs( 0x0604u, "请输入出厂编号:" );
//       ConfigureLoad();
//       if( EditI32U( 0x0A05u, &Configure.ExNum, 0x0700u ))
//         if( vbYes == MsgBox("是否保存编号?",vbYesNo) )
//           ConfigureSave();
//         else
//           ConfigureLoad();				
//     }
//   }
//   
// 	return item;
// }
void	menu_show_env_state( void )
{

	Lputs( 0x0616u, "状  态" );
	show_env_state();

}

/********  (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司  **** End Of File ****/
