/**************** (C) COPYRIGHT 2012 青岛金仕达电子科技有限公司 *****************
* 文 件 名: SED1335.C
* 创 建 者: Dean
* 描  述  : LM2068E 液晶读写程序
*         : 
* 最后修改: 2015年8月6日
*********************************** 修订记录 ***********************************
* 版  本: 
* 修订人: 
*******************************************************************************/
#include "BSP.H"
#include "BIOS.H"

extern	void	SED1335_PortInit( void );
extern	void	SED1335_Write_Command( uint8_t uDat );
extern	void	SED1335_Write_Data( uint8_t uDat );
extern  void	LM2068E_GrayInit( void );
extern	uint8_t	SED1335_Read_Data( void );

/*SED1335 命令定义*/
#define	SYSTEM_SET	0x40U
#define SCROLL			0x44U
#define SLEEP_IN		0x53U
#define DISP_ON			0x59U
#define DISP_OFF		0x58U
#define OVLAY			  0x5BU
#define HDOT_SCR		0x5AU
#define CSRRFORM		0x5DU
//#define CSRDIR
#define CGRAM_ADR		0x5CU
#define CSRW			  0x46U
#define CSRR			  0x47U
#define MWRITE			0x42U
#define MREAD			  0x43U
#define	GRAYSCALE		0x60U

/* LCM320240 尺寸定义 */
#define ScreenWidth		320u
#define	ScreenHeight	240u
#define	Row_Height		8u		// 每行点数
#define max_txt_row		30u		// 行（=ScreenHeight/RowHeight）
#define max_txt_col		40u		// 列
static	void	moveto( uint8_t xx, uint8_t yy )
{
    uint16_t	locate;
		locate = ( max_txt_col * (xx) ) + yy;
		
    SED1335_Write_Command( CSRW );
    SED1335_Write_Data( locate % 0x100u );	// 显示内存低8位数值
    SED1335_Write_Data( locate / 0x100u );	// 显示内存高8位数值
}
// static	void	moveto24( uint16_t xx, uint16_t yy )
// {
//     uint16_t	locate = ( max_txt_col * ( uint8_t )(xx) ) + (uint8_t)( yy / 8 );

//     SED1335_Write_Command( CSRW );
//     SED1335_Write_Data( locate % 0x100u );	// 显示内存低8位数值
//     SED1335_Write_Data( locate / 0x100u );	// 显示内存高8位数值
// }
static	const uint8_t * transpose16x16( const uint8_t * pDot, uint8_t Dot16x16[32] )
{
	uint8_t i, j;
	
	for ( j = 0; j < 8; ++j )
	{
		uint8_t tmp = 0u;
		uint8_t mask = 1 << j;

		for ( i = 0; i < 8; ++i )
		{
			tmp <<= 1;
			if ( pDot[i] & mask )
			{	tmp |= 1; }
		}
		
		Dot16x16[j] = tmp;
	}

	for ( j = 0; j < 8; ++j )
	{
		uint8_t tmp = 0u;
		uint8_t mask = 1 << j;

		for ( i = 0; i < 8; ++i )
		{
			tmp <<= 1;
			if ( pDot[i+8] & mask )
			{	tmp |= 1; }
		}
		
		Dot16x16[j+16] = tmp;
	}

	for ( j = 0; j < 8; ++j )
	{
		uint8_t tmp = 0u;
		uint8_t mask = 1 << j;

		for ( i = 0; i < 8; ++i )
		{
			tmp <<= 1;
			if ( pDot[i+16] & mask )
			{	tmp |= 1; }
		}
		
		Dot16x16[j+8] = tmp;
	}

	for ( j = 0; j < 8; ++j )
	{
		uint8_t tmp = 0u;
		uint8_t mask = 1 << j;

		for ( i = 0; i < 8; ++i )
		{
			tmp <<= 1;
			if ( pDot[i+24] & mask )
			{	tmp |= 1; }
		}
		
		Dot16x16[j+24] = tmp;
	}

	return	Dot16x16;
}

