/**************** (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司 ****************
* 文 件 名: Sample.C
* 创 建 者: 董峰
* 描  述  : KB-6120E 采样任务控制
* 最后修改: 2014年1月13日
*********************************** 修订记录 ***********************************
* 版  本: 
* 修订人: 
*******************************************************************************/
#include "AppDEF.H"
#include <math.h>
#include <cmsis_os.h>

struct	uSamplerQuery		Q_Sampler[SP_Max];				/*	采样泵 状态查询数据	*/
/*	采样器 状态控制数据	*/
static	struct	
{
	BOOL	Clean;
	BOOL	Pause;
	BOOL	Fatal;
} SampleSwitch[SP_Max];

/**
	采样任务控制
*/
#include "Sample_TSP.C"
#include "Sample_R24A.C"
#include "Sample_R24B.C"
#include "Sample_SHIC.C"
#include "Sample_SHID.C"
// #include "Sample_AIR.C"


/********************************** 功能说明 ***********************************
*	套接调用采样定时程序
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
	
	(HandlerList[SamplerID])();	//	调用采样控制程序，然后结束任务

	osThreadTerminate( osThreadGetId());
}

static	void	TaskCreate_Sample( enum enumSamplerSelect SamplerSelect ) 
{
	static	osThreadDef( funSocket, osPriorityAboveNormal, SP_Max, 0 );

	osThreadCreate( osThread( funSocket ), (void * )SamplerSelect );
}

/********************************** 功能说明 ***********************************
*  采样任务的启动、暂停/恢复、停止控制，以及电机故障控制
*******************************************************************************/
void Sample_R( enum enumSamplerSelect	SamplerSelect )
{
	static	struct uMenu menu_TSP[] =
	{
		{ 0x0102u, "恢复粉尘采样?" },
		{ 0x0F06u, "是" }, { 0x0F15u, "否" },
 	};
	static	struct uMenu menu_R24A[] =
	{
		{ 0x0102u, "恢复日均A采样?" },
		{ 0x0F06u, "是" }, { 0x0F15u, "否" },
 	};
		static	struct uMenu menu_R24B[] =
	{
		{ 0x0102u, "恢复日均B采样?" },
		{ 0x0F06u, "是" }, { 0x0F15u, "否" },
 	};
	static	struct uMenu menu_SHIC[] =
	{
		{ 0x0102u, "恢复时均C采样?" },
		{ 0x0F06u, "是" }, { 0x0F15u, "否" },
 	};	
	static	struct uMenu menu_SHID[] =
	{
		{ 0x0102u, "恢复时均D采样?" },
		{ 0x0F06u, "是" }, { 0x0F15u, "否" },
 	};
// 	static	struct uMenu menu_AIR[] =
// 	{
// 		{ 0x0102u, "恢复大气采样?" },
// 		{ 0x0F06u, "是" }, {0x0F15u, "否" },
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
	 

 
//	采样恢复：上电时，检查有无未完成的采样任务，如果有则恢复采样。
void	Sampler_BootResume( void )
{  
	//	应根据仪器型号来确定有哪些采样需要开启
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

//	启动键处理：用户启动采样
void	Sampler_Start( enum enumSamplerSelect SamplerSelect )
{
	if ( ! Sampler_isRunning( SamplerSelect ))
	{
		if (( 0u != SampleSet[SamplerSelect].sample_time ) && ( 0u != SampleSet[SamplerSelect].set_loops ))
		{			
			SampleSwitch[SamplerSelect].Clean = FALSE;
			SampleSwitch[SamplerSelect].Pause = FALSE;
			SampleSwitch[SamplerSelect].Fatal = FALSE;
			
			SampleSet[SamplerSelect].iloop = 0u;				//	采样未初始化标志
			SampleSet[SamplerSelect].start = get_Now() / 60u;	//	记录采样启动时间
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
//	暂停键处理：用户暂停 或者 恢复 采样任务
void Sampler_PauseKey( enum enumSamplerSelect SamplerSelect )
{
	if ( SampleSwitch[SamplerSelect].Pause || SampleSwitch[SamplerSelect].Fatal ) 
	{	//	清除采样故障，尝试恢复采样
		SampleSwitch[SamplerSelect].Pause = FALSE;
		SampleSwitch[SamplerSelect].Fatal = FALSE;
	}	
	else
	{
		SampleSwitch[SamplerSelect].Pause = TRUE;
	}
}


//	停止键处理：用户提前中止采样任务
void	Sample_Terminate( enum enumSamplerSelect SamplerSelect )
{	
	SampleSwitch[SamplerSelect].Clean = TRUE;
}

//	故障处理：通知任务任务有致命故障发生，采样任务应提前中止，不应继续执行。
void	Sample_Fatal( enum enumSamplerSelect SamplerSelect )
{
	SampleSwitch[SamplerSelect].Fatal = TRUE; 
	SampleSwitch[SamplerSelect].Clean = TRUE;
}

//	故障处理：通知任务任务有致命故障发生，采样任务应暂停，用户可选择恢复。
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

/********  (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司  **** End Of File ****/
