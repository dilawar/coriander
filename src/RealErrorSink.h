/*
 * Copyright (C) 2000-2002 Damien Douxchamps  <douxchamps@ieee.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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
