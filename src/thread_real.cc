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

#ifdef HAVE_REALLIB
#include "pntypes.h"
#include "RealGuids.h"
#include "RealErrorSink.h"
#endif

extern "C" {

#include <pthread.h>
#include <libdc1394/dc1394_control.h>
#include <sys/times.h>
#include "thread_base.h"
#include "thread_real.h"
#include "definitions.h"
#include "preferences.h"
#include "conversions.h"
#include "tools.h"
 
extern PrefsInfo preferences;
extern GtkWidget *commander_window;
extern CtxtInfo ctxt;
extern int current_camera;

gint
RealStartThread(void)
{
  chain_t* real_service=NULL;
  realthread_info_t *info=NULL;
  gchar *tmp;

  real_service=GetService(SERVICE_REAL,current_camera);

  if (real_service==NULL)// if no REAL service running...
    {
      //fprintf(stderr,"No REAL service found, inserting new one\n");
      real_service=(chain_t*)malloc(sizeof(chain_t));
      real_service->data=(void*)malloc(sizeof(realthread_info_t));
      info=(realthread_info_t*)real_service->data;
      pthread_mutex_init(&real_service->mutex_data, NULL);
      pthread_mutex_init(&real_service->mutex_struct, NULL);
      pthread_mutex_init(&info->mutex_cancel_real, NULL);

      /* if you want a clean-interrupt thread:*/
      pthread_mutex_lock(&info->mutex_cancel_real);
      info->cancel_real_req=0;
      info->maxFrameRate=100;
      pthread_mutex_unlock(&info->mutex_cancel_real);

      /* setup real_thread: */
      pthread_mutex_lock(&real_service->mutex_data);
      // first copy prefs data into the info struct
      //fprintf(stderr," Copy prefs data\n");
      strcpy(info->realServerAddress, preferences.real_address);
      strcpy(info->realServerStreamName, preferences.real_filename);
      strcpy(info->realServerLogin, preferences.real_user);
      strcpy(info->realServerPassword, preferences.real_password);
      strcpy(info->streamTitle, preferences.real_title);
      strcpy(info->streamAuthor, preferences.real_author);
      strcpy(info->streamCopyright, preferences.real_copyright);
      info->realServerPort=preferences.real_port;
      info->recordable=preferences.real_recordable;
      info->audienceFlags=preferences.real_audience;
      info->videoQuality=preferences.real_quality;
      info->realPlayerCompatibility=6;/////////////////////////
      info->period=preferences.real_period;

      CommonChainSetup(real_service,SERVICE_REAL,current_camera);

      info->real_buffer=NULL;

      RealSetup(info, real_service); // THIS SHOULD BE MOVED

      //fprintf(stderr,"SETUP FINISHED\n");
      pthread_mutex_unlock(&real_service->mutex_data);

      /* Insert chain and start service*/
      pthread_mutex_lock(&real_service->mutex_struct);
      InsertChain(real_service,current_camera);
      pthread_mutex_unlock(&real_service->mutex_struct);

      pthread_mutex_lock(&real_service->mutex_data);
      pthread_mutex_lock(&real_service->mutex_struct);
      if (pthread_create(&real_service->thread, NULL,
			 RealThread,(void*) real_service))
	  {
	    /* error starting thread. You should cleanup here
	       (free, unset global vars,...):*/

	    /* Mendatory cleanups:*/
	    RemoveChain(real_service,current_camera);
	    pthread_mutex_unlock(&real_service->mutex_struct);
	    pthread_mutex_unlock(&real_service->mutex_data);
	    free(info->real_buffer);
	    FreeChain(real_service);
	    return(-1);
	  }
      info->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)RealShowFPS, (gpointer*) real_service);
      pthread_mutex_unlock(&real_service->mutex_struct);
      pthread_mutex_unlock(&real_service->mutex_data);
      
    }
  //fprintf(stderr," REAL service started\n");

  return (1);
}


void*
RealCleanupThread(void* arg) 
{
  chain_t* real_service;
  realthread_info_t *info;

  real_service=(chain_t*)arg;
  info=(realthread_info_t*)real_service->data;
  /* Specific cleanups: */

  /* Mendatory cleanups: */
  pthread_mutex_unlock(&real_service->mutex_data);

  return (NULL);
}
  

