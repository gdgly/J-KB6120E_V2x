/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: Setup.C
* �� �� ��: Dean
* ��  ��  : KB-6120E �������ó���
* ����޸�: 2013��12��18��
*********************************** �޶���¼ ***********************************
* ��  ��: 
* �޶���: 
*******************************************************************************/
#include "AppDEF.H"
#include "BMP.H"
/********************************** ���ݶ��� **********************************/
struct	uSampleSet	SampleSet[SP_Max];	//	��������
extern  void	Show_std_clock( void );
/********************************** ����˵�� ***********************************
*  ���ò�������Ŀ��Ʋ�����ԭ�������úõĲ�����ÿ�β�������Ҫ�䶯��
*******************************************************************************/
static	BOOL	setup_x( enum	enumSamplerSelect SamplerSelect, struct uMenu const menu[] )	//SHI\R24
{
	uint8_t	item = 0u;
	BOOL	changed = FALSE;
	do {
	
		Part_cls();
		Menu_Redraw( menu );
	
		ShowTIME( 0x061Cu, SampleSet[SamplerSelect].sample_time );
		ShowTIME( 0x0C1Cu, SampleSet[SamplerSelect].suspend_time );
		ShowI16U( 0x121Cu, SampleSet[SamplerSelect].set_loops, 0x0200u, NULL );
		ShowI16U( 0x181Cu, Configure.SetFlow[SamplerSelect], 0x0301u, "L/m" );
		
		item = Menu_Select2( menu, item + 1u, FALSE, Show_std_clock );

		switch( item )
		{	
		case 1:	if ( EditTIME( 0x061Cu, &( SampleSet[SamplerSelect].sample_time )))					{ changed = TRUE; }	break;
		case 2:	if ( EditTIME( 0x0C1Cu, &( SampleSet[SamplerSelect].suspend_time )))				{ changed = TRUE; }	break;
		case 3:	if ( EditI16U( 0x121Cu, &( SampleSet[SamplerSelect].set_loops ), 0x0200u ))	{ changed = TRUE; }	break;
		case 4:	if ( EditI16U( 0x181Cu, &( Configure.SetFlow[SamplerSelect] ), 0x0301u ))
						{
							if( Configure.SetFlow[SamplerSelect] < 1 )
								Configure.SetFlow[SamplerSelect] = 1;
							if( Configure.SetFlow[SamplerSelect] > 10 )
								 Configure.SetFlow[SamplerSelect] = 10;
								ConfigureSave();
						}	
						break;	
			
		case enumSelectXCH:		SamplerTypeSwitch();	return	FALSE;	//	���ص��ϼ��˵����л�������������
		}
		
	} while( enumSelectESC != item );

	if ( changed )
	{
		SampleSetSave();
	}
	return	TRUE;
}

static	BOOL	setup_TSP( enum	enumSamplerSelect SamplerSelect, struct uMenu const menu[] )
{
	uint8_t	item = 0u;
	BOOL	changed = FALSE;
// 	BOOL	need_redraw = TRUE;

	do {
// 		if ( need_redraw )
// 		{
			Part_cls();
			Menu_Redraw( menu );
			
// 			need_redraw = FALSE;
// 		}
		
		ShowTIME( 0x081Cu, SampleSet[SamplerSelect].sample_time );
		ShowTIME( 0x101Cu, SampleSet[SamplerSelect].suspend_time );
		ShowI16U( 0x181Cu, SampleSet[SamplerSelect].set_loops, 0x0200u, NULL );

		item = Menu_Select2( menu, item + 1u, FALSE, Show_std_clock );

		switch( item )
		{
		case 1:	if ( EditTIME( 0x081Cu, &( SampleSet[SamplerSelect].sample_time )))					{ changed = TRUE; }	break;
		case 2:	if ( EditTIME( 0x101Cu, &( SampleSet[SamplerSelect].suspend_time )))				{ changed = TRUE; }	break;
		case 3:	if ( EditI16U( 0x181Cu, &( SampleSet[SamplerSelect].set_loops ), 0x0200u ))	{ changed = TRUE; }	break;	
		
		case enumSelectXCH:		SamplerTypeSwitch();	return	FALSE;	//	���ص��ϼ��˵����л�������������
		}
		
	} while ( enumSelectESC != item );

	if ( changed )
	{
		SampleSetSave();
	}
	return	TRUE;
}




