/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: Configure.C
* �� �� ��: ����
* ��  ��  : KB-6120E ���ó���(�����û����õĲ���)
* ����޸�: 2014��8��25��
*********************************** �޶���¼ ***********************************
* ��  ��:
* �޶���:
*******************************************************************************/
#include "AppDEF.H"

struct	uConfigure	Configure;

/********************************** ����˵�� ***********************************
*  ����ʱ�䡢����
*******************************************************************************/
static	void	menu_SetupClock( void )
{
    static  struct  uMenu  const  menu[] =
    {
        { 0x0102u, "ʱ������" },
        { 0x0810u, "��������" },
        { 0x1210u, "����ʱ��" },
    };
    enum
    {
        opt_exit, opt_date, opt_time,
        opt_max, opt_min = 1
    };
    uint8_t	option = opt_min;
		uint16_t gray  = Configure.DisplayGray;
		BOOL	graychanged = FALSE;

    uClock standard;

    Part_cls();
    Menu_Redraw( menu );
    do
    {
        Menu_Item_Mask( menu, option );
        do
        {
            RTC_Load( &standard );
            ShowClockDate(0x0C11u, &standard );
            ShowClockTime(0x1611u, &standard );
        }
        while ( ! hitKey( 25u ));
        Menu_Item_Mask( menu, option );

        switch( getKey() )
        {
        case K_RIGHT:
            break;

        case K_LEFT:
            break;

        case K_DOWN:
            ++option;
            if ( option >= opt_max )
            {
                option = opt_min;
            }
            break;

        case K_UP:
            if ( option <= opt_min )
            {
                option = opt_max;
            }
            --option;
            break;

        case K_ESC:
            option = opt_exit;
            break;

        case K_OK:
            switch( option )
            {
            case opt_date:
                if ( EditClockDate(0x0C11u, &standard ))
                {
                    RTC_Save( &standard );
                }
                break;
            case opt_time:
                if ( EditClockTime(0x1611u, &standard ))
                {
                    RTC_Save( &standard );
                }
                break;
            default:
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
					graychanged = FALSE;
				}		

    }
    while( opt_exit != option );
}

static	void	menu_ChangePassword( void )
{
 BOOL Done = FALSE;
 
 do{
		Part_cls();
		Lputs( 0x0B10u, "������������:" );
		if ( EditI32U( 0x1214u, &Configure.Password, 0x0600u ))
		{			 
			 MsgBox( "�����޸ĳɹ�!", vbOKOnly );
			 cls();
			 ConfigureSave();
		}
		Done = TRUE;
 }while( !Done );
 
}
void	menu_SetDisplay( void )
{
	static	struct  uMenu  const  menu[] =
	{
		{ 0x0301u, "��ʾ����" },
		{ 0x0810u, "�Ҷ�" },
		{ 0x1010u, "����" },
		{ 0x1810u, "��ʱ" },
	};
	uint8_t item = 1u;
	
	uint16_t gray  = Configure.DisplayGray;
	uint16_t light = Configure.DisplayLight;
	uint8_t  ltime = Configure.TimeoutLight;
	BOOL	changed = FALSE;
	uint32_t Gray;
	Part_cls();
	Menu_Redraw( menu );
	Gray = (FP32)(gray * 10) / 22u;
	do {
		ShowI16U( 0x0818u, Gray,  0x0401u, "% " );
		ShowI16U( 0x1018u, light, 0x0300u, "% " );
		switch ( ltime )
		{
		case 0:	Lputs( 0x1817u, "[�ر�] " );	break;
		case 1:	Lputs( 0x1817u, "[15��] " );	break;
		case 2:	Lputs( 0x1817u, "[30��] " );	break;
		case 3:	Lputs( 0x1817u, "[60��] " );	break;
		default:
		case 4:	Lputs( 0x1817u, "[����] " );	break;
		}
		item = Menu_Select( menu, item, NULL );

		switch( item )
		{
		case 1:	
			if ( EditI32U( 0x0818u, &Gray, 0x0401u ))
			{
				if ( Gray > 1000u ){ Gray = 1000u; }
				if ( Gray <  1u ){ Gray =  1u; }
				DisplaySetGrayVolt( Gray * 0.022f );
				changed = TRUE;
			}
			break;
		case 2:
			if ( EditI16U( 0x1018u, &light, 0x0300u ))
			{
				if ( light > 100u ){ light = 100u; }
				DisplaySetLight( light );
				changed = TRUE;
			}
			break;
		case 3:	
			if ( ++ltime > 4 ){  ltime = 0u; }
			DisplaySetTimeout( ltime );
			changed = TRUE;
			break;
		}
		
	} while ( enumSelectESC != item );
	
	if ( changed )
	{
		Configure.DisplayGray  = Gray * 22 /10;
		Configure.DisplayLight = light;
		Configure.TimeoutLight = ltime;
		ConfigureSave();
	}
}

void	Instrument_Comments( void )
{
	Lputs( 0x080F,"����ʱ������");
	Lputs( 0x0C0F,"ά��������޸�");
	Lputs( 0x100F,"�Լ�Һ����Ļ��");
	Lputs( 0x140F,"��ʾ�Աȶ� ����");
	Lputs( 0x180F,"������ʱ��");
}

void	Instrument_Set( void )
{
	uint8_t	item = 1u;
	static	struct uMenu  const  menu[] =
	{
			{ 0x0301u, "ά��" },
			{ 0x0901u, "ʱ������" },
			{ 0x0F01u, "��������" },
			{ 0x1501u, "��ʾ����" },
	};
	cls();
	do{		
		Part_cls();
		Menu_Redraw( menu );
		item = Menu_Select( menu, item, Instrument_Comments );	
		switch( item )
		{
		case 1:
			menu_SetupClock();
				break;
		case 2:
			menu_ChangePassword();
				break;
		case 3:
			menu_SetDisplay();
			break;
		default:
				break;
		}

	}
	while( enumSelectESC != item );
}





void menu_SamplerSum( void )
{
	static	struct uMenu  const  menu[] =
	{
			{ 0x0102u, "������������¼" },
			{ 0x1A06u, "�˳�" }, { 0x1A18u, "���" }
	};
	uint8_t	item = 1u;
	uint8_t i;
	static	char	* Table;
	static	char Table2[] = { 0x0D, 0x13, };
	static	char Table3[] = { 0x0C, 0x10, 0x14, };
	static	char Table4[] = { 0x0B, 0x0E, 0x11, 0x14, };
	static	char Table5[] = { 0x0A, 0x0D, 0x10, 0x13, 0x16, };
	static	char	* const	TableSampler[] = 
	{
		"�� ��:",
		"�վ�A:",
		"�վ�B:",
		"ʱ��C:",
		"ʱ��D:",
	};
	switch( SamplerHasMax  )
	{
	case 2: Table = Table2;	break;
	case 3:	Table = Table3;	break;
	case 4:	Table = Table4;	break;
	case 5:	Table = Table5;	break;
	default:
		break;
	}
	do
	{
		cls();			
		Lputs( 0x0605u, "������" );		 Lputs( 0x0616u, "��������" );
		for( i = 0; i < SamplerHasMax; i ++ )
		{
			Lputs		( (Table[i] * 256 + 0x06 ), TableSampler[SamplerTypeHas[i]] );
			ShowI16U( (Table[i] * 256 + 0x12 ), SampleSet[(enum enumSamplerSelect)SamplerTypeHas[i]].FileNum, 0x0400u, " ��" );
		}
		Menu_Redraw( menu );

		item = Menu_Select( menu, item, NULL );
		switch( item )
		{
		case 1:
			item = enumSelectESC;
			break;
		case 2:
			beep();
			if ( vbYes == MsgBox( "��������ļ� ?", vbYesNo | vbDefaultButton2 ) )
			{
				File_Clean();
				SampleSet[SP_TSP].FileNum =
				SampleSet[SP_R24_A].FileNum =
				SampleSet[SP_R24_B].FileNum =
				SampleSet[SP_SHI_C].FileNum =
				SampleSet[SP_SHI_D].FileNum = 0u;
				SampleSetSave();
			}
			break;
		default:
			break;
		}
	}while( enumSelectESC != item );

}



/********************************** ����˵�� ***********************************
*  ���ñ�������Ķ����¶�
*******************************************************************************/
static	void	menu_SelectTstd( void )
{
    static  struct  uMenu  const  menu[] =
    {
        { 0x0301u, "ѡ�����¶�" },
        { 0x080Au, " 0��(273K)" },
        { 0x100Au, "20��(293K)" },
        { 0x180Au, "25��(298K)" }
    };
    uint8_t	item;

    uint8_t	SetTstd = Configure.SetTstd;
    BOOL	changed = FALSE;

    cls();
    Menu_Redraw( menu );

    do
    {
        switch ( SetTstd )
        {
        default:
        case enum_273K:
            Lputs( 0x0802u,"��"	);
            Lputs( 0x1002u,"  "	);
            Lputs( 0x1802u,"  "	);
            item = 1;
            break;
        case enum_293K:
            Lputs( 0x0802u,"  "	);
            Lputs( 0x1002u,"��"	);
            Lputs( 0x1802u,"  "	);
            item = 2;
            break;
        case enum_298K:
            Lputs( 0x0802u,"  "	);
            Lputs( 0x1002u,"  "	);
            Lputs( 0x1802u,"��"	);
            item = 3;
            break;
        }
        item = Menu_Select( menu, item, NULL );
        switch ( item )
        {
        case 1:
            SetTstd = enum_273K;
            changed = TRUE;
            break;
        case 2:
            SetTstd = enum_293K;
            changed = TRUE;
            break;
        case 3:
            SetTstd = enum_298K;
            changed = TRUE;
            break;
        default:
            break;
        }
    }
    while( enumSelectESC != item );

    if ( changed )
    {
        Configure.SetTstd = SetTstd;
        ConfigureSave();
    }
}
/********************************** ����˵�� ***********************************
*  ϵͳ���� -> ������
*******************************************************************************/
static	void	Configure_HCBox( void )
{
    static  CHAR  const  MDS[][7] =
    {
        { "[ͣ��]" }, { "[����]" }, { "[����]" } , { "[�Զ�]" }
    };
    static	struct	uMenu	const	menu[] =
    {
        {	0x0201u, "���ú�����"	},
        {	0x0C02u, "���Ʒ�ʽ"	},
        {	0x1402u, "�����¶�"	},
    };
    uint8_t 	item = 1u;

    uint8_t		SetMode = Configure.HCBox_SetMode;
    uint16_t	SetTemp = Configure.HCBox_SetTemp;
    BOOL		changed = FALSE;
    BOOL		need_redraw = TRUE;

    do
    {
        if ( need_redraw )
        {
            cls();
            Menu_Redraw( menu );
            need_redraw = FALSE;
        }

        Lputs( 0x0C16U, MDS[SetMode] );
        ShowI16U( 0x1414U, SetTemp, 0x0501U, "��" );

        item = Menu_Select( menu, item, NULL );
        switch( item )
        {
        case 1u:	/*	EditBoxMode	*/
            SetMode = ( SetMode + 1u) % 4u;
            changed = TRUE;
            break;

        case 2u:
            /* ע��: ��Ϊ������������¶ȷ�Χ������, ���Բſ���ʹ���޷������༭������б༭ */
            if ( EditI16U( 0x1414U, &SetTemp, 0x0501U ))
            {
                if ( SetTemp <  150u )
                {
                    SetTemp = 150u;
                    beep();
                }
                if ( SetTemp >  300u )
                {
                    SetTemp = 300u;
                    beep();
                }
                changed = TRUE;
            }
            break;

        case 0u:
            if ( changed )
            {
                need_redraw = TRUE;
                switch ( MsgBox( "������������ ?", vbYesNoCancel + vbDefaultButton3 ))
                {
                case vbYes:
                    Configure.HCBox_SetMode = SetMode;
                    Configure.HCBox_SetTemp = SetTemp;
                    ConfigureSave();
                    set_HCBoxTemp( Configure.HCBox_SetTemp * 0.1f, Configure.HCBox_SetMode );
                    break;
                case vbNo:
                    ConfigureLoad();
                    break;
                case vbCancel:
                    item = 1u;
                    break;
                default:
                    break;
                }
            }
            break;

        default:
            break;
        }
    }
    while ( enumSelectESC != item );
}
static	void	Configure_Heater( void )
{
    static	struct	uMenu	const	menu[] =
    {
        {	0x0201u, "���ü�����"	},
        {	0x0C02u, "��  ��"	},
				{	0x1802u, "�����¶�"	},
    };
		 static	struct	uMenu	const	menu1[] =
    {
        {	0x0101u, "���ü�����"	},
				{	0x0C02u, "��  ��"	},
    };
static	CHAR  const * const HeaterSW[] = {"[ͣ��]", "[����]" };
			
    uint8_t 	item = 1u;
		uint8_t 		state  = Configure.Heater_SW;          //	����������
    uint16_t	SetTemp = Configure.Heater_SetTemp;			//	�����������¶�
    BOOL		changed = FALSE;
    BOOL		need_redraw = TRUE;
   
    do {
        if ( need_redraw )
        {
            cls();
            if( state == FALSE )
							Menu_Redraw( menu1 );
						else
							Menu_Redraw( menu );
            need_redraw = FALSE;
        }
				if( state == TRUE )
					ShowI16U( 0x0C14U, SetTemp, 0x0501U, "��" );
        Lputs( 0x0C19, HeaterSW[state] );
       if( state == FALSE )
					item = Menu_Select( menu1, item, NULL );
				else
					item = Menu_Select( menu, item, NULL );
        switch( item )
        {
        case 1u:
					need_redraw = TRUE;
					state = ! state; 
					changed = TRUE;
					break;
         case 2u: /* ע��: ��Ϊ������������¶ȷ�Χ������, ���Բſ���ʹ���޷������༭������б༭ */
					if( state == TRUE )
					 if ( EditI16U( 0x1814U, &SetTemp, 0x0501U ))
						{
							if ( SetTemp <  150u )
							{
								SetTemp = 150u;
								beep();
							}
							if ( SetTemp >  350u )
							{
								SetTemp = 350u;
								beep();
							}
							changed = TRUE;
						}	
					need_redraw = TRUE;
					break;
        case 0u:
				if ( changed )
				{
						need_redraw = TRUE;
						switch ( MsgBox( "������������ ?", vbYesNoCancel + vbDefaultButton3 ))
						{
						case vbYes:
								Configure.Heater_SetTemp = SetTemp;
								Configure.Heater_SW = state;
								ConfigureSave();
								set_HeaterTemp( Configure.Heater_SetTemp * 0.1f );
								break;
						case vbNo:
								ConfigureLoad();
								break;
						case vbCancel:
								item = 1u;
								break;
						default:
								break;
						}
					}
					break;
        default:
					break;
        }
    }
    while ( enumSelectESC != item );
}

static	void	menu_ConfigureHCBox( void )
{
    switch ( Configure.HeaterType )
    {
    default:
    case enumHeaterNone:
        MsgBox( "δ��װ������", vbOKOnly );
        break;
    case enumHCBoxOnly:
        Configure_HCBox();
        break;
    case enumHeaterOnly:
        Configure_Heater();
        break;
    case enumHCBoxHeater:
        MsgBox( "Ӳ������֧��", vbOKOnly );
        break;
    }
}

/********************************** ����˵�� ***********************************
*	ϵͳ���� -> ����ѹ
*******************************************************************************/
static	void	menu_Configure_Ba( void )
{
    static const struct uMenu menu[] =
    {
        { 0x0301u, "���ô���ѹ" },
        { 0x0802u, "��ʽ" },
        { 0x1002u, "Ԥ�����ѹ" },
        { 0x1802u, "����:" }
    };
		static const struct uMenu menu1[] =
    {
        { 0x0101u, "���ô���ѹ" },
        { 0x0802u, "��ʽ" },
     
    };
    uint8_t item = 1u;
    BOOL	need_redraw = TRUE;

    uint8_t 	Mothed_Ba = Configure.Mothed_Ba;
    uint16_t	set_Ba    = Configure.set_Ba;
    BOOL	changed = FALSE;

    do
    {
        if ( need_redraw )
        {
            cls();
           if( Mothed_Ba == enumUserInput )
            Menu_Redraw( menu );
					else
						Menu_Redraw( menu1 );
            need_redraw = FALSE;
        }
				if( Mothed_Ba == enumUserInput )
					ShowI16U( 0x180Cu, set_Ba, 0x0602u, "kPa" );

        switch ( Mothed_Ba )
        {
        default:
        case enumUserInput:
            Lputs( 0x080Cu, "[Ԥ������]" );
            break;
        case enumMeasureBa:
            Lputs( 0x080Cu, "[ʵʱ����]" );
            break;
        }
        if( Mothed_Ba == enumUserInput )
					item = Menu_Select( menu, item, NULL );
				else
					item = Menu_Select( menu1, item, NULL );

        switch ( item )
        {
        case 0:
					if ( changed )
					{
							need_redraw = TRUE;
							switch( MsgBox( "�����޸Ľ�� ?", vbYesNoCancel + vbDefaultButton3 ))
							{
							case vbYes:
									Configure.Mothed_Ba = Mothed_Ba;
									Configure.set_Ba    = set_Ba;
									ConfigureSave();
									break;
							case vbNo:
									break;
							case vbCancel:
									item = 1u;
									break;
							default:
									break;
							}
					}
					break;

        case 1:
					switch ( Mothed_Ba )
					{
					default:
					case enumUserInput:
							Mothed_Ba = enumMeasureBa;
							break;
					case enumMeasureBa:
							Mothed_Ba = enumUserInput;
							break;
					}
					changed = TRUE;
					need_redraw = TRUE;	 
					break;
        case 2:
					if( Mothed_Ba == enumUserInput )
          {
						cls();
            Lputs( 0x0202u, "��������ѹ" );
            do
            {
                FP32 Ba = get_Ba();
                set_Ba = Ba * 100.0f + 0.5f;
                if ( set_Ba > 12000u )
                {
                    set_Ba = 12000u;
                }
                if ( set_Ba <  3000u )
                {
                    set_Ba =  3000u;
                }
                ShowI16U( 0x0C05u, set_Ba, 0x0602u, "kPa" );
            }
            while ( ! hitKey( 100u ));
            ( void ) getKey();
            changed = TRUE;
					}
            need_redraw = TRUE;
            break;
        case 3:
           if ( EditI16U( 0x180Cu, &set_Ba, 0x0602u ))
            {
                changed = TRUE;
            }
            if ( set_Ba > 12000u )
            {
                set_Ba = 12000u;
            }
            if ( set_Ba <  3000u )
            {
                set_Ba =  3000u;
            }break;

        default:
            break;
        }
    }
    while ( 0u != item );
}



static	void 	menu_Pr_Portect( void )
{
	uint8_t	item = 1u;
	uint8_t i;
	BOOL Changed = FALSE;
	static	char	* Table;
	static	char Table2[] = {
	0x0D, 
	0x13, };
	static	char Table3[] = {
	0x0C, 
	0x10, 
	0x14, };
	static	char Table4[] = { 
	0x0B, 
	0x0E, 
	0x11, 
	0x14, };
	static	char Table5[] = { 
	0x0A, 
	0x0D, 
	0x10, 
	0x13, 
	0x16, };

	static	struct	uMenu  menu2[] =
	{
		{ 0x0201u, "��ѹ����"},
		{ 0x0D05u, "ʱ��C"	},
		{ 0x1305u, "ʱ��D"	} ,
	};	
	static	struct	uMenu  menu3[] =
	{
		{ 0x0301u, "��ѹ����"},
		{ 0x0C05u, "�� ��"	},
		{ 0x1005u, "ʱ��C"	},
		{ 0x1405u, "ʱ��D"	} ,
	};	
	static	struct	uMenu  menu4[] =
	{
		{ 0x0401u, "��ѹ����"},
		{ 0x0B05u, "�վ�A"	},
		{ 0x0E05u, "�վ�B"	},
		{ 0x1105u, "ʱ��C"	},
		{ 0x1405u, "ʱ��D"	} ,
	};
	static	struct	uMenu  menu5[] =
	{
		{ 0x0501u, "��ѹ����"},
		{ 0x0A05u, "�� ��"	},
		{ 0x0D05u, "�վ�A"	},
		{ 0x1005u, "�վ�B"	},
		{ 0x1305u, "ʱ��C"	},
		{ 0x1605u, "ʱ��D"	} ,
	};
	static	struct	uMenu * menu[] =
	{
		menu2,
		menu3,
		menu4,
		menu5,
	};
	switch( SamplerHasMax  )
	{
	case 2: Table = Table2;	break;
	case 3:	Table = Table3;	break;
	case 4:	Table = Table4;	break;
	case 5:	Table = Table5;	break;
	default:
		break;
	}
	do
	{
		cls();			
		Lputs( 0x0605u, "������" );		 Lputs( 0x0614u, "��ѹֵ" );
		for( i = 0; i < SamplerHasMax; i ++ )
		{
			Lputs		( (Table[i] * 256 + 0x0D ), ":" );
			ShowI16U( (Table[i] * 256 + 0x12 ), Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[i]], 0x0402u, " KPa" );
		}
		Menu_Redraw( menu[SamplerHasMax-2] );

		item = Menu_Select( menu[SamplerHasMax-2], item, NULL );
		switch( item )
		{
		case 1:
			if(  EditI16U( ( Table[0] * 256 + 0x12 ), &Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[0]], 0x0402 ))
			{
// 				if( SamplerSelect == SP_TSP )
// 				{
// 					if( Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[0]] > 500 )
// 						Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[0]] = 500;
// 				}
// 				else
// 				{
// 					if( Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[0]] > 1000 )
// 						Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[0]] = 1000;						
// 				}
				Changed = TRUE;
			}
			break;
		case 2:
			if(  EditI16U( ( Table[1] * 256 + 0x12 ), &Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[1]], 0x0502 ))
			{
				Changed = TRUE;
			}
			break;
		case 3:
			if(  EditI16U( ( Table[2] * 256 + 0x12 ), &Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[2]], 0x0502 ))
			{
				Changed = TRUE;
			}
			break;
		case 4:
			if(  EditI16U( ( Table[3] * 256 + 0x12 ), &Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[3]], 0x0502 ))
			{
				Changed = TRUE;
			}
			break;
		case 5:
			if(  EditI16U( ( Table[4] * 256 + 0x12 ), &Configure.Pr_Portect[(enum enumSamplerSelect)SamplerTypeHas[4]], 0x0502 ))
			{
				Changed = TRUE;
			}
			break;
		default:
			break;
		}		
	}while( enumSelectESC != item );
	
	if( Changed == TRUE )
	{
		switch( MsgBox( "�����޸Ľ����",vbYesNoCancel | vbDefaultButton3 ) )
		{
		case vbYes:
			ConfigureSave();
			break;
		case vbNo:
			ConfigureLoad();
			break;
		case vbCancel:
			break;
		}
	}
}


