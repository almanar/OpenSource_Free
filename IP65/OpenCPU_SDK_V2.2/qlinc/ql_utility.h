/*==========================================================================
 |               Quectel OpenCPU --  User API
 |
 |              Copyright (c) 2011 Quectel Ltd.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |      Definitions for Utility
 |
 |--------------------------------------------------------------------------
 |
 |  Designed by   :     Stanley YONG
 |  Coded by      :     Stanley YONG
 |  Tested by     :     Stanley YONG
 |
 \=========================================================================*/
#ifndef __QL_OCPU_UTILITY_H__
#define __QL_OCPU_UTILITY_H__
#include "ql_type.h"


typedef enum tagQl_CodecType
{
  QL_CODEC_UNICODE,
  QL_CODEC_GB2312,
  QL_CODEC_NUMS
}Ql_CodecType;

/*
*  Codec/dstCode:
*   one value of Ql_CodecType.
***********************************/
s32 Ql_ConvertCodec(u8 srcCode, u8* src_p, u8 dstCode, u8* dst_p);

#endif  // End-of __QL_OCPU_UTILITY_H__