static	const uint8_t * transpose8x16( const uint8_t * pDot, uint8_t Dot8x16[16] )
{
	uint8_t i, j;
	
	for ( j = 0; j < 8; ++j )
	{
		uint8_t tmp = 0u;
		uint8_t mask = 1 << j;

		for ( i = 0; i < 8; ++i )
		{
			tmp <<= 1;
			if ( pDot[i] & mask )
			{	tmp |= 1; }
		}
		
		Dot8x16[j] = tmp;
	}

	for ( j = 0; j < 8; ++j )
	{
		uint8_t tmp = 0u;
		uint8_t mask = 1 << j;

		for ( i = 0; i < 8; ++i )
		{
			tmp <<= 1;
			if ( pDot[i+8] & mask )
			{	tmp |= 1; }
		}
		
		Dot8x16[j+8] = tmp;
	}

	return	Dot8x16;
}

void	SED1335_puts( uint16_t yx, const CHAR * sz )
{
	uint8_t	DotArray[32];
	CGROM		pDot;
	CHAR		sDat;
	uint8_t		i, row, col, col_end;

	assert( sz );
	row = ( yx ) / 256u;
	col = ( yx ) % 256u;
	assert( row < max_txt_row );
	assert( col < max_txt_col );
	
	col_end = col + strlen( sz );
	if ( col_end > max_txt_col )
	{
		col_end = max_txt_col;
	}

	SED1335_Write_Command( 0x4Fu );	//	设置连续读写的方向

	do {
			sDat = *sz++;

			if (( sDat & 0x80u ) == 0u )
			{ 	// DBC 半角英文
				pDot = transpose8x16( DotSeekDBC( sDat ), DotArray );
				moveto( Row_Height * row , col++ );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入
				for( i = 0u; i < 16u; ++i )
				{
						SED1335_Write_Data( *pDot++ );
				}
			}
			else
			{	// SBC 全角汉字
				pDot = transpose16x16( DotSeekSBC( sDat, *sz++ ), DotArray );
				moveto( Row_Height * row , col++ );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入
				for( i = 0u; i < 16u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
				moveto( Row_Height * row, col++ );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入
				for( i = 0u; i < 16u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
			}
	} while ( col < col_end  );
}

void	SED1335_puts24( uint16_t yx, const CHAR * sz )
{
 	uint8_t i;
	uint8_t row, col, szcol, col_end, slen;
	CHAR		sDat;
	CGROM		pDot;
	assert( sz );
	row = ( yx ) / 256u;
	col = ( ( yx ) % 256u );
	szcol = col;
	assert( row < max_txt_row );
	assert( col < max_txt_col );
	slen = strlen( sz );
	col_end = col + slen;
	if ( col_end > max_txt_col )
	{
		col_end = max_txt_col;
	}

	SED1335_Write_Command( 0x4Fu );	//	设置连续读写的方向

	do {
			sDat = *sz++;
			
			if (( sDat & 0x80u ) == 0u )
			{ 	// DBC 半角英文
				moveto( Row_Height * row , szcol);	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入
				pDot = DotSeekDBC_12x24( sDat );		
				for( i = 0u; i < 24u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
				moveto( Row_Height * row , szcol + 1 );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入		
				for( i = 0u; i < 24u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
				col += 1;
				szcol +=2;
			}
			else
			{	// SBC 全角汉字
				pDot = ( DotSeekSBC_24x24( sDat, *sz++ ));
				moveto( Row_Height * row , szcol );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入						
				for( i = 0u; i < 24u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
				moveto( Row_Height * row , szcol + 1 );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入
				for( i = 0u; i < 24u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
				moveto( Row_Height * row , szcol + 2 );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续写入
				for( i = 0u; i < 24u; ++i )
				{
					SED1335_Write_Data( *pDot++ );
				}
				col += 2;
				szcol += 3;
				}
	} while ( ( col < col_end ) &&( szcol < max_txt_col ) );
}

void	SED1335_mask( uint16_t yx, uint8_t xlen )
{
	uint8_t i, row, col, col_end;

	assert( 0u != xlen );

	row = ( yx ) / 256u;
	col = ( yx ) % 256u;

	assert( row < max_txt_row );
	assert( col < max_txt_col );

	col_end = col + xlen;
	if ( col_end > max_txt_col )
	{
		col_end = max_txt_col;
	}

	SED1335_Write_Command( 0x4Fu );	//	设置连续读写的方向

	do {
		uint8_t	tmp[16u];

		moveto( Row_Height * row, col );	//	设置地址
		SED1335_Write_Command( MREAD ); 	//	连续 读
		for( i = 0u; i < 16u; i++ )
		{
			tmp[i] = SED1335_Read_Data( );
		}
        
		moveto( Row_Height * row, col );	//	设置地址
		SED1335_Write_Command( MWRITE );	//	连续 写
		for( i = 0u; i < 16u; i++ )
		{
			SED1335_Write_Data( ~ tmp[i] );
		}
	} while ( ++col < col_end );
}
const	CHAR	CHARsz[24]={"00000000000000000000000"};
void	SED1335_mask24( uint16_t yx, uint8_t xlen, const CHAR * sz )
{
	uint8_t	t,i, row, col, szcol, col_end;
	uint8_t	tmp[24u];
	CHAR		sDat = 0;
	assert( 0u != xlen );
	
	row = ( yx ) / 256u;
	col = ( yx ) % 256u;
	szcol = col;
	assert( row < max_txt_row );
	assert( col < max_txt_col );

	col_end = col + xlen;
	if ( col_end > max_txt_col )
	{
		col_end = max_txt_col;
	}

	SED1335_Write_Command( 0x4Fu );	//	设置连续读写的方向
	 
	do {		
		sDat = * sz++  ;			
		if ((( sDat & 0x80u ) != 0u ) && ( * sz != '0' ) )
		{
			for( t = 0; t < 3; t ++ )
			{
				moveto( Row_Height * row, szcol + t );	//	设置地址
				SED1335_Write_Command( MREAD ); 	//	连续 读
				for( i = 0u; i < 24u; i++ )
				{
					tmp[i] = SED1335_Read_Data( );
				}
				moveto( Row_Height * row, szcol + t );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续 写
				for( i = 0u; i < 24u; i++ )
				{
					SED1335_Write_Data( ~ tmp[i] );
				}
			}
			col   += 2;
			szcol += 3;
			sz++;	
		}			
		else
		{
			for( t = 0; t < 2; t ++ )
			{
				moveto( Row_Height * row, szcol + t );	//	设置地址
				SED1335_Write_Command( MREAD ); 	//	连续 读
				for( i = 0u; i < 24u; i++ )
				{
					tmp[i] = SED1335_Read_Data( );
				}
				moveto( Row_Height * row, szcol + t );	//	设置地址
				SED1335_Write_Command( MWRITE );	//	连续 写
				for( i = 0u; i < 24u; i++ )
				{
					SED1335_Write_Data( ~ tmp[i] );
				}
			}	
			col   += 1;
			szcol += 2;
		}
	} while ( col < col_end );
}
void SED1335_WALL( const CHAR *sz )
{
	uint8_t i,c;
	uint8_t row, col;
	assert( sz );
	SED1335_Write_Command( 0x4Fu );	//	设置连续读写的方向
	row = col = 0;
	for( c =0u; c < max_txt_col; c++  )//从上到下从左到右
	{
		row = 0;
		moveto( row, col );	//	设置地址
		SED1335_Write_Command( MWRITE );	//	连续 写
		for( i = 0u; i < max_txt_row * 8 ; i++ )
		{
			SED1335_Write_Data( * sz++ );
			moveto(  row++, col );	//	设置地址
			SED1335_Write_Command( MWRITE );	//	连续 写
      delay_us( 50u );
		}
		col++;
		delay( 20u );
	}	
}
void SED1335_WBMP( uint16_t bmpyx, uint16_t yx, const CHAR *sz )
{
	uint16_t i,c;
	uint8_t row, col;
	uint8_t brow, bcol;
	brow = bmpyx / 256;
	bcol = bmpyx % 256; 
	row = ( yx ) / 256u;
	col = ( yx ) % 256u;
	assert( row < max_txt_row );
	assert( col < max_txt_col );
// 	assert( sz );
	assert( brow < max_txt_row );
	assert( bcol < max_txt_col );

	SED1335_Write_Command( 0x4Fu );	//	设置连续读写的方向
	for( c =0u; c < brow; c++  )
	{
		moveto( row * Row_Height + c, col);	//	设置地址
		SED1335_Write_Command( MWRITE );	//	连续 写
		for( i = 0u; i < ( bcol + 7 ) / 8; i++ )
		{
			moveto( row * Row_Height + c, col + i );	//	设置地址
			SED1335_Write_Command( MWRITE );	//	连续 写
			SED1335_Write_Data( * sz++ );
		}
	}	
}

void	SED1335_cls( void )
{
	uint16_t i;

	SED1335_Write_Command( 0x4Cu );		//	设置连续读写的方向
	moveto( 0u, 0u );					//	设置地址
	SED1335_Write_Command( MWRITE );	//	连续写入

	for ( i = 32768u; i != 0u; --i )
	{
		SED1335_Write_Data( 0u );
	}
}
void	SED1335Part_cls( uint16_t clsyx, uint16_t yx )	//局部清屏
{
	uint8_t i,c;
	uint8_t row, col;
	uint8_t brow, bcol;
  
	brow = clsyx / 256;
	bcol = clsyx % 256; 
	row = ( yx ) / 256u;
	col = ( yx ) % 256u;

	SED1335_Write_Command( 0x4Cu );	//	设置连续读写的方向
	for( c =0u; c < brow; c++  )
	{
		moveto( row * Row_Height + c, col);	//	设置地址
		SED1335_Write_Command( MWRITE );	//	连续 写
		for( i = 0u; i < ( bcol + 7 ) / 8; i++ )
		{
			SED1335_Write_Data( 0x00u );
		}
	}	
}
static	void	SystemSet( void )
{
	//	系统设置指令
	SED1335_Write_Command( 0x40 );	// System Set(带8个参数)
	SED1335_Write_Data( 0x30 ); 	// IV=1, W/S=0, M2=0, M0=0
	SED1335_Write_Data( 0x87 ); 	// WF=1 FX=8
	SED1335_Write_Data( 0x07 ); 	// FY=8
	SED1335_Write_Data( 0x28 ); 	// C/R=40个字符/行，LM2068
	SED1335_Write_Data( 0x47 ); 	// TC/R
	SED1335_Write_Data( 0xEF ); 	// L/F=240, LM2068
	SED1335_Write_Data( 0x28 ); 	// AP= 0030H
	SED1335_Write_Data( 0x00 );
}

bool	SED1335_Init( void )
{
  SED1335_PortInit( );

	SystemSet( );
	delay( 10u );
	SystemSet( );

	//	显示区域设置指令
	SED1335_Write_Command( 0x44 );	// Scroll(带10个参数)
	SED1335_Write_Data( 0x00 ); 	//	SAD1=3000H
	SED1335_Write_Data( 0x30 ); 	//	
	SED1335_Write_Data( 0xEF );		//	SL1=240行
	SED1335_Write_Data( 0x00 ); 	//	SAD2=0000H
	SED1335_Write_Data( 0x00 ); 	//	
	SED1335_Write_Data( 0xEF ); 	//	SL2=240行
	SED1335_Write_Data( 0x00 ); 	//	SAD3=6000H
	SED1335_Write_Data( 0x60 ); 	//	
	SED1335_Write_Data( 0x00 ); 	//	SAD4=9000H
	SED1335_Write_Data( 0x90 ); 	//	
	//	显示功能设置
	SED1335_Write_Command( 0x5A );	// HDOT SCR(带1个参数)
	SED1335_Write_Data( 0x00 );		//
	SED1335_Write_Command( 0x5B );	// OVLAY(带1个参数)
	SED1335_Write_Data( 0x01 );		//	OV=0, DM1=0, DM0=0, MX=01(XOR)
	SED1335_Write_Command( 0x5D );	// CSRFORM(带2个参数)
	SED1335_Write_Data( 0x07 );		//	CX=8
	SED1335_Write_Data( 0x17 );		//	CM=1, CY=8
	//	显示开关指令
	SED1335_Write_Command( 0x59 );	// Display ON/OFF(带1个参数)
	SED1335_Write_Data( 0x14 );		//	第1、2显示区显示开
	
	SED1335_cls();
	LM2068E_GrayInit();
	
	return	true;
}

/********  (C) COPYRIGHT 2015 青岛金仕达电子科技有限公司  **** End of File ****/
