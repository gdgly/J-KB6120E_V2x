/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: Monitor.C
* �� �� ��: KB-6120E
* ��  ��  : �����������ʾ���������еĲ���״̬
* ����޸�: 2013��12��17��
*********************************** �޶���¼ ***********************************
* ��  ��:
* �޶���:
*******************************************************************************/
#include "AppDEF.H"
#include "BMP.H"
/*
������������Ҫ��ʾ��״̬�ֳ����¼�����
0.��Ӧ����������
1.ʱ�����״̬������������������ʱ�䡢ʣ��ʱ�䡢��ǰ����״̬�ǲ���������ͣ���ǹ��ϵȵ�
2.����״̬�������¶ȡ�ѹ��������ѹ���������������꣩����������Ϣ��
3.ͳ����Ϣ�������ۼ�ʱ�䡢�ۻ�����������꣩��
4.������Ϣ���� �������¶ȼ�����״̬�������¶ȡ�����ѹ�� ��

ԭ���ϣ���س���ֻ��ʾ�������ơ�����ʾ�����ݣ���ȫ�ֱ������ݣ�����Ӧ���Ƴ����������¡�
*/



/********************************** ����˵�� ***********************************
*  ��ʾ���״̬
*******************************************************************************/
FP32	get_Bat_Voltage( void );
FP32	get_Bat_Current( void );
bool	get_Bat_Charging( void );


void	show_battery( uint16_t yx )
{
	static const char * BatteryIdentify[]= {"���","���","���","���","���","���"};

#define	BatVolStateFULL	5u
	static	uint8_t	BatVolState = BatVolStateFULL;

	uint8_t i;
	bool isCharging;

	FP32 inVoltage = get_Bat_Voltage();

	if( Configure.Battery_SW == TRUE )
	{
		if ( inVoltage < 1.0f )
		{
			return;	//	��ص�ѹ�쳣����Ϊ���ûװ������û�е�ء�
		}

		isCharging = get_Bat_Charging();

		if ( ! isCharging )
		{
			//	����״̬ �� �ŵ�״̬����ʾ��طֵ�
			FP32	voltage = inVoltage * 0.2f; // 5 �ڵ��
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
			//	���״̬������˸����ʾ�����š�

			BatVolState = BatVolStateFULL;

			i = get_Now() % BatVolStateFULL + 1;
		}

		Lputs( yx, BatteryIdentify[i] );
	}

}

