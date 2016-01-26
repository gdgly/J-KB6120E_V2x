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
	static const char * BatteryIdentify[]= {"㈠⑴","㈠⑴","㈡⑵","㈢⑶","㈣⑷","㈤⑸"};

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
		{
			//	充满状态 或 放电状态，显示电池分档
			FP32	voltage = inVoltage * 0.2f; // 5 节电池
			uint8_t NewState;

			if     ( voltage >= 3.20f )
			{
				NewState = 5;
			}
			else if( voltage >= 3.10f )
			{
				NewState = 4;
			}
			else if( voltage >= 3.00f )
			{
				NewState = 3;
			}
			else if( voltage >= 2.95f )
			{
				NewState = 2;
			}
			else if( voltage >= 2.85f )
			{
				NewState = 1;
			}
			else
			{
				NewState = 0;  // Battery Empty.
			}

			if( NewState < BatVolState )
			{
				BatVolState = NewState;
			}

			i = BatVolState;
		}
		else
		{
			//	充电状态，（闪烁）显示充电符号。

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
{
	switch ( Configure.Mothed_Ba )
	{
		case enumUserInput:
			Lputs( 0x0910u, "大气压=" );
			break;
		case enumMeasureBa:
			Lputs( 0x0910u, "大气压:" );
			break;
	}

	ShowFP32( 0x0C14u, get_Ba(), 0x0602u, "kPa" );

	//	如果配有恒温箱
	switch ( Configure.HeaterType )
	{
		case enumHCBoxOnly :
			{
				FP32	OutValue = get_HCBoxOutput();
				FP32	RunTemp  = get_HCBoxTemp();
				Lputs	 ( 0x0F10u, "恒温箱:" );
				ShowFP32( 0x1214u, RunTemp, 0x0501u, "℃" );

				if      ( OutValue > 0.0f )
				{
					Lputs( 0x1510u, "加 热: " );
					ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
				}
				else if ( OutValue < 0.0f )
				{
					Lputs( 0x1510u, "制 冷: " );
					ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
				}
				else	//	OutValue == 0.0f
				{
					Lputs( 0x1510u, "输 出:" );
					Lputs( 0x1814u,"   --       " );
				}
			}
			break;
		case enumHeaterOnly:
			{
				FP32	OutValue = get_HeaterOutput();
				FP32	RunTemp  = get_HeaterTemp();
				uint8_t state = Configure.Heater_SW;
				Lputs( 0x0F10u, "加热器:" );
				ShowFP32( 0x1214u, RunTemp, 0x0501u, "℃" );

				if( state )
				{
					if      ( OutValue > 0.0f )
					{
						Lputs( 0x1510u, "输 出:   " );
						ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
					}
					else
					{
						Lputs( 0x1510u, "输 出:");
						Lputs( 0x1814u,"   --       " );
					}
				}
				else
				{
					Lputs( 0x1510u, "输 出:");
					Lputs( 0x1814u," [停用]    " );
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
static	FP32	fstdx[4][SP_Max];
static	FP32	fstd ;
static	void Sampler_Tdfilter( void )
{
	static	uint32_t tt,lastsec;
	tt = get_Now();

	if( lastsec != tt )
	{
		lastsec = tt;

		if( Sampler_isRunning( SP_TSP ) )
		{
			fstdx[0][SP_TSP] = fstdx[1][SP_TSP];
			fstdx[1][SP_TSP] = fstdx[2][SP_TSP];
			fstdx[2][SP_TSP] = fstdx[3][SP_TSP];
			fstdx[3][SP_TSP] = get_fstd( SP_TSP );
		}
		else
		{
			fstdx[0][SP_TSP] =
			  fstdx[1][SP_TSP] =
			    fstdx[2][SP_TSP] =
			      fstdx[3][SP_TSP] = 0;
		}

		if( Sampler_isRunning( SP_SHI_C ) )
		{
			fstdx[0][SP_SHI_C] = fstdx[1][SP_SHI_C];
			fstdx[1][SP_SHI_C] = fstdx[2][SP_SHI_C];
			fstdx[2][SP_SHI_C] = fstdx[3][SP_SHI_C];
			fstdx[3][SP_SHI_C] = get_fstd( SP_SHI_C );
		}
		else
		{
			fstdx[0][SP_SHI_C] =
			  fstdx[1][SP_SHI_C] =
			    fstdx[2][SP_SHI_C] =
			      fstdx[3][SP_SHI_C] = 0;
		}

		if( Sampler_isRunning( SP_SHI_D ) )
		{
			fstdx[0][SP_SHI_D] = fstdx[1][SP_SHI_D];
			fstdx[1][SP_SHI_D] = fstdx[2][SP_SHI_D];
			fstdx[2][SP_SHI_D] = fstdx[3][SP_SHI_D];
			fstdx[3][SP_SHI_D] = get_fstd( SP_SHI_D );
		}
		else
		{
			fstdx[0][SP_SHI_D] =
			  fstdx[1][SP_SHI_D] =
			    fstdx[2][SP_SHI_D] =
			      fstdx[3][SP_SHI_D] = 0;
		}

		if( Sampler_isRunning( SP_R24_A ) )
		{
			fstdx[0][SP_R24_A] = fstdx[1][SP_R24_A];
			fstdx[1][SP_R24_A] = fstdx[2][SP_R24_A];
			fstdx[2][SP_R24_A] = fstdx[3][SP_R24_A];
			fstdx[3][SP_R24_A] = get_fstd( SP_R24_A );
		}
		else
		{
			fstdx[0][SP_R24_A] =
			  fstdx[1][SP_R24_A] =
			    fstdx[2][SP_R24_A] =
			      fstdx[3][SP_R24_A] = 0;
		}

		if( Sampler_isRunning( SP_R24_B ) )
		{
			fstdx[0][SP_R24_B] = fstdx[1][SP_R24_B];
			fstdx[1][SP_R24_B] = fstdx[2][SP_R24_B];
			fstdx[2][SP_R24_B] = fstdx[3][SP_R24_B];
			fstdx[3][SP_R24_B] = get_fstd( SP_R24_B );
		}
		else
		{
			fstdx[0][SP_R24_B] =
			  fstdx[1][SP_R24_B] =
			    fstdx[2][SP_R24_B] =
			      fstdx[3][SP_R24_B] = 0;
		}
	}

}
/********************************** 功能说明 ***********************************
*  显示指定采样器的时间状态
*******************************************************************************/
static	void	ShowTimeState ( enum enumSamplerSelect SamplerSelect)
{
	struct uSamplerQuery * p = & Q_Sampler[SamplerSelect];
	Lputs ( 0x0102u, "工况" );		//	ShowQueryType ( 0x000Au );
// 	if( SamplerSelect != SP_AIR )
	{
		switch ( p->state )
		{
			default:
			case state_ERROR:
				Lputs ( 0x0705u, "   !!故障!! "	);
				break;
			case state_FINISH:
				Lputs ( 0x0705u, "   完成采样 "	);
				break;
			case state_SAMPLE:
				Lputs ( 0x0705u, "   正在采样 "	);
				break;
			case state_SUSPEND:
				Lputs ( 0x0705u, "   等待采样 "	);
				break;
			case state_PAUSE:
				Lputs ( 0x0705u, "   暂停采样 "	);
				break;
		}

		//TODO:	2路同开不同关的时间不一样！！！
		Lputs ( 0x0B02u, "温   度:" 	);
		ShowFP32 ( 0x0B11u, get_Tr( SamplerSelect ), 0x0602u, "℃" );
		Lputs ( 0x0E02u, "压   力:" 	);
		ShowFP32 ( 0x0E11u, get_Pr( SamplerSelect ), 0x0602u, "kPa" );
		Lputs ( 0x1102u, "大气压力:" 	);
		ShowFP32 ( 0x1111u, get_Ba(),             0x0602u, "kPa" );
		Lputs ( 0x1402u, "剩余时间:" 	);
		ShowTIME ( 0x1416u, p->timer );
		Lputs ( 0x1702u, "采样时间:" 	);
		ShowTIME ( 0x1716u, p->sum_time );
		Lputs ( 0x1A02u, "当前次数:" 	);
		ShowI16U ( 0x1A11u, p->loops, 0x0500u, NULL );
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
// 		Lputs ( 0x0902u, "温   度:" );	ShowFP32 ( 0x0911u, get_Tr( SamplerSelect ), 0x0602u, "℃" );
// 		Lputs ( 0x0C02u, "压   力:" );	ShowFP32 ( 0x0C11u, get_Pr( SamplerSelect ), 0x0602u, "kPa" );
// 		Lputs ( 0x0F02u, "大气压力:" );	ShowFP32 ( 0x0F11u, get_Ba(),             0x0602u, "kPa" );
// 		Lputs ( 0x1202u, "剩余时间:" );	ShowTIME ( 0x1216u, p->timer );
// 		Lputs ( 0x1502u, "当前次数:" );	ShowI16U ( 0x1511u, p->loops, 0x0500u, NULL );
// 		Lputs ( 0x1802u, "累积时间:" );	ShowTIME ( 0x1816u, pT->sum_time );
// 	}

}

/********************************** 功能说明 ***********************************
*  显示指定采样泵的计前工况状态
*******************************************************************************/
static	void	ShowPumpBefore( enum enumSamplerSelect SamplerSelect )
{
	struct uSamplerQuery * p = & Q_Sampler[SamplerSelect];
	Lputs ( 0x0102u, "流量" );

	if ( ! Q_Sampler[SamplerSelect].xp_state )
	{

		switch ( SamplerSelect )
		{
			case SP_TSP: // TSP_KB120F
				Lputs( 0x0602u, "工况体积: "						);
				ShowFP32 ( 0x0613u, p->vd,   0x0600u, "L" );
				Lputs( 0x0A02u, "标况体积: "						);
				ShowFP32 ( 0x0A13u, p->vnd,  0x0600u, "L" );
				Lputs( 0x0E02u, "                   " );
				Lputs( 0x1202u, "    泵已关闭        "	);
				Lputs( 0x1602u, "                   "	);

				return;
			case SP_R24_A:
			case SP_R24_B:
			case SP_SHI_C:
			case SP_SHI_D:
				Lputs( 0x0602u, "标况体积: "		 				);
				ShowFP32 ( 0x0613u, p->vnd,  0x0702u, "L" );
				Lputs( 0x0A02u, "                   " );
				Lputs( 0x0E02u, "                   " );
				Lputs( 0x1202u, "    泵已关闭        "	);
				Lputs( 0x1602u, "                   " );
				Lputs( 0x1A02u, "                   " );
				return;
// 		case SP_AIR:
// 			break;
		}
	}
	else
	{

		FP32	OutValue = Pump_GetOutput( SamplerSelect );
		fstd = ( fstdx[0][SamplerSelect] + fstdx[1][SamplerSelect] + fstdx[2][SamplerSelect] + fstdx[3][SamplerSelect] ) / 4;

		switch ( SamplerSelect )
		{
			case SP_TSP: // TSP_KB120F
				{
					FP32	Te   = get_Te();
					FP32	Ba   = get_Ba();
					FP32	flow;
					flow = Calc_flow( fstd, Te, 0.0f, Ba, SP_TSP );
					Lputs ( 0x0602u, "工况体积: " 	);
					ShowFP32    ( 0x0613u, p->vd,  0x0600u, "L" );
					Lputs ( 0x0A02u, "标况体积: " 	);
					ShowFP32    ( 0x0A13u, p->vnd, 0x0600u, "L" );
					Lputs ( 0x0E02u, "工   况: "	);
					ShowFP32    ( 0x0E11u, flow,   0x0701u, "L/m" );
					Lputs ( 0x1202u, "标   况: "	);
					ShowFP32    ( 0x1211u, fstd,   0x0701u, "L/m" );
					Lputs ( 0x1602u, "输   出:   ");
					ShowPercent ( 0x1615u, OutValue );

				}
				break;
			case SP_R24_A:
			case SP_R24_B:
			case SP_SHI_C:
			case SP_SHI_D:
				Lputs ( 0x0602u, "标况体积: "	);
				ShowFP32 ( 0x0613u, p->vnd,  0x0702u, "L" );
				Lputs ( 0x0A02u, "标   况: "	);
				ShowFP32 ( 0x0A13u, fstd,    0x0703u, "L/m" );
				Lputs ( 0x0E02u, "输   出: "	);
				ShowPercent ( 0x0E17u, OutValue );

				switch ( Configure.HeaterType		)
				{
					default:
					case enumHeaterNone:
						break;	//	MsgBox( "未安装恒温箱", vbOKOnly );	break;
					case enumHCBoxOnly://			空格->防止花屏
						Lputs ( 0x1202u, "恒温箱温度:  " );
						ShowFP32 ( 0x1215u, get_HCBoxTemp(),     0x0602u, "℃" );
						Lputs ( 0x1602u, "恒温箱输出:" );
						ShowFP32 ( 0x1617u, get_HCBoxOutput(),   0x0501u, "% " );
						Lputs ( 0x1A02u, "恒温箱风扇:" );
						ShowI16U ( 0x1A17u, get_HCBoxFanSpeed(), 0x0500u, "rpm" );
						break;
					case enumHeaterOnly:
						Lputs ( 0x1202u, "加热器            " );
						Lputs ( 0x1602u, "加热器温度:" );
						ShowFP32 ( 0x1615u, get_HeaterTemp(),     0x0602u, "℃" );
						Lputs ( 0x1A02u, "加热器输出:" );
						ShowFP32 ( 0x1A17u, get_HeaterOutput(),   0x0501u, "%" );
						break;
					case enumHCBoxHeater:
// 			set_HCBoxTemp( Configure.HCBox_SetTemp * 0.1f, Configure.HCBox_SetMode );
// 			set_HeaterTemp( Configure.Heater_SetTemp*0.1f);
						Lputs ( 0x1602u, "硬件不支持!" );
						break;
				}

				break;
// 		case SP_AIR:
// 			break;
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
				fstdx[0][SamplerSelect] =
				  fstdx[1][SamplerSelect] =
				    fstdx[2][SamplerSelect] =
				      fstdx[3][SamplerSelect] = 0;
				item = enumSelectESC;
				break;
			case 2:
				Sample_Terminate( SamplerSelect );
				fstdx[0][SamplerSelect] =
				  fstdx[1][SamplerSelect] =
				    fstdx[2][SamplerSelect] =
				      fstdx[3][SamplerSelect] = 0;
				item = enumSelectESC;
				break;
			case 3:
				item = enumSelectESC;
				break;
			default:
				break;
		}
	}
	while( enumSelectESC != item );
}
/********************************** 功能说明 ***********************************
*  按键处理，集中统一处理各种情况下的按键
*******************************************************************************/
void	disposeKey( const enum enumSamplerSelect SamplerSelect, uint8_t * pOption, const uint8_t opt_max )
{
	enum
	{
		opt_exit,
		opt_min = 1u
	};
	static	uint16_t gray  = 1900u;
	BOOL graychanged = FALSE;

	uint8_t option  = * pOption;

	Sampler_Tdfilter();//	流量滤波

	if ( hitKey ( 100u ) )
	{
		switch ( getKey() )
		{
		case K_UP:
			{
				SamplerTypeSwitchBack();
				option = opt_exit;
			}
			break;
		case K_DOWN:
			{
				SamplerTypeSwitch();
				option = opt_exit;
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
			graychanged = FALSE;
		}

		cls();

		*pOption = option;
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

	const	enum	enumSamplerSelect  SamplerSelect = SP_TSP;

	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();
				option = opt_min;
			}
		}

// 		SamplerTypeShow( 0x010Eu );

		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);
		Show_std_clock();
		PumpWorkFlag();
		Lputs ( 0x010Eu, "粉 尘" );

		switch ( option )
		{
			case opt_qt_1:
				ShowTimeState( SamplerSelect );
				break;
			case opt_qt_2:
				ShowPumpBefore( SamplerSelect );
				break;
// 		case opt_qt_3:	ShowPumpRunFlow( SamplerSelect );	break;
// 		case opt_qt_4:	ShowSumCubage( SamplerSelect );	break;
			default:
				break;
		}

		disposeKey( SamplerSelect, &option, opt_max );
	}
}

static	void	monitor_R24_A ( void )
{
	enum
	{
		opt_exit,
		opt_qt_1, opt_qt_2,
		opt_max, opt_min = 1u
	};
	uint8_t option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	const	enum	enumSamplerSelect  SamplerSelect = SP_R24_A;

	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();
				option = opt_min;
			}
		}

		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);

		PumpWorkFlag();

		Lputs ( 0x010Eu, "日均A" );
		Show_std_clock();

		switch ( option )
		{
			case opt_qt_1:
				ShowTimeState( SamplerSelect );
				show_battery( 0x0020u );
				break;
			case opt_qt_2:
				ShowPumpBefore( SamplerSelect );
				break;
			default:
				break;
		}

		disposeKey( SamplerSelect, &option, opt_max );
	}

}

