/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef HAVE_REALLIB
#include "RealErrorSink.h"

//============================================================================ 
PN_RESULT RealErrorSink::CreateInstance(IUnknown **ppUnk) {
//============================================================================ 
  RealErrorSink *pObj = new RealErrorSink();
  
  if(pObj == NULL) {
    *ppUnk = NULL;
    return PNR_OUTOFMEMORY;
  }
  else {
    *ppUnk = (IUnknown*)pObj;
    (*ppUnk)->AddRef();
    return PNR_OK;
  }
}


//============================================================================ 
RealErrorSink::RealErrorSink() : m_nRefCount(0) {
//============================================================================ 
  g_bErrorOccurred = FALSE;
  g_nError = PNR_OK;
}


//============================================================================ 
RealErrorSink::~RealErrorSink() {
//============================================================================ 
}


//============================================================================ 
PN_RESULT RealErrorSink::QueryInterface (const IID& riid, void** ppvObj) {
//============================================================================ 
  if(!ppvObj)
    return PNR_POINTER;

  else if(IsEqualIID(IID_IUnknown, riid)) {
    AddRef();
    *ppvObj = (IUnknown*)this;
    return PNR_OK;
  }

  else if(IsEqualIID(IID_IRMAErrorSink, riid)) {
    AddRef();
    *ppvObj = (IRMAErrorSink*)this;
    return PNR_OK;
  }
  
  *ppvObj = NULL;
  return PNR_NOINTERFACE;
}


//============================================================================ 
ULONG32 RealErrorSink::AddRef() {
//============================================================================ 
  return InterlockedIncrement(&m_nRefCount);
}


//============================================================================ 
ULONG32 RealErrorSink::Release() {
//============================================================================ 
  if (InterlockedDecrement(&m_nRefCount) > 0)
    return m_nRefCount;
  
  delete this;
  return 0;
}


//============================================================================ 
STDMETHODIMP RealErrorSink::ErrorOccurred (const UINT8 unSeverity,  
					   const ULONG32 ulRMACode,
					   const ULONG32 ulUserCode,
					   const char* pUserString,
					   const char* pMoreInfoURL
					   ) {
//============================================================================ 
  printError (ulRMACode);
  
  if(unSeverity <= PNLOG_ERR) {
    g_bErrorOccurred = TRUE;
    g_nError = ulRMACode;
  }
  
  return PNR_OK;
}