/********************************** ����˵�� ***********************************
*  ��ʾ��������������ѹ�������� �� ������ ״̬
*******************************************************************************/
void	show_env_state( void )
{
	switch ( Configure.Mothed_Ba )
	{
		case enumUserInput:
			Lputs( 0x0910u, "����ѹ=" );
			break;
		case enumMeasureBa:
			Lputs( 0x0910u, "����ѹ:" );
			break;
	}

	ShowFP32( 0x0C14u, get_Ba(), 0x0602u, "kPa" );

	//	������к�����
	switch ( Configure.HeaterType )
	{
		case enumHCBoxOnly :
			{
				FP32	OutValue = get_HCBoxOutput();
				FP32	RunTemp  = get_HCBoxTemp();
				Lputs	 ( 0x0F10u, "������:" );
				ShowFP32( 0x1214u, RunTemp, 0x0501u, "��" );

				if      ( OutValue > 0.0f )
				{
					Lputs( 0x1510u, "�� ��: " );
					ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
				}
				else if ( OutValue < 0.0f )
				{
					Lputs( 0x1510u, "�� ��: " );
					ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
				}
				else	//	OutValue == 0.0f
				{
					Lputs( 0x1510u, "�� ��:" );
					Lputs( 0x1814u,"   --       " );
				}
			}
			break;
		case enumHeaterOnly:
			{
				FP32	OutValue = get_HeaterOutput();
				FP32	RunTemp  = get_HeaterTemp();
				uint8_t state = Configure.Heater_SW;
				Lputs( 0x0F10u, "������:" );
				ShowFP32( 0x1214u, RunTemp, 0x0501u, "��" );

				if( state )
				{
					if      ( OutValue > 0.0f )
					{
						Lputs( 0x1510u, "�� ��:   " );
						ShowFP32( 0x1814u, OutValue, 0x0601u, "% " );
					}
					else
					{
						Lputs( 0x1510u, "�� ��:");
						Lputs( 0x1814u,"   --       " );
					}
				}
				else
				{
					Lputs( 0x1510u, "�� ��:");
					Lputs( 0x1814u," [ͣ��]    " );
				}

			}
			break;
	}
}
/********************************** ����˵�� ***********************************
*  ��ʾ �ù���״̬
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
/********************************** ����˵�� ***********************************
*  ��ʾָ����������ʱ��״̬
*******************************************************************************/
static	void	ShowTimeState ( enum enumSamplerSelect SamplerSelect)
{
	struct uSamplerQuery * p = & Q_Sampler[SamplerSelect];
	Lputs ( 0x0102u, "����" );		//	ShowQueryType ( 0x000Au );
// 	if( SamplerSelect != SP_AIR )
	{
		switch ( p->state )
		{
			default:
			case state_ERROR:
				Lputs ( 0x0705u, "   !!����!! "	);
				break;
			case state_FINISH:
				Lputs ( 0x0705u, "   ��ɲ��� "	);
				break;
			case state_SAMPLE:
				Lputs ( 0x0705u, "   ���ڲ��� "	);
				break;
			case state_SUSPEND:
				Lputs ( 0x0705u, "   �ȴ����� "	);
				break;
			case state_PAUSE:
				Lputs ( 0x0705u, "   ��ͣ���� "	);
				break;
		}

		//TODO:	2·ͬ����ͬ�ص�ʱ�䲻һ��������
		Lputs ( 0x0B02u, "��   ��:" 	);
		ShowFP32 ( 0x0B11u, get_Tr( SamplerSelect ), 0x0602u, "��" );
		Lputs ( 0x0E02u, "ѹ   ��:" 	);
		ShowFP32 ( 0x0E11u, get_Pr( SamplerSelect ), 0x0602u, "kPa" );
		Lputs ( 0x1102u, "����ѹ��:" 	);
		ShowFP32 ( 0x1111u, get_Ba(),             0x0602u, "kPa" );
		Lputs ( 0x1402u, "ʣ��ʱ��:" 	);
		ShowTIME ( 0x1416u, p->timer );
		Lputs ( 0x1702u, "����ʱ��:" 	);
		ShowTIME ( 0x1716u, p->sum_time );
		Lputs ( 0x1A02u, "��ǰ����:" 	);
		ShowI16U ( 0x1A11u, p->loops, 0x0500u, NULL );
	}
// 	else
// 	{
//
// 		switch ( p->state )
// 		{
// 		default:
// 		case state_ERROR:		Lputs ( 0x0605u, "    !!����!!    " ); break;
// 		case state_FINISH:	Lputs ( 0x0605u, "    ��ɲ���    " );	break;
// 		case state_SAMPLE:	Lputs ( 0x0605u, "    ���ڲ���    " );	break;
// 		case state_SUSPEND:	Lputs ( 0x0605u, "    �ȴ�����    " );	break;
// 		case state_PAUSE:		Lputs ( 0x0605u, "    ��ͣ����    " );	break;
// 		}
// 		Lputs ( 0x0902u, "��   ��:" );	ShowFP32 ( 0x0911u, get_Tr( SamplerSelect ), 0x0602u, "��" );
// 		Lputs ( 0x0C02u, "ѹ   ��:" );	ShowFP32 ( 0x0C11u, get_Pr( SamplerSelect ), 0x0602u, "kPa" );
// 		Lputs ( 0x0F02u, "����ѹ��:" );	ShowFP32 ( 0x0F11u, get_Ba(),             0x0602u, "kPa" );
// 		Lputs ( 0x1202u, "ʣ��ʱ��:" );	ShowTIME ( 0x1216u, p->timer );
// 		Lputs ( 0x1502u, "��ǰ����:" );	ShowI16U ( 0x1511u, p->loops, 0x0500u, NULL );
// 		Lputs ( 0x1802u, "�ۻ�ʱ��:" );	ShowTIME ( 0x1816u, pT->sum_time );
// 	}

}

