/**************** (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司 ****************
* 文 件 名: ConfigureEx.C
* 创 建 者: 董峰(2014年4月8日)
* 描  述  : KB-6120E 定义、显示版本信息
* 最后修改: 2014年4月8日
*********************************** 修订记录 ***********************************
* 版  本: 
* 修订人: 
*******************************************************************************/
#include "AppDEF.H"
#include "BMP.H"
/********************************** 功能说明 ***********************************
*  根据仪器型号显示版本信息
*  根据销售人员要求，可能会改成其他厂家名称，或者干脆不显示。
*******************************************************************************/
CHAR  const * const ExNameIdent1[] =
{
  "/* 无厂家名称 */",
  " 青 岛 金 仕 达 ",
//  ...  
};
CHAR  const * const ExNameIdent2[] =
{
  " ",
  "电 子 科 技 有 限 公 司",
//  ...  
};
CHAR  const * const EditionNum[] =
{
   "KB6120E V2.07",	//	内部版本
	__DATE__" V2.00",	//	显示版本
};
static	void	ShowEdition_NoName1( void )
{
	cls();
//   Lputs( 0x0808u, szNameIdent[Configure.InstrumentName] );
	Lputs( 0x0803u, EditionNum[1] );
	Lputs( 0x1209u, "编号:" );	ShowFP32( 0x1212u, Configure.ExNum, 0x0700u, NULL );
}
static	void	ShowEdition_NoName2( void )
{
	cls();
  Lputs( 0x0808u, szNameIdent[Configure.InstrumentName] );
	Lputs( 0x0F03u, EditionNum[1] );
	Lputs( 0x1609u, "编号:" );	ShowFP32( 0x1612u, Configure.ExNum, 0x0700u, NULL );
}
static	void	ShowEdition_HasName1( void )
{
  cls();
//  	Lputs( 0x0108u, szNameIdent [Configure.InstrumentName] );
	Lputs( 0x0306u, ExNameIdent1[Configure.ExName] );
	Lputs( 0x0A01u, ExNameIdent2[Configure.ExName] );
	Lputs( 0x1003u, EditionNum[1] );
	Lputs( 0x1609u, "编号:" );ShowFP32( 0x1612u, Configure.ExNum, 0x0700u, NULL );
}

static	void	ShowEdition_HasName2( void )
{
  cls();
 	Lputs( 0x0108u, szNameIdent [Configure.InstrumentName] );
	Lputs( 0x0606u, ExNameIdent1[Configure.ExName] );
	Lputs( 0x0C01u, ExNameIdent2[Configure.ExName] );
	Lputs( 0x1203u, EditionNum[1] );
	Lputs( 0x1809u, "编号:" );ShowFP32( 0x1812u, Configure.ExNum, 0x0700u, NULL );
}

//  ...
void	ShowEdition( void )
{
  switch( Configure.ExName )
  {
  case  (enum enumExNameIdent) 0: 
		if( Configure.InstrumentName == 0 )
			ShowEdition_NoName1();
		else
			ShowEdition_NoName2();
    break;
  default: 
	if( Configure.InstrumentName == 0 )
    ShowEdition_HasName1(); 
	else
		ShowEdition_HasName2(); 
    break;
  }
}

/*开机动画*/
void	Animation( void )
{
	switch( Configure.ExName )
  {
	case	Name_JSD: 
    WALL( WALL1 );
    break;
  default:
		WALL( WALL2 );	
    break;
	}
}
	
/********************************** 功能说明 ***********************************
*  显示程序版本 根据仪器型号显示版本信息
*******************************************************************************/
void	ShowEdition_Inner( void )
{
	cls();
	Lputs( 0x0308u, szNameIdent[Configure.InstrumentName] );
	Lputs( 0x0908u, __DATE__ );
	Lputs( 0x0F0Au, __TIME__ );
	Lputs( 0x1506u, EditionNum[0] );

}

/********************************** 数据定义 ***********************************
*  可用的仪器名称列表
*******************************************************************************/
CHAR  const * const szTypeIdent[] =
{
		"KB-6120C",
		"KB-6120B",
		"KB-2400D",
		"KB-2400 ",   
//		"KB-6120BD",
//		"KB-6120BH",
};

