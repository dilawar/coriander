
#ifndef _REAL_ERROR_SINK_H_
#define _REAL_ERROR_SINK_H_


#ifdef HAVE_REALLIB

// headers from RealProducer 8.5 SDK

#include "pnwintyp.h"
#include "pnresult.h"
#include "pncom.h"
#include "rmaenum.h"
#include "rmapckts.h"
#include "rmmetain.h"
#include "rmaerror.h"
#include "engtypes.h"
#include "engtargs.h"
#include "progsink.h"
#include "rmbldeng.h"
#include "setdllac.h"
#include "RealGuids.h"

class RealErrorSink : public IRMAErrorSink {

public:
  BOOL g_bErrorOccurred;
  PN_RESULT g_nError;

  static void printError (PN_RESULT res);

  static PN_RESULT CreateInstance (IUnknown **pUnk);
  
  virtual PN_RESULT QueryInterface (const IID& riid, void** ppvObj);
  virtual ULONG32 AddRef ();
  virtual ULONG32 Release ();
  
  virtual PN_RESULT ErrorOccurred (
				   const UINT8 unSeverity,  
				   const ULONG32 ulRMACode,
				   const ULONG32 ulUserCode,
				   const char* pUserString,
				   const char* pMoreInfoURL
				   );
  
private:
  UINT32 m_nRefCount;
  
  RealErrorSink();
  virtual ~RealErrorSink();
};

#endif //HAVE_REALLIB

#endif // _REAL_ERROR_SINK_H_