void	menu_SampleSetup( void )
{
	static	struct uMenu  const  menu_x[] =
	{
		{ 0x0401u, "��������" },
		{ 0x060Eu, "����ʱ��" },
		{ 0x0C0Eu, "���ʱ��" },
		{ 0x120Eu, "��������" },
		{ 0x180Eu, "��������" },
	};
	
	static	struct uMenu  const  menu_TSP[] =
	{
		{	0x0301u, "��������" },
		{ 0x080Eu, "����ʱ��" },
		{ 0x100Eu, "���ʱ��" },
		{ 0x180Eu, "��������" },
	};

	BOOL	done = FALSE;

	do {
		monitor();
		SamplerTypeShow( 0x010Fu );
		switch ( SamplerSelect )
		{		
		case SP_TSP:		done = setup_TSP( SamplerSelect, menu_TSP );	break;
		case SP_R24_A:	
		case SP_R24_B:	
		case SP_SHI_C:	
		case SP_SHI_D:	done = setup_x( SamplerSelect, menu_x );	break;
		default:
			break;
		}
	} while( ! done );
}			

/********************************** ����˵�� ***********************************
*  �������������Ŀ��Ʋ�����ԭ����ÿ�β���ǰ��Ҫ�������õĲ���������������
*******************************************************************************/
static	BOOL	SampleStart( enum enumSamplerSelect SamplerSelect )
{
	static	struct uMenu  const  menu_Accurate[] =
	{
		{ 0x0301u, "��������" },
		{ 0x100Eu, "��ʼʱ��" },
		{ 0x1814u, "����" }, 
		{ 0x181Cu, "����" },
	};

	static	struct uMenu  const  menu_Delay[] =
	{
		{ 0x0301u, "��������" },
		{ 0x100Eu, "��ʱʱ��" },
		{ 0x1814u, "����" }, 
		{ 0x181Cu, "����" },
	};
	static	struct uMenu  const  menuL[] =
	{
		{ 0x0401u, "��������" },
		{ 0x0606u, "����" },	//	����Ĭ�ϲ���������ԭ���ϲ���Ҫÿ�ζ��޸�
		{ 0x0C06u, "����" },	//	����ÿ�β�������Ҫȷ�ϵĲ���������������
		{ 0x1206u, "��ѯ" },	//	�鿴��Ӧ�������Ĳ�����¼�ļ�������ӡ����
		{ 0x1806u, "ά��" }
	};

	static	struct uMenu const * menu;
	uint8_t	item = 1u;
	BOOL	need_redraw = TRUE;
	
	BOOL	changed_Delay = FALSE;
	BOOL	changed = FALSE;

	
		
	do {
		switch( Configure.Mothed_Delay )
		{
		case enumByAccurate:	menu = menu_Accurate;	break;
		case enumByDelay:			menu = menu_Delay;		break;
		default:
			break;
		}
		if ( need_redraw )
		{
			cls();			
			Menu_Redraw( menuL );
			SamplerTypeShow( 0x010Fu );
			WBMP( 0x1818, menuL[1].yx-0x04, SAMPLESET );
			WBMP( 0x1818, menuL[2].yx-0x04, SAMPLE );
			WBMP( 0x1818, menuL[3].yx-0x04, QUERY );
			WBMP( 0x1818, menuL[4].yx-0x04, MAINTAIN );
			WBMP( 0xB001, 0x050D, STROCK1 );
			Menu_Redraw( menu );
			need_redraw = FALSE;
		}

		Menu_Item_Mask( menu, item );
		do {
			uint16_t	now_hour_min = (( get_Now() / 60 ) % 1440 );
		
			Show_std_clock();
			Lputs( 0x080Eu, "��ǰʱ��=" );
			ShowTIME( 0x081Cu, now_hour_min );	//	��ʾ��ǰʱ��
			if ( enumByAccurate == Configure.Mothed_Delay )
			{
				if ( ! changed_Delay )
				{
					SampleSet[SamplerSelect].delayt = now_hour_min + 1u;
				}
			}
			ShowTIME( 0x101Cu, SampleSet[SamplerSelect].delayt );
		} while ( ! hitKey( 100u ));
		Menu_Item_Mask( menu, item );

		item = Menu_Select2( menu, item, FALSE, Show_std_clock );

		switch( item )
		{
		case 1:
			if ( EditTIME( 0x101Cu, &( SampleSet[SamplerSelect].delayt )))
			{
				changed_Delay = TRUE;
				changed = TRUE;
			}		
			++item;
			break;
		case 2:
			need_redraw = TRUE;
			CalibrateZero_x( SamplerSelect );
			++item;
			//lint -fallthrough
			break;
		case 3:	//	��������
			if ( changed )
			{
				SampleSetSave();
			}
			Sampler_Start ( SamplerSelect );
			
			return	FALSE;	//	������ʾ����״̬

		case enumSelectXCH:
			SamplerTypeSwitch();
			return	FALSE;
		case	enumSelectESC:
			break;
		}
	} while ( enumSelectESC != item );
	
	return	TRUE;	//	���أ���ʾ�ϼ��˵�
}