static	void	monitor_R24_B ( void )
{
	enum
	{
		opt_exit,
		opt_qt_1, opt_qt_2,
		opt_max, opt_min = 1u
	};
	uint8_t option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	const	enum	enumSamplerSelect  SamplerSelect = SP_R24_B;

	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();
				option = opt_min;
			}
		}

		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);

		PumpWorkFlag();

		Lputs ( 0x010Eu, "日均B" );
		Show_std_clock();

		switch ( option )
		{
			case opt_qt_1:
				ShowTimeState( SamplerSelect );
				show_battery( 0x0020u );
				break;
			case opt_qt_2:
				ShowPumpBefore( SamplerSelect );
				break;
			default:
				break;
		}

		disposeKey( SamplerSelect, &option, opt_max );
	}


}

static	void	monitor_SHI_C ( void )
{
	enum
	{
		opt_exit,
		opt_qt_1, opt_qt_2,
		opt_max, opt_min = 1u
	};

	uint8_t	option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	const	enum	enumSamplerSelect  SamplerSelect = SP_SHI_C;

	while (( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();
				option = opt_min;
			}
		}


		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);


		PumpWorkFlag();

		Lputs ( 0x010Eu, "时均C" );
		Show_std_clock();

		switch ( option )
		{
			case opt_qt_1:
				ShowTimeState( SamplerSelect);
				break;
			case opt_qt_2:
				ShowPumpBefore( SamplerSelect );
				break;
			default:
				break;
		}

		disposeKey( SamplerSelect, &option, opt_max);
	}
}