/********************************** ����˵�� ***********************************
*  ��ʾָ�������õļ�ǰ����״̬
*******************************************************************************/
static	void	ShowPumpBefore( enum enumSamplerSelect SamplerSelect )
{
	struct uSamplerQuery * p = & Q_Sampler[SamplerSelect];
	Lputs ( 0x0102u, "����" );

	if ( ! Q_Sampler[SamplerSelect].xp_state )
	{

		switch ( SamplerSelect )
		{
			case SP_TSP: // TSP_KB120F
				Lputs( 0x0602u, "�������: "						);
				ShowFP32 ( 0x0613u, p->vd,   0x0600u, "L" );
				Lputs( 0x0A02u, "������: "						);
				ShowFP32 ( 0x0A13u, p->vnd,  0x0600u, "L" );
				Lputs( 0x0E02u, "                   " );
				Lputs( 0x1202u, "    ���ѹر�        "	);
				Lputs( 0x1602u, "                   "	);

				return;
			case SP_R24_A:
			case SP_R24_B:
			case SP_SHI_C:
			case SP_SHI_D:
				Lputs( 0x0602u, "������: "		 				);
				ShowFP32 ( 0x0613u, p->vnd,  0x0702u, "L" );
				Lputs( 0x0A02u, "                   " );
				Lputs( 0x0E02u, "                   " );
				Lputs( 0x1202u, "    ���ѹر�        "	);
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
					Lputs ( 0x0602u, "�������: " 	);
					ShowFP32    ( 0x0613u, p->vd,  0x0600u, "L" );
					Lputs ( 0x0A02u, "������: " 	);
					ShowFP32    ( 0x0A13u, p->vnd, 0x0600u, "L" );
					Lputs ( 0x0E02u, "��   ��: "	);
					ShowFP32    ( 0x0E11u, flow,   0x0701u, "L/m" );
					Lputs ( 0x1202u, "��   ��: "	);
					ShowFP32    ( 0x1211u, fstd,   0x0701u, "L/m" );
					Lputs ( 0x1602u, "��   ��:   ");
					ShowPercent ( 0x1615u, OutValue );

				}
				break;
			case SP_R24_A:
			case SP_R24_B:
			case SP_SHI_C:
			case SP_SHI_D:
				Lputs ( 0x0602u, "������: "	);
				ShowFP32 ( 0x0613u, p->vnd,  0x0702u, "L" );
				Lputs ( 0x0A02u, "��   ��: "	);
				ShowFP32 ( 0x0A13u, fstd,    0x0703u, "L/m" );
				Lputs ( 0x0E02u, "��   ��: "	);
				ShowPercent ( 0x0E17u, OutValue );

				switch ( Configure.HeaterType		)
				{
					default:
					case enumHeaterNone:
						break;	//	MsgBox( "δ��װ������", vbOKOnly );	break;
					case enumHCBoxOnly://			�ո�->��ֹ����
						Lputs ( 0x1202u, "�������¶�:  " );
						ShowFP32 ( 0x1215u, get_HCBoxTemp(),     0x0602u, "��" );
						Lputs ( 0x1602u, "���������:" );
						ShowFP32 ( 0x1617u, get_HCBoxOutput(),   0x0501u, "% " );
						Lputs ( 0x1A02u, "���������:" );
						ShowI16U ( 0x1A17u, get_HCBoxFanSpeed(), 0x0500u, "rpm" );
						break;
					case enumHeaterOnly:
						Lputs ( 0x1202u, "������            " );
						Lputs ( 0x1602u, "�������¶�:" );
						ShowFP32 ( 0x1615u, get_HeaterTemp(),     0x0602u, "��" );
						Lputs ( 0x1A02u, "���������:" );
						ShowFP32 ( 0x1A17u, get_HeaterOutput(),   0x0501u, "%" );
						break;
					case enumHCBoxHeater:
// 			set_HCBoxTemp( Configure.HCBox_SetTemp * 0.1f, Configure.HCBox_SetMode );
// 			set_HeaterTemp( Configure.Heater_SetTemp*0.1f);
						Lputs ( 0x1602u, "Ӳ����֧��!" );
						break;
				}

				break;
// 		case SP_AIR:
// 			break;
		}
	}

}


/************** ����˵�� ****************
        ֹͣ��ͣ״̬ѡ��˵�
 ****************************************/
void Samplestate_Select( BOOL state )
{
	static	struct  uMenu	menu1[] =
	{
		{ 0x0103u, "����״̬����" },
		{ 0x0C02u, "��ͣ" },{ 0x0C0Eu, "ֹͣ" },{ 0x0C19u, "ȡ��" }
	};
	static	struct  uMenu	menu2[] =
	{
		{ 0x0103u, "����״̬����" },
		{ 0x0C02u, "�ָ�" },{ 0x0C0Eu, "ֹͣ" },{ 0x0C1Bu, "ȡ��" }
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
/********************************** ����˵�� ***********************************
*  ������������ͳһ�����������µİ���
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

	Sampler_Tdfilter();//	�����˲�

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
	uint16_t	iDelay = 1200u;	//	��ʱһ��ʱ�䷵��Ĭ��״̬

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
		Lputs ( 0x010Eu, "�� ��" );

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
	uint16_t	iDelay = 1200u;	//	��ʱһ��ʱ�䷵��Ĭ��״̬

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

		Lputs ( 0x010Eu, "�վ�A" );
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
	uint16_t	iDelay = 1200u;	//	��ʱһ��ʱ�䷵��Ĭ��״̬

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

		Lputs ( 0x010Eu, "�վ�B" );
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
	uint16_t	iDelay = 1200u;	//	��ʱһ��ʱ�䷵��Ĭ��״̬

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

		Lputs ( 0x010Eu, "ʱ��C" );
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
	uint16_t	iDelay = 1200u;	//	��ʱһ��ʱ�䷵��Ĭ��״̬

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

		Lputs ( 0x010Eu, "ʱ��D" );
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
// 	uint16_t	iDelay = 1200u;	//	��ʱһ��ʱ�䷵��Ĭ��״̬

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

/********************************** ����˵�� ***********************************
*  ������������ʾ����״̬
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

	Sampler_Tdfilter();//	�����˲�
}

/********************************** ����˵�� ***********************************
*  ���������ͺţ���ʾ���л���ǰ�Ĳ�����
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
			Lputs( yx, "�� ��" );
			break;
		case SP_R24_A:
			Lputs( yx, "�վ�A" );
			break;
		case SP_R24_B:
			Lputs( yx, "�վ�B" );
			break;
		case SP_SHI_C:
			Lputs( yx, "ʱ��C" );
			break;
		case SP_SHI_D:
			Lputs( yx, "ʱ��D" );
			break;
		default:
			Lputs( yx, "δ ֪" );
			break;
	}

	WBMP( 0x0240,yx+0x0300, STROCKx); //��ʾ����
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

	Lputs( 0x0616u, "״  ̬" );
	show_env_state();

}

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/
