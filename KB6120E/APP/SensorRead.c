/**************** (C) COPYRIGHT 2015 青岛金仕达电子科技有限公司 ****************
* 文 件 名: SensorRead.C
* 创 建 者: 董峰
* 描  述  : KB-6120E 读取传感器
* 最后修改: 2014年2月11日
*********************************** 修订记录 ***********************************
* 版  本: 
* 修订人: 
*******************************************************************************/
#include "AppDEF.H"
#include <math.h>
#include <cmsis_os.h>
#include "mbm.h"
#include <stdio.h>

uint32_t CalcMCLK( void );
void	RTC_IRQ_Init( void );
extern	void	HSITRIM_UP( void );
extern	void	HSITRIM_DOWN( void );
extern	uint16_t	Read_HSITrim( void );
BOOL	PrinterInit( BOOL (*CB_UserTerminate)( void ));
void	PrinterTestString( const CHAR * sz );

#define	LCDisPlusVoltage
FP32	_CV_LCD_Volt( uint16_t ADCValue )
{
#ifdef LCDisPlusVoltage
	const FP32	Gain = 10.0f * 3.3f / 4096;
	return	ADCValue * Gain;
	
#else
	FP32  inVolt = 3.3f * ADCValue / 4096 ;	//	求输入脚的电压
	FP32  current = ( 3.3f - inVolt ) / 3.0f;	//	求电流
	FP32  outVolt = 3.3f - current * ( 3.0f + 27.0f );
	
	return	outVolt;
#endif
}