// void	State_Finish( enum enumSamplerSelect SamplerSelect )
// {

// 	if( (	Q_Sampler[SamplerSelect].state	== state_FINISH ) && SampleShowState[SamplerSelect] )
// 	{
// 		SampleShowState[SamplerSelect] = FALSE;
// 		cls();
// 	// 	SamplerTypeShow( 0x0102u );
// 		WBMP( 0x0290,0x0502, STROCK);
// 		WBMP( 0x0290,0x0514, STROCK);

// 		switch( SamplerSelect )
// 		{
// 		case	SP_TSP: Lputs( 0x0102,  "TSP����");	break;
// 		case	SP_R24ASP_R24B: Lputs( 0x0102, "�վ�����");	break;
// 		case	SP_SHI_CSP_SHI_D: Lputs( 0x0102, "ʱ������");	break;
// 		case	Q_AIR: Lputs( 0x0102, "��������");	break;
// 		}
// 		Lputs( 0x0A0C,	"�������!");
// 		Lputs( 0x0F03,  "��ȷ�ϼ���ѯ�������!");
// 		do
// 		{
// 			Show_std_clock();
// 		}while( !hitKey( 50 ) );

// 		switch( getKey() )
// 		{
// 		case K_OK:	menu_SampleQuery();	break;
// 		default:	break;
// 		}
// 	}
// }
/********************************** ����˵�� ***********************************
*  ���˵��������ó���Ľӿ�
*******************************************************************************/
void	menu_SampleStart( void )
{
	BOOL	done = FALSE;
	
	do {
		monitor();
// 		State_Finish(	SamplerSelect );
		done = SampleStart( SamplerSelect );
	} while ( ! done );
}

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/
// void	SetupFlow_SHI( void )
// {
// 	static	struct uMenu  const  menu[] =
// 	{
// 		{ 0x0201u, "��������" },
// 		{ 0x0C0Eu, "����C" },
// 		{ 0x140Eu, "����D" },
// 	};
// 	static	struct uMenu  const  menuL[] =
// 	{
// 		{ 0x0401u, "      " },
// 		{ 0x0606u, "����" },	//	����Ĭ�ϲ���������ԭ���ϲ���Ҫÿ�ζ��޸�
// 		{ 0x0C06u, "����" },	//	����ÿ�β�������Ҫȷ�ϵĲ���������������
// 		{ 0x1206u, "��ѯ" },	//	�鿴��Ӧ�������Ĳ�����¼�ļ�������ӡ����
// 		{ 0x1806u, "ά��" }
// 	};
// 	uint8_t item = 0u;
// 	
// 	uint16_t	cflow = Configure.SetFlow[SP_SHI_C];
// 	uint16_t	dflow = Configure.SetFlow[SP_SHI_D];
// 	BOOL	changed = FALSE;
// 	BOOL	need_redraw = TRUE;
// 	
// 	do {
// 		if ( need_redraw )
// 		{
// 			cls();			
// 			Menu_Redraw( menuL );
// 			SamplerTypeShow( 0x010Eu );
// 			WBMP( 0x1818, menuL[1].yx-0x04, SAMPLESET );
// 			WBMP( 0x1818, menuL[2].yx-0x04, SAMPLE );
// 			WBMP( 0x1818, menuL[3].yx-0x04, QUERY );
// 			WBMP( 0x1818, menuL[4].yx-0x04, MAINTAIN );
// 			Part_cls();
// 			Menu_Redraw( menu );
// 			need_redraw = FALSE;
// 		}
// 		ShowI16U( 0x0C1Au, cflow, 0x0301u, "L/m" );
// 		ShowI16U( 0x141Au, dflow, 0x0301u, "L/m" );