/********************************** ����˵�� ***********************************
*  ϵͳ���� -> ������������ʱ��ʽ
*******************************************************************************/
static	void	menu_SelectDelayMode( void )
{
    static	struct  uMenu  const   menu[] =
    {
			{ 0x0201u, "ѡ�������ʼ��ʽ" },
			{ 0x0C14u, "[��ʱ����]" },
			{ 0x1414u, "[��ʱ����]" },
    };

    uint8_t	item;

    Part_cls();
    Menu_Redraw( menu );
    switch ( Configure.Mothed_Delay )
    {
    default:
			Lputs( 0x0C0Eu, " >>" );
			item = 1u;
			break;
    case enumByDelay:
			Lputs( 0x0C0Eu, " ��" );
			item = 1u;
			break;
    case enumByAccurate:
			Lputs( 0x140Eu, " ��" );
			item = 2u;
			break;
    }

    item = Menu_Select( menu, item, NULL );

    switch( item )
    {
    case 1:
			if ( Configure.Mothed_Delay != enumByDelay )
			{
				uint8_t i;
				for( i = 0; i < SP_Max; i ++ )
				{
					SampleSet[(enum enumSamplerSelect)i].delayt = 1u;
				}	
				SampleSetSave();
				Configure.Mothed_Delay = enumByDelay;
				ConfigureSave();
			}
			break;
    case 2:
			if ( Configure.Mothed_Delay != enumByAccurate )
			{
				Configure.Mothed_Delay = enumByAccurate;
				ConfigureSave();
			}
			break;
    default:
			break;
    }
}