CHAR  const * const szNameIdent[] =
{
	" /* 无 名 称 */ ",
	" 智能综合采样器 ",
	" 综合大气采样器 ",
	"智能中流量采样器",
	"智能大流量采样器",
	" 恒温恒流采样器 ",
	" 智能恒流采样器 ",
};
void	ConfigureLoadDefault_KB6120E( void )
{
		Configure.Heater_SetTemp = 300u;		//	加热器恒温温度 30.0 ℃
		Configure.Heater_SW = FALSE;         //加热器开关			
		Configure.HCBox_SetMode = MD_Shut;		//	恒温箱控制模式 [关闭]
		Configure.HCBox_SetTemp = 240u;			//	恒温箱恒温温度 24.0 ℃
				 
		Configure.SetTstd = enum_293K;			//	标况流量的定义温度 
		
		Configure.Mothed_Delay = enumByDelay;	//	采样开始时间方式--延时启动
		Configure.Mothed_Sample = enumBySet;	//	采样时间控制方式--停电扣除

		Configure.Mothed_Ba = enumMeasureBa;	//	大气压力获取方式
		Configure.set_Ba    = 10133u;			//	保存用户输入气压

		Configure.DisplayGray  = 2000u;	//	显示灰度设置
		Configure.DisplayLight = 50u;	//	显示亮度设置
		Configure.TimeoutLight = 2u;	//	背光延时时间
		
		Configure.slope_Bat_Voltage = 1000;
		Configure.slope_Bat_Current = 1000;
		Configure.threshold_Current = 100;
		
		Configure.shouldCalcPbv = 0;		//	饱和水汽压是否参与计算。
		
		Configure.Password = 633817;
		Configure.ExName = Name_JSD;
		
}
void	ConfigureLoad_KB6120C( void )
{
		Configure.InstrumentType = type_KB6120C;//6120C
		
		Configure.PumpType[SP_TSP  ] = enumOrifice_1;	
		Configure.SetFlow[SP_TSP  ]  = 1000u;	Configure.Pr_Portect[SP_TSP  ]  =  500u; 		//	粉尘  采样流量 100.0	L/m		限压值	  5KPa
		
		Configure.PumpType[SP_R24_A] = enumOrifice_1;	
		Configure.SetFlow[SP_R24_A]  =  	2u;	Configure.Pr_Portect[SP_R24_A]  = 1000u;		//	日均1 采样流量 0.200	L/m		限压值	 10KPa
		
		Configure.PumpType[SP_R24_B] = enumOrifice_1;	
		Configure.SetFlow[SP_R24_B]  =  	2u;	Configure.Pr_Portect[SP_R24_B]  = 1000u;		//	日均2 采样流量 0.200	L/m		限压值	 10KPa
		
		Configure.PumpType[SP_SHI_C] = enumOrifice_1; 
		Configure.SetFlow[SP_SHI_C]  =  	5u;	Configure.Pr_Portect[SP_SHI_C]  = 1000u;		//	时均1 采样流量 0.5 	L/m		限压值	 10KPa
		
		Configure.PumpType[SP_SHI_D] = enumOrifice_1; 
		Configure.SetFlow[SP_SHI_D]  =  	5u;	Configure.Pr_Portect[SP_SHI_D]  = 1000u;		//	时均2 采样流量 0.5 	L/m		限压值	 10KPa


		Configure.HeaterType = enumHCBoxOnly;	//	只有恒温箱
		
		Configure.Battery_SW = FALSE;    //电池是否存在

}


void	ConfigureLoad_KB6120B( void )
{
		Configure.InstrumentType = type_KB6120B; // 6120B

		Configure.PumpType[SP_TSP  ] = enumOrifice_1;	Configure.SetFlow[SP_TSP  ]  = 1000u;	//	粉尘  采样流量 100.0 L/m
		Configure.PumpType[SP_R24_A] = enumPumpNone;	Configure.SetFlow[SP_R24_A]  =  2u;	//	日均1 采样流量 0.200 L/m
		Configure.PumpType[SP_R24_B] = enumPumpNone;	Configure.SetFlow[SP_R24_B]  =  2u;	//	日均2 采样流量 0.200 L/m
		Configure.PumpType[SP_SHI_C] = enumOrifice_1;	Configure.SetFlow[SP_SHI_C]  =  5u;	//	时均1 采样流量 0.5 L/m
		Configure.PumpType[SP_SHI_D] = enumOrifice_1;	Configure.SetFlow[SP_SHI_D]  =  5u;	//	时均2 采样流量 0.5 L/m

		

		
		Configure.HeaterType = enumHCBoxOnly;	//	只有恒温箱
		
		Configure.Battery_SW = FALSE;    //电池是否存在
}