// 		item = Menu_Select2( menu, item + 1u, FALSE, Show_std_clock );
// 		switch ( item )
// 		{
// 		case 1:
// 			if ( EditI16U( 0x0C1Au, &cflow, 0x0301u ))
// 			{
// 				switch ( Configure.PumpType[SP_SHI_C] )
// 				{
// 				case enumOrifice_1:
// 					if ( cflow <  1 )  { cflow =  1u; }
// 					if ( cflow > 10u ) { cflow = 10u; }
// 					break;
// 				case enumOrifice_2:
// 					if ( cflow <  5u ) { cflow =  5u; }
// 					if ( cflow > 20u ) { cflow = 20u; }
// 					
// 					break;
// 				default:
// 					break;
// 				}
// 				changed = TRUE;
// 			}
// 			break;
// 		case 2:
// 			if ( EditI16U( 0x141Au, &dflow, 0x0301u ))
// 			{
// 				switch ( Configure.PumpType[SP_SHI_D] )
// 				{
// 				case enumOrifice_1:
// 					if ( dflow <  1u ) { dflow =  1u; }
// 					if ( dflow > 10u ) { dflow = 10u; }
// 					break;
// 				case enumOrifice_2:
// 					if ( dflow <  5u ) { dflow =  5u; }
// 					if ( dflow > 20u ) { dflow = 20u; }

// 					break;
// 				default:
// 					break;
// 				}
// 				changed = TRUE;
// 			}
// 			break;
// 		case enumSelectESC:
// 			if ( changed )
// 			{
// 				Configure.SetFlow[SP_SHI_C] = cflow;
// 				Configure.SetFlow[SP_SHI_D] = dflow;               
// 				switch( MsgBox( "������������?", vbYesNoCancel + vbDefaultButton3 ))
// 				{
// 				case vbYes:
// 						ConfigureSave();
// 						break;
// 				case vbNo:
// 						ConfigureLoad();
// 						break;
// 				case vbCancel:
// 						item = 1u;
// 						break;
// 				default:
// 						break;
// 				}
// 			}
// 			need_redraw = TRUE;
// 			break;
// 		default:
// 			break;
// 		}
// 	} while ( enumSelectESC != item );
// 	
// }

#define	LOBYTE(__w)		((uint8_t)((__w)&0x0FF))
#define	HIBYTE(__w)		((uint8_t)(((__w)>>8)&0x0FF))

