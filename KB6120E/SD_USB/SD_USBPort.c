/**************** (C) COPYRIGHT 2014 青岛金仕达电子科技有限公司 ****************
* 文 件 名: SD_USBPort.c
* 创 建 者: Kaiser
* 描  述  : SD卡及U盘接口
* 最后修改: 2015/09/10
*********************************** 修订记录 ***********************************
* 版  本:
* 修订人:
*******************************************************************************/
#include "AppDEF.H"
#include "string.h"
#include "SD_USBPort.h"

#define SD_Card         0x03
#define USB_Flash_Disk  0x06

UINT8     bufread[100];
uint8_t     i,s;
uint16_t xy;

//创建根目录下的目录并打开 (目录名必须为大写英文数字或者特殊符号且不能超过8个字节)
void Byte_CREAT_CON_DIR(const char * BUF_Name)
{
	bus_SPI_mutex_apply();

	s = CH376DirCreate( (PUINT8)BUF_Name );       /* 在根目录下新建目录(文件夹)并打开,如果目录已经存在那么直接打开 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/***/
//创建根目录下的文件并写
void Byte_CREAT_WRITE_DIR(const char * BUF_Name, uint8_t * Content, uint32_t offset )
{
	bus_SPI_mutex_apply();

	CH376FileCreate( (PUINT8)BUF_Name );          /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	CH376ByteWrite( Content, offset, NULL );      /* 以字节为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/***/
//打开根目录下的文件并读
void  Byte_OPEN_READ_DIR (const char * BUF_Name , uint32_t Addr, uint8_t * bufread , uint32_t offset  )
{
	bus_SPI_mutex_apply();

	CH376FileOpen( (PUINT8)BUF_Name );            /* 在根目录或者当前目录下打开文件或者目录(文件夹) */
	CH376ByteLocate( Addr );

	CH376ByteRead( bufread, offset , NULL );      /* 以字节为单位从当前位置读取数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */


	bus_SPI_mutex_release();
}
/***/
//打开根目录下的文件并写
void  Byte_OPEN_WRITE_DIR (const char * BUF_Name, uint8_t * Content , uint32_t Addr, uint32_t offset  )
{
	bus_SPI_mutex_apply();

	CH376FileOpen( (PUINT8)BUF_Name );            /* 在根目录或者当前目录下打开文件或者目录(文件夹) */

	CH376ByteLocate( Addr );
	CH376ByteWrite( Content, offset, NULL );      /* 以字节为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */


	bus_SPI_mutex_release();
}
/***/
//创建多级目录下的文件并写
void Byte_CREAT_WRITE_PATH(const char * BUF_Name, uint8_t * Content , uint32_t offset )
{
	//                          文件名                文件内容
	bus_SPI_mutex_apply();
	/* 目标文件名 */
	CH376FileCreatePath( (PUINT8)BUF_Name );      /* 新建多级目录下的文件,支持多级目录路径,输入缓冲区必须在RAM中 */
	CH376ByteWrite( Content, offset, NULL );      /* 以字节为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */


	bus_SPI_mutex_release();
}
/***/
//打开多级目录下的文件并读
uint8_t  Byte_OPEN_READ_PATH (const char * BUF_Name , uint32_t Addr , uint8_t * bufread , uint32_t offset )
{
	bus_SPI_mutex_apply();

	CH376FileOpenPath( (PUINT8)BUF_Name );        /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */

	CH376ByteLocate( Addr );
	s = CH376ByteRead( bufread, offset, NULL );   /* 以字节为单位从当前位置读取数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */


	bus_SPI_mutex_release();
	return s;
}
/**/
//打开多级目录下的文件并写
uint8_t Byte_OPEN_WRITE_PATH (const char * BUF_Name, uint8_t * Content , uint32_t Addr ,uint32_t offset)
{
	bus_SPI_mutex_apply();

	CH376FileOpenPath( (PUINT8)BUF_Name );        /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */

	CH376ByteLocate( Addr );
	s = CH376ByteWrite( Content, offset, &xy );   /* 以字节为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */


	bus_SPI_mutex_release();
	return s;
}
/**/
//字节读
BOOL ByteLoad( const char * B_Name , uint32_t Addr, uint8_t * bufread, uint32_t offset  )
{
	//文件名称       读起始地址    要读的数据长度
	bus_SPI_mutex_apply();

	if( Byte_OPEN_READ_PATH ( B_Name  , Addr  ,bufread , offset ) == USB_INT_SUCCESS )
	{
		bus_SPI_mutex_release();
		return TRUE;
	}
	else
	{
		bus_SPI_mutex_release();
		return FALSE;
	}

}
//字节存
BOOL ByteSave( const char * BUF_Name ,  uint8_t * Content , uint32_t Addr, uint32_t offset  )
{
	//    文件名称        要写的内容    写起始地址
	bus_SPI_mutex_apply();
	s = Byte_OPEN_WRITE_PATH ( BUF_Name , Content , Addr, offset );
	bus_SPI_mutex_release();

	if( s == USB_INT_SUCCESS )
		return TRUE;
	else if(s == USB_INT_BUF_OVER)
	{
		beep();
		return FALSE;
	}
	else
		return FALSE;

}
//字节填充
BOOL ByteFill( const char * BUF_Name , uint32_t Addr , uint16_t size)
{
	//    文件名称      写起始地址          要写的数量
	UINT8     buf[100];
	bus_SPI_mutex_apply();

	CH376FileOpenPath( (PUINT8)BUF_Name );        /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	CH376ByteLocate( Addr );
//  memset( buf, 0x00, 100 );
	s = CH376ByteWrite( buf, size, NULL );        /* 以字节为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */


	bus_SPI_mutex_release();

	if( s == USB_INT_SUCCESS )
		return TRUE;
	else if(s == USB_INT_BUF_OVER)
	{
		beep();
		return FALSE;
	}
	else
		return FALSE;
}
//以字节为单位获得文件指针的位置
uint32_t ByteGetSize(const char * BUF_Name)
{
	uint32_t bufoffset;
	bus_SPI_mutex_apply();
//  strcpy( (char *)TarName, BUF_Name );
	CH376FileOpenPath( (PUINT8)BUF_Name );        /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	bufoffset = CH376GetFileSize( );
	bus_SPI_mutex_release();
	return bufoffset;
}
//创建根目录下的目录并打开
void Sec_CREAT_CON_DIR(const char * BUF_Name)
{
	bus_SPI_mutex_apply();

	s=CH376DirCreate( (PUINT8)BUF_Name );         /* 在根目录下新建目录(文件夹)并打开,如果目录已经存在那么直接打开 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/***/

//创建根目录下的文件并写
void Sec_CREAT_WRITE_DIR(const char * BUF_Name, uint8_t * Content, uint8_t offset )
{
	bus_SPI_mutex_apply();

	CH376FileCreate( (PUINT8)BUF_Name );          /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	CH376SecLocate( 0 );
	CH376SecWrite( Content, offset, NULL );       /* 以扇区为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于扇区读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/***/

//打开根目录下的文件并读
void  Sec_OPEN_READ_DIR (const char * BUF_Name , uint32_t Addr, uint8_t * bufread , uint8_t offset  )
{
	bus_SPI_mutex_apply();

	CH376FileOpen( (PUINT8)BUF_Name );            /* 在根目录或者当前目录下打开文件或者目录(文件夹) */
	CH376SecLocate( Addr );

	CH376SecRead( bufread, offset , NULL );       /* 以扇区为单位从当前位置读取数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于扇区读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/***/

//打开根目录下的文件并写
void  Sec_OPEN_WRITE_DIR (const char * BUF_Name, uint8_t * Content , uint32_t Addr, uint8_t offset  )
{
	bus_SPI_mutex_apply();

	CH376FileOpen( (PUINT8)BUF_Name );            /* 在根目录或者当前目录下打开文件或者目录(文件夹) */
	CH376SecLocate( Addr );
	CH376SecWrite( Content , offset, NULL );      /* 以扇区为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于扇区读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/**/

//创建多级目录下的文件并写
void Sec_CREAT_WRITE_PATH(const char * BUF_Name, uint8_t * Content , uint8_t offset )
{
	//													文件名                文件内容               文件大小
	bus_SPI_mutex_apply();
	
	s = CH376FileCreatePath( (PUINT8)BUF_Name );  	/* 新建多级目录下的文件,支持多级目录路径,输入缓冲区必须在RAM中 */

	s = CH376SecWrite( Content , offset, NULL ); /* 以扇区为单位向当前位置写入数据块 */
	s = CH376FileClose( TRUE );   /* 关闭文件,对于扇区读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
}
/**/

//打开多级目录下的文件并读
uint8_t  Sec_OPEN_READ_PATH (const char * BUF_Name , uint32_t Addr , uint8_t * bufread , uint8_t offset )
{
	bus_SPI_mutex_apply();

	CH376FileOpenPath( (PUINT8)BUF_Name );/* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	//bufoffset =	CH376GetFileSize( );
	CH376SecLocate( Addr );
	s = CH376SecRead(  bufread , offset ,NULL );/* 以扇区为单位从当前位置读取数据块 */
	CH376FileClose( TRUE ); /* 关闭文件,对于扇区读写建议自动更新文件长度 */
	bus_SPI_mutex_release();
	return s;
}
/**/
//打开多级目录下的文件并写
uint8_t Sec_OPEN_WRITE_PATH (const char * BUF_Name, uint8_t * Content , uint32_t Addr ,uint8_t offset)
{
	bus_SPI_mutex_apply();
	
	CH376FileOpenPath( (PUINT8)BUF_Name );/* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */

	CH376SecLocate( Addr );
	s = CH376SecWrite(Content, offset, NULL ); /* 以扇区为单位向当前位置写入数据块 */
	CH376FileClose( TRUE ); /* 关闭文件,对于扇区读写建议自动更新文件长度 */

	bus_SPI_mutex_release();
	return s;
}
/**/
//扇区读
BOOL SecLoad( const char * B_Name , uint32_t Addr, uint8_t * bufread, uint8_t offset  )
{
	//文件名称       读起始地址    									要读的扇区长度
	bus_SPI_mutex_apply();

	if( Sec_OPEN_READ_PATH ( B_Name, Addr, bufread, offset ) == USB_INT_SUCCESS )
	{
		bus_SPI_mutex_release();
		return TRUE;
	}
	else
	{
		bus_SPI_mutex_release();
		return FALSE;
	}
}
//扇区存
BOOL SecSave( const char * BUF_Name,  uint8_t * Content, uint32_t Addr, uint8_t offset  )
{
	//		文件名称 				要写的内容 		写起始地址		     要写的扇区数
	bus_SPI_mutex_apply();
	s = Sec_OPEN_WRITE_PATH ( BUF_Name, Content, Addr, offset );
	bus_SPI_mutex_release();

	if( s == USB_INT_SUCCESS )
		return TRUE;
	else if( s == USB_INT_BUF_OVER )
	{
		return FALSE;
	}
	else
		return FALSE;

}
//以扇区为单位填充
BOOL SecFill( const char * BUF_Name , uint32_t Addr , uint8_t size)
{
	//    文件名称      写起始地址          要写的数量
	UINT8     buf[100];
	bus_SPI_mutex_apply();

	CH376FileOpenPath( (PUINT8)BUF_Name );        /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	CH376ByteLocate( Addr );
	memset( buf, 0xaa, 100 );
	s = CH376ByteWrite( buf, size, NULL );        /* 以字节为单位向当前位置写入数据块 */
	CH376FileClose( TRUE );                       /* 关闭文件,对于字节读写建议自动更新文件长度 */

	memset( buf, 0, 100 );
	bus_SPI_mutex_release();

	if( s == USB_INT_SUCCESS )
		return TRUE;
	else if(s == USB_INT_BUF_OVER)
	{
		beep();
		return FALSE;
	}
	else
		return FALSE;
}
//以扇区为单位获得文件指针位置
uint32_t SecGetSize(const char * BUF_Name)
{
	uint32_t bufoffset;
	bus_SPI_mutex_apply();

	CH376FileOpenPath( (PUINT8)BUF_Name );        /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
	bufoffset = CH376GetFileSize( );
	bus_SPI_mutex_release();
	return bufoffset;
}


/*
	初始化
*/

void SD_Init(void)
{

	s = mInitCH376Host(SD_Card);       /* 初始化CH376--SD卡模式*/

	if( s != 0xFA )
	{
		for ( i = 0; i < 50; i ++ )
		{
			mDelaymS(2);
			s = CH376DiskMount( );  //初始化磁盘并测试磁盘是否就绪.

			if ( s == USB_INT_SUCCESS ) /* 准备好 */
			{
				break;
			}
			else if ( s == ERR_DISK_DISCON )/* 检测到断开,重新检测并计时 */
			{
				break;
			}

			if ( CH376GetDiskStatus( ) >= DEF_DISK_MOUNTED && i >= 5 ) /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
			{
				break;
			}
		}

		SD_File_Creat();
	}

	
}

// #define	FilePageSize_TSP_SHI_R24  40u

uint8_t  sdinit[16]= {"SDInitSuccessful"};
void	SD_File_Creat( void )
{
	BOOL SetFlag = FALSE;

	Byte_CREAT_CON_DIR("\\SAMPLER");	//创建工作目录	( 如果已经存在，直接打开 )

	for ( i = 0; i < SP_Max; i ++ )
	{
		if( ! ByteLoad( Save_NameChar[i], 0, (uint8_t *) bufread, 15 ) )
		{
			Byte_CREAT_WRITE_PATH( Save_NameChar[i], sdinit,16 );// 不正常 重新创建
			ByteFill( Save_NameChar[i], 16, (FilePageSize_TSP_SHI_R24 - sizeof ( sdinit ) + FilePageSize_TSP_SHI_R24 ));
			SampleSet[(enum enumSamplerSelect)i].FileNum = 0u;
			SetFlag = TRUE;
		}

		if( strncmp( (char*)sdinit, (char*)bufread, 15 ) ) //检测文件是否正常 返回零为正常
		{
			Byte_CREAT_WRITE_PATH( Save_NameChar[i], sdinit,16 );//非零 不正常 重新创建
			ByteFill( Save_NameChar[i], 16, ( FilePageSize_TSP_SHI_R24 - sizeof ( sdinit ) + FilePageSize_TSP_SHI_R24 ));
			SampleSet[(enum enumSamplerSelect)i].FileNum = 0u;
			SetFlag = TRUE;
		}
	}

	if( ! ByteLoad( "\\SAMPLER\\POWER", 0, (uint8_t *) bufread, 15 ) )
	{
		Byte_CREAT_WRITE_PATH( "\\SAMPLER\\POWER", sdinit,16 );// 不正常 重新创建
		ByteFill( "\\SAMPLER\\POWER", 16, ( 100 + 4 -  sizeof ( sdinit ) ));
	}

	if( strncmp( (char*)sdinit, (char*)bufread, 15 ) ) //检测文件是否正常 返回零为正常
	{
		Byte_CREAT_WRITE_PATH( "\\SAMPLER\\POWER", sdinit,16 );//非零 不正常 重新创建
		ByteFill( "\\SAMPLER\\POWER", 16, (100 + 4 - sizeof ( sdinit ) ));
	}

	if(SetFlag == TRUE )
	{
		SampleSetSave();
		SetFlag = FALSE;
	}

	Byte_CREAT_CON_DIR("\\SD_ERR");	//创建工作目录	( 如果已经存在，直接打开 )

	if( ! ByteLoad( "\\SD_ERR\\ERR.TXT"	, 0, (uint8_t *) bufread, 15 ) )
	{
		Byte_CREAT_WRITE_PATH("\\SD_ERR\\ERR.TXT"	, sdinit, 15 );// 不正常 重新创建
	}

}
/**/


void USB_File_Creat(void)
{

	const	char	* const USB_Loading[6]=
	{
		"Loading.",
		"Loading..",
		"Loading...",
		"Loading....",
		"Loading.....",
		"Loading......",
	};
	uint8_t USBinit[512]= {"USBInitSuccessful"};
	uint8_t i;
	Byte_CREAT_CON_DIR("\\SAMPLER");	//创建工作目录	( 如果已经存在，直接打开 )

	for( i = 0; i < SamplerHasMax; i ++ )
	{
		Lputs( 0x0C0A,USB_Loading[SamplerTypeHas[i]]);

		if( ! ByteLoad( Save_NameChar[SamplerTypeHas[i]], 0, (uint8_t *) bufread, 15 ) )
		{
			Byte_CREAT_WRITE_PATH( Save_NameChar[SamplerTypeHas[i]], USBinit,512);// 不正常 重新创建
		}

		if( strncmp( (char*)USBinit, (char*)bufread, 15 ) ) //检测文件是否正常 返回零为正常
		{
			Byte_CREAT_WRITE_PATH( Save_NameChar[SamplerTypeHas[i]], USBinit,512 );//非零 不正常 重新创建
		}
	}

}


uint8_t USB_CHAK(void)
{
	return CH376DiskMount();
}

uint8_t USB_Init(void)
{
	uint8_t s;

	s = mInitCH376Host(USB_Flash_Disk);   /* 初始化CH376--USB卡模式*/

	if( s != USB_INT_SUCCESS )
		return 1;														  //	硬件故障!

	for ( i = 0; i < 20; i ++ )
	{
		mDelaymS(100);

		if( USB_INT_SUCCESS == CH376DiskConnect( ) )    /* 检查U盘是否连接,不支持SD卡 */
		{
			for ( i = 0; i < 10; i ++ )
			{
				delay_us( 50000 );
				s =	USB_CHAK( );  //初始化磁盘并测试磁盘是否就绪.

				if ( s == USB_INT_SUCCESS ) /* 准备好 */
				{
					return 0;  										 //	正常
				}
				else if ( s == ERR_DISK_DISCON )/* 检测到断开,重新检测并计时 */
				{
					return 3; 										 //	连接已断开!
				}

				if ( CH376GetDiskStatus( ) >= DEF_DISK_MOUNTED && i >= 5 ) /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
				{
					return 4; 										 //	勉强正常
				}
			}
		}
		else
			return 2;													 //	U盘未连接!
	}

	return 5;															 //	未知故障!
}
/**/

/********  (C) COPYRIGHT 2015 青岛金仕达电子科技有限公司  **** End Of File ****/

// 	i = CH376ReadBlock( buf );  /* 如果需要,可以读取数据块CH376_CMD_DATA.DiskMountInq,返回长度 */
// 	if ( i == sizeof( INQUIRY_DATA ) )  /* U盘的厂商和产品信息 */
// 	{
// 		buf[ i ] = 0;
// 	}
// 	s = CH376DiskCapacity( (PUINT32) buf  );

// 	s = CH376DiskQuery ( (PUINT32) buf );	/* 查询磁盘剩余空间信息,扇区数 */