int
RealShowFPS(gpointer *data)
{
  chain_t* real_service;
  realthread_info_t *info;
  char tmp_string[20];
  float tmp, fps;

  real_service=(chain_t*)data;
  info=(realthread_info_t*)real_service->data;
  
  tmp=(float)(info->current_time-info->prev_time)/sysconf(_SC_CLK_TCK);
  if (tmp==0)
    fps=0;
  else
    fps=(float)info->frames/tmp;

  sprintf(tmp_string," %.2f",fps);

  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"fps_real"),
		       ctxt.fps_real_ctxt, ctxt.fps_real_id);
  ctxt.fps_real_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"fps_real"),
					 ctxt.fps_real_ctxt, tmp_string);
  
  pthread_mutex_lock(&real_service->mutex_data);
  info->prev_time=info->current_time;
  info->frames=0;
  pthread_mutex_unlock(&real_service->mutex_data);

  return 1;
}


void*
RealThread(void* arg)
{
  static gchar filename_out[STRING_SIZE];
  chain_t* real_service=NULL;
  realthread_info_t *info=NULL;
  long int skip_counter;
  char tmp_string[20];
  float delay;

  real_service=(chain_t*)arg;
  pthread_mutex_lock(&real_service->mutex_data);
  info=(realthread_info_t*)real_service->data;
  skip_counter=0;

  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&real_service->mutex_data);

  // time inits:
  info->prev_time = times(&info->tms_buf);
  info->frames=0;

  //fprintf(stderr,"About to enter while\n");
#ifdef HAVE_REALLIB
  while (SUCCEEDED(info->res))
    { 
      /* Clean cancel handlers */
      //fprintf(stderr,"Check cancel\n");
      pthread_mutex_lock(&info->mutex_cancel_real);
      if (info->cancel_real_req>0)
	{
	  pthread_mutex_unlock(&info->mutex_cancel_real);
	  return ((void*)1);
	}
      else
	{
	  //fprintf(stderr,"Locking mutexes\n");
	  pthread_mutex_unlock(&info->mutex_cancel_real);
	  pthread_mutex_lock(&real_service->mutex_data);
	  //fprintf(stderr,"Rolling buffers\n");
	  if(RollBuffers(real_service)) // have buffers been rolled?
	    {

	      RealThreadCheckParams(real_service);

	      if (real_service->current_buffer->width!=-1) {
		if (skip_counter==(info->period-1)) {
		  skip_counter=0;
		  // note that Real supports different video modes (RGB, YUVxxx)
		  // so that it would be smarter to convert from IIDC mode to a
		  // similar Real mode in order to avoid unecessary CPU load.
		  convert_to_rgb(real_service->current_buffer->image,
				 info->real_buffer, real_service->current_buffer->mode,
				 real_service->current_buffer->width, real_service->current_buffer->height,
				 real_service->current_buffer->format7_color_mode, real_service->current_buffer->bayer);// RGB

		  //fprintf(stderr,"Setting pointer to sample\n");
		  info->res = info->pSample->SetBuffer(info->real_buffer,
						       real_service->current_buffer->width*real_service->current_buffer->height*3, 0 ,0); // RGB
		  
	      
		  if(SUCCEEDED(info->res))
		    {
		      //fprintf(stderr,"Encoding\n");
		      info->res = info->pVideoPin->Encode(info->pSample);
		      //fprintf(stderr,"Encoded!\n");
		    }
		  
		  if(SUCCEEDED(info->res) && info->pErrorSinkObject->g_bErrorOccurred)
		    info->res = info->pErrorSinkObject->g_nError;
		  
		  info->res = info->pBuildEngine->CancelEncoding();
		  
		  if (SUCCEEDED(info->res))
		    info->res = info->pBuildEngine->PrepareToEncode();
		  else
		    MainError("Error in CancelEncoding()");
		  
		  info->pErrorSinkObject->g_bErrorOccurred = false;
		  if (SUCCEEDED(info->res))
		    continue;
		  else 
		    MainError("Error in CancelEncoding()");
		}
		else
		  skip_counter++;

		// FPS display
		info->current_time=times(&info->tms_buf);
		info->frames++;
	      }
	      pthread_mutex_unlock(&real_service->mutex_data);
	    }
	  else
	    {
	      pthread_mutex_unlock(&real_service->mutex_data);
	      usleep(THREAD_LOOP_SLEEP_TIME_US);
	    }
	}
    }