// static	BOOL	setup_R24( struct uMenu  const  menu[], const CHAR * szPrompt )
// {
// 	uint8_t	item = 0u;
// 	BOOL	changed = FALSE;
// 	BOOL	need_redraw = TRUE;
// 	static	struct uMenu  const  menuL[] =
// 	{
// 		{ 0x0401u, "      " },
// 		{ 0x0606u, "����" },	//	����Ĭ�ϲ���������ԭ���ϲ���Ҫÿ�ζ��޸�
// 		{ 0x0C06u, "����" },	//	����ÿ�β�������Ҫȷ�ϵĲ���������������
// 		{ 0x1206u, "��ѯ" },	//	�鿴��Ӧ�������Ĳ�����¼�ļ�������ӡ����
// 		{ 0x1806u, "ά��" }
// 	};

// 	do {
// 		if ( need_redraw )
// 		{
// 			cls();			
// 			Menu_Redraw( menuL );
// 			SamplerTypeShow( 0x010Eu );
// 			WBMP( 0x1818, menuL[1].yx-0x04, SAMPLESET );
// 			WBMP( 0x1818, menuL[2].yx-0x04, SAMPLE );
// 			WBMP( 0x1818, menuL[3].yx-0x04, QUERY );
// 			WBMP( 0x1818, menuL[4].yx-0x04, MAINTAIN );
// 			Part_cls();
// 			Menu_Redraw( menu );
// 			need_redraw = FALSE;
// 		}
// 		
// 		ShowTIME( 0x061Cu, SampleSet[SamplerSelect].sample_time );

// 		ShowTIME( 0x0F1Cu, SampleSet[SamplerSelect].suspend_time );
// 		Lputs(0x140Eu, szPrompt );
// 	
// 		ShowI16U( 0x1816u, SampleSet[SamplerSelect].set_loops, 0x0200u, NULL );

// 		item = Menu_Select2( menu, item + 1u, FALSE, Show_std_clock );

// 		switch ( item )
// 		{
// 		case 1:
// 			if ( EditTIME( 0x061Cu, &SampleSet[SamplerSelect].sample_time ))
// 			{
// 				if ( SampleSet[SamplerSelect].sample_time < SampleSet[SamplerSelect].sample_time )
// 				{
// 					SampleSet[SamplerSelect].sample_time = SampleSet[SamplerSelect].sample_time;
// 					beep();
// 				}
// 				else
// 				{
// 					changed = TRUE;
// 				}
// 			}
// 			break;
// 		case 2:
// 			if ( EditTIME( 0x0A1Cu, &SampleSet[SamplerSelect].sample_time ))
// 			{
// 				if ( SampleSet[SamplerSelect].sample_time < SampleSet[SamplerSelect].sample_time )
// 				{
// 					SampleSet[SamplerSelect].sample_time = SampleSet[SamplerSelect].sample_time;
// 					beep();
// 				}
// 				else
// 				{
// 					changed = TRUE;
// 				}
// 			}
// 			break;
// 		case 3:
// 			if ( EditTIME( 0x0F1Cu, &SampleSet[SamplerSelect].suspend_time ))
// 			{
// 				changed = TRUE;
// 			}
// 			break;

// 		case 4:
// 			if ( EditI16U( 0x1816u, &( SampleSet[SamplerSelect].set_loops ), 0x0200u ))
// 			{
// 				changed = TRUE;
// 			}
// 			break;
// 			
// 		case 5:
// 		//	���� 4·С����������������C/D ʱ�� ����������A/B������������Թ̶��ģ�
// 		//	���� 2·С����������������A/B ��������
// 			SetupFlow_SHI();
// 			need_redraw = TRUE;
// 			break;
// 		
// 		case enumSelectXCH:
// 			SamplerTypeSwitch();
// 			return	FALSE;
// 		}
// 	} while ( enumSelectESC != item );

// 	if ( changed )
// 	{
// 		SampleSetSave();
// 	}
// 	return	TRUE;
// }
