/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: Sample.C
* �� �� ��: ����
* ��  ��  : KB-6120E �����������
* ����޸�: 2014��1��13��
*********************************** �޶���¼ ***********************************
* ��  ��: 
* �޶���: 
*******************************************************************************/
#include "AppDEF.H"
#include <math.h>
#include <cmsis_os.h>

struct	uSamplerQuery		Q_Sampler[SP_Max];				/*	������ ״̬��ѯ����	*/
/*	������ ״̬��������	*/
static	struct	
{
	BOOL	Clean;
	BOOL	Pause;
	BOOL	Fatal;
} SampleSwitch[SP_Max];

/**
	�����������
*/
#include "Sample_TSP.C"
#include "Sample_R24A.C"
#include "Sample_R24B.C"
#include "Sample_SHIC.C"
#include "Sample_SHID.C"
// #include "Sample_AIR.C"


/********************************** ����˵�� ***********************************
*	�׽ӵ��ò�����ʱ����
*******************************************************************************/
static	__task	void	funSocket( void const * p_arg )
{
	static	void ( * const HandlerList[SP_Max])( void ) =
	{
		_task_Sample_TSP,
		_task_Sample_R24_A,
		_task_Sample_R24_B,
		_task_Sample_SHI_C,
		_task_Sample_SHI_D,
// 		_task_Sample_AIR,
	};

	uint32_t SamplerID = (uint32_t)p_arg;
	
	(HandlerList[SamplerID])();	//	���ò������Ƴ���Ȼ���������

	osThreadTerminate( osThreadGetId());
}

static	void	TaskCreate_Sample( enum enumSamplerSelect SamplerSelect ) 
{
	static	osThreadDef( funSocket, osPriorityAboveNormal, SP_Max, 0 );

	osThreadCreate( osThread( funSocket ), (void * )SamplerSelect );
}

/********************************** ����˵�� ***********************************
*  �����������������ͣ/�ָ���ֹͣ���ƣ��Լ�������Ͽ���
*******************************************************************************/
void Sample_R( enum enumSamplerSelect	SamplerSelect )
{
	static	struct uMenu menu_TSP[] =
	{
		{ 0x0102u, "�ָ��۳�����?" },
		{ 0x0F06u, "��" }, { 0x0F15u, "��" },
 	};
	static	struct uMenu menu_R24A[] =
	{
		{ 0x0102u, "�ָ��վ�A����?" },
		{ 0x0F06u, "��" }, { 0x0F15u, "��" },
 	};
		static	struct uMenu menu_R24B[] =
	{
		{ 0x0102u, "�ָ��վ�B����?" },
		{ 0x0F06u, "��" }, { 0x0F15u, "��" },
 	};
	static	struct uMenu menu_SHIC[] =
	{
		{ 0x0102u, "�ָ�ʱ��C����?" },
		{ 0x0F06u, "��" }, { 0x0F15u, "��" },
 	};	
	static	struct uMenu menu_SHID[] =
	{
		{ 0x0102u, "�ָ�ʱ��D����?" },
		{ 0x0F06u, "��" }, { 0x0F15u, "��" },
 	};
// 	static	struct uMenu menu_AIR[] =
// 	{
// 		{ 0x0102u, "�ָ���������?" },
// 		{ 0x0F06u, "��" }, {0x0F15u, "��" },
//  	};
	static	struct	uMenu	 * menux[SP_Max] =
	{	
		menu_TSP,
		menu_R24A,
		menu_R24B,
		menu_SHIC,
		menu_SHID,
// 		menu_AIR,
	};
	uint8_t item = 1u; 
	cls();
	Menu_Redraw( menux[SamplerSelect] );
	TaskCreate_Sample( SamplerSelect );	
	do
	{		
		item = Menu_Select3( menux[SamplerSelect], item, 10, NULL );
		switch ( item )
		{
		case 1:			
			item = enumSelectESC; 
			break;
		default:
			SampleSwitch[SamplerSelect].Clean = TRUE;
			item = enumSelectESC;
			break;
		}	
	}while ( enumSelectESC != item );
}
	 

 
//	�����ָ����ϵ�ʱ���������δ��ɵĲ��������������ָ�������
void	Sampler_BootResume( void )
{  
	//	Ӧ���������ͺ���ȷ������Щ������Ҫ����
  uint32_t Rsec = get_Now();
	  			
// 	if( Configure.PumpType[SP_TSP] ==  enumOrifice_1 )
		if ( SampleSet[SP_TSP].start != 0 )
		   Sample_R( SP_TSP );

// 	if( Configure.PumpType[SP_R24_A] ==  enumOrifice_1 )
		if ( SampleSet[SP_R24_A].start != 0 )
			Sample_R( SP_R24_A );
			
// 	if( Configure.PumpType[SP_R24_B] ==  enumOrifice_1 )
		if ( SampleSet[SP_R24_B].start != 0 )
			Sample_R( SP_R24_B );

// 	if( Configure.PumpType[SP_SHI_C] ==  enumOrifice_1 )
		if ( SampleSet[SP_SHI_C].start != 0 )
			Sample_R( SP_SHI_C );
			
// 	if( Configure.PumpType[SP_SHI_D] ==  enumOrifice_1 )
		if ( SampleSet[SP_SHI_D].start != 0 )
			Sample_R( SP_SHI_D );

// 	if( Configure.PumpType[SP_AIR] ==  enumOrifice_1 )	
// 		if ( SampleSet[SP_AIR].start != 0 )
// 			Sample_R( SP_AIR );
}