/********************************** ����˵�� ***********************************
*  ϵͳ���� -> ������ʱ��ʽ
*******************************************************************************/
static	void	menu_SelectTimeMode( void )
{
    static	struct uMenu  const	menu[] =
    {
        { 0x0201u, "ѡ��ͣ���ʱ��ʽ" },
        { 0x0C14u, "[ͣ��۳�]" },
        { 0x1414u, "[ͣ�粹��]" },
    };

    uint8_t item;

		Part_cls();
    Menu_Redraw( menu );
    switch ( Configure.Mothed_Sample )
    {
    default:
        Lputs( 0x0C0Eu, " >>" );
        item = 1u;
        break;
    case enumBySet:
        Lputs( 0x0C0Eu, " ��" );
        item = 1u;
        break;
    case enumBySum:
        Lputs( 0x140Eu, " ��" );
        item = 2u;
        break;
    }

    item = Menu_Select( menu, item, NULL );

    switch ( item )
    {
    case 1:
        Configure.Mothed_Sample = enumBySet;
        ConfigureSave();
        break;
    case 2:
        Configure.Mothed_Sample = enumBySum;
        ConfigureSave();
        break;
    default:
        break;
    }
}

static	void	Time_Comments( void )
{	
	Lputs( 0x0810, "���ÿ�ʼ����" );	
	Lputs( 0x0B10, "ʱ��������ʽ" );	

	Lputs( 0x0F10,"---------" );

	Lputs( 0x1210, "����ͣ���" );
	Lputs( 0x1510, "����ָ�����" );
	Lputs( 0x1810, "ʱ�ļ�ʱ��ʽ" );
}
static	void	menu_ConfigureTime( void )
{
    static	struct uMenu  const	menu[] =
    {
        { 0x0201u, "ѡ��ʱ����Ʒ�ʽ" },
        { 0x0A01u, "������ʱ" },
        { 0x1401u, "ͣ���ʱ" },
    };

    uint8_t item = 1;
		cls();
    do
    {			
        Part_cls();
        Menu_Redraw( menu );
        item = Menu_Select( menu, item, Time_Comments );
        switch ( item )
        {
        case 1:
            menu_SelectDelayMode();
            break;
        case 2:
            menu_SelectTimeMode();
            break;
        default:
            break;
        }
    }
    while ( enumSelectESC != item );
}