void	ConfigureLoad_KB2400D( void )
{
		Configure.InstrumentType = type_KB2400D;//2400D
		
		Configure.PumpType[SP_TSP  ] = enumPumpNone;	Configure.SetFlow[SP_TSP  ]  = 1000u;	//	粉尘 采样流量 100.0 L/m
		Configure.PumpType[SP_R24_A] = enumOrifice_1;	Configure.SetFlow[SP_R24_A]  =  2u;	//	日均1采样流量 0.200 L/m
		Configure.PumpType[SP_R24_B] = enumOrifice_1;	Configure.SetFlow[SP_R24_B]  =  2u;	//	日均2采样流量 0.200 L/m
		Configure.PumpType[SP_SHI_C] = enumOrifice_1;	Configure.SetFlow[SP_SHI_C]  =  5u;	//	时均1采样流量 0.5 L/m
		Configure.PumpType[SP_SHI_D] = enumOrifice_1;	Configure.SetFlow[SP_SHI_D]  =  5u;	//	时均2采样流量 0.5 L/m

		

		
		Configure.HeaterType = enumHCBoxOnly;	//	只有恒温箱
		
		Configure.Battery_SW = FALSE;    //电池是否存在
}

void	ConfigureLoad_KB2400( void )
{
		Configure.InstrumentType = type_KB2400;//2400
		
		Configure.PumpType[SP_TSP  ] = enumPumpNone;	Configure.SetFlow[SP_TSP  ]  = 1000u;	//	粉尘 采样流量 100.0 L/m
		Configure.PumpType[SP_R24_A] = enumPumpNone;	Configure.SetFlow[SP_R24_A]  =  2u;	//	日均1采样流量 0.200 L/m
		Configure.PumpType[SP_R24_B] = enumPumpNone;	Configure.SetFlow[SP_R24_B]  =  2u;	//	日均2采样流量 0.200 L/m
		Configure.PumpType[SP_SHI_C] = enumOrifice_1;	Configure.SetFlow[SP_SHI_C]  =  5u;	//	时均1采样流量 0.5 L/m
		Configure.PumpType[SP_SHI_D] = enumOrifice_1;	Configure.SetFlow[SP_SHI_D]  =  5u;	//	时均2采样流量 0.5 L/m

		

		
		Configure.HeaterType = enumHCBoxOnly;	//	只有恒温箱
		
		Configure.Battery_SW = FALSE;    //电池是否存在

}


