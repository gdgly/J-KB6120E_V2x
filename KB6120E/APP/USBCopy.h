#include "stm32f10x.h"
#include "AppDEF.H"
#ifndef	__bool_true_false_are_defined
	#include <stdbool.h>
	typedef	bool	BOOL;
#endif
extern BOOL	USBPrint_TSP( uint16_t FileNum, struct uFile_TSP_SHI_R24 const * pFile );
extern BOOL USBPrint_R24_SHI( enum enumSamplerSelect SamplerSelect, uint16_t FileNum, struct uFile_TSP_SHI_R24 const * pFile );

extern BOOL USB_PrintInit(void);
/***************END************************/