static	void	menu_SelectRange( enum enumSamplerSelect SamplerSelect )
{
	 static	struct  uMenu  const   menu[] =
    {
        { 0x0201u, "�ǳ��ò�������" },
        { 0x0C12u, "[0.2 - 1.0]" },
        { 0x1612u, "[0.5 - 2.0]" },
    };
		BOOL changed = FALSE;
    uint8_t	item;

    
		do
		{
			Part_cls();
			Menu_Redraw( menu );
			if ( Configure.PumpType[SamplerSelect] == enumOrifice_1 )
			{
				Lputs( 0x0C0Eu, "��" );
				item = 1u;
			}
			else	
			{
				Lputs( 0x160Eu, "��" );
				item = 2u;
			}

			item = Menu_Select( menu, item, NULL );

			switch( item )
			{
			case 1:
				Configure.PumpType[SamplerSelect] = enumOrifice_1;
				changed = TRUE;
				break;
			case 2:
				Configure.PumpType[SamplerSelect] = enumOrifice_2;	
				changed = TRUE;
				break;
			default:
				break;
			}
		} while ( enumSelectESC != item );
	if( changed == TRUE )
	{
// 		switch( MsgBox( "�����޸Ľ����",vbYesNoCancel | vbDefaultButton3 ) )
// 		{
// 		case vbYes:
			ConfigureSave();
// 			break;
// 		case vbNo:
// 			ConfigureLoad();
// 			break;
// 		case vbCancel:
// 			break;
// 		}
	}
}



