/**************** (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司 ****************
* 文 件 名: Display.C
* 创 建 者: 董峰
* 描  述  : 使用不同接口形式的模块实现点阵显示功能
* 最后修改: 2014年4月18日
*********************************** 修订记录 ***********************************
* 版  本: V2.0
* 修订人: 董峰 2014年4月18日
* 说  明: 自动检测确定显示屏的类型。
*******************************************************************************/
#include "BSP.H"
#include "BIOS.H"

/********************************** 功能说明 ***********************************
*	液晶(TM12864)的背光、灰度控制
*******************************************************************************/
static	void	TM12864_SetLight( uint8_t setLight )
{	//	调整电流调整背光亮度
	PWM1_SetOutput( setLight * PWM_Output_Max / 100u );
}

FP32	LCDSetGrayVolt;

/********************************** 功能说明 ***********************************
 *	设置显示亮度、灰度
*******************************************************************************/

static	uint8_t	savLight;

void	Backlight_ON( void )
{
	TM12864_SetLight( savLight );

}

void	Backlight_OFF( void )
{
	TM12864_SetLight( 0u );

}

void	DisplaySetLight( uint8_t ValueOfBrightness )
{	//	背光亮度
	savLight = ValueOfBrightness;

	Backlight_ON();
}

void	DisplaySetGrayVolt( FP32 SetGrayVolt )
{	//	灰度  或 对比度

	LCDSetGrayVolt = SetGrayVolt;

}

/********************************** 功能说明 ***********************************
 *	点阵显示功能
*******************************************************************************/
void	cls ( void )
{ 
	SED1335_cls();	
}
void Part_cls( void )
{
	SED1335Part_cls( 0xC0D6, 0x060E );
	SED1335_WBMP( 0xB001, 0x050D, STROCK1 );
}
void WBMP( uint16_t bmpyx, uint16_t yx, const CHAR *sz )
{
	SED1335_WBMP( bmpyx, yx, sz );
}
void WALL( const CHAR *sz )
{
	cls();
	SED1335_WALL( sz );
}
void	Lputs( uint16_t yx, const CHAR * sz )
{
	SED1335_puts24( yx, sz );
}
void	SEDLputs( uint16_t yx, const CHAR * sz )
{
	SED1335_puts( yx, sz );
}
void	LcmMask( uint16_t yx, uint8_t xlen, const CHAR * sz )
{
	SED1335_mask24 ( yx, xlen, sz );
}

/********************************** 功能说明 ***********************************
 *	显示模块初始化
*******************************************************************************/
void	Display_Init( void )
{
	//	自动检测显示屏的类型
	SED1335_Init();

}

/********  (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司  **** End Of File ****/