//	�����������û���������
void	Sampler_Start( enum enumSamplerSelect SamplerSelect )
{
	if ( ! Sampler_isRunning( SamplerSelect ))
	{
		if (( 0u != SampleSet[SamplerSelect].sample_time ) && ( 0u != SampleSet[SamplerSelect].set_loops ))
		{			
			SampleSwitch[SamplerSelect].Clean = FALSE;
			SampleSwitch[SamplerSelect].Pause = FALSE;
			SampleSwitch[SamplerSelect].Fatal = FALSE;
			
			SampleSet[SamplerSelect].iloop = 0u;				//	����δ��ʼ����־
			SampleSet[SamplerSelect].start = get_Now() / 60u;	//	��¼��������ʱ��
			SampleSetSave();

			TaskCreate_Sample( SamplerSelect );
		}
	}
}
BOOL StateRead( enum enumSamplerSelect SamplerSelect )
{
   if ( SampleSwitch[SamplerSelect].Pause || SampleSwitch[SamplerSelect].Fatal ) 
		return TRUE;
	 else
		return FALSE;
}
//	��ͣ�������û���ͣ ���� �ָ� ��������
void Sampler_PauseKey( enum enumSamplerSelect SamplerSelect )
{
	if ( SampleSwitch[SamplerSelect].Pause || SampleSwitch[SamplerSelect].Fatal ) 
	{	//	����������ϣ����Իָ�����
		SampleSwitch[SamplerSelect].Pause = FALSE;
		SampleSwitch[SamplerSelect].Fatal = FALSE;
	}	
	else
	{
		SampleSwitch[SamplerSelect].Pause = TRUE;
	}
}


//	ֹͣ�������û���ǰ��ֹ��������
void	Sample_Terminate( enum enumSamplerSelect SamplerSelect )
{	
	SampleSwitch[SamplerSelect].Clean = TRUE;
}

//	���ϴ���֪ͨ�����������������Ϸ�������������Ӧ��ǰ��ֹ����Ӧ����ִ�С�
void	Sample_Fatal( enum enumSamplerSelect SamplerSelect )
{
	SampleSwitch[SamplerSelect].Fatal = TRUE; 
	SampleSwitch[SamplerSelect].Clean = TRUE;
}

//	���ϴ���֪ͨ�����������������Ϸ�������������Ӧ��ͣ���û���ѡ��ָ���
void	Sample_Error( enum enumSamplerSelect SamplerSelect )
{
	SampleSwitch[SamplerSelect].Fatal = TRUE; 
}


BOOL	Sampler_isRunning( enum enumSamplerSelect SamplerSelect )
{
	switch ( SamplerSelect )
	{
	case SP_TSP:
	case SP_R24_A:
	case SP_R24_B:
	case SP_SHI_C:
	case SP_SHI_D:
// 	case SP_AIR:
		return	( 0u != SampleSet[SamplerSelect].start ) ? TRUE : FALSE;
	default:
	case SP_Max:
		if ( 0u != SampleSet[SP_TSP].start )	{ return TRUE; }
		if ( 0u != SampleSet[SP_R24_A].start ){ return TRUE; }
		if ( 0u != SampleSet[SP_R24_B].start ){ return TRUE; }
		if ( 0u != SampleSet[SP_SHI_C].start ){ return TRUE; }
		if ( 0u != SampleSet[SP_SHI_D].start ){ return TRUE; }
// 		if ( 0u != SampleSet[SP_AIR].start )	{ return TRUE; }
		return	FALSE;
	}
}

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/