#endif // HAVE_REALLIB
  return ((void*)0); // error: thread exited.
  //fprintf(stderr,"Thread exited\n");
}


gint
RealStopThread(void)
{
  realthread_info_t *info;
  chain_t *real_service;
  real_service=GetService(SERVICE_REAL,current_camera);

  if (real_service!=NULL)// if REAL service running...
    {
      //fprintf(stderr,"REAL service found, stopping\n");
      info=(realthread_info_t*)real_service->data;
      /* Clean cancel handler: */
      pthread_mutex_lock(&info->mutex_cancel_real);
      info->cancel_real_req=1;
      pthread_mutex_unlock(&info->mutex_cancel_real);

      /* common handlers...*/
      pthread_join(real_service->thread, NULL);

      pthread_mutex_lock(&real_service->mutex_data);
      pthread_mutex_lock(&real_service->mutex_struct);

      gtk_timeout_remove(info->timeout_func_id);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"fps_real"),
			   ctxt.fps_real_ctxt, ctxt.fps_real_id);
      ctxt.fps_real_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"fps_real"),
					  ctxt.fps_real_ctxt, "");

      RemoveChain(real_service,current_camera);

      /* Do custom cleanups here...*/

#ifdef HAVE_REALLIB
      // REAL STUFF:
      //info->res = info->pBuildEngine->CancelEncoding(); // THIS SEGFAULTS IF ENCODING FAILED
      PN_RELEASE(info->pVideoPin);
      PN_RELEASE(info->pSample);
      PN_RELEASE(info->pBuildEngine);
#endif
      if (info->real_buffer!=NULL) {
	free(info->real_buffer);
	info->real_buffer=NULL;
      }
      
      /* Mendatory cleanups: */
      pthread_mutex_unlock(&real_service->mutex_struct);
      pthread_mutex_unlock(&real_service->mutex_data);
      FreeChain(real_service);

      //fprintf(stderr," REAL service stopped\n");
    }

  return (1);
}


