/**************** (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司 ****************
* 文 件 名: Configure.C
* 创 建 者: 董峰
* 描  述  : KB-6120E 配置程序(允许用户配置的部分)
* 最后修改: 2014年8月25日
*********************************** 修订记录 ***********************************
* 版  本:
* 修订人:
*******************************************************************************/
#include "AppDEF.H"

struct	uConfigure	Configure;

/********************************** 功能说明 ***********************************
*  设置时间、日期
*******************************************************************************/
static	void	menu_SetupClock( void )
{
    static  struct  uMenu  const  menu[] =
    {
        { 0x0102u, "时间设置" },
        { 0x0810u, "设置日期" },
        { 0x1210u, "设置时间" },
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
		Lputs( 0x0B10u, "请输入新密码:" );
		if ( EditI32U( 0x1214u, &Configure.Password, 0x0600u ))
		{			 
			 MsgBox( "密码修改成功!", vbOKOnly );
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
		{ 0x0301u, "显示配置" },
		{ 0x0810u, "灰度" },
		{ 0x1010u, "亮度" },
		{ 0x1810u, "定时" },
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
		case 0:	Lputs( 0x1817u, "[关闭] " );	break;
		case 1:	Lputs( 0x1817u, "[15秒] " );	break;
		case 2:	Lputs( 0x1817u, "[30秒] " );	break;
		case 3:	Lputs( 0x1817u, "[60秒] " );	break;
		default:
		case 4:	Lputs( 0x1817u, "[常亮] " );	break;
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
	Lputs( 0x080F,"包括时间设置");
	Lputs( 0x0C0F,"维护密码的修改");
	Lputs( 0x100F,"以及液晶屏幕的");
	Lputs( 0x140F,"显示对比度 亮度");
	Lputs( 0x180F,"和亮屏时长");
}

void	Instrument_Set( void )
{
	uint8_t	item = 1u;
	static	struct uMenu  const  menu[] =
	{
			{ 0x0301u, "维护" },
			{ 0x0901u, "时间设置" },
			{ 0x0F01u, "密码设置" },
			{ 0x1501u, "显示设置" },
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
			{ 0x0102u, "采样器采样记录" },
			{ 0x1A06u, "退出" }, { 0x1A18u, "清除" }
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
		"粉 尘:",
		"日均A:",
		"日均B:",
		"时均C:",
		"时均D:",
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
		Lputs( 0x0605u, "采样器" );		 Lputs( 0x0616u, "采样次数" );
		for( i = 0; i < SamplerHasMax; i ++ )
		{
			Lputs		( (Table[i] * 256 + 0x06 ), TableSampler[SamplerTypeHas[i]] );
			ShowI16U( (Table[i] * 256 + 0x12 ), SampleSet[(enum enumSamplerSelect)SamplerTypeHas[i]].FileNum, 0x0400u, " 次" );
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
			if ( vbYes == MsgBox( "清除所有文件 ?", vbYesNo | vbDefaultButton2 ) )
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



/********************************** 功能说明 ***********************************
*  配置标况流量的定义温度
*******************************************************************************/
static	void	menu_SelectTstd( void )
{
    static  struct  uMenu  const  menu[] =
    {
        { 0x0301u, "选择标况温度" },
        { 0x080Au, " 0℃(273K)" },
        { 0x100Au, "20℃(293K)" },
        { 0x180Au, "25℃(298K)" }
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
            Lputs( 0x0802u,"→"	);
            Lputs( 0x1002u,"  "	);
            Lputs( 0x1802u,"  "	);
            item = 1;
            break;
        case enum_293K:
            Lputs( 0x0802u,"  "	);
            Lputs( 0x1002u,"→"	);
            Lputs( 0x1802u,"  "	);
            item = 2;
            break;
        case enum_298K:
            Lputs( 0x0802u,"  "	);
            Lputs( 0x1002u,"  "	);
            Lputs( 0x1802u,"→"	);
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
/********************************** 功能说明 ***********************************
*  系统配置 -> 恒温箱
*******************************************************************************/
static	void	Configure_HCBox( void )
{
    static  CHAR  const  MDS[][7] =
    {
        { "[停用]" }, { "[加热]" }, { "[制冷]" } , { "[自动]" }
    };
    static	struct	uMenu	const	menu[] =
    {
        {	0x0201u, "配置恒温箱"	},
        {	0x0C02u, "控制方式"	},
        {	0x1402u, "设置温度"	},
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
        ShowI16U( 0x1414U, SetTemp, 0x0501U, "℃" );

        item = Menu_Select( menu, item, NULL );
        switch( item )
        {
        case 1u:	/*	EditBoxMode	*/
            SetMode = ( SetMode + 1u) % 4u;
            changed = TRUE;
            break;

        case 2u:
            /* 注意: 因为恒温箱的设置温度范围是正数, 所以才可以使用无符号数编辑程序进行编辑 */
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
                switch ( MsgBox( "保存配置数据 ?", vbYesNoCancel + vbDefaultButton3 ))
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
        {	0x0201u, "配置加热器"	},
        {	0x0C02u, "开  关"	},
				{	0x1802u, "设置温度"	},
    };
		 static	struct	uMenu	const	menu1[] =
    {
        {	0x0101u, "配置加热器"	},
				{	0x0C02u, "开  关"	},
    };
static	CHAR  const * const HeaterSW[] = {"[停用]", "[启用]" };
			
    uint8_t 	item = 1u;
		uint8_t 		state  = Configure.Heater_SW;          //	加热器开关
    uint16_t	SetTemp = Configure.Heater_SetTemp;			//	加热器恒温温度
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
					ShowI16U( 0x0C14U, SetTemp, 0x0501U, "℃" );
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
         case 2u: /* 注意: 因为恒温箱的设置温度范围是正数, 所以才可以使用无符号数编辑程序进行编辑 */
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
						switch ( MsgBox( "保存配置数据 ?", vbYesNoCancel + vbDefaultButton3 ))
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
        MsgBox( "未安装恒温箱", vbOKOnly );
        break;
    case enumHCBoxOnly:
        Configure_HCBox();
        break;
    case enumHeaterOnly:
        Configure_Heater();
        break;
    case enumHCBoxHeater:
        MsgBox( "硬件不能支持", vbOKOnly );
        break;
    }
}

/********************************** 功能说明 ***********************************
*	系统配置 -> 大气压
*******************************************************************************/
static	void	menu_Configure_Ba( void )
{
    static const struct uMenu menu[] =
    {
        { 0x0301u, "配置大气压" },
        { 0x0802u, "方式" },
        { 0x1002u, "预测大气压" },
        { 0x1802u, "输入:" }
    };
		static const struct uMenu menu1[] =
    {
        { 0x0101u, "配置大气压" },
        { 0x0802u, "方式" },
     
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
            Lputs( 0x080Cu, "[预测输入]" );
            break;
        case enumMeasureBa:
            Lputs( 0x080Cu, "[实时测量]" );
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
							switch( MsgBox( "保存修改结果 ?", vbYesNoCancel + vbDefaultButton3 ))
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
            Lputs( 0x0202u, "测量大气压" );
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
		{ 0x0201u, "限压保护"},
		{ 0x0D05u, "时均C"	},
		{ 0x1305u, "时均D"	} ,
	};	
	static	struct	uMenu  menu3[] =
	{
		{ 0x0301u, "限压保护"},
		{ 0x0C05u, "粉 尘"	},
		{ 0x1005u, "时均C"	},
		{ 0x1405u, "时均D"	} ,
	};	
	static	struct	uMenu  menu4[] =
	{
		{ 0x0401u, "限压保护"},
		{ 0x0B05u, "日均A"	},
		{ 0x0E05u, "日均B"	},
		{ 0x1105u, "时均C"	},
		{ 0x1405u, "时均D"	} ,
	};
	static	struct	uMenu  menu5[] =
	{
		{ 0x0501u, "限压保护"},
		{ 0x0A05u, "粉 尘"	},
		{ 0x0D05u, "日均A"	},
		{ 0x1005u, "日均B"	},
		{ 0x1305u, "时均C"	},
		{ 0x1605u, "时均D"	} ,
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
		Lputs( 0x0605u, "采样器" );		 Lputs( 0x0614u, "限压值" );
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
		switch( MsgBox( "保存修改结果？",vbYesNoCancel | vbDefaultButton3 ) )
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


/********************************** 功能说明 ***********************************
*  系统配置 -> 采样开启的延时方式
*******************************************************************************/
static	void	menu_SelectDelayMode( void )
{
    static	struct  uMenu  const   menu[] =
    {
			{ 0x0201u, "选择采样开始方式" },
			{ 0x0C14u, "[延时启动]" },
			{ 0x1414u, "[定时启动]" },
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
			Lputs( 0x0C0Eu, " →" );
			item = 1u;
			break;
    case enumByAccurate:
			Lputs( 0x140Eu, " →" );
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

/********************************** 功能说明 ***********************************
*  系统配置 -> 采样计时方式
*******************************************************************************/
static	void	menu_SelectTimeMode( void )
{
    static	struct uMenu  const	menu[] =
    {
        { 0x0201u, "选择停电计时方式" },
        { 0x0C14u, "[停电扣除]" },
        { 0x1414u, "[停电补齐]" },
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
        Lputs( 0x0C0Eu, " →" );
        item = 1u;
        break;
    case enumBySum:
        Lputs( 0x140Eu, " →" );
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
	Lputs( 0x0810, "设置开始采样" );	
	Lputs( 0x0B10, "时的启动方式" );	

	Lputs( 0x0F10,"---------" );

	Lputs( 0x1210, "设置停电后" );
	Lputs( 0x1510, "来电恢复采样" );
	Lputs( 0x1810, "时的计时方式" );
}
static	void	menu_ConfigureTime( void )
{
    static	struct uMenu  const	menu[] =
    {
        { 0x0201u, "选择时间控制方式" },
        { 0x0A01u, "开机延时" },
        { 0x1401u, "停电计时" },
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
        { 0x0201u, "非常用采样配置" },
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
				Lputs( 0x0C0Eu, "→" );
				item = 1u;
			}
			else	
			{
				Lputs( 0x160Eu, "→" );
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
// 		switch( MsgBox( "保存修改结果？",vbYesNoCancel | vbDefaultButton3 ) )
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
	Lputs( 0x0C10, "粉尘流量设置");
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
	if( vbYes == MsgBox( "是否保存更改?", vbYesNo|vbDefaultButton2 ) )
			ConfigureSave();
		else
			ConfigureLoad();
}
static	void	SlectOther_Comments( void )
{
	if( (enum enumSamplerSelect)SamplerTypeHas[0] == SP_TSP )
	{
		Lputs(0x0A0F, "包括水汽压设置");
		Lputs(0x0E0F, "以及两路时均泵的");
		Lputs(0x120F, "流量量程配置和");
		Lputs(0x160F, "粉尘泵流量设置");
	}
	else
	{
		Lputs(0x0B0F, "包括水汽压设置");
		Lputs(0x100F, "以及两路时均泵的");
		Lputs(0x150F, "量流量程配置");
	}
}
static	void	menu_SelectOther( void )
{
	static  struct  uMenu  menu1[] =
	{
		{ 0x0401u, 	"非常用采样配置" 	},
		{ 0x0602u,  "水汽压"	},
		{ 0x0C01u,  "C路量程"},
		{ 0x1201u,  "D路量程"},
		{ 0x1802u, 	"粉尘泵"	},
	};
	static  struct  uMenu  menu2[] =
	{
		{ 0x0301u, 	"非常用采样配置" 	},
		{ 0x0802u,  "水汽压"	},
		{ 0x1001u,  "C路量程"},
		{ 0x1801u,  "D路量程"},
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
/********************************** 功能说明 ***********************************
*  采样配置（允许用户操作的部分）
*******************************************************************************/
static	void	menu_SampleConfigure( void )
{
		static	struct	uMenu  const menu[] =
    {
        { 0x0302u, "采样设定"  },
        { 0x0802u, "标况温度"},	{ 0x0814u, "限压保护" },
        { 0x1002u, "恒温装置"},	{ 0x1014u, "计时方式" },
        { 0x1802u, "大气压"  },	{ 0x1814u, "其他设置" } 
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
/********************************** 功能说明 ***********************************
*	主菜单 -> 维护菜单
*******************************************************************************/
extern	void	menu_ConfigureEx( void );
extern	menu_Clean_FileNum( void );
void	menu_UserMaintenance( void )
{
	static	struct uMenu  const  menu[] =
		{
				{ 0x0302u, "仪器维护" },
				{ 0x0802u, "仪器设置" }, { 0x0814u, "采样设定" },
				{ 0x1002u, "采样累计" }, { 0x1014u, "仪器标定" },
				{ 0x1802u, "运行记录" }, { 0x1814u, "型号版本" }
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
		{ 0x0301, "PID参数"	},

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
	Lputs( 0x0102, "维 护  " );
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
		MsgBox( "仪器采样中,请勿设置!", vbOKOnly );
		return;
	}

}


/********  (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司  **** End Of File ****/
// 		if ( Done == FALSE )
// 		{
// 			Lputs( 0x0808u, "请确认新密码:" );
// 			if ( ! EditI32U( 0x0E0Cu, &passwordii, 0x0600u ))
// 			{
// 				Done = TRUE;
// 			}
// 		}
// 		if( ( passwordi == passwordii ) && ( Done == FALSE ))
// 		{
// 			cls();
// 			switch( MsgBox( "是否保存新密码?", vbYesNoCancel | vbDefaultButton3 ) )
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
// 			MsgBox( "密码不一致!", vbOKOnly );
// 			if( vbYes != MsgBox( "是否继续修改?", vbYesNo ) )
// 				Done = TRUE;
// 		}	

// 	static	char Table2[] = { 0x0C, 0x14, };
// 	static	char Table3[] = { 0x0A, 0x10, 0x16, };
// 	static	char Table4[] = { 0x08, 0x0C, 0x12, 0x16, };
// 	static	char Table5[] = { 0x06, 0x0A, 0x0E, 0x12, 0x16, };