static	void	menu_Configure_Flow_TSP( void )
{
	Part_cls();
	Lputs( 0x0C10, "�۳���������");
	ShowI16U( 0x1210u, Configure.SetFlow[SP_TSP], 0x0501u, "L/m" );
	if( EditI16U( 0x1210u, & Configure.SetFlow[SP_TSP  ], 0x0501u ))
	{
		if ( Configure.SetFlow[SP_TSP  ] > 1400u )
		{
			Configure.SetFlow[SP_TSP  ] = 1400u;
		}
		if ( Configure.SetFlow[SP_TSP  ] <  600u )
		{
			Configure.SetFlow[SP_TSP  ] =  600u;
		}	
		
		
	}
	if( vbYes == MsgBox( "�Ƿ񱣴����?", vbYesNo|vbDefaultButton2 ) )
			ConfigureSave();
		else
			ConfigureLoad();
}
static	void	SlectOther_Comments( void )
{
	if( (enum enumSamplerSelect)SamplerTypeHas[0] == SP_TSP )
	{
		Lputs(0x0A0F, "����ˮ��ѹ����");
		Lputs(0x0E0F, "�Լ���·ʱ���õ�");
		Lputs(0x120F, "�����������ú�");
		Lputs(0x160F, "�۳�����������");
	}
	else
	{
		Lputs(0x0B0F, "����ˮ��ѹ����");
		Lputs(0x100F, "�Լ���·ʱ���õ�");
		Lputs(0x150F, "������������");
	}
}
static	void	menu_SelectOther( void )
{
	static  struct  uMenu  menu1[] =
	{
		{ 0x0401u, 	"�ǳ��ò�������" 	},
		{ 0x0602u,  "ˮ��ѹ"	},
		{ 0x0C01u,  "C·����"},
		{ 0x1201u,  "D·����"},
		{ 0x1802u, 	"�۳���"	},
	};
	static  struct  uMenu  menu2[] =
	{
		{ 0x0301u, 	"�ǳ��ò�������" 	},
		{ 0x0802u,  "ˮ��ѹ"	},
		{ 0x1001u,  "C·����"},
		{ 0x1801u,  "D·����"},
	};
	static  struct  uMenu  * menu;
		uint8_t	item = 1;
		cls();
		do{
			Part_cls();
			if( (enum enumSamplerSelect)SamplerTypeHas[0] == SP_TSP )
				menu = menu1;
			else
				menu = menu2;
			Menu_Redraw( menu );
			item = Menu_Select( menu, item, SlectOther_Comments );
			switch ( item )
			{
			case 1:
				menu_Select_Calc_Pbv( );
				break;
			case 2:	
				menu_SelectRange( SP_SHI_C );
				break;
			case 3:
				menu_SelectRange( SP_SHI_D );
				break;
			case 4:
				menu_Configure_Flow_TSP();
				cls();
				break;
			default:
				break;
			}
		}while( enumSelectESC != item );
}
/********************************** ����˵�� ***********************************
*  �������ã������û������Ĳ��֣�
*******************************************************************************/
static	void	menu_SampleConfigure( void )
{
		static	struct	uMenu  const menu[] =
    {
        { 0x0302u, "�����趨"  },
        { 0x0802u, "����¶�"},	{ 0x0814u, "��ѹ����" },
        { 0x1002u, "����װ��"},	{ 0x1014u, "��ʱ��ʽ" },
        { 0x1802u, "����ѹ"  },	{ 0x1814u, "��������" } 
    };

    uint8_t	item = 1u;

    do
    {
      cls();		
			Menu_Redraw( menu );
			item = Menu_Select( menu, item, NULL );
			switch( item )
			{
			case 1:
				menu_SelectTstd();
				break;
			case 3:
				menu_ConfigureHCBox();
				break;
			case 5:
				menu_Configure_Ba();
				break;
			case 2:
				menu_Pr_Portect();
				break;
			case 4:
				menu_ConfigureTime();
				break;
			case 6:
				menu_SelectOther();
				break;
			default:
				break;
			}
    }
    while ( enumSelectESC != item );
}
/********************************** ����˵�� ***********************************
*	���˵� -> ά���˵�
*******************************************************************************/
extern	void	menu_ConfigureEx( void );
extern	menu_Clean_FileNum( void );
void	menu_UserMaintenance( void )
{
	static	struct uMenu  const  menu[] =
		{
				{ 0x0302u, "����ά��" },
				{ 0x0802u, "��������" }, { 0x0814u, "�����趨" },
				{ 0x1002u, "�����ۼ�" }, { 0x1014u, "�����궨" },
				{ 0x1802u, "���м�¼" }, { 0x1814u, "�ͺŰ汾" }
		};
		uint8_t	item = 1u;

	do
		{
			cls();
			Menu_Redraw( menu );

			item = Menu_Select( menu, item, NULL );
			switch( item )
			{
			case 1:
				Instrument_Set();
				break;
			case 2:
				menu_SampleConfigure();
				break;
			case 3:
				menu_SamplerSum();
				break;
			case 4:
				menu_Calibrate();
				 break;
			case 5:
				PowerLog_Query();
				break;
			case 6:
			 ShowEdition();
				if ( K_RIGHT == getKey())
				{
					if ( ! releaseKey( K_RIGHT, 100u ))
					{
						beep();
						menu_ConfigureEx();
					}
				}
				break;
			default:
					break;
			}
		}
		while( enumSelectESC != item );
}
/***********************************************************************
*
***********************************************************************/