static	void	monitor_SHI_D ( void )
{
	enum
	{
		opt_exit,
		opt_qt_1, opt_qt_2,
		opt_max, opt_min = 1u
	};

	uint8_t	option = opt_min;
	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

	const	enum	enumSamplerSelect  SamplerSelect = SP_SHI_D;

	while (( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
	{
		if ( 0u != iDelay )
		{
			if ( --iDelay == 0u )
			{
				cls();
				option = opt_min;
			}
		}


		WBMP( 0x0290,0x0502, STROCK);
		WBMP( 0x0290,0x0514, STROCK);


		PumpWorkFlag();

		Lputs ( 0x010Eu, "时均D" );
		Show_std_clock();

		switch ( option )
		{
			case opt_qt_1:
				ShowTimeState( SamplerSelect);
				break;
			case opt_qt_2:
				ShowPumpBefore( SamplerSelect );
				break;
			default:
				break;
		}

		disposeKey( SamplerSelect, &option, opt_max);
	}
}

// static	void	monitor_AIR ( void )
// {
// 	enum
// 	{
// 	    opt_exit,
// 	    opt_qt_1,
// 	    opt_max, opt_min = 1u
// 	};
// 	uint8_t	option = opt_min;
// 	uint16_t	iDelay = 1200u;	//	延时一段时间返回默认状态

// 	const enum	enumSamplerSelect  SamplerSelect = SP_AIR;
//
// 	while ( ( opt_exit != option ) && Sampler_isRunning( SamplerSelect ))
// 	{
// 		if ( 0u != iDelay )
// 		{
// 			if ( --iDelay == 0u )
// 			{
// 				cls();	option = opt_min;
// 			}
// 		}
//
// 		PumpWorkFlag();
// 		WBMP( 0x0290,0x0502, STROCK);
// 		WBMP( 0x0290,0x0514, STROCK);
// 		ShowTimeState( SamplerSelect, SamplerSelect );
// 		show_battery( 0x0020u );
// 		SamplerTypeShow( 0x010Eu );
//
// 		disposeKey( SamplerSelect, &option, opt_max, NULL );
// 	}
// }

/********************************** 功能说明 ***********************************
*  采样过程中显示各种状态
*******************************************************************************/
void	monitor ( void )
{

	while ( Sampler_isRunning( SamplerSelect ) )
	{
// 		SampleShowState[SamplerSelect] = TRUE;
		cls();

		switch ( SamplerSelect )
		{
			default:
			case SP_TSP:
				monitor_TSP();
				break;
			case SP_R24_A:
				monitor_R24_A();
				break;
			case SP_R24_B:
				monitor_R24_B();
				break;
			case SP_SHI_C:
				monitor_SHI_C();
				break;
			case SP_SHI_D:
				monitor_SHI_D();
				break;
		}

	}

	Sampler_Tdfilter();//	流量滤波
}

/********************************** 功能说明 ***********************************
*  根据仪器型号，显示、切换当前的采样器
*******************************************************************************/
enum  enumSamplerSelect  SamplerSelect; 	//	= SP_TSP;

void	SamplerTypeShow( uint16_t yx )
{
	static	const	char STROCKx[] =
	{
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	};

	switch ( SamplerSelect )
	{
		case SP_TSP:
			Lputs( yx, "粉 尘" );
			break;
		case SP_R24_A:
			Lputs( yx, "日均A" );
			break;
		case SP_R24_B:
			Lputs( yx, "日均B" );
			break;
		case SP_SHI_C:
			Lputs( yx, "时均C" );
			break;
		case SP_SHI_D:
			Lputs( yx, "时均D" );
			break;
		default:
			Lputs( yx, "未 知" );
			break;
	}

	WBMP( 0x0240,yx+0x0300, STROCKx); //显示横线
}
uint8_t	SamplerTypeHas[SP_Max];
uint8_t SamplerHasMax = 0;
void	SamplerTypeSelect( void )
{
	uint8_t i,t;
	t = 0;

	for( i = 0; i < (enum	enumSamplerSelect) SP_Max; i ++ )
	{
		if( enumPumpNone != Configure.PumpType[ (enum	enumSamplerSelect)i ] )
			SamplerTypeHas[t++] = (enum	enumSamplerSelect)i;
	}

	SamplerHasMax =  t;
	SamplerSelect = (enum	enumSamplerSelect)SamplerTypeHas[0];
}

void	SamplerTypeSwitch( void )
{
	static	uint8_t SamplerExchange;

	for( SamplerExchange = 0; SamplerExchange < SamplerHasMax; SamplerExchange ++ )
	{
		if( SamplerSelect == SamplerTypeHas[SamplerExchange] )
		{
			SamplerExchange += 1u;
			break;
		}
	}

	if( SamplerSelect == SamplerTypeHas[SamplerHasMax - 1u] )
		SamplerSelect =(enum enumSamplerSelect )SamplerTypeHas[0];
	else
		SamplerSelect = (enum	enumSamplerSelect)SamplerTypeHas[SamplerExchange];
}
void	SamplerTypeSwitchBack( void )
{
	static	uint8_t SamplerExchange;

	if( SamplerSelect == SamplerTypeHas[0] )
		SamplerSelect = (enum enumSamplerSelect )SamplerTypeHas[SamplerHasMax - 1u];
	else
	{
		for( SamplerExchange = SamplerHasMax-1; SamplerExchange > 0; SamplerExchange -- )
		{
			if( SamplerSelect == SamplerTypeHas[SamplerExchange] )
			{
				SamplerExchange -= 1u;
				break;
			}
		}
		SamplerSelect = (enum enumSamplerSelect )SamplerTypeHas[SamplerExchange];
	}	

}
void	menu_show_env_state( void )
{

	Lputs( 0x0616u, "状  态" );
	show_env_state();

}

/********  (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司  **** End Of File ****/