void
RealThreadCheckParams(chain_t *real_service)
{

  realthread_info_t *info;
  int buffer_size_change=0;
  info=(realthread_info_t*)real_service->data;

  // copy harmless parameters anyway:
  real_service->local_param_copy.bpp=real_service->current_buffer->bpp;
  real_service->local_param_copy.bayer_pattern=real_service->current_buffer->bayer_pattern;

  // if some parameters changed, we need to re-allocate the local buffers and restart the real
  if ((real_service->current_buffer->width!=real_service->local_param_copy.width)||
      (real_service->current_buffer->height!=real_service->local_param_copy.height)||
      (real_service->current_buffer->bytes_per_frame!=real_service->local_param_copy.bytes_per_frame)||
      (real_service->current_buffer->mode!=real_service->local_param_copy.mode)||
      (real_service->current_buffer->format!=real_service->local_param_copy.format)||
      // check F7 color mode change
      ((real_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE)&&
       (real_service->current_buffer->format7_color_mode!=real_service->local_param_copy.format7_color_mode)
       ) ||
      // check bayer and stereo decoding
      (real_service->current_buffer->stereo_decoding!=real_service->local_param_copy.stereo_decoding)||
      (real_service->current_buffer->bayer!=real_service->local_param_copy.bayer)
      )
    {
      if (real_service->current_buffer->width*real_service->current_buffer->height!=
	  real_service->local_param_copy.width*real_service->local_param_copy.height) {
	buffer_size_change=1;
      }
      else {
	buffer_size_change=0;
      }

      // copy all new parameters:
      real_service->local_param_copy.width=real_service->current_buffer->width;
      real_service->local_param_copy.height=real_service->current_buffer->height;
      real_service->local_param_copy.bytes_per_frame=real_service->current_buffer->bytes_per_frame;
      real_service->local_param_copy.mode=real_service->current_buffer->mode;
      real_service->local_param_copy.format=real_service->current_buffer->format;
      real_service->local_param_copy.format7_color_mode=real_service->current_buffer->format7_color_mode;
      real_service->local_param_copy.stereo_decoding=real_service->current_buffer->stereo_decoding;
      real_service->local_param_copy.bayer=real_service->current_buffer->bayer;

      // DO SOMETHING
      if (buffer_size_change!=0) {

	if (info->real_buffer!=NULL) {
	  free(info->real_buffer);
	  info->real_buffer=NULL;
	}
	info->real_buffer=(unsigned char*)malloc(real_service->current_buffer->width*real_service->current_buffer->height*3
						 *sizeof(unsigned char));
      }
    }
  
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////   THIS STUFF MOSTLY COMES FROM THE 'LIVE' SAMPLE OF THE SDK ////////////////
/////////////////////////////////////////////////////////////////////////////////////////


int RealSetup(realthread_info_t *info, chain_t *service)
{
  // First, check existence and access rights to the directory 
  // where the .so dynamic libs of the RealProducer 8 SDK.

  //struct stat libDirStats;
  //stat (REAL_DYNAMIC_LIBS_ABSOLUTE_PATH, &libDirStats);

    //////////////////////////////////////////////////////////
    //
    // Initialize the DLL locations before you call any 
    // functions in the RealProducer Core SDK
    //
    //////////////////////////////////////////////////////////

#ifdef HAVE_REALLIB
  //fprintf(stderr,"Setting up REAL thread\n");

  if(SUCCEEDED(info->res))
    {
      // Before you can call into the Build Engine DLL, you must 
      // initialize the DLL paths so that the Build Engine will 
      // know where to locate its required
      // DLLs when you create your first Build Engine

      // Create a null delimited, double-null terminated string containing
      // DLL category name/path pairs. In this sample, we use the same 
      // directory for all DLLs (the current directory). You can split 
      // up the DLLs into different directories by category if you wish.

      char szDllPath[2048];   
      UINT32 ulNumChars = 0;
	    
      char cwd[2048];
      getcwd(cwd, 2048);

      ulNumChars += sprintf(szDllPath+ulNumChars, 
			    "%s=%s", "DT_Plugins", cwd) + 1;
      ulNumChars += sprintf(szDllPath+ulNumChars, 
			    "%s=%s", "DT_Codecs", cwd) + 1;
      ulNumChars += sprintf(szDllPath+ulNumChars, 
			    "%s=%s", "DT_EncSDK", cwd) + 1;
      ulNumChars += sprintf(szDllPath+ulNumChars, 
			    "%s=%s", "DT_Common", cwd) + 1;
      // terminator
      ulNumChars += sprintf(szDllPath+ulNumChars, "") + 1;

      // Now that we have created the path, we hand it to the build engine
      // by using the exported SetDLLAccessPath function.

      info->res = SetDLLAccessPath(szDllPath);
    }

  //////////////////////////////////////////////////////////
  //
  // Create the build engine object itself
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
  {
    //fprintf(stderr," Building engine\n");
    info->res = RMACreateRMBuildEngine(&info->pBuildEngine);
  }
  
  //////////////////////////////////////////////////////////
  //
  // Enumerate the pins, and get the video pin
  //
  //////////////////////////////////////////////////////////

  if (!SUCCEEDED(info->res))
    {
      fprintf(stderr,"\n===== WARNING =====\n");
      fprintf(stderr,"There was a problem launching the encoder engine.\n");
      fprintf(stderr,"This is most probably due to the fact that you have a recent distro (typicaly\n");
      fprintf(stderr,"RH 7.1 or 7.2). If you wan to be a betatester for Real streaming in Coriander,\n");
      fprintf(stderr,"you should install a RH 6.2 or equivalent distro. Real Networks has no SDK for\n");
      fprintf(stderr,"recent Linux versions!\n");
    }

  if(SUCCEEDED(info->res))
    {
      //fprintf(stderr," Get video pin\n");
      IUnknown* tempUnk = NULL;
      IRMAEnumeratorIUnknown* pPinEnum = NULL;
	
      // get the pin enumerator

      info->res = info->pBuildEngine->GetPinEnumerator(&pPinEnum);
      assert(SUCCEEDED(info->res));

      // enumerate the pins.
	
      PN_RESULT resEnum = PNR_OK;
      resEnum = pPinEnum->First(&tempUnk);
	
      char* outputTypeStr = new char[ENC_MAX_STR];

      while(SUCCEEDED(info->res) &&
	    SUCCEEDED(resEnum) && 
	    resEnum != PNR_ELEMENT_NOT_FOUND)
	{
	  IRMAInputPin* tempPin = NULL;
	  info->res = tempUnk->QueryInterface(IID_IRMAInputPin, (void**)&tempPin);
	  PN_RELEASE(tempUnk);
	    
	  if(SUCCEEDED(info->res))
	    info->res = tempPin->GetOutputMimeType(outputTypeStr, ENC_MAX_STR);
	    
	  if(SUCCEEDED(info->res))
	    {
	      if(strcmp(outputTypeStr, MIME_REALVIDEO) == 0)
		{
		  // save this pin in pVideoPin
		  info->pVideoPin = tempPin;
		  // addref it because we are going to use it later
		  info->pVideoPin->AddRef();
		}
		
	      PN_RELEASE(tempPin);
	      resEnum = pPinEnum->Next(&tempUnk);
	    } 
	  else 
	    {
	      MainStatus("Cannot query input pin interface.");
	    }
	} 
      delete [] outputTypeStr;
	
      PN_RELEASE(pPinEnum);
    }
	
  //////////////////////////////////////////////////////////
  //
  // create the media sample object that we will use to
  // pass data to the pins
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
    {
      //fprintf(stderr," Create media sample object\n");
	    
      IRMABuildClassFactory*  pClassFactory = NULL;
	    
      info->res = info->pBuildEngine->QueryInterface(IID_IRMABuildClassFactory,
					 (void**)&pClassFactory);
	    
      if(SUCCEEDED(info->res))
	{
	  // create an instance of class IRMAMediaSample

	  info->res = pClassFactory->CreateInstance(CLSID_IRMAMediaSample,
					      NULL, IID_IRMAMediaSample, 
					      (void **)&info->pSample);
	}

      PN_RELEASE(pClassFactory);
    }
	
  //////////////////////////////////////////////////////////
  //
  // Setup the build engine main properties
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
    {
      //fprintf(stderr," Build engine properties\n");
      //
      // Do only video, no audio, no events, or image maps 
      //

      info->res = info->pBuildEngine->SetDoOutputMimeType(MIME_REALAUDIO, FALSE);

      if(SUCCEEDED(info->res))
	info->res = info->pBuildEngine->SetDoOutputMimeType(MIME_REALVIDEO, TRUE);

      if(SUCCEEDED(info->res))
	info->res = info->pBuildEngine->SetDoOutputMimeType(MIME_REALEVENT, FALSE);

      if(SUCCEEDED(info->res))
	info->res = info->pBuildEngine->SetDoOutputMimeType(MIME_REALIMAGEMAP, FALSE);

	//
	// for live, we must encode in realtime.
	//

      if(SUCCEEDED(info->res))
	info->res = info->pBuildEngine->SetRealTimeEncoding(TRUE);
	    
	//
	// We will create a surestream file.  Set this to FALSE for a
	// single rate file for web servers.  
	//
	// With this set to TRUE, the build engine will generate multiple
	// streams even though we have selected only one target audience.
	// The extra streams are duress streams for low-bandwidth conditions.
	//

      if(SUCCEEDED(info->res))
	info->res = info->pBuildEngine->SetDoMultiRateEncoding(TRUE);
    }
	
  //////////////////////////////////////////////////////////
  //
  // Setup main clip properties.  This configures what
  // we will output to: in this case to a server and
  // a static file [live archiving]
  //
  // Also, we set up title, author, copyright, etc...
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
    {
      //fprintf(stderr," Setup clip properties\n");
      IRMAClipProperties* pClipProps = NULL;

      //
      // get the clip properties object
      //

      info->res = info->pBuildEngine->GetClipProperties(&pClipProps);
	    
      //
      // we don't want to create a file
      //

      if(SUCCEEDED(info->res))
	info->res = pClipProps->SetDoOutputFile(FALSE);

      //
      // we want to stream live to a server
      //

      if(SUCCEEDED(info->res))
	info->res = pClipProps->SetDoOutputServer(TRUE);

      /*
	//
	// here is the filename we want to create
	//
	
	if(SUCCEEDED(info->res))
	res = pClipProps->SetOutputFilename("test.rm");
      */

      //
      // here is the server we want to stream to, and the
      // name of the file on that server.  If the server
      // is set up in the standard way, then you can play
      // it with this url in the player:
      //
      // rtsp://macleod:6060/encoder/test.rm
      //

      if(SUCCEEDED(info->res))
	info->res = pClipProps->SetOutputServerInfo((const char *)info->realServerAddress,
					            (const char *)info->realServerStreamName,
					            info->realServerPort,
					            (const char *)info->realServerLogin,
					            (const char *)info->realServerPassword);
      
      //
      // title, author, etc...
      //
	
      if(SUCCEEDED(info->res))
	info->res = pClipProps->SetTitle(info->streamTitle);
		    
      if(SUCCEEDED(info->res))
	info->res = pClipProps->SetAuthor(info->streamAuthor);
		    
      if(SUCCEEDED(info->res))
	info->res = pClipProps->SetCopyright(info->streamCopyright);


      //
      // Authorize clip recording if requested by constructor caller.
      //

      if (SUCCEEDED(info->res))
	if (info->recordable)
	  info->res = pClipProps->SetSelectiveRecord (TRUE);

      PN_RELEASE(pClipProps);
    }

  //////////////////////////////////////////////////////////
  //
  // Select some target audiences, and other settings
  // related to how things are encoded.
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
    {
      IRMABasicTargetSettings* pBasicSettings = NULL;
      IRMATargetSettings *pTargSettings = NULL;
	    
      //
      // Build Engine returns a TargetSettings object,
      // we must query for BasicTargetSettings
      //

      info->res = info->pBuildEngine->GetTargetSettings(&pTargSettings);

      if(SUCCEEDED(info->res))
	info->res = pTargSettings->QueryInterface(IID_IRMABasicTargetSettings,
					    (void**)&pBasicSettings);
      PN_RELEASE(pTargSettings);
	    
      //
      // reset the target audiences
      //

      if(SUCCEEDED(info->res))
	info->res = pBasicSettings->RemoveAllTargetAudiences();
		    
      //
      // Add target audiences as specified in preferences.real_audience:
      //

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_28_MODEM)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_28_MODEM);
	  
      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_56_MODEM)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_56_MODEM);

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_SINGLE_ISDN)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_SINGLE_ISDN);

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_DUAL_ISDN)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_DUAL_ISDN);

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_LAN_HIGH)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_LAN_HIGH);

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_256_DSL_CABLE)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_256_DSL_CABLE);

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_384_DSL_CABLE)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_384_DSL_CABLE);

      if(SUCCEEDED(info->res))
	if (preferences.real_audience & REAL_AUDIENCE_512_DSL_CABLE)
	  info->res = pBasicSettings->AddTargetAudience(ENC_TARGET_512_DSL_CABLE);
      /*
      //
      // select audio optimized for content containing mostly music
      //

      if(SUCCEEDED(info->res))
	res = pBasicSettings->SetAudioContent(ENC_AUDIO_CONTENT_MUSIC);
      */
	
      //
      // set video quality
      //

      if(SUCCEEDED(info->res))
	info->res = pBasicSettings->SetVideoQuality(info->videoQuality);
	
      //
      // set RealPlayer compatibility level
      //

      if(SUCCEEDED(info->res))
	info->res = pBasicSettings->SetPlayerCompatibility(info->realPlayerCompatibility);

      PN_RELEASE(pBasicSettings);
    }

  /*
  //////////////////////////////////////////////////////////
  //
  // Setup the audio format on the audio pin
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
    {
      IRMAPinProperties* pPinProps = NULL;
      IRMAAudioPinProperties* pAudioPinProps = NULL;

      res = pAudioPin->GetPinProperties(&pPinProps);
			    
      if(SUCCEEDED(info->res))
	res = pPinProps->QueryInterface(IID_IRMAAudioPinProperties,
					(void**)&pAudioPinProps);

      PN_RELEASE(pPinProps);

      //
      // Here, we configure the audio pin with the format
      // of the data that we will be passing to it.
      // These constants are defined at the top of this file.
      //

      if(SUCCEEDED(info->res))
	res = pAudioPinProps->SetSampleRate(cAudioFormat_SampleRate);

      if(SUCCEEDED(info->res))
	res = pAudioPinProps->SetNumChannels(cAudioFormat_NumChannels);

      if(SUCCEEDED(info->res))
	res = pAudioPinProps->SetSampleSize(cAudioFormat_BitsPerSample);

    }
  */
	
  //////////////////////////////////////////////////////////
  //
  // Setup the video format on the video pin
  //
  //////////////////////////////////////////////////////////

  if(SUCCEEDED(info->res))
    {
      //fprintf(stderr," Setup video format\n");
      IRMAPinProperties* pPinProps = NULL;
      IRMAVideoPinProperties* pVideoPinProps = NULL;

      info->res = info->pVideoPin->GetPinProperties(&pPinProps);
			    
      if(SUCCEEDED(info->res))
	info->res = pPinProps->QueryInterface(IID_IRMAVideoPinProperties,
					(void**)&pVideoPinProps);

      PN_RELEASE(pPinProps);

      //
      // Here, we configure the video pin with the video size and frame
      // rate, and video format.  This is the format in which we will
      // be providing video to the Encode() call.
      //

      if(SUCCEEDED(info->res))
	info->res = pVideoPinProps->SetFrameRate(info->maxFrameRate);

      if(SUCCEEDED(info->res))
	info->res = pVideoPinProps->SetVideoSize(service->width,
						 service->height);

      if(SUCCEEDED(info->res))
	info->res = pVideoPinProps->SetVideoFormat(ENC_VIDEO_FORMAT_RGB24); // RGB
    }	
	

    //////////////////////////////////////////////////////////
    //
    // Prepare the build engine for encoding
    //
    //////////////////////////////////////////////////////////

    if(SUCCEEDED(info->res))
      info->res = info->pBuildEngine->PrepareToEncode();

    /*
    //////////////////////////////////////////////////////////
    //
    // The Build Engine will recommend a size for audio
    // buffers.  You should pass buffers of this size
    // into the audio pin
    //
    //////////////////////////////////////////////////////////

    if(SUCCEEDED(info->res))
    {
	UINT32 nSize = 0;

	IRMAAudioInputPin *pAudioSpecificPin = NULL;

	info->res = pAudioPin->QueryInterface(IID_IRMAAudioInputPin,
					(void **)&pAudioSpecificPin);
	
	if(SUCCEEDED(info->res))
	    info->res = pAudioSpecificPin->GetSuggestedInputSize(&nSize);

	if(SUCCEEDED(info->res))
	{
	    nSuggestedAudioBufferSize = nSize;
	}

	PN_RELEASE(pAudioSpecificPin);

    }
    */

    //////////////////////////////////////////////////////////
    //
    // Register our error sink
    //
    //////////////////////////////////////////////////////////

    if(SUCCEEDED(info->res))
    {
      //fprintf(stderr," Register error sink\n");
        IUnknown *pMyErrorSinkObject = NULL;
	IRMAErrorSink *pErrorSink = NULL;
	IRMAErrorSinkControl *pErrorSinkControl = NULL;

	info->res = RealErrorSink::CreateInstance(&pMyErrorSinkObject);

	if(SUCCEEDED(info->res))
	{
	    info->pErrorSinkObject = (RealErrorSink *) pMyErrorSinkObject;

	    info->res = pMyErrorSinkObject->QueryInterface(IID_IRMAErrorSink,
							(void **)&pErrorSink);
	}

	if(SUCCEEDED(info->res))
	{
	    info->res = info->pBuildEngine->QueryInterface(IID_IRMAErrorSinkControl,
						(void **)&pErrorSinkControl);
	}

	if(SUCCEEDED(info->res))
	{
	   info->res = pErrorSinkControl->AddErrorSink(pErrorSink,
						  PNLOG_EMERG, PNLOG_INFO); // Error levels 0-6
	}

	PN_RELEASE(pErrorSink);
	PN_RELEASE(pErrorSinkControl);
	PN_RELEASE(pMyErrorSinkObject);
    }
    //fprintf(stderr,"Setup finished.\n");

#endif //HAVE_REALLIB

  return(1);
}


} // end extern C