#define	PWM_Output_Max	27648u
uint16_t Out;
void	LCD_Volt_Adjust( void )
{
	extern	FP32	LCDSetGrayVolt ;
	FP32	LCDVolt;	
	static	FP32	Ui	 = 0.0f;
	static	FP32	Uout = 0.30f;
	static	FP32	Ek   = 0.0f;
	
	#ifdef LCDisPlusVoltage
		static	FP32 Up, Ud = 0.0f;
		FP32  LCDSet; 
		static	FP32	 Ek1 = 0.0f; 
// 		LCDSet = 0; 
// 		LCDSet = LCDSetGrayVolt + ( 38 - _CV_CPU_Temp( SensorLocal.CPU_IntTemp ) ) * 0.01f; //	温补
		LCDSet = LCDSetGrayVolt;
		if( LCDSet > 22.0f )
			LCDSet = 22.0f;	 
		LCDVolt = _CV_LCD_Volt( SensorLocal.LCD_Voltage );
		Ek1	 = Ek;
		Ek	 = ( LCDSet - LCDVolt ) * 1.0f;
		Up	 = Ek * 0.007f;
		Ui  += Ek * 0.015f;
		Ud	 = 0.003f * ( Ek - Ek1 );
		Uout = Uout * 0.1f + ( Up + Ui + Ud ) * 0.9f;

		if ( Uout > 0.667f ){ Uout = 0.667f; }//	最大输出 66.7 %
		if ( Uout < 0.02f )	{ Uout = 0.02f; }//	最小输出  2 %

		PWM2_SetOutput((uint16_t)( Uout * PWM_Output_Max ));

	#else
		LCDSet = LCDSetGrayVolt;
		LCDVolt = - _CV_LCD_Volt( SensorLocal.LCD_Voltage );
		Ek = ( LCDSet - LCDVolt ) * 1.0f;
		Ui += Ek * 0.010f;
		Uout = Uout * 0.1f + ( Ek * 0.003f + Ui ) * 0.9f;

		if ( Uout > 0.20f ){ Uout = 0.20f; }//	最大输出 20 %
		if ( Uout < 0.02f ){ Uout = 0.02f; }//	最小输出  2 %

		PWM2_SetOutput((uint16_t)(  Uout * PWM_Output_Max );
	#endif
}

/********************************** 数据定义 ***********************************
//	传感器原始数据缓冲区
*******************************************************************************/
struct	uSensorLocal		SensorLocal;	//	本地传感器
struct	uSensorRemote		SensorRemote;	//	远程传感器

/********************************** 功能说明 ***********************************
*	读取本地传感器，如 AD7705, DS18B20, CPS120 等
*	方案1：读取，不转换，不滤波(N)
*	方案2：读取，不转换，对流量滤波
*	方案3：读取，转换，对全部需要的信号滤波
*******************************************************************************/
__task  void	_task_SensorRead( void const * p_arg )
{
	( void )p_arg;

	ADC12_Init();
	SensorLocal.CPU_IntTemp = ADC12_Readout( ADC_Cx_IntTemp );
	SensorLocal.CPU_IntVolt = ADC12_Readout( ADC_Cx_IntVolt );
	SensorLocal.LCD_Voltage = ADC12_Readout( ADC_Cx_LCDVolt );
	SensorLocal.Bat_Current = ADC12_Readout( ADC_Cx_BatCurr );
	SensorLocal.Bat_Voltage = ADC12_Readout( ADC_Cx_BatVolt );
	CPS120_Read( &SensorLocal.CPS120_Ba, &SensorLocal.CPS120_Temp );

	for(;;)
	{
		uint32_t	sum[5];
		uint_fast8_t i;

		//	液晶电压控制
		LCD_Volt_Adjust();

		//	电池电压测量
		if( Configure.Battery_SW )
			MeasureBattery_OutCmd( true );
		else
			MeasureBattery_OutCmd( false );
		delay( 300u );

		sum[0] =
		sum[1] =
		sum[2] =
		sum[3] =
		sum[4] = 0u;
		
		for ( i = 0u; i < 8u; ++i )
		{
			sum[0] += ADC12_Readout( ADC_Cx_IntTemp );
			sum[1] += ADC12_Readout( ADC_Cx_IntVolt );
			sum[2] += ADC12_Readout( ADC_Cx_LCDVolt );
			sum[3] += ADC12_Readout( ADC_Cx_BatCurr );
			sum[4] += ADC12_Readout( ADC_Cx_BatVolt );
		}

		SensorLocal.CPU_IntTemp = sum[0] / 8u;
		SensorLocal.CPU_IntVolt = sum[1] / 8u;
		SensorLocal.LCD_Voltage = sum[2] / 8u; 
		SensorLocal.Bat_Current = sum[3] / 8u;		
		SensorLocal.Bat_Voltage = sum[4] / 8u;

		CPS120_Read( &SensorLocal.CPS120_Ba, &SensorLocal.CPS120_Temp );
	}
}

/********************************** 功能说明 ***********************************
//	读取远程传感器
*******************************************************************************/
#define	OutShut		0x0000u
#define	OutOpen		0xFF00u

#define	X_Base_AIR 		10u
#define	X_Base_TSP 		15u
#define	X_Base_R24_A	20u
#define	X_Base_R24_B 	25u
#define	X_Base_SHI_C	30u
#define	X_Base_SHI_D 	35u

const uint16_t BaseList[PP_Max] = 
{
	15,	//	PP_TSP
	20,	//	PP_R24_A
	25,	//	PP_R24_B
	30,	//	PP_SHI_C
	35,	//	PP_SHI_D
	10,	//	PP_AIR
};

#define	DO_Base	1u
#define	DI_Base 10001u

#define	AI_Base 30001u

#define	_base_Tr	0
#define	_base_Pr	1
#define	_base_pf	2

#define	AO_Base	40001u


/********************************** 功能说明 ***********************************
*  电机任务的启停控制
*******************************************************************************/
const uint8_t SubSlave = 1u;

void	Motor_OutCmd( enum enumPumpSelect PumpSelect, BOOL NewState )
{
	uint16_t	RegAddress = DO_Base + BaseList[PumpSelect];
	uint8_t		RegValue   = NewState ? 0x01u : 0x00u;
	if( PumpSelect == PP_AIR)
		AIRLightOutCmd( NewState );
	eMBMWrite( SubSlave, RegAddress, 1u, &RegValue );
}

void	Motor_SetOutput( enum enumPumpSelect PumpSelect, uint16_t OutValue )
{
	uint16_t	RegAddress = AO_Base + BaseList[PumpSelect];
	uint16_t	RegValue   = OutValue;
	eMBMWrite( SubSlave, RegAddress, 1u, &RegValue );
}

static	uint16_t	err_count = 0u;
#define	BITN(_b_buf, _n)	(_b_buf[(_n)/8] & ( 1 << (( _n ) % 8 )))

__task	void	_task_ModbusRead( void const * p_arg )
{
	(void)p_arg;

	eMBMInit( MB_RTU, 9600u, MB_PAR_EVEN );
	{
		uint8_t DI_Buf[(40+7)/8];

		eMBErrorCode eStatus = eMBMRead( SubSlave, DI_Base, 40u, DI_Buf );
		
		if ( MB_ENOERR == eStatus )
		{
			SensorRemote.has_Heater = BITN( DI_Buf, 8 ) ? true : false;
			SensorRemote.has_HCBox  = BITN( DI_Buf, 5 ) ? true : false;
			SensorRemote.isExist[PP_AIR] = BITN( DI_Buf, 10 ) ? true : false;
			SensorRemote.isExist[PP_TSP] = BITN( DI_Buf, 15 ) ? true : false;
		}
		else
		{
			err_count ++;
		}
	}

	for(;;)
	{
		static	uint16_t	AI_Buf[40];
		eMBErrorCode eStatus = eMBMRead( SubSlave, AI_Base, 40u, AI_Buf );
	
		if ( MB_ENOERR == eStatus )
		{
			SensorRemote.iCounter = AI_Buf[0];
			
			SensorRemote.Ba = AI_Buf[1];	//	大气压力
			SensorRemote.Te = AI_Buf[2];	//	环境温度
			SensorRemote.Tm = AI_Buf[3];	//	电机温度

			SensorRemote.HCBoxRunTemp  = AI_Buf[5];
			SensorRemote.HCBoxOutValue = AI_Buf[6];
			SensorRemote.HCBoxFanSpeed = AI_Buf[7];

			SensorRemote.HeaterRunTemp  = AI_Buf[8];
			SensorRemote.HeaterOutValue = AI_Buf[9];
			
			SensorRemote.pf[PP_TSP] = AI_Buf[15];
			SensorRemote.pr[PP_TSP] = AI_Buf[16];
			SensorRemote.tr[PP_TSP] = AI_Buf[17];
			
			SensorRemote.pf[PP_R24_A] = AI_Buf[20];
			SensorRemote.pr[PP_R24_A] = AI_Buf[21];
			SensorRemote.tr[PP_R24_A] = AI_Buf[22];
			
			SensorRemote.pf[PP_R24_B] = AI_Buf[25];
			SensorRemote.pr[PP_R24_B] = AI_Buf[26];
			SensorRemote.tr[PP_R24_B] = AI_Buf[27];

			SensorRemote.pf[PP_SHI_C] = AI_Buf[30];
			SensorRemote.pr[PP_SHI_C] = AI_Buf[31];
			SensorRemote.tr[PP_SHI_C] = AI_Buf[32];
			
			SensorRemote.pf[PP_SHI_D] = AI_Buf[35];
			SensorRemote.pr[PP_SHI_D] = AI_Buf[36];
			SensorRemote.tr[PP_SHI_D] = AI_Buf[37];
		}
		else
		{
			err_count ++;
		}
		delay( 200u );

//		{
//			static	uint8_t	DI_Buf[(40+7)/8];
//			eMBErrorCode eStatus = eMBMRead( SubSlave, DI_Base, 40u, DI_Buf );
//		}
//		{
//			static	uint8_t	DO_Buf[(40+7)/8];
//			eMBErrorCode eStatus = eMBMRead( SubSlave, DO_Base, 40u, DO_Buf );
//		}
//		{
//			static	uint16_t	AO_Buf[40];
//			eMBErrorCode eStatus = eMBMRead( SubSlave, AO_Base, 40u, AO_Buf );
//		}
	}
}

void	SENSOR_Init( void )
{
	static	osThreadDef( _task_SensorRead, osPriorityAboveNormal, 1, 0 );
	static	osThreadDef( _task_ModbusRead, osPriorityAboveNormal, 1, 0 );

	osThreadCreate( osThread( _task_SensorRead ), NULL );	//	传感器读取任务(本地)
	osThreadCreate( osThread( _task_ModbusRead ), NULL );	//	传感器读取任务(远程)
}

/********************************** 功能说明 ***********************************
*  厂家调试
	由于程序需要访问传感器缓冲区中的原始数据，所以放在此处。
*******************************************************************************/
void	menu_FactoryDebug( void )
{
	enum
	{
		opt_exit,
		opt_COMM, opt_TSP, opt_R24_A, opt_R24_B, opt_SHI_C, opt_SHI_D, opt_AIR,
		opt_CPU,
		opt_HCBox, opt_BAT,
		opt_max, opt_min = 1
	};
	uint8_t	option = opt_min;
	uint16_t gray  = Configure.DisplayGray;
	BOOL	graychanged = FALSE;

	BOOL		OutState[PP_Max];
	uint16_t	OutValue[PP_Max];

	enum enumPumpSelect	PumpSelect = PP_TSP;
	
	CHAR	sbuffer[36];

	uint8_t	i;

	for ( i = 0u; i < PP_Max; ++i )
	{
		OutState[i] = FALSE;
		OutValue[i] = 10000;
	}
	
	RTC_IRQ_Init();
	do {
		cls();

		switch ( option )
		{
		case opt_COMM:		Lputs( 0x0102u, "通信" );	break;
		case opt_TSP:		  Lputs( 0x0102u, "粉尘" );		  PumpSelect = PP_TSP;	break;
		case opt_R24_A:		Lputs( 0x0102u, "日均(A)" );	PumpSelect = PP_R24_A;	break;
		case opt_R24_B:		Lputs( 0x0102u, "日均(B)" );	PumpSelect = PP_R24_B;	break;
		case opt_SHI_C:		Lputs( 0x0102u, "时均(C)" );	PumpSelect = PP_SHI_C;	break;
		case opt_SHI_D:		Lputs( 0x0102u, "时均(D)" );	PumpSelect = PP_SHI_D;	break;
		case opt_AIR:		  Lputs( 0x0102u, "大气" );	    PumpSelect = PP_AIR;	break;

		case opt_HCBox: Lputs( 0x0102u, "恒温箱" );Lputs( 0x1002u, "加热器" );	break;
		
		case opt_BAT:		Lputs( 0x0102u, "温度" );  Lputs( 0x1002u, "电池" );	 break;
	
		case opt_CPU:		Lputs( 0x0102u, " 其他" ); Lputs( 0x1002u, "CPS120" );break;
		}

		do {
			switch ( option )
			{
			case opt_COMM:
				sprintf( sbuffer, "Errors: %5u", err_count );						Lputs( 0x0600u, sbuffer );
				sprintf( sbuffer, " AI[0]: %5u", SensorRemote.iCounter );	 Lputs( 0x1000u, sbuffer );
				break;

			default:
				sprintf( sbuffer, "Tr: %.4f", _CV_DS18B20_Temp( SensorRemote.tr[PumpSelect] ));	Lputs( 0x0600u, sbuffer );
				sprintf( sbuffer, "Pr: %04X", SensorRemote.pr[PumpSelect] );					Lputs( 0x0C00u, sbuffer );
				sprintf( sbuffer, "Pf: %04X", SensorRemote.pf[PumpSelect] );					Lputs( 0x1200u, sbuffer );
				sprintf( sbuffer, "调速:  [%s]%5u", ( OutState[PumpSelect] ? "+" :"-" ),OutValue[PumpSelect] );	Lputs( 0x1800u, sbuffer );
				break;
			case opt_AIR:
				sprintf( sbuffer, "  Ba:%6.3f", _CV_CPS120_Ba( SensorLocal.CPS120_Ba ));	Lputs( 0x0600u, sbuffer );
				sprintf( sbuffer, "Temp:%6.4f", _CV_CPS120_Temp( SensorLocal.CPS120_Temp ));	Lputs( 0x0C00u, sbuffer );
				sprintf( sbuffer, "环境温度:%6.4f", _CV_DS18B20_Temp( SensorRemote.Te ));		Lputs( 0x1200u, sbuffer );
				sprintf( sbuffer, "开关 [%s]", ( OutState[PumpSelect] ?  "+" :"-" ));	Lputs( 0x1800u, sbuffer );
				break;


			case opt_HCBox:
			{	
				int16_t Output;
				if ( ! SensorRemote.has_HCBox ) 
					Lputs( 0x010Fu, "[?]");
				else
					Lputs( 0x010Fu, "[Y]");	
					
				sprintf( sbuffer, "温度:%6.2f",     SensorRemote.HCBoxRunTemp * 0.0625f );	Lputs( 0x0400u, sbuffer );
				if( SensorRemote.HCBoxOutValue >= 1000 )
				{
					Output =  ( SensorRemote.HCBoxOutValue - 1000 );
					sprintf( sbuffer, "输出: %5u", Output );			Lputs( 0x0800u, sbuffer );
				}
				else
				{
					Output =  ( 1000 - SensorRemote.HCBoxOutValue );
					sprintf( sbuffer, "输出: -%5u", Output );			Lputs( 0x0800u, sbuffer );
				}	
				sprintf( sbuffer, "Fan: %5u rpm",  SensorRemote.HCBoxFanSpeed );			Lputs( 0x0C00u, sbuffer );
				if ( ! SensorRemote.has_Heater ) 
					Lputs( 0x100Fu, "[?]"); 
				else 
					Lputs( 0x100Fu, "[Y]");
				sprintf( sbuffer, "温度:%6.2f",     SensorRemote.HeaterRunTemp * 0.0625f );	Lputs( 0x1400u, sbuffer );
				sprintf( sbuffer, "输出: %5u",      SensorRemote.HeaterOutValue );			Lputs( 0x1800u, sbuffer );
			}	
				break;
		
			case opt_BAT:
				sprintf( sbuffer, "CPU:%.2f ", _CV_CPU_Temp( SensorLocal.CPU_IntTemp ));	Lputs( 0x0400u, sbuffer );
				sprintf( sbuffer, "电机:%.4f ", _CV_DS18B20_Temp( SensorRemote.Tm ));		Lputs( 0x0800u, sbuffer );
				sprintf( sbuffer, "环境:%.4f ", _CV_DS18B20_Temp( SensorRemote.Te ));		Lputs( 0x0C00u, sbuffer );
				sprintf( sbuffer, "电压:%6.3f V ", _CV_Bat_Voltage( SensorLocal.Bat_Voltage ));	Lputs( 0x1400u, sbuffer );
				sprintf( sbuffer, "电流:%6.4f A ", _CV_Bat_Current( SensorLocal.Bat_Current ));	Lputs( 0x1800u, sbuffer );
				break;

			case opt_CPU:
				sprintf( sbuffer, "[%04X]", Read_HSITrim());									Lputs( 0x010Fu, sbuffer );
				sprintf( sbuffer, " MCLK : %7.4f", CalcMCLK() * 1.0e-6f  );						Lputs( 0x0400u, sbuffer );
				sprintf( sbuffer, "CPU电压:%6.3f", _CV_CPU_Volt( SensorLocal.CPU_IntVolt ));	Lputs( 0x0800u, sbuffer );
				sprintf( sbuffer, "LCD电压:%6.2f", _CV_LCD_Volt( SensorLocal.LCD_Voltage ));	Lputs( 0x0C00u, sbuffer );
				sprintf( sbuffer, "    Ba :%6.3f ", _CV_CPS120_Ba( SensorLocal.CPS120_Ba ));		Lputs( 0x1400u, sbuffer );
				sprintf( sbuffer, "   Temp:%6.4f ", _CV_CPS120_Temp( SensorLocal.CPS120_Temp ));	Lputs( 0x1800u, sbuffer );
				break;
			}
		} while( ! hitKey( 25u ));
		
		switch( getKey())
		{
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
			
		case K_UP:
			switch ( option )
			{
			case opt_TSP:
			case opt_R24_A:
			case opt_R24_B:
			case opt_SHI_C:
			case opt_SHI_D:
				OutState[PumpSelect] = TRUE;
				Motor_SetOutput( PumpSelect, OutValue[PumpSelect] );
				Motor_OutCmd   ( PumpSelect, OutState[PumpSelect] );
				break;
      case opt_AIR:		
				OutState[PumpSelect] = TRUE;
				Motor_OutCmd   ( PumpSelect, OutState[PumpSelect] );
				break;
			}
			break;
		case K_DOWN:
			switch ( option )
			{
			case opt_TSP:
            case opt_R24_A:
            case opt_R24_B:
            case opt_SHI_C:
            case opt_SHI_D:
				OutState[PumpSelect] = FALSE;
				Motor_SetOutput( PumpSelect, 0 );
				Motor_OutCmd   ( PumpSelect, OutState[PumpSelect] );
				break;
            case opt_AIR:		
				OutState[PumpSelect] = FALSE;
				break;
			}
			break;
			
		case K_ESC:
			option = opt_exit;
			break;

		case K_OK:
			switch ( option )
			{
			case opt_TSP:
      case opt_R24_A: 
      case opt_R24_B:
      case opt_SHI_C:      
      case opt_SHI_D:                 
				if ( EditI16U( 0x1812u, &OutValue[PumpSelect], 0x0500u ))
				{
					if ( OutState[PumpSelect] )
					{
						Motor_SetOutput( PumpSelect, OutValue[PumpSelect] );
					}
				}
				break;
			}
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

	} while ( opt_exit != option );
	
	for ( i = 0; i < PP_Max; ++i )
	{
		Motor_OutCmd((enum enumPumpSelect)i,   FALSE );
	}
}

/********************************** 功能说明 ***********************************
* 传感器自动调零
	由于程序需要访问传感器缓冲区中的原始数据，所以放在此处。
*******************************************************************************/
static	uint16_t	average( uint16_t const * pArray, const uint8_t count )
{	//	计算缓冲区中前N个数的平均值
	uint32_t	sum = 0u;
	uint8_t 	i;

	for ( i = 0u; i < count; ++i )
	{	
		sum += pArray[i];
	}

	return	(uint16_t)( sum / count );
}

static	void	KB6102_CalibrateZero( enum enumPumpSelect PumpSelect_1, enum enumPumpSelect PumpSelect_2 )
{
	#define	f_len 10u
	uint16_t	sensor[4][f_len];
	uint8_t		index;
	BOOL		cnt_full;

	uint16_t gray  = Configure.DisplayGray;
	BOOL	graychanged = FALSE;
	index = 0u;
	cnt_full = FALSE;

	for(;;)
	{
		do {
			sensor[0][index] = SensorRemote.pf[PumpSelect_1];
			sensor[1][index] = SensorRemote.pf[PumpSelect_2];
			sensor[2][index] = SensorRemote.pr[PumpSelect_1];
			sensor[3][index] = SensorRemote.pr[PumpSelect_2];
			if ( ++index == f_len )
			{
				index = 0u;
				cnt_full = TRUE;
			}
	
			CalibrateRemote.origin[esid_pf][PumpSelect_1] = average( sensor[0], cnt_full ? f_len : index );
			CalibrateRemote.origin[esid_pf][PumpSelect_2] = average( sensor[1], cnt_full ? f_len : index );
			CalibrateRemote.origin[esid_pr][PumpSelect_1] = average( sensor[2], cnt_full ? f_len : index );
			CalibrateRemote.origin[esid_pr][PumpSelect_2] = average( sensor[3], cnt_full ? f_len : index );

			if ( PumpSelect_1 == PumpSelect_2 )
			{
				CalibrateRemote.origin[esid_pf][PumpSelect_1] = average( sensor[0], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pr][PumpSelect_1] = average( sensor[2], cnt_full ? f_len : index );

				Lputs( 0x0F0Fu, "流量:" );
				ShowFP32( 0x150Eu, get_pf( PumpSelect_1 ), 0x0602u, NULL );

				Lputs( 0x0F1Cu, "计压:" );
				ShowFP32( 0x151Bu, get_Pr( PumpSelect_1 ), 0x0602u, NULL );
			}
			else
			{
				CalibrateRemote.origin[esid_pf][PumpSelect_1] = average( sensor[0], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pf][PumpSelect_2] = average( sensor[1], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pr][PumpSelect_1] = average( sensor[2], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pr][PumpSelect_2] = average( sensor[3], cnt_full ? f_len : index );

				Lputs( 0x0F0Fu, "流量:" );
				ShowFP32( 0x140Eu, get_pf( PumpSelect_1 ), 0x0602u, NULL );
				ShowFP32( 0x180Eu, get_pf( PumpSelect_2 ), 0x0602u, NULL );
				Lputs( 0x0F1Cu, "计压:" );
				ShowFP32( 0x141Bu, get_Pr( PumpSelect_1 ), 0x0602u, NULL );
				ShowFP32( 0x181Bu, get_Pr( PumpSelect_2 ), 0x0602u, NULL );
			}
		} while( ! hitKey( 40u ));

		switch( getKey())
		{
		case K_OK:	
		case K_ESC:		CalibrateSave( );		return;
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

	}
}
static	void	KB6102Main_CalibrateZero( enum enumPumpSelect PumpSelect_1, enum enumPumpSelect PumpSelect_2 )
{
	#define	f_len 10u
	uint16_t	sensor[4][f_len];
	uint8_t		index;
	BOOL		cnt_full;
	uint16_t gray  = Configure.DisplayGray;
	BOOL	graychanged = FALSE;

	index = 0u;
	cnt_full = FALSE;

	for(;;)
	{
		do {
			sensor[0][index] = SensorRemote.pf[PumpSelect_1];
			sensor[1][index] = SensorRemote.pf[PumpSelect_2];
			sensor[2][index] = SensorRemote.pr[PumpSelect_1];
			sensor[3][index] = SensorRemote.pr[PumpSelect_2];
			if ( ++index == f_len )
			{
				index = 0u;
				cnt_full = TRUE;
			}
	
			CalibrateRemote.origin[esid_pf][PumpSelect_1] = average( sensor[0], cnt_full ? f_len : index );
			CalibrateRemote.origin[esid_pf][PumpSelect_2] = average( sensor[1], cnt_full ? f_len : index );
			CalibrateRemote.origin[esid_pr][PumpSelect_1] = average( sensor[2], cnt_full ? f_len : index );
			CalibrateRemote.origin[esid_pr][PumpSelect_2] = average( sensor[3], cnt_full ? f_len : index );

			if ( PumpSelect_1 == PumpSelect_2 )
			{
				CalibrateRemote.origin[esid_pf][PumpSelect_1] = average( sensor[0], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pr][PumpSelect_1] = average( sensor[2], cnt_full ? f_len : index );

				Lputs( 0x0C04u, "流量:" );
				ShowFP32( 0x1503u, get_pf( PumpSelect_1 ), 0x0602u, NULL );

				Lputs( 0x0C18u, "计压:" );
				ShowFP32( 0x1517u, get_Pr( PumpSelect_1 ), 0x0602u, NULL );
			}
			else
			{
				CalibrateRemote.origin[esid_pf][PumpSelect_1] = average( sensor[0], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pf][PumpSelect_2] = average( sensor[1], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pr][PumpSelect_1] = average( sensor[2], cnt_full ? f_len : index );
				CalibrateRemote.origin[esid_pr][PumpSelect_2] = average( sensor[3], cnt_full ? f_len : index );

				Lputs( 0x0C04u, "流量:" );
				ShowFP32( 0x1403u, get_pf( PumpSelect_1 ), 0x0602u, NULL );
				ShowFP32( 0x1803u, get_pf( PumpSelect_2 ), 0x0602u, NULL );
				Lputs( 0x0C18u, "计压:" );
				ShowFP32( 0x1417u, get_Pr( PumpSelect_1 ), 0x0602u, NULL );
				ShowFP32( 0x1817u, get_Pr( PumpSelect_2 ), 0x0602u, NULL );
			}
		} while( ! hitKey( 40u ));

		switch( getKey())
		{
		case K_OK:	
		case K_ESC:		CalibrateSave( );		return;
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

	}
}


void	CalibrateZero_R24( void )
{
	Part_cls();
	Lputs( 0x080Eu, "日均 自动调零" );
	KB6102_CalibrateZero( PP_R24_A, PP_R24_B );
}

void	CalibrateZero_SHI( void )
{
	Part_cls();
	Lputs( 0x080Eu, "时均 自动调零" );
	KB6102_CalibrateZero( PP_SHI_C, PP_SHI_D );
}

void	CalibrateZero_TSP( void )
{
	Part_cls();
	Lputs( 0x080Eu, "粉尘 自动调零" );
	KB6102_CalibrateZero( PP_TSP, PP_TSP );
}


void	CalibrateZeromain_R24( void )
{
	cls();
	Lputs( 0x0402u, "日均   自动调零" );
	KB6102Main_CalibrateZero( PP_R24_A, PP_R24_B );
}

void	CalibrateZeromain_SHI( void )
{
	cls();
	Lputs( 0x0402u, "时均   自动调零" );
	KB6102Main_CalibrateZero( PP_SHI_C, PP_SHI_D );
}

void	CalibrateZeromain_TSP( void )
{
	cls();
	Lputs( 0x0402u, "粉尘   自动调零" );
	KB6102Main_CalibrateZero( PP_TSP, PP_TSP );
}






/********************************** 功能说明 ***********************************
*  恒温箱、加热器 通讯
*******************************************************************************/
void	set_HeaterTemp( FP32 SetTemp )//;, bool EnableHeater )
{
	{
		uint16_t	RegAddress = 9u;
		uint8_t 	RegValue   = 0x01;
		eMBMWrite( SubSlave, RegAddress, 1u, &RegValue );
	}
	{
		uint16_t	RegAddress = 40009u;
		uint16_t	RegValue   = (uint16_t)( SetTemp * 16.0f );

		//	eMBMWriteSingleRegister( SubSlave, RegAddress, RegValue );
		eMBMWrite( SubSlave, RegAddress, 1u, &RegValue );
	}
}

FP32	get_HeaterTemp( void )
{
	uint16_t	Value = SensorRemote.HeaterRunTemp;
	FP32		Temp = _CV_DS18B20_Temp( Value );
	FP32		slope  = CalibrateRemote.slope_Heater_Temp * 0.001f;
	FP32		origin = CalibrateRemote.origin_Heater_Temp * 0.01f;

	return	(( Temp + 273.15f ) - origin ) * slope;
}

FP32	get_HeaterOutput( void )
{
	return	SensorRemote.HeaterOutValue * 0.1f;
}

uint16_t	get_FanSpeed( void )
{
	return	SensorRemote.HCBoxFanSpeed ;
}

void	set_HCBoxTemp( FP32 TempSet, uint8_t ModeSet )
{
	uint16_t	RegAddress = 40006u;
	uint16_t	RegValueBuf[2];

	RegValueBuf[0] = (uint16_t)( TempSet * 16.0f );
	RegValueBuf[1] = (uint16_t)( ModeSet );
	//	eMBMWriteSingleRegister( SubSlave, RegAddress, RegValue );
	eMBMWrite( SubSlave, RegAddress, 2u, RegValueBuf );
}

FP32	get_HCBoxTemp( void )
{
	uint16_t	Value = SensorRemote.HCBoxRunTemp;
	FP32		Temp = _CV_DS18B20_Temp( Value );
	FP32		slope  = CalibrateRemote.slope_Heater_Temp * 0.001f;
	FP32		origin = CalibrateRemote.origin_Heater_Temp * 0.01f;

	return	(( Temp + 273.15f ) - origin ) * slope;
}

FP32	get_HCBoxOutput( void )
{
	return	( SensorRemote.HCBoxOutValue - 1000 ) * 0.1;
}

uint16_t	get_HCBoxFanSpeed( void )
{
	return	SensorRemote.HCBoxFanSpeed;
}


void	HCBox_Init( void )
{
	switch ( Configure.HeaterType )
	{
	default:
	case enumHeaterNone:	break;	//	MsgBox( "未安装恒温箱", vbOKOnly );	break;
	case enumHCBoxOnly:
		set_HCBoxTemp( Configure.HCBox_SetTemp * 0.1f, Configure.HCBox_SetMode );
		break;
	case enumHeaterOnly:
		set_HeaterTemp( Configure.Heater_SetTemp*0.1f);
		break;
	case enumHCBoxHeater:
		set_HCBoxTemp( Configure.HCBox_SetTemp * 0.1f, Configure.HCBox_SetMode );
		set_HeaterTemp( Configure.Heater_SetTemp*0.1f);
		break;
	}

}

/********  (C) COPYRIGHT 2015 青岛金仕达电子科技有限公司  **** End Of File ****/
