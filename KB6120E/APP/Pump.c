/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: Pump.C
* �� �� ��: KB-6120E
* ��  ��  : �����ÿ��ؿ���
* ����޸�: 2014��3��17��
*********************************** �޶���¼ ***********************************
* ��  ��: 
* �޶���: 
*******************************************************************************/
#include "AppDEF.H"
#include <math.h>
#include <cmsis_os.h>

/********************************** ����˵�� ***********************************
*	�����õ�����������
*******************************************************************************/
static	FP32	MotorPID_fetchRunValue( enum enumSamplerSelect SamplerSelect )
{
	switch( SamplerSelect )
	{
    case SP_TSP:
		{
			FP32	fstd = get_fstd( SamplerSelect );	
			FP32	Te   = get_Te();
			FP32	Ba   = get_Ba();
			FP32	flow = Calc_flow( fstd, Te, 0.0f, Ba, SP_TSP );

			return	flow;
		}
		case SP_R24_A:
		case SP_R24_B:
		case SP_SHI_C:
		case SP_SHI_D:
		{
			FP32	fstd = get_fstd( SamplerSelect );
			return	fstd;
		}
		default:
// 		case SP_AIR:
			return 0;
	}	
}
// struct  uMotorPID_Set const MotorPID_Set[SP_Max] =
// {	// SetGain   Kp       Ki      Kd 
// 	{ 0.100f, 0.0025f, 0.00015f, 0.0015f },	//	MT_TSP
// 	{ 0.001f, 0.0200f, 0.025f, 0.0500f },	//	MT_R24_A
// 	{ 0.001f, 0.0200f, 0.025f, 0.0500f },	//	MT_R24_B
// 	{ 0.100f, 0.0140f, 0.0150f, 0.0430f },	//	MT_SHI_C
// 	{ 0.100f, 0.0140f, 0.0150f, 0.0430f },	//	MT_SHI_D
// };// 	{ 0.100f, 0.0200f, 0.025f, 0.0500f },//	 

/********************************** ����˵�� ***********************************
*	�����õ�����������
*******************************************************************************/
static	enum	enumMotorOp_State
{	//	�����ò���״̬
	eShut, eRunning, eShutReq 
} volatile  MotorOp_State[SP_Max];

struct uMotorPID_Set
{	//	�����������
	FP32	SetGain;
	FP32	Kp, Ki, Kd;
	//	hardware access interface.
	//	fetch_set, outCmd, setOutput
};

static	FP32	MotorOutValue[SP_Max];

struct  uMotorPID_Set const MotorPID_Set[SP_Max] =
{	// SetGain   Kp       Ki      Kd 
	{ 0.100f, 0.00110f, 0.000105f, 0.00008f },
	{ 0.100f, 0.0190f, 0.025f, 0.0100f },	//	MT_R24_A{ 0.001f, 0.0200f, 0.025f, 0.0500f },	//	MT_R24_A
	{ 0.100f, 0.0190f, 0.025f, 0.0100f },	//	MT_R24_B 
	{ 0.100f, 0.0140f, 0.0150f, 0.0430f },	//	MT_SHI_C
	{ 0.100f, 0.0140f, 0.0150f, 0.0430f },	//	MT_SHI_D
// 	{ 0.100f, 0.0200f, 0.025f, 0.0500f },//	 
};