//============================================================================ 
void RealErrorSink::printError (PN_RESULT res)
//============================================================================ 
{
  unsigned char cat = res >> 6; // Extract general error category

  char *catString = NULL;

  switch (cat)
    {
    case SS_GLO:
      catString = "General error";
      break;

    case SS_NET:
      catString = "Networking error";
      break;

    case SS_FIL:
      catString = "File error";
      break;

    case SS_PRT:
      catString = "Protocol error";
      break;

    case SS_AUD:
      catString = "Audio error";
      break;

    case SS_INT:
      catString = "General internal error";
      break;

    case SS_USR:
      catString = "User error";
      break;

    case SS_MSC:
      catString = "Miscellaneous error";
      break;

    case SS_DEC:
      catString = "Decoder error";
      break;

    case SS_ENC:
      catString = "Encoder error";
      break;

    case SS_REG:
      catString = "Registry error";
      break;

    case SS_PPV:
      catString = "Pay Per View error";
      break;

    case SS_RSC:
      catString = "Error for PNXRES";
      break;

    case SS_UPG:
      catString = "Auto-upgrade & Certificate error";
      break;

    case SS_PLY:
      catString = "RealPlayer/Plus specific error";
      break;

    case SS_RMT:
      catString = "RMTools error";
      break;

    case SS_CFG:
      catString = "AutoConfig error";
      break;

    case SS_RPX:
      catString = "RealPix-related error";
      break;

    case SS_XML:
      catString = "XML-related error";
      break;

    case SS_DPR:
      catString = "Deprecated error";
      break;

    default:
      catString = "Unidentified error";
    }

  char *errString = NULL; // Precise error message string

  switch (res)
    {
    case PNR_OK:
      fprintf(stderr,"No error detected, everything OK.");
      return;

    case PNR_NOTIMPL:
      errString = "not implemented";
      break;

    case PNR_OUTOFMEMORY:
      errString = "out of memory";
      break;

    case PNR_INVALID_PARAMETER:
      errString = "invalid parameter";
      break;

    case PNR_NOINTERFACE:
      errString = "no interface";
      break;

    case PNR_POINTER:
      errString = "pointer error";
      break;

    case PNR_HANDLE:
      errString = "handle error";
      break;

    case PNR_ABORT:
      errString = "aborted";
      break;

    case PNR_FAIL:
      errString = "failed";
      break;

    case PNR_ACCESSDENIED:
      errString = "access denied";
      break;

    case PNR_IGNORE:
      errString = "ignored";
      break;

    case PNR_INVALID_OPERATION:
      errString = "invalid operation";
      break;

    case PNR_INVALID_VERSION:
      errString = "invalid version";
      break;

    case PNR_INVALID_REVISION:
      errString = "invalid revision";
      break;

    case PNR_NOT_INITIALIZED:
      errString = "not initialized";
      break;

    case PNR_DOC_MISSING:
      errString = "doc missing";
      break;

    case PNR_UNEXPECTED:
      errString = "unexpected error";
      break;

    case PNR_INCOMPLETE:
      errString = "incomplete";
      break;

    case PNR_BUFFERTOOSMALL:
      errString = "buffer too small";
      break;

    case PNR_UNSUPPORTED_VIDEO:
      errString = "unsupported video";
      break;

    case PNR_UNSUPPORTED_AUDIO:
      errString = "unsupported audio";
      break;

    case PNR_INVALID_BANDWIDTH:
      errString = "invalid bandwidth";
      break;

    case PNR_NO_RENDERER:
      errString = "no renderer, no file format or missing components";
      break;

    case PNR_ELEMENT_NOT_FOUND:
      errString = "element not found";
      break;

    case PNR_NOCLASS:
      errString = "no class";
      break;

    case PNR_CLASS_NOAGGREGATION:
      errString = "no aggregation";
      break;

    case PNR_NOT_LICENSED:
      errString = "not licensed";
      break;

    case PNR_NO_FILESYSTEM:
      errString = "no filesystem";
      break;

    case PNR_REQUEST_UPGRADE:
      errString = "request upgrade";
      break;

    case PNR_BUFFERING:
      errString = "buffering";
      break;

    case PNR_PAUSED:
      errString = "paused";
      break;

    case PNR_NO_DATA:
      errString = "no data";
      break;

    case PNR_STREAM_DONE:
      errString = "stream done";
      break;

    case PNR_NET_SOCKET_INVALID:
      errString = "invalid socket";
      break;

    case PNR_NET_CONNECT:
      errString = "couldn't connect";
      break;

    case PNR_BIND:
      errString = "couldn't bind";
      break;

    case PNR_SOCKET_CREATE:
      errString = "couldn't create socket";
      break;

    case PNR_INVALID_HOST:
      errString = "invalid host";
      break;

    case PNR_NET_READ:
      errString = "couldn't read from network";
      break;

    case PNR_NET_WRITE:
      errString = "couldn't write to network";
      break;

    case PNR_NET_UDP:
      errString = "UDP error";
      break;

    case PNR_RETRY:
      errString = "retry";
      break;

    case PNR_SERVER_TIMEOUT:
      errString = "server timeout";
      break;

    case PNR_SERVER_DISCONNECTED:
      errString = "server disconnected";
      break;

    case PNR_WOULD_BLOCK:
      errString = "would block";
      break;

    case PNR_GENERAL_NONET:
      errString = "general no network";
      break;

    case PNR_BLOCK_CANCELED:
      errString = "block cancelled";
      break;

    case PNR_MULTICAST_JOIN:
      errString = "multicast join";
      break;

    case PNR_GENERAL_MULTICAST:
      errString = "general multicast";
      break;

    case PNR_MULTICAST_UDP:
      errString = "multicast UDP";
      break;

    case PNR_AT_INTERRUPT:
      errString = "at interrupt";
      break;

    case PNR_MSG_TOOLARGE:
      errString = "message too large";
      break;

    case PNR_NET_TCP:
      errString = "TCP error";
      break;

    case PNR_TRY_AUTOCONFIG:
      errString = "try autoconfig";
      break;

    case PNR_NOTENOUGH_BANDWIDTH:
      errString = "not enough bandwidth";
      break;

    case PNR_HTTP_CONNECT:
      errString = "HTTP connect error";
      break;

    case PNR_PORT_IN_USE:
      errString = "port in use";
      break;

    case PNR_LOADTEST_NOT_SUPPORTED:
      errString = "loadtest not supported";
      break;

    case PNR_AT_END:
      errString = "at end";
      break;

    case PNR_INVALID_FILE:
      errString = "invalid file";
      break;

    case PNR_INVALID_PATH:
      errString = "invalid path";
      break;

    case PNR_RECORD:
      errString = "record";
      break;

    case PNR_RECORD_WRITE:
      errString = "record write";
      break;

    case PNR_TEMP_FILE:
      errString = "temp file";
      break;

    case PNR_ALREADY_OPEN:
      errString = "already open";
      break;

    case PNR_SEEK_PENDING:
      errString = "seek pending";
      break;

    case PNR_CANCELLED:
      errString = "cancelled";
      break;

    case PNR_FILE_NOT_FOUND:
      errString = "file not found";
      break;

    case PNR_WRITE_ERROR:
      errString = "write error";
      break;

    case PNR_FILE_EXISTS:
      errString = "file exists";
      break;

    case PNR_FILE_NOT_OPEN:
      errString = "file not open";
      break;

    case PNR_ADVISE_PREFER_LINEAR:
      errString = "prefer linear";
      break;

    case PNR_PARSE_ERROR:
      errString = "parse error";
      break;

    case PNR_BAD_SERVER:
      errString = "bad server";
      break;

    case PNR_ADVANCED_SERVER:
      errString = "advanced server error";
      break;

    case PNR_OLD_SERVER:
      errString = "old server error";
      break;

    case PNR_REDIRECTION:
      errString = "redirection eroor";
      break;

    case PNR_SERVER_ALERT:
      errString = "server alert";
      break;

    case PNR_PROXY:
      errString = "proxy error";
      break;

    case PNR_PROXY_RESPONSE:
      errString = "proxy response error";
      break;

    case PNR_ADVANCED_PROXY:
      errString = "advanced proxy error";
      break;

    case PNR_OLD_PROXY:
      errString = "old proxy error";
      break;

    case PNR_INVALID_PROTOCOL:
      errString = "invalid protocol";
      break;

    case PNR_INVALID_URL_OPTION:
      errString = "invalid url option";
      break;

    case PNR_INVALID_URL_HOST:
      errString = "invalid url host";
      break;

    case PNR_INVALID_URL_PATH:
      errString = "invalid url path";
      break;

    case PNR_HTTP_CONTENT_NOT_FOUND:
      errString = "HTTP content not found";
      break;

    case PNR_NOT_AUTHORIZED:
      errString = "not authorized";
      break;

    case PNR_UNEXPECTED_MSG:
      errString = "unexpected message";
      break;

    case PNR_BAD_TRANSPORT:
      errString = "bad transport";
      break;

    case PNR_NO_SESSION_ID:
      errString = "no session id";
      break;

    case PNR_PROXY_DNR:
      errString = "proxy dnr error";
      break;

    case PNR_PROXY_NET_CONNECT:
      errString = "proxy net connect error";
      break;

    case PNR_AUDIO_DRIVER:
      errString = "audio driver error";
      break;

    case PNR_LATE_PACKET:
      errString = "late packet";
      break;

    case PNR_OVERLAPPED_PACKET:
      errString = "overlapped packet";
      break;

    case PNR_OUTOFORDER_PACKET:
      errString = "out of order packet";
      break;

    case PNR_NONCONTIGUOUS_PACKET:
      errString = "non contiguous packet";
      break;

    case PNR_OPEN_NOT_PROCESSED:
      errString = "open not processed";
      break;

    case PNR_EXPIRED:
      errString = "expired";
      break;

    case PNR_INVALID_INTERLEAVER:
      errString = "invalid interleaver";
      break;

    case PNR_BAD_FORMAT:
      errString = "bad format";
      break;

    case PNR_CHUNK_MISSING:
      errString = "chunk missing";
      break;

    case PNR_INVALID_STREAM:
      errString = "invalid stream";
      break;

    case PNR_DNR:
      errString = "DNR error";
      break;

    case PNR_OPEN_DRIVER:
      errString = "open driver error";
      break;

    case PNR_UPGRADE:
      errString = "upgrade error";
      break;

    case PNR_NOTIFICATION:
      errString = "notification error";
      break;

    case PNR_NOT_NOTIFIED:
      errString = "not notified";
      break;

    case PNR_STOPPED:
      errString = "stopped";
      break;

    case PNR_CLOSED:
      errString = "closed";
      break;

    case PNR_INVALID_WAV_FILE:
      errString = "invalid wav file";
      break;

    case PNR_NO_SEEK:
      errString = "no seek";
      break;

    case PNR_DEC_INITED:
      errString = "decoder inited";
      break;

    case PNR_DEC_NOT_FOUND:
      errString = "decoder not found";
      break;

    case PNR_DEC_INVALID:
      errString = "invalid decoder";
      break;

    case PNR_DEC_TYPE_MISMATCH:
      errString = "type mismatch";
      break;

    case PNR_DEC_INIT_FAILED:
      errString = "init failed";
      break;

    case PNR_DEC_NOT_INITED:
      errString = "decoder not inited";
      break;

    case PNR_DEC_DECOMPRESS:
      errString = "decompression error";
      break;

    case PNR_OBSOLETE_VERSION:
      errString = "obsolete version";
      break;

    case PNR_ENC_FILE_TOO_SMALL:
      errString = "file too small";
      break;

    case PNR_ENC_UNKNOWN_FILE:
      errString = "unknown file";
      break;

    case PNR_ENC_BAD_CHANNELS:
      errString = "bad channels";
      break;

    case PNR_ENC_BAD_SAMPSIZE:
      errString = "bad sample size";
      break;

    case PNR_ENC_BAD_SAMPRATE:
      errString = "bad sampling rate";
      break;

    case PNR_ENC_INVALID:
      errString = "invalid encoder";
      break;

    case PNR_ENC_NO_OUTPUT_FILE:
      errString = "no output file";
      break;

    case PNR_ENC_NO_INPUT_FILE:
      errString = "no input file";
      break;

    case PNR_ENC_NO_OUTPUT_PERMISSIONS:
      errString = "no output permissions";
      break;

    case PNR_ENC_BAD_FILETYPE:
      errString = "bad filetype";
      break;

    case PNR_ENC_INVALID_VIDEO:
      errString = "invalid video";
      break;

    case PNR_ENC_INVALID_AUDIO:
      errString = "invalid audio";
      break;

    case PNR_ENC_NO_VIDEO_CAPTURE:
      errString = "no video capture";
      break;

    case PNR_ENC_INVALID_VIDEO_CAPTURE:
      errString = "invalid video capture";
      break;

    case PNR_ENC_NO_AUDIO_CAPTURE:
      errString = "no audio capture";
      break;

    case PNR_ENC_INVALID_AUDIO_CAPTURE:
      errString = "invalid audio capture";
      break;

    case PNR_ENC_TOO_SLOW_FOR_LIVE:
      errString = "too slow for live";
      break;

    case PNR_ENC_ENGINE_NOT_INITIALIZED:
      errString = "not initialized";
      break;

    case PNR_ENC_CODEC_NOT_FOUND:
      errString = "codec not found";
      break;

    case PNR_ENC_CODEC_NOT_INITIALIZED:
      errString = "codec not initialized";
      break;

    case PNR_ENC_INVALID_INPUT_DIMENSIONS:
      errString = "invalid input dimensions";
      break;

    case PNR_ENC_MESSAGE_IGNORED:
      errString = "message ignored";
      break;

    case PNR_ENC_NO_SETTINGS:
      errString = "no settings";
      break;

    case PNR_ENC_NO_OUTPUT_TYPES:
      errString = "no output types";
      break;

    case PNR_ENC_IMPROPER_STATE:
      errString = "improper state";
      break;

    case PNR_ENC_INVALID_SERVER:
      errString = "invalid server";
      break;

    case PNR_ENC_INVALID_TEMP_PATH:
      errString = "invalid temp path";
      break;

    case PNR_ENC_MERGE_FAIL:
      errString = "merge fail";
      break;

    case PNR_BIN_DATA_NOT_FOUND:
      errString = "data not found";
      break;

    case PNR_BIN_END_OF_DATA:
      errString = "end of data";
      break;

    case PNR_BIN_DATA_PURGED:
      errString = "data purged";
      break;

    case PNR_BIN_FULL:
      errString = "full";
      break;

    case PNR_BIN_OFFSET_PAST_END:
      errString = "offset past end";
      break;

    case PNR_ENC_NO_ENCODED_DATA:
      errString = "no encoded data";
      break;

    case PNR_ENC_INVALID_DLL:
      errString = "invalid dll";
      break;

    case PNR_NOT_INDEXABLE:
      errString = "not indexable";
      break;

    case PNR_ENC_NO_BROWSER:
      errString = "no browser";
      break;

    case PNR_ENC_NO_FILE_TO_SERVER:
      errString = "no file to server";
      break;

    case PNR_ENC_INSUFFICIENT_DISK_SPACE:
      errString = "insufficient disk space";
      break;

    case PNR_ENC_SAMPLE_DISCARDED:
      errString = "sample discarded";
      break;

    case PNR_RMT_USAGE_ERROR:
      errString = "RMT usage error";
      break;

    case PNR_RMT_INVALID_ENDTIME:
      errString = "invalid endtime";
      break;

    case PNR_RMT_MISSING_INPUT_FILE:
      errString = "missing input file";
      break;

    case PNR_RMT_MISSING_OUTPUT_FILE:
      errString = "missing output file";
      break;

    case PNR_RMT_INPUT_EQUALS_OUTPUT_FILE:
      errString = "input equals output file";
      break;

    case PNR_RMT_UNSUPPORTED_AUDIO_VERSION:
      errString = "unsupported audio version";
      break;

    case PNR_RMT_DIFFERENT_AUDIO:
      errString = "different audio";
      break;

    case PNR_RMT_DIFFERENT_VIDEO:
      errString = "different video";
      break;

    case PNR_RMT_PASTE_MISSING_STREAM:
      errString = "paste missing stream";
      break;

    case PNR_RMT_END_OF_STREAM:
      errString = "end of stream";
      break;

    case PNR_RMT_IMAGE_MAP_PARSE_ERROR:
      errString = "image map parse error";
      break;

    case PNR_RMT_INVALID_IMAGEMAP_FILE:
      errString = "invalid imagemap file";
      break;

    case PNR_RMT_EVENT_PARSE_ERROR:
      errString = "event parse error";
      break;

    case PNR_RMT_INVALID_EVENT_FILE:
      errString = "invalid event file";
      break;

    case PNR_RMT_INVALID_OUTPUT_FILE:
      errString = "invalid ouput file";
      break;

    case PNR_RMT_INVALID_DURATION:
      errString = "invalid duration";
      break;

    case PNR_RMT_NO_DUMP_FILES:
      errString = "no dump files";
      break;

    case PNR_RMT_NO_EVENT_DUMP_FILE:
      errString = "no event dump file";
      break;

    case PNR_RMT_NO_IMAP_DUMP_FILE:
      errString = "no image map dump file";
      break;

    case PNR_RMT_NO_DATA:
      errString = "no data";
      break;

    case PNR_RMT_EMPTY_STREAM:
      errString = "empty stream";
      break;

    case PNR_RMT_READ_ONLY_FILE:
      errString = "read only file";
      break;

    case PNR_RMT_PASTE_MISSING_AUDIO_STREAM:
      errString = "paste error - missing audio stream";
      break;

    case PNR_RMT_PASTE_MISSING_VIDEO_STREAM:
      errString = "paste error - missing video stream";
      break;

    case PNR_PROP_NOT_FOUND:
      errString = "prop not found";
      break;

    case PNR_PROP_NOT_COMPOSITE:
      errString = "prop not composite";
      break;

    case PNR_PROP_DUPLICATE:
      errString = "prop duplicate";
      break;

    case PNR_PROP_TYPE_MISMATCH:
      errString = "prop type mismatch";
      break;

    case PNR_PROP_ACTIVE:
      errString = "prop active";
      break;

    case PNR_PROP_INACTIVE:
      errString = "prop inactive";
      break;

    case PNR_COULDNOTINITCORE:
      errString = "couldn't init core";
      break;

    case PNR_PERFECTPLAY_NOT_SUPPORTED:
      errString = "PerfectPlay not supported";
      break;

    case PNR_NO_LIVE_PERFECTPLAY:
      errString = "no live PerfectPlay";
      break;

    case PNR_PERFECTPLAY_NOT_ALLOWED:
      errString = "PerfectPlay not allowed";
      break;

    case PNR_NO_CODECS:
      errString = "no codecs";
      break;

    case PNR_SLOW_MACHINE:
      errString = "slow machine";
      break;

    case PNR_FORCE_PERFECTPLAY:
      errString = "force PerfectPlay";
      break;

    case PNR_INVALID_HTTP_PROXY_HOST:
      errString = "invalid HTTP proxy host";
      break;

    case PNR_INVALID_METAFILE:
      errString = "invalid metafile";
      break;

    case PNR_BROWSER_LAUNCH:
      errString = "browser launch error";
      break;

    case PNR_VIEW_SOURCE_NOCLIP:
      errString = "view source - no clip";
      break;

    case PNR_VIEW_SOURCE_DISSABLED:
      errString = "view source - disabled";
      break;

    case PNR_RESOURCE_NOT_CACHED:
      errString = "resource not cached";
      break;

    case PNR_RESOURCE_NOT_FOUND:
      errString = "resource not found";
      break;

    case PNR_RESOURCE_CLOSE_FILE_FIRST:
      errString = "close file first";
      break;

    case PNR_RESOURCE_NODATA:
      errString = "no data";
      break;

    case PNR_RESOURCE_BADFILE:
      errString = "bad file";
      break;

    case PNR_RESOURCE_PARTIALCOPY:
      errString = "partial copy";
      break;

    case PNR_PPV_NO_USER:
      errString = "no user";
      break;

    case PNR_PPV_GUID_READ_ONLY:
      errString = "GUID read only";
      break;

    case PNR_PPV_GUID_COLLISION:
      errString = "GUID collision";
      break;

    case PNR_REGISTER_GUID_EXISTS:
      errString = "GUID exists";
      break;

    case PNR_PPV_AUTHORIZATION_FAILED:
      errString = "authorization failed";
      break;

    case PNR_PPV_OLD_PLAYER:
      errString = "old player error";
      break;

    case PNR_PPV_ACCOUNT_LOCKED:
      errString = "account locked";
      break;

    case PNR_PPV_DBACCESS_ERROR:
      errString = "dbaccess error";
      break;

    case PNR_PPV_USER_ALREADY_EXISTS:
      errString = "user already exists";
      break;

    case PNR_UPG_AUTH_FAILED:
      errString = "auth failed";
      break;

    case PNR_UPG_CERT_AUTH_FAILED:
      errString = "cert auth failed";
      break;

    case PNR_UPG_CERT_EXPIRED:
      errString = "cert expired";
      break;

    case PNR_UPG_CERT_REVOKED:
      errString = "cert revoked";
      break;

    case PNR_UPG_RUP_BAD:
      errString = "bad RUP";
      break;

    case PNR_AUTOCFG_SUCCESS:
      errString = "autoconfig success";
      break;

    case PNR_AUTOCFG_FAILED:
      errString = "autoconfig failed";
      break;

    case PNR_AUTOCFG_ABORT:
      errString = "autoconfig abort";
      break;

    default:
      errString = "unidentified error";
    }

  //fprintf(stderr, catString << ", cat. " << static_cast<int> (cat) << " (" << errString << " - errcode " << (res - ((res >> 6) << 6)) << ") !");

}
#endif
