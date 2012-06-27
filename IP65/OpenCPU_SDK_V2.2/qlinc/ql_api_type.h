#ifndef __QL_API_TYPE_H__
#define __QL_API_TYPE_H__
#include "ql_type.h"
#include "ql_appinit.h"
#include "ql_stdlib.h"
#include "ql_trace.h"

typedef void  (*OCPU_CB_SET_CSCA)(bool result);
typedef void  (*OCPU_CB_GET_CSCA)(bool result, u8* srvNum, u8 len);
typedef void  (*OCPU_CB_CMGR)(char* buffer,u16 length);
typedef void  (*OCPU_CB_CMTI)(char* buffer,u16 length);
typedef void  (*OCPU_CB_CMGL)(char* buffer,u16 length);
#endif  // End-of __QL_OCPU_API_H__