static	void	MotorPID_Task( enum enumSamplerSelect SamplerSelect )
{
	FP32	SetValue, RunValue, OutValue = 0.0f;
	FP32	Ek_1, Ek = 0.0f;
	FP32	Up = 0.0f, Ui = 0.0f, Ud = 0.0f;
	FP32	Upid = 0.0f;
	FP32  Initout = 0.35f;
	FP32	UpidSetGain = 0.800f;
	uint32_t  SaveTick;
	SetValue = ( Configure.SetFlow[SamplerSelect] * MotorPID_Set[SamplerSelect].SetGain );
	switch( SamplerSelect )
	{
	case SP_TSP:
		UpidSetGain = 0.900f;
		break;
	default:
	case SP_R24_A:
	case SP_R24_B:
	case SP_SHI_C:
	case SP_SHI_D:	
		UpidSetGain = 0.800f;
		break;
	}	
	MotorOutValue[SamplerSelect] = 0.0f;
	Motor_SetOutput( SamplerSelect, 0u );
	Motor_OutCmd( SamplerSelect, TRUE );
	//	��ʼ����ʱ
	delay( 100u );	SaveTick = osKernelSysTick( );

	while ( eRunning == MotorOp_State[SamplerSelect] )
	{
		
		RunValue  = MotorPID_fetchRunValue( SamplerSelect );
		//	PID����
		Ek_1 = Ek;  Ek = ( SetValue - RunValue );
		Up   = MotorPID_Set[SamplerSelect].Kp * Ek;
		Ui  += MotorPID_Set[SamplerSelect].Ki * Ek;
		if ( Ui < -1.0f ){  Ui = -1.0f; }
		if ( Ui > +1.0f ){  Ui = +1.0f; }
		Ud   = ( Ud * 0.667f ) + ( MotorPID_Set[SamplerSelect].Kd * ( Ek - Ek_1 ) * ( 1.0f - 0.667f ));
		Upid =  ( Upid * UpidSetGain ) +( Up + Ui + Ud ) * ( 1.0f - UpidSetGain ); 
		OutValue = Upid + Initout;

		//	PID���
		if ( OutValue < 0.0f ){  OutValue = 0.0f; }
		if ( OutValue > 1.0f ){  OutValue = 1.0f; }
		MotorOutValue[SamplerSelect] = OutValue;
		Motor_SetOutput( SamplerSelect, 27648u * OutValue );

		//	�������ʱ
		{
			const	uint32_t	oneTick = osKernelSysTickMicroSec( 1000u );
			uint32_t	now = osKernelSysTick();
			uint32_t	lostTick, delayCount;
			//	�����Ѿ���ȥ��tick��ʱ��λ
			lostTick = (uint32_t)( now + oneTick - 1u - SaveTick ) / oneTick;
			//	����������Ҫ����ʱ����(�趨���ʱ�䣺300ms)
			if ( lostTick < 300u )
			{
				delayCount = 300u - lostTick;
			}
			else
			{	//	�Ѿ���ȥ��ʱ�䳬���趨�ļ�������¼���ʱ������������ʱ��
				SaveTick = now;
				delayCount = 30u;
			}
			delay( delayCount );
			SaveTick += delayCount * oneTick;
		}
	}
	//	PID ֹͣ
	MotorOutValue[SamplerSelect] = 0.0f;
	Motor_SetOutput( SamplerSelect, 0u );
	Motor_OutCmd( SamplerSelect, FALSE );

	MotorOp_State[SamplerSelect] = eShut;
}

/********************************** ����˵�� ***********************************
*	�׽ӵ��õ�����Ƴ���
*******************************************************************************/
static	void	MotorTask_Socket( void const * p_arg )
{	//	���ò������Ƴ���Ȼ���������
	uint32_t	arg = (uint32_t)p_arg;

	MotorPID_Task((enum enumSamplerSelect)arg );

	osThreadTerminate( osThreadGetId());
}

osThreadDef( MotorTask_Socket, osPriorityHigh, ( SP_Max - 1 ), 0 );

static	void	MotorTask_Init( enum enumSamplerSelect SamplerSelect )
{
	osThreadCreate( osThread( MotorTask_Socket ), ( void * )SamplerSelect );
}
/********************************** ����˵�� ***********************************
*  ����������ͣ����///	����ָ���Ĳ�����
*******************************************************************************/
//	#include "BIOS.H"

void  Pump_OutCmd( enum enumSamplerSelect SamplerSelect, BOOL NewState )
{
// 	if ( SamplerSelect == SP_AIR )
// 	{
// 		MotorOp_State[SamplerSelect] = NewState ? eRunning : eShut;
// 		Motor_OutCmd( SamplerSelect, NewState );
// 		return;
// 	}
	
	if ( ! NewState )
	{	//	����־����������־������ֹ��	//	osThread_SetReady;			
		MotorOp_State[SamplerSelect] = eShutReq;
	}
	else if ( eRunning == MotorOp_State[SamplerSelect] )
	{	//	���󿪱ã����ò�û�й�?!
		;
	}
	else
	{	//	���󿪱ã�����֪���ǹص�
		MotorOp_State[SamplerSelect] = eRunning;
		MotorTask_Init( SamplerSelect );	
	}
// 	if( MotorOp_State[SamplerSelect] == eRunning)
// 		lightopen();
// 	else
// 		lightclose();
}

///	��ȡָ���Ĳ����õĵ������[0.0 ~ 1.0]
FP32	Pump_GetOutput( enum enumSamplerSelect SamplerSelect )
{
// 	if ( SamplerSelect == SP_AIR )
// 	{
// 		return	( eShutReq == MotorOp_State[SamplerSelect] ) ? 0.0f : 1.0f;
// 	}

	if ( eShutReq == MotorOp_State[SamplerSelect] )
	{
		return	0.0f;
	}
	else
	{
		return	MotorOutValue[SamplerSelect];
	}
}

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/