struct	uPID_Parament PID;

void	HCBoxPIDParament( void )
{
	BOOL changed = FALSE;
	uint8_t item = 0;
	static struct uMenu const menu[] =
	{
		{ 0x0301, "PID����"	},

		{ 0x0606, "Kp:" },
		{ 0x0906, "Ti:" },
		{ 0x0C06, "Td:" },		
	};
	cls();
	Menu_Redraw( menu );
	PIDLoad();
	do
	{
		ShowI16U( 0x0618, PID.Kp, 0x0400, NULL );
		ShowI16U( 0x0918, PID.Ti, 0x0400, NULL );
		ShowI16U( 0x0C18, PID.Td, 0x0400, NULL );
		
		item = Menu_Select( menu, item, NULL);
		
		switch( item )
		{
			case 1:	EditI16U( 0x0618, &PID.Kp, 0x0400 );	changed = TRUE;	break;
			case 2:	EditI16U( 0x0918, &PID.Ti, 0x0502 );	changed = TRUE;	break;
			case 3:	EditI16U( 0x0C18, &PID.Td, 0x0502 );	changed = TRUE;	break;
			
		}
	}while( enumSelectESC != item );
	
	if( changed == TRUE )
		PIDSave();
}
	
	
/************************************************************************/
void	menu_Maintenance( void )
{

	static	uint32_t  password = 0;
	Lputs( 0x0102, "ά ��  " );
	if ( ! Sampler_isRunning( SP_Max ))
	{	
		password = InputPassword();
		if( password == Configure.Password )
		{
			menu_UserMaintenance();
		}
		else
		{
			switch( password )
			{
				default:
					return;
				case  SysPassword1a:
					menu_UserMaintenance();
					break;
				case  SysPassword2a:
					menu_ConfigureEx();
					break;
				case  SysPassword3a:
					HCBoxPIDParament();
					break;
			}
		}	
	
	}
	else
	{
		MsgBox( "����������,��������!", vbOKOnly );
		return;
	}

}