/********************************** 功能说明 ***********************************
* 				泵安装情况
*******************************************************************************/
CHAR  const * const Pump[]=//ToDo:加枚举
{
	"粉尘泵 ",	//0
	"日均A",	//1
	"日均B",	//2
	"时均C",	//3
	"时均D",	//4
// 	"大气泵",	//5
	"恒温器"		//6
}; 
CHAR  const * const Type[]=
{
	"[K-100L]",	//0
	"[K-1.Ｍ]",	//1
	"[1L孔板]",	//2
	"[2L孔板]",	//3
// 	"[1-单泵]",		//4
// 	"[2-双泵]",		//5
	"[加热器]",	//6	4
	"[恒温箱]",	//7	5
	"[全安装]" 	//8	6
};
CHAR  PumpC[SP_Max+1];
CHAR  TypeC[SP_Max+1];
static	void	PumpConfigure( void )
{
// 	enum 
// 	{
// 		opt_exit,
// 		opt_tsp_1,   opt_tsp_2,   opt_tsp_3,
// 		opt_max, opt_min = opt_tsp_1
// 	};
// 	uint16_t gray  = Configure.DisplayGray;
// 	BOOL	graychanged = FALSE;
// 	uint8_t opt_m;
// 	uint8_t	option = opt_min;
	uint16_t dispchar [6] ={ 0x0602u, 0x0A02u, 0x0E02u, 0x1202u, 0x1602u, 0x1A02u };
	uint16_t dispchar2[6] ={ 0x060Bu, 0x0A0Bu, 0x0E0Bu, 0x120Bu, 0x160Bu, 0x1A0Bu };
	uint8_t i,imax,ii;
		
// 	do {
		imax = 
		i    = 0;
		if( Configure.PumpType[SP_TSP] != enumPumpNone )
		{	
			switch ( Configure.PumpType[SP_TSP] )
			{
			case enumOrifice_1: TypeC[i] = 0;break; 
			case enumOrifice_2: TypeC[i] = 1;break; 
			}
			PumpC[i++] = 0; 		
		}	
		for( ii = 1; ii < 5; ii ++ )
		{	
			if( Configure.PumpType[(enum enumSamplerSelect) ii ] != enumPumpNone )
			{		
				switch (Configure.PumpType[(enum enumSamplerSelect) ii] )
				{
				case enumOrifice_1:TypeC[i] = 2;break; 
				case enumOrifice_2:TypeC[i] = 3;break; 
				}
				PumpC[i++] = (enum enumSamplerSelect) ii; 	
			}
		}

		if( Configure.HeaterType != enumPumpNone )
		{
			switch ( Configure.HeaterType )
			{ 
			case enumHeaterOnly:	 TypeC[i] = 4;break; 
			case enumHCBoxOnly :	 TypeC[i] = 5;break; 
			case enumHCBoxHeater:  TypeC[i] = 6;break; 

			}
			PumpC[i++] = 5; 
		}
		imax = i;
		
// 		cls();
// 		WBMP( 0x0290,0x0502, STROCK); //显示横线
// 		WBMP( 0x0290,0x0514, STROCK); //显示横线
// 		switch ( option )
// 		{
// 		default:
// 		case opt_tsp_1:				
			for( i = 0; i < 6; i++)
			{
				if( i < imax)
				{ 
// 					if( imax <= 6 )
// 						Lputs(0x0102, "泵安装情况  ");
// 					else					
// 						Lputs(0x0102, "泵安装情况 1"); 
					Lputs( dispchar[i%6],   Pump[PumpC[i]] ); 
					Lputs( dispchar2[i%6], Type[TypeC[i]]); 
				}	
			}	
// 			break;
// 		case opt_tsp_2:
// 			for( i = 6; i < 12; i++)
// 			{
// // 				Lputs(0x0102, "泵安装情况 2");
// 				if( i < imax )
// 				{
// 					Lputs( dispchar[i%6],   Pump[PumpC[i]] ); 
// 					Lputs( dispchar2[i%6], Type[TypeC[i]]); 
// 				}		
// 			}	
// 			break;
// 		case opt_tsp_3:
// 			for( i = 12; i < 18; i++)
// 			{
// // 				Lputs(0x0102, "泵安装情况 3");
// 				if( i < imax)
// 				{
// 					Lputs( dispchar[i%6],   Pump[PumpC[i]] ); 
// 					Lputs( dispchar2[i%6], Type[TypeC[i]]); 
// 				}					
// 			}	
// 			break;
// 		}
// 		
// 		opt_m = ( imax + 5 ) / 6 ;
// 		
// 		switch ( getKey() )
// 		{
// 		case K_UP:		
// 			--option;
// 			if ( option < opt_min )
// 			{
// 				option = opt_m;
// 			}
// 			break;
// 		case K_DOWN:	
// 			++option;
// 			if ( option > opt_m )
// 			{
// 				option = opt_min;
// 			}
// 			break;
// 		case K_RIGHT:
// 			++option;
// 			if ( option > opt_m )
// 			{
// 				option = opt_min;
// 			}
// 			break;
// 		case K_LEFT:
// 			--option;
// 			if ( option < opt_min )
// 			{
// 				option = opt_m;
// 			}
// 			break;
// 		case K_OK:
// 			option = opt_exit;
// 			break;
// 		case K_SHIFT:		
// 			break;
// 		case K_ESC:
// 			option = opt_exit;
// 			break;
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
// 			graychanged = FALSE;
// 		}		

// 	} while ( opt_exit != option );
}