/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/
// 		if ( Done == FALSE )
// 		{
// 			Lputs( 0x0808u, "��ȷ��������:" );
// 			if ( ! EditI32U( 0x0E0Cu, &passwordii, 0x0600u ))
// 			{
// 				Done = TRUE;
// 			}
// 		}
// 		if( ( passwordi == passwordii ) && ( Done == FALSE ))
// 		{
// 			cls();
// 			switch( MsgBox( "�Ƿ񱣴�������?", vbYesNoCancel | vbDefaultButton3 ) )
// 			{
// 			case	vbYes:
// 				Configure.Password = passwordii;
// 				ConfigureSave();
// 				Done = TRUE;
// 				break;
// 			case	vbNo:
// 				Done = TRUE;
// 				break;
// 			case	vbCancel:
// 				break;
// 			}		
// 		}
// 		else	if( Done == FALSE )
// 		{
// 			cls();
// 			MsgBox( "���벻һ��!", vbOKOnly );
// 			if( vbYes != MsgBox( "�Ƿ�����޸�?", vbYesNo ) )
// 				Done = TRUE;
// 		}	

// 	static	char Table2[] = { 0x0C, 0x14, };
// 	static	char Table3[] = { 0x0A, 0x10, 0x16, };
// 	static	char Table4[] = { 0x08, 0x0C, 0x12, 0x16, };
// 	static	char Table5[] = { 0x06, 0x0A, 0x0E, 0x12, 0x16, };