extern BOOL	EditI32U( uint16_t yx, uint32_t * pNUM, uint16_t fmt );
/********************************** 功能说明 ***********************************
*	扩展配置 -> 选择泵、流量、仪器名称
*******************************************************************************/
void	Configure_Instrument( void )
{
	BOOL	changed = FALSE;
	BOOL	Done = FALSE;
	do {
		cls();
		WBMP( 0x0290,0x0502, STROCK); //显示横线
		WBMP( 0x0290,0x0514, STROCK); //显示横线
		Lputs( 0x0102, "仪器型号:" );
		LcmMask( 0x0102, 6 ,CHARsz );
		Lputs( 0x0112u, szTypeIdent[Configure.InstrumentType] );
		ConfigureLoadDefault();	
		PumpConfigure();
		switch ( getKey() )
		{
		case K_OK:				
			++Configure.InstrumentType;
			if ( Configure.InstrumentType >= (sizeof(szTypeIdent) / sizeof(szTypeIdent[0] )))
			{
				Configure.InstrumentType = 0u;
			}
			changed = TRUE;
			
			break;
		case K_ESC:
			if( changed == TRUE )
			{
				switch( MsgBox( "保存修改结果?", vbYesNoCancel | vbDefaultButton3 ) )
				{
				case vbYes :
					Done = TRUE;
					break;
				case vbNo :
					changed = FALSE;
					Done = TRUE;
					break;
				default :
				case vbCancel :
					break;
				}
			}
			else
				Done = TRUE;
			break;
		default:
			break;
		}
		
	} while ( !Done );
	
	if ( changed )
	{
		ConfigureLoadDefault( );
		ConfigureSave();
		SamplerTypeSelect();
	}
	else
		ConfigureLoad();
}




void menu_ExName( void )
{
	static  struct  uMenu  const  menu[] = 
	{
		{ 0x0201u, "名称配置" },
		{ 0x0602u, "仪器名称" },	
		{ 0x0E02u, "厂家名称" },
	};
	uint8_t item = 0u;
	BOOL	changed = FALSE;
	BOOL	need_draw = TRUE;
	do {
		if( need_draw )
		{
			cls();
			Menu_Redraw( menu );	
			need_draw = FALSE;
		}	
		Lputs( 0x0602u, "仪器名称:" );	 
		Lputs( 0x0A02, szNameIdent[Configure.InstrumentName] );
		Lputs( 0x0E02u, "厂家名称:" );	 
		Lputs( 0x1202, ExNameIdent1[Configure.ExName] );
		Lputs( 0x1602, ExNameIdent2[Configure.ExName] );
		item = Menu_Select( menu, item, NULL );
		switch ( item )
		{
		case 1:
			need_draw = TRUE;
			++Configure.InstrumentName;
			if ( Configure.InstrumentName >= (sizeof( szNameIdent ) / sizeof( szNameIdent[0] )))
			{
				Configure.InstrumentName = 0u;
			}
			changed = TRUE;
			break;
		case 2:	
			need_draw = TRUE;
			++ Configure.ExName;
			if ( Configure.ExName >= Name_Max )
			{
				Configure.ExName = 0u;
			}
			changed = TRUE;
			break;
		case enumSelectESC:
			if( changed == TRUE )
				switch( MsgBox( "保存修改结果?", vbYesNoCancel | vbDefaultButton3 ) )
				{
				case vbYes :
					break;
				case vbNo :
					changed = FALSE;
					break;
				default :
				case vbCancel :
					item = 1;
					break;
				}
			break;
		default:
			break;
		}
	} while ( enumSelectESC != item );
	
	if ( changed )
	{
		ConfigureSave();
	}
	else
		ConfigureLoad();
}

void	ConfigureLoadDefault( void )
{
	switch ( Configure.InstrumentType )
	{
	case type_KB6120C:		ConfigureLoad_KB6120C();	break;
	case type_KB6120B:		ConfigureLoad_KB6120B();	break;
	case type_KB2400D:		ConfigureLoad_KB2400D();	break;
	case type_KB2400:			ConfigureLoad_KB2400();		break;
	}
 	if ( Configure.DataValidMask != 0x5A8B )  //注意：此校验值与 DataAccess.c中的校验值要保持一致!
	{
		ConfigureLoadDefault_KB6120E();
		Configure.DataValidMask = 0x5A8B;
	}
}

/********  (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司  **** End Of File ****/

