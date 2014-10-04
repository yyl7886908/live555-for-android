#include <android/log.h>
#include <pthread.h>
#include "recorder.hh"
#include "../../live555/BasicUsageEnvironment/include/BasicUsageEnvironment.hh"
#include "../../live555/groupsock/include/GroupsockHelper.hh"
#include "loghelp.h"
#include "tx_recorder_jni_define.h"

#if defined(__WIN32__) || defined(_WIN32)
#define snprintf _snprintf
#else
#include <signal.h>
#define USE_SIGNALS 1
#endif

// Forward function definitions:
void continueAfterClientCreation0(RTSPClient* client, Boolean requestStreamingOverTCP);
void continueAfterClientCreation1();
void continueAfterOPTIONS(RTSPClient* client, int resultCode, char* resultString);
void continueAfterDESCRIBE(RTSPClient* client, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* client, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* client, int resultCode, char* resultString);
void continueAfterTEARDOWN(RTSPClient* client, int resultCode, char* resultString);

void createOutputFiles(char const* periodicFilenameSuffix);

void createPeriodicOutputFiles();
void setupStreams();
void closeMediaSinks();
void subsessionAfterPlaying(void* clientData);
void subsessionByeHandler(void* clientData);
void sessionAfterPlaying(void* clientData = NULL);
void sessionTimerHandler(void* clientData);
void periodicFileOutputTimerHandler(void* clientData);
void shutdown(int exitCode = 1);
void signalHandlerShutdown(int sig);
void checkForPacketArrival(void* clientData);
void checkInterPacketGaps(void* clientData);
void beginQOSMeasurement();
void printQOSData(int exitCode = 1);
 
char const* progName;
UsageEnvironment* env;
Medium* ourClient = NULL;
Authenticator* ourAuthenticator = NULL;
char const* streamURL = NULL;
MediaSession* session = NULL;
TaskToken sessionTimerTask = NULL;
TaskToken arrivalCheckTimerTask = NULL;
TaskToken interPacketGapCheckTimerTask = NULL;
TaskToken qosMeasurementTimerTask = NULL;
TaskToken periodicFileOutputTask = NULL;
Boolean createReceivers = True;
Boolean outputQuickTimeFile = False;
Boolean generateMP4Format = False;
QuickTimeFileSink* qtOut = NULL;
Boolean outputAVIFile = False;
AVIFileSink* aviOut = NULL;
Boolean audioOnly = False;
Boolean videoOnly = False;
char const* singleMedium = NULL;
int verbosityLevel = 1; // by default, print verbose output
double duration = 0;
double durationSlop = -1.0; // extra seconds to play at the end
double initialSeekTime = 0.0f;
char* initialAbsoluteSeekTime = NULL;
float scale = 1.0f;
double endTime;
unsigned interPacketGapMaxTime = 0;
unsigned totNumPacketsReceived = ~0; // used if checking inter-packet gaps
Boolean playContinuously = False;
int simpleRTPoffsetArg = -1;
Boolean sendOptionsRequest = True;
Boolean sendOptionsRequestOnly = False;
Boolean oneFilePerFrame = False;
Boolean notifyOnPacketArrival = False;
Boolean streamUsingTCP = False;
Boolean forceMulticastOnUnspecified = False;
unsigned short desiredPortNum = 0;
portNumBits tunnelOverHTTPPortNum = 0;
char* username = NULL;
char* password = NULL;
char* proxyServerName = NULL;
unsigned short proxyServerPortNum = 0;
unsigned char desiredAudioRTPPayloadFormat = 0;
char* mimeSubtype = NULL;
unsigned short movieWidth = 240; // default
Boolean movieWidthOptionSet = False;
unsigned short movieHeight = 180; // default
Boolean movieHeightOptionSet = False;
unsigned movieFPS = 15; // default
Boolean movieFPSOptionSet = False;
char const* fileNamePrefix = "";
unsigned fileSinkBufferSize = 100000;
unsigned socketInputBufferSize = 0;
Boolean packetLossCompensate = False;
Boolean syncStreams = False;
Boolean generateHintTracks = False;
Boolean waitForResponseToTEARDOWN = True;
unsigned qosMeasurementIntervalMS = 0; // 0 means: Don't output QOS data
char* userAgent = NULL;
unsigned fileOutputInterval = 0; // seconds
unsigned fileOutputSecondsSoFar = 0; // seconds
Boolean createHandlerServerForREGISTERCommand = False;
portNumBits handlerServerForREGISTERCommandPortNum = 0;
HandlerServerForREGISTERCommand* handlerServerForREGISTERCommand;
char* usernameForREGISTER = NULL;
char* passwordForREGISTER = NULL;
UserAuthenticationDatabase* authDBForREGISTER = NULL;

struct timeval startTime;

pthread_t recorder_thread;
char filePath[200];
char rtspPath[200];

// 录制线程
void *thread_recording(void*)
{
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    gettimeofday(&startTime, NULL);
    streamURL = rtspPath;
    // Create (or arrange to create) our client object:
    if (createHandlerServerForREGISTERCommand) {
        handlerServerForREGISTERCommand
            = HandlerServerForREGISTERCommand::createNew(*env, continueAfterClientCreation0, handlerServerForREGISTERCommandPortNum, authDBForREGISTER,  verbosityLevel, progName);
        if (handlerServerForREGISTERCommand == NULL) {
            *env << "Failed to create a server for handling incoming \"REGISTER\" commands: " << env->getResultMsg() << "\n";
        } else {
            *env << "Awaiting an incoming \"REGISTER\" command on port " << handlerServerForREGISTERCommand->serverPortNum() << "\n";
        }
    } else {
        ourClient = createClient(*env, streamURL, verbosityLevel, progName);
        if (ourClient == NULL) {
            *env << "Failed to create " << clientProtocolName << " client: " << env->getResultMsg() << "\n";
            shutdown();
        }
        ALOG(TX_LOG_INFO, TAG,"client create success\n");
        continueAfterClientCreation1();
    }
    env->taskScheduler().doEventLoop();
}


//开始录制的接口
int recordRTSPStream(char* filepath, char* streamUri) {
	ALOG(TX_LOG_INFO, TAG, "filepath = %s, rtsppath = %s\n",filepath, streamUri);
    memset(filePath, 0 , 200);
    memset(rtspPath, 0 , 200);
    sprintf(filePath, "%s", filepath);
    sprintf(rtspPath, "%s", "rtsp://admin:12345@192.168.1.101:554/Streaming/Channels/1?transportmode=unicast&profile=Profile_1");
    int ret;
    if((ret = pthread_create(&recorder_thread, NULL, thread_recording, NULL)) != 0)
        {
            ALOG(TX_LOG_INFO, TAG, "create pthread failed\n");
        }else{
        ALOG(TX_LOG_INFO, TAG, "create pthread success\n");
    }

  return 0;
}

Boolean areAlreadyShuttingDown = False;
int shutdownExitCode;
//停止录制的接口
void stopRecordingRtspStream()
{
    shutdown(0);
    return;
}

void continueAfterClientCreation0(RTSPClient* newRTSPClient, Boolean requestStreamingOverTCP) {
  if (newRTSPClient == NULL) return;

  streamUsingTCP = requestStreamingOverTCP;

  assignClient(ourClient = newRTSPClient);
  streamURL = newRTSPClient->url();

  // Having handled one "REGISTER" command (giving us a "rtsp://" URL to stream from), we don't handle any more:
  Medium::close(handlerServerForREGISTERCommand); handlerServerForREGISTERCommand = NULL;

  continueAfterClientCreation1();
}

void continueAfterClientCreation1() {
   ALOG(TX_LOG_INFO, TAG,"continueAfterClientCreation1\n");
  setUserAgentString(userAgent);

  if (sendOptionsRequest) {
    // Begin by sending an "OPTIONS" command:
    getOptions(continueAfterOPTIONS);
  } else {
    continueAfterOPTIONS(NULL, 0, NULL);
  }
}

void continueAfterOPTIONS(RTSPClient*, int resultCode, char* resultString) {
    ALOG(TX_LOG_INFO, TAG,"continueAfterOPTIONS\n");
  if (sendOptionsRequestOnly) {
    if (resultCode != 0) {
      *env << clientProtocolName << " \"OPTIONS\" request failed: " << resultString << "\n";
    } else {
      *env << clientProtocolName << " \"OPTIONS\" request returned: " << resultString << "\n";
    }
    shutdown();
  }
  delete[] resultString;

  // Next, get a SDP description for the stream:
  getSDPDescription(continueAfterDESCRIBE);
}

void continueAfterDESCRIBE(RTSPClient*, int resultCode, char* resultString) {
    ALOG(TX_LOG_INFO, TAG,"continueAfterDESCRIBE\n");
  if (resultCode != 0) {
      ALOG(TX_LOG_INFO, TAG,"Failed to get a SDP description for the URL %s, %s\n", streamURL, resultString);
    *env << "Failed to get a SDP description for the URL \"" << streamURL << "\": " << resultString << "\n";
    delete[] resultString;
    shutdown();
  }

  char* sdpDescription = resultString;
  *env << "Opened URL \"" << streamURL << "\", returning a SDP description:\n" << sdpDescription << "\n";

  // Create a media session object from this SDP description:
  session = MediaSession::createNew(*env, sdpDescription);
  delete[] sdpDescription;
  if (session == NULL) {
      ALOG(TX_LOG_INFO, TAG, "Failed to create a MediaSession object from the SDP description:");
    shutdown();
  } else if (!session->hasSubsessions()) {
      ALOG(TX_LOG_INFO, TAG, "This session has no media subsessions (i.e., no \"m=\" lines)\n");
    shutdown();
  }

  // Then, setup the "RTPSource"s for the session:
  MediaSubsessionIterator iter(*session);
  MediaSubsession *subsession;
  Boolean madeProgress = False;
  char const* singleMediumToTest = singleMedium;
  while ((subsession = iter.next()) != NULL) {
    // If we've asked to receive only a single medium, then check this now:
    if (singleMediumToTest != NULL) {
      if (strcmp(subsession->mediumName(), singleMediumToTest) != 0) {
          ALOG(TX_LOG_INFO, TAG, "Ignoring codecName = %s\n", subsession->codecName());
		  *env << "Ignoring \"" << subsession->mediumName()
			  << "/" << subsession->codecName()
			  << "\" subsession, because we've asked to receive a single " << singleMedium
			  << " session only\n";
	continue;
      } else {
	// Receive this subsession only
	singleMediumToTest = "xxxxx";
	    // this hack ensures that we get only 1 subsession of this type
      }
    }

    if (desiredPortNum != 0) {
      subsession->setClientPortNum(desiredPortNum);
      desiredPortNum += 2;
    }

    if (createReceivers) {
      if (!subsession->initiate(simpleRTPoffsetArg)) {
          ALOG(TX_LOG_INFO, TAG, "Unable to create receiver for\n");
	*env << "Unable to create receiver for \"" << subsession->mediumName()
	     << "/" << subsession->codecName()
	     << "\" subsession: " << env->getResultMsg() << "\n";
      } else {
      ALOG(TX_LOG_INFO, TAG, "Created receiver\n");
	*env << "Created receiver for \"" << subsession->mediumName()
	     << "/" << subsession->codecName() << "\" subsession (";
	if (subsession->rtcpIsMuxed()) {
        ALOG(TX_LOG_INFO, TAG, "subsession->rtcpIsMuxed()  client port = %d\n", subsession->clientPortNum());
	  *env << "client port " << subsession->clientPortNum();
	} else {
        ALOG(TX_LOG_INFO, TAG, "subsession->rtcpIsMuxed(),,,,else");
	  *env << "client ports " << subsession->clientPortNum()
	       << "-" << subsession->clientPortNum()+1;
	}
	*env << ")\n";
	madeProgress = True;

	if (subsession->rtpSource() != NULL) {
	  unsigned const thresh = 1000000; // 1 second
	  subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

	  int socketNum = subsession->rtpSource()->RTPgs()->socketNum();
	  unsigned curBufferSize = getReceiveBufferSize(*env, socketNum);
	  if (socketInputBufferSize > 0 || fileSinkBufferSize > curBufferSize) {
	    unsigned newBufferSize = socketInputBufferSize > 0 ? socketInputBufferSize : fileSinkBufferSize;
	    newBufferSize = setReceiveBufferTo(*env, socketNum, newBufferSize);
	    if (socketInputBufferSize > 0) {
            ALOG(TX_LOG_INFO, TAG, "socketInputBufferSize > 0  Changed socket receive buffer size for the\n");
	      *env << "Changed socket receive buffer size for the \""
		   << subsession->mediumName()
		   << "/" << subsession->codecName()
		   << "\" subsession from "
		   << curBufferSize << " to "
		   << newBufferSize << " bytes\n";
	    }
	  }
	}
      }
    } else {
          ALOG(TX_LOG_INFO, TAG, "socketInputBufferSize > 0=====else\n");
      if (subsession->clientPortNum() == 0) {
	*env << "No client port was specified for the \""
	     << subsession->mediumName()
	     << "/" << subsession->codecName()
	     << "\" subsession.  (Try adding the \"-p <portNum>\" option.)\n";
      } else {
		madeProgress = True;
      }
    }
  }
  if (!madeProgress)
      {
          ALOG(TX_LOG_INFO, TAG,"(!madeProgress)");
          shutdown();
      }
  // Perform additional 'setup' on each subsession, before playing them:
  setupStreams();
}

MediaSubsession *subsession;
Boolean madeProgress = False;
void continueAfterSETUP(RTSPClient*, int resultCode, char* resultString) {
    ALOG(TX_LOG_INFO, TAG,"continueAfterSETUP\n");
  if (resultCode == 0) {
      *env << "Setup \"" << subsession->mediumName()
	   << "/" << subsession->codecName()
	   << "\" subsession (";
      if (subsession->rtcpIsMuxed()) {
	*env << "client port " << subsession->clientPortNum();
      } else {
	*env << "client ports " << subsession->clientPortNum()
	     << "-" << subsession->clientPortNum()+1;
      }
      *env << ")\n";
      madeProgress = True;
  } else {
    *env << "Failed to setup \"" << subsession->mediumName()
	 << "/" << subsession->codecName()
	 << "\" subsession: " << resultString << "\n";
  }
  delete[] resultString;

  // Set up the next subsession, if any:
  setupStreams();
}

void createOutputFiles(char const* periodicFilenameSuffix) {
  char outFileName[1000];

  if (outputQuickTimeFile || outputAVIFile) {
    if (periodicFilenameSuffix[0] == '\0') {
      // Normally (unless the '-P <interval-in-seconds>' option was given) we output to 'stdout':
      sprintf(outFileName, "stdout");
    } else {
      // Otherwise output to a type-specific file name, containing "periodicFilenameSuffix":
      char const* prefix = fileNamePrefix[0] == '\0' ? "output" : fileNamePrefix;
      snprintf(outFileName, sizeof outFileName, "%s%s.%s", prefix, periodicFilenameSuffix,
	       outputAVIFile ? "avi" : generateMP4Format ? "mp4" : "mov");
    }

    if (outputQuickTimeFile) {
      qtOut = QuickTimeFileSink::createNew(*env, *session, outFileName,
					   fileSinkBufferSize,
					   movieWidth, movieHeight,
					   movieFPS,
					   packetLossCompensate,
					   syncStreams,
					   generateHintTracks,
                                           generateMP4Format);
      if (qtOut == NULL) {
	*env << "Failed to create a \"QuickTimeFileSink\" for outputting to \""
	     << outFileName << "\": " << env->getResultMsg() << "\n";
	shutdown();
      } else {
	*env << "Outputting to the file: \"" << outFileName << "\"\n";
      }

      qtOut->startPlaying(sessionAfterPlaying, NULL);
    } else { // outputAVIFile
      aviOut = AVIFileSink::createNew(*env, *session, outFileName,
				      fileSinkBufferSize,
				      movieWidth, movieHeight,
				      movieFPS,
				      packetLossCompensate);
      if (aviOut == NULL) {
	*env << "Failed to create an \"AVIFileSink\" for outputting to \""
	     << outFileName << "\": " << env->getResultMsg() << "\n";
	shutdown();
      } else {
	*env << "Outputting to the file: \"" << outFileName << "\"\n";
      }

      aviOut->startPlaying(sessionAfterPlaying, NULL);
    }
  } else {
    // Create and start "FileSink"s for each subsession:
    madeProgress = False;
    MediaSubsessionIterator iter(*session);
    while ((subsession = iter.next()) != NULL) {
      if (subsession->readSource() == NULL) continue; // was not initiated

      // Create an output file for each desired stream:
      if (singleMedium == NULL || periodicFilenameSuffix[0] != '\0') {
	// Output file name is
          // taixin
     //     "<filename-prefix><medium_name>-<codec_name>-<counter><periodicFilenameSuffix>"
     static unsigned streamCounter = 0;
     snprintf(outFileName, sizeof outFileName, "%s%s-%s-%d%s",
          fileNamePrefix, subsession->mediumName(),
          subsession->codecName(), ++streamCounter, periodicFilenameSuffix);
       } else {
     // When outputting a single medium only, we output to 'stdout
     // (unless the '-P <interval-in-seconds>' option was given):
     sprintf(outFileName, "stdout");
       }

		 memset(outFileName, 0, 1000);
			 sprintf(outFileName, "/mnt/sdcard/test.ts");
			 ALOG(TX_LOG_INFO, TAG, "set file name test.ts\n");
       FileSink* fileSink = NULL;
       Boolean createOggFileSink = False; // by default
       if (strcmp(subsession->mediumName(), "video") == 0) {
     if (strcmp(subsession->codecName(), "H264") == 0) {
       // For H.264 video stream, we use a special sink that adds 'start codes',
       // and (at the start) the SPS and PPS NAL units:
    	 ALOG(TX_LOG_INFO, TAG, "h264-------\n");
       fileSink = H264VideoFileSink::createNew(*env, outFileName,
                           subsession->fmtp_spropparametersets(),
                           fileSinkBufferSize, oneFilePerFrame);
     } else if (strcmp(subsession->codecName(), "H265") == 0) {
       // For H.265 video stream, we use a special sink that adds 'start codes',
       // and (at the start) the VPS, SPS, and PPS NAL units:
       fileSink = H265VideoFileSink::createNew(*env, outFileName,
                           subsession->fmtp_spropvps(),
                           subsession->fmtp_spropsps(),
                           subsession->fmtp_sproppps(),
                           fileSinkBufferSize, oneFilePerFrame);
     } else if (strcmp(subsession->codecName(), "THEORA") == 0) {
       createOggFileSink = True;
     }
       } else if (strcmp(subsession->mediumName(), "audio") == 0) {
     if (strcmp(subsession->codecName(), "AMR") == 0 ||
         strcmp(subsession->codecName(), "AMR-WB") == 0) {
       // For AMR audio streams, we use a special sink that inserts AMR frame hdrs:
       fileSink = AMRAudioFileSink::createNew(*env, outFileName,
                          fileSinkBufferSize, oneFilePerFrame);
     } else if (strcmp(subsession->codecName(), "VORBIS") == 0 ||
            strcmp(subsession->codecName(), "OPUS") == 0) {
       createOggFileSink = True;
     }
       }
       if (createOggFileSink) {
     fileSink = OggFileSink
       ::createNew(*env, outFileName,
               subsession->rtpTimestampFrequency(), subsession->fmtp_config());
       } else if (fileSink == NULL) {
     // Normal case:
     fileSink = FileSink::createNew(*env, outFileName,
                        fileSinkBufferSize, oneFilePerFrame);
       }
       subsession->sink = fileSink;

       if (subsession->sink == NULL) {
    	   ALOG(TX_LOG_INFO, TAG, "Failed to create FileSink for test.ts\n");
     *env << "Failed to create FileSink for \"" << outFileName
          << "\": " << env->getResultMsg() << "\n";
       } else {
     if (singleMedium == NULL) {
    	 ALOG(TX_LOG_INFO, TAG,"created out file : test.ts\n");
       *env << "Created output file: \"" << outFileName << "\"\n";
     } else {
    	 ALOG(TX_LOG_INFO, TAG,"--------else\n");
       *env << "Outputting data from the \"" << subsession->mediumName()
            << "/" << subsession->codecName()
            << "\" subsession to \"" << outFileName << "\"\n";
     }

     if (strcmp(subsession->mediumName(), "video") == 0 &&
         strcmp(subsession->codecName(), "MP4V-ES") == 0 &&
         subsession->fmtp_config() != NULL) {
       // For MPEG-4 video RTP streams, the 'config' information
       // from the SDP description contains useful VOL etc. headers.
       // Insert this data at the front of the output file:
       unsigned configLen;
       unsigned char* configData
         = parseGeneralConfigStr(subsession->fmtp_config(), configLen);
       struct timeval timeNow;
       gettimeofday(&timeNow, NULL);
       fileSink->addData(configData, configLen, timeNow);
       delete[] configData;
     }

     subsession->sink->startPlaying(*(subsession->readSource()),
                        subsessionAfterPlaying,
                        subsession);

     // Also set a handler to be called if a RTCP "BYE" arrives
     // for this subsession:
     if (subsession->rtcpInstance() != NULL) {
       subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, subsession);
     }

     madeProgress = True;
       }
     }
     if (!madeProgress)
    	 {
    	 ALOG(TX_LOG_INFO, TAG,"!madeProgress\n");
    	 shutdown();
    	 }
   }
 }

 void createPeriodicOutputFiles() {
   // Create a filename suffix that notes the time interval that's being recorded:
   char periodicFileNameSuffix[100];
   snprintf(periodicFileNameSuffix, sizeof periodicFileNameSuffix, "-%05d-%05d",
        fileOutputSecondsSoFar, fileOutputSecondsSoFar + fileOutputInterval);
   createOutputFiles(periodicFileNameSuffix);

   // Schedule an event for writing the next output file:
   periodicFileOutputTask
     = env->taskScheduler().scheduleDelayedTask(fileOutputInterval*1000000,
                            (TaskFunc*)periodicFileOutputTimerHandler,
                            (void*)NULL);
 }

 void setupStreams() {
      ALOG(TX_LOG_INFO, TAG,"setup Streams\n");
   static MediaSubsessionIterator* setupIter = NULL;
   if(setupIter == NULL)
   {
       ALOG(TX_LOG_INFO, TAG,"setuplter == NULL\n");
   }else{
       ALOG(TX_LOG_INFO, TAG,"setuplter !!!!== NULL\n");
   }
   if (setupIter == NULL) setupIter = new MediaSubsessionIterator(*session);
   ALOG(TX_LOG_INFO, TAG,"1111\n");
   while ((subsession = setupIter->next()) != NULL) {
     // We have another subsession left to set up:
     if (subsession->clientPortNum() == 0) continue; // port # was not set

     setupSubsession(subsession, streamUsingTCP, forceMulticastOnUnspecified, continueAfterSETUP);
     return;
   }

   // We're done setting up subsessions.
   delete setupIter;
   if (!madeProgress) shutdown();

   // Create output files:
   if (createReceivers) {
     if (fileOutputInterval > 0) {
       createPeriodicOutputFiles();
     } else {
       createOutputFiles("");
     }
   }

   // Finally, start playing each subsession, to start the data flow:
   if (duration == 0) {
     if (scale > 0) duration = session->playEndTime() - initialSeekTime; // use SDP end time
     else if (scale < 0) duration = initialSeekTime;
   }
   if (duration < 0) duration = 0.0;

   endTime = initialSeekTime;
   if (scale > 0) {
     if (duration <= 0) endTime = -1.0f;
     else endTime = initialSeekTime + duration;
   } else {
     endTime = initialSeekTime - duration;
     if (endTime < 0) endTime = 0.0f;
   }

   char const* absStartTime = initialAbsoluteSeekTime != NULL ? initialAbsoluteSeekTime : session->absStartTime();
   if (absStartTime != NULL) {
     // Either we or the server have specified that seeking should be done by 'absolute' time:
     startPlayingSession(session, absStartTime, session->absEndTime(), scale, continueAfterPLAY);
   } else {
     // Normal case: Seek by relative time (NPT):
     startPlayingSession(session, initialSeekTime, endTime, scale, continueAfterPLAY);
   }
 }

 void continueAfterPLAY(RTSPClient*, int resultCode, char* resultString) {
   if (resultCode != 0) {
     *env << "Failed to start playing session: " << resultString << "\n";
     delete[] resultString;
     shutdown();
     return;
   } else {
     *env << "Started playing session\n";
   }
   delete[] resultString;

   if (qosMeasurementIntervalMS > 0) {
     // Begin periodic QOS measurements:
     beginQOSMeasurement();
   }

   // Figure out how long to delay (if at all) before shutting down, or
   // repeating the playing
   Boolean timerIsBeingUsed = False;
   double secondsToDelay = duration;
   if (duration > 0) {
     // First, adjust "duration" based on any change to the play range (that was specified in the "PLAY" response):
     double rangeAdjustment = (session->playEndTime() - session->playStartTime()) - (endTime - initialSeekTime);
     if (duration + rangeAdjustment > 0.0) duration += rangeAdjustment;

     timerIsBeingUsed = True;
     double absScale = scale > 0 ? scale : -scale; // ASSERT: scale != 0
     secondsToDelay = duration/absScale + durationSlop;

     int64_t uSecsToDelay = (int64_t)(secondsToDelay*1000000.0);
     sessionTimerTask = env->taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)sessionTimerHandler, (void*)NULL);
   }

   char const* actionString
     = createReceivers? "Receiving streamed data":"Data is being streamed";
   if (timerIsBeingUsed) {
     *env << actionString
         << " (for up to " << secondsToDelay
         << " seconds)...\n";
   } else {
 #ifdef USE_SIGNALS
     pid_t ourPid = getpid();
     *env << actionString
         << " (signal with \"kill -HUP " << (int)ourPid
         << "\" or \"kill -USR1 " << (int)ourPid
         << "\" to terminate)...\n";
 #else
     *env << actionString << "...\n";
 #endif
   }

   // Watch for incoming packets (if desired):
   checkForPacketArrival(NULL);
   checkInterPacketGaps(NULL);
 }

 void closeMediaSinks() {
   Medium::close(qtOut); qtOut = NULL;
   Medium::close(aviOut); aviOut = NULL;

   if (session == NULL) return;
   MediaSubsessionIterator iter(*session);
   MediaSubsession* subsession;
   while ((subsession = iter.next()) != NULL) {
     Medium::close(subsession->sink);
     subsession->sink = NULL;
   }
 }

 void subsessionAfterPlaying(void* clientData) {
   // Begin by closing this media subsession's stream:
   MediaSubsession* subsession = (MediaSubsession*)clientData;
   Medium::close(subsession->sink);
   subsession->sink = NULL;

   // Next, check whether *all* subsessions' streams have now been closed:
   MediaSession& session = subsession->parentSession();
   MediaSubsessionIterator iter(session);
   while ((subsession = iter.next()) != NULL) {
     if (subsession->sink != NULL) return; // this subsession is still active
   }

   // All subsessions' streams have now been closed
   sessionAfterPlaying();
 }

 void subsessionByeHandler(void* clientData) {
   struct timeval timeNow;
   gettimeofday(&timeNow, NULL);
   unsigned secsDiff = timeNow.tv_sec - startTime.tv_sec;

   MediaSubsession* subsession = (MediaSubsession*)clientData;
   *env << "Received RTCP \"BYE\" on \"" << subsession->mediumName()
     << "/" << subsession->codecName()
     << "\" subsession (after " << secsDiff
     << " seconds)\n";

   // Act now as if the subsession had closed:
   subsessionAfterPlaying(subsession);
 }

 void sessionAfterPlaying(void* /*clientData*/) {
   if (!playContinuously) {
     shutdown(0);
   } else {
     // We've been asked to play the stream(s) over again.
     // First, reset state from the current session:
     if (env != NULL) {
       env->taskScheduler().unscheduleDelayedTask(periodicFileOutputTask);
       env->taskScheduler().unscheduleDelayedTask(sessionTimerTask);
       env->taskScheduler().unscheduleDelayedTask(arrivalCheckTimerTask);
       env->taskScheduler().unscheduleDelayedTask(interPacketGapCheckTimerTask);
       env->taskScheduler().unscheduleDelayedTask(qosMeasurementTimerTask);
     }
     totNumPacketsReceived = ~0;

    startPlayingSession(session, initialSeekTime, endTime, scale, continueAfterPLAY);
  }
}

void sessionTimerHandler(void* /*clientData*/) {
  sessionTimerTask = NULL;

  sessionAfterPlaying();
}

void periodicFileOutputTimerHandler(void* /*clientData*/) {
  fileOutputSecondsSoFar += fileOutputInterval;

  // First, close the existing output files:
  closeMediaSinks();

  // Then, create new output files:
  createPeriodicOutputFiles();
}

class qosMeasurementRecord {
public:
  qosMeasurementRecord(struct timeval const& startTime, RTPSource* src)
    : fSource(src), fNext(NULL),
      kbits_per_second_min(1e20), kbits_per_second_max(0),
      kBytesTotal(0.0),
      packet_loss_fraction_min(1.0), packet_loss_fraction_max(0.0),
      totNumPacketsReceived(0), totNumPacketsExpected(0) {
    measurementEndTime = measurementStartTime = startTime;

    RTPReceptionStatsDB::Iterator statsIter(src->receptionStatsDB());
    // Assume that there's only one SSRC source (usually the case):
    RTPReceptionStats* stats = statsIter.next(True);
    if (stats != NULL) {
      kBytesTotal = stats->totNumKBytesReceived();
      totNumPacketsReceived = stats->totNumPacketsReceived();
      totNumPacketsExpected = stats->totNumPacketsExpected();
    }
  }
  virtual ~qosMeasurementRecord() { delete fNext; }

  void periodicQOSMeasurement(struct timeval const& timeNow);

public:
  RTPSource* fSource;
  qosMeasurementRecord* fNext;

public:
  struct timeval measurementStartTime, measurementEndTime;
  double kbits_per_second_min, kbits_per_second_max;
  double kBytesTotal;
  double packet_loss_fraction_min, packet_loss_fraction_max;
  unsigned totNumPacketsReceived, totNumPacketsExpected;
};

static qosMeasurementRecord* qosRecordHead = NULL;

static void periodicQOSMeasurement(void* clientData); // forward

static unsigned nextQOSMeasurementUSecs;

static void scheduleNextQOSMeasurement() {
  nextQOSMeasurementUSecs += qosMeasurementIntervalMS*1000;
  struct timeval timeNow;
  gettimeofday(&timeNow, NULL);
  unsigned timeNowUSecs = timeNow.tv_sec*1000000 + timeNow.tv_usec;
  int usecsToDelay = nextQOSMeasurementUSecs - timeNowUSecs;

  qosMeasurementTimerTask = env->taskScheduler().scheduleDelayedTask(
     usecsToDelay, (TaskFunc*)periodicQOSMeasurement, (void*)NULL);
}

static void periodicQOSMeasurement(void* /*clientData*/) {
  struct timeval timeNow;
  gettimeofday(&timeNow, NULL);

  for (qosMeasurementRecord* qosRecord = qosRecordHead;
       qosRecord != NULL; qosRecord = qosRecord->fNext) {
    qosRecord->periodicQOSMeasurement(timeNow);
  }

  // Do this again later:
  scheduleNextQOSMeasurement();
}

void qosMeasurementRecord
::periodicQOSMeasurement(struct timeval const& timeNow) {
  unsigned secsDiff = timeNow.tv_sec - measurementEndTime.tv_sec;
  int usecsDiff = timeNow.tv_usec - measurementEndTime.tv_usec;
  double timeDiff = secsDiff + usecsDiff/1000000.0;
  measurementEndTime = timeNow;

  RTPReceptionStatsDB::Iterator statsIter(fSource->receptionStatsDB());
  // Assume that there's only one SSRC source (usually the case):
  RTPReceptionStats* stats = statsIter.next(True);
  if (stats != NULL) {
    double kBytesTotalNow = stats->totNumKBytesReceived();
    double kBytesDeltaNow = kBytesTotalNow - kBytesTotal;
    kBytesTotal = kBytesTotalNow;

    double kbpsNow = timeDiff == 0.0 ? 0.0 : 8*kBytesDeltaNow/timeDiff;
    if (kbpsNow < 0.0) kbpsNow = 0.0; // in case of roundoff error
    if (kbpsNow < kbits_per_second_min) kbits_per_second_min = kbpsNow;
    if (kbpsNow > kbits_per_second_max) kbits_per_second_max = kbpsNow;

    unsigned totReceivedNow = stats->totNumPacketsReceived();
    unsigned totExpectedNow = stats->totNumPacketsExpected();
    unsigned deltaReceivedNow = totReceivedNow - totNumPacketsReceived;
    unsigned deltaExpectedNow = totExpectedNow - totNumPacketsExpected;
    totNumPacketsReceived = totReceivedNow;
    totNumPacketsExpected = totExpectedNow;

    double lossFractionNow = deltaExpectedNow == 0 ? 0.0
      : 1.0 - deltaReceivedNow/(double)deltaExpectedNow;
    //if (lossFractionNow < 0.0) lossFractionNow = 0.0; //reordering can cause
    if (lossFractionNow < packet_loss_fraction_min) {
      packet_loss_fraction_min = lossFractionNow;
    }
    if (lossFractionNow > packet_loss_fraction_max) {
      packet_loss_fraction_max = lossFractionNow;
    }
  }
}

void beginQOSMeasurement() {
  // Set up a measurement record for each active subsession:
  struct timeval startTime;
  gettimeofday(&startTime, NULL);
  nextQOSMeasurementUSecs = startTime.tv_sec*1000000 + startTime.tv_usec;
  qosMeasurementRecord* qosRecordTail = NULL;
  MediaSubsessionIterator iter(*session);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    RTPSource* src = subsession->rtpSource();
    if (src == NULL) continue;

    qosMeasurementRecord* qosRecord
      = new qosMeasurementRecord(startTime, src);
    if (qosRecordHead == NULL) qosRecordHead = qosRecord;
    if (qosRecordTail != NULL) qosRecordTail->fNext = qosRecord;
    qosRecordTail  = qosRecord;
  }

  // Then schedule the first of the periodic measurements:
  scheduleNextQOSMeasurement();
}

void printQOSData(int exitCode) {
  *env << "begin_QOS_statistics\n";

  // Print out stats for each active subsession:
  qosMeasurementRecord* curQOSRecord = qosRecordHead;
  if (session != NULL) {
    MediaSubsessionIterator iter(*session);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
      RTPSource* src = subsession->rtpSource();
      if (src == NULL) continue;

      *env << "subsession\t" << subsession->mediumName()
	   << "/" << subsession->codecName() << "\n";

      unsigned numPacketsReceived = 0, numPacketsExpected = 0;

      if (curQOSRecord != NULL) {
	numPacketsReceived = curQOSRecord->totNumPacketsReceived;
	numPacketsExpected = curQOSRecord->totNumPacketsExpected;
      }
      *env << "num_packets_received\t" << numPacketsReceived << "\n";
      *env << "num_packets_lost\t" << int(numPacketsExpected - numPacketsReceived) << "\n";

      if (curQOSRecord != NULL) {
	unsigned secsDiff = curQOSRecord->measurementEndTime.tv_sec
	  - curQOSRecord->measurementStartTime.tv_sec;
	int usecsDiff = curQOSRecord->measurementEndTime.tv_usec
	  - curQOSRecord->measurementStartTime.tv_usec;
	double measurementTime = secsDiff + usecsDiff/1000000.0;
	*env << "elapsed_measurement_time\t" << measurementTime << "\n";

	*env << "kBytes_received_total\t" << curQOSRecord->kBytesTotal << "\n";

	*env << "measurement_sampling_interval_ms\t" << qosMeasurementIntervalMS << "\n";

	if (curQOSRecord->kbits_per_second_max == 0) {
	  // special case: we didn't receive any data:
	  *env <<
	    "kbits_per_second_min\tunavailable\n"
	    "kbits_per_second_ave\tunavailable\n"
	    "kbits_per_second_max\tunavailable\n";
	} else {
	  *env << "kbits_per_second_min\t" << curQOSRecord->kbits_per_second_min << "\n";
	  *env << "kbits_per_second_ave\t"
	       << (measurementTime == 0.0 ? 0.0 : 8*curQOSRecord->kBytesTotal/measurementTime) << "\n";
	  *env << "kbits_per_second_max\t" << curQOSRecord->kbits_per_second_max << "\n";
	}

	*env << "packet_loss_percentage_min\t" << 100*curQOSRecord->packet_loss_fraction_min << "\n";
	double packetLossFraction = numPacketsExpected == 0 ? 1.0
	  : 1.0 - numPacketsReceived/(double)numPacketsExpected;
	if (packetLossFraction < 0.0) packetLossFraction = 0.0;
	*env << "packet_loss_percentage_ave\t" << 100*packetLossFraction << "\n";
	*env << "packet_loss_percentage_max\t"
	     << (packetLossFraction == 1.0 ? 100.0 : 100*curQOSRecord->packet_loss_fraction_max) << "\n";

	RTPReceptionStatsDB::Iterator statsIter(src->receptionStatsDB());
	// Assume that there's only one SSRC source (usually the case):
	RTPReceptionStats* stats = statsIter.next(True);
	if (stats != NULL) {
	  *env << "inter_packet_gap_ms_min\t" << stats->minInterPacketGapUS()/1000.0 << "\n";
	  struct timeval totalGaps = stats->totalInterPacketGaps();
	  double totalGapsMS = totalGaps.tv_sec*1000.0 + totalGaps.tv_usec/1000.0;
	  unsigned totNumPacketsReceived = stats->totNumPacketsReceived();
	  *env << "inter_packet_gap_ms_ave\t"
	       << (totNumPacketsReceived == 0 ? 0.0 : totalGapsMS/totNumPacketsReceived) << "\n";
	  *env << "inter_packet_gap_ms_max\t" << stats->maxInterPacketGapUS()/1000.0 << "\n";
	}

	curQOSRecord = curQOSRecord->fNext;
      }
    }
  }

  *env << "end_QOS_statistics\n";
  delete qosRecordHead;
}

// Boolean areAlreadyShuttingDown = False;
// int shutdownExitCode;
void shutdown(int exitCode) {
  if (areAlreadyShuttingDown) return; // in case we're called after receiving a RTCP "BYE" while in the middle of a "TEARDOWN".
  areAlreadyShuttingDown = True;

  shutdownExitCode = exitCode;
  if (env != NULL) {
    env->taskScheduler().unscheduleDelayedTask(periodicFileOutputTask);
    env->taskScheduler().unscheduleDelayedTask(sessionTimerTask);
    env->taskScheduler().unscheduleDelayedTask(arrivalCheckTimerTask);
    env->taskScheduler().unscheduleDelayedTask(interPacketGapCheckTimerTask);
    env->taskScheduler().unscheduleDelayedTask(qosMeasurementTimerTask);
  }

  if (qosMeasurementIntervalMS > 0) {
    printQOSData(exitCode);
  }

  // Teardown, then shutdown, any outstanding RTP/RTCP subsessions
  Boolean shutdownImmediately = True; // by default
  if (session != NULL) {
    RTSPClient::responseHandler* responseHandlerForTEARDOWN = NULL; // unless:
    if (waitForResponseToTEARDOWN) {
      shutdownImmediately = False;
      responseHandlerForTEARDOWN = continueAfterTEARDOWN;
    }
    tearDownSession(session, responseHandlerForTEARDOWN);
  }
  ALOG(TX_LOG_INFO, TAG, "show down ok --------------\n");
  // 不会直接停止， 直接return
  if (shutdownImmediately) continueAfterTEARDOWN(NULL, 0, NULL);
}

void continueAfterTEARDOWN(RTSPClient*, int /*resultCode*/, char* resultString) {
    ALOG(TX_LOG_INFO, TAG, "continueAfterTEARDOWN\n");
    delete[] resultString;

    // Now that we've stopped any more incoming data from arriving, close our output files:
    ALOG(TX_LOG_INFO, TAG, "before closeMediaSinks\n");
    closeMediaSinks();
    ALOG(TX_LOG_INFO, TAG, "after closeMediaSinks\n");
    Medium::close(session);
    ALOG(TX_LOG_INFO, TAG, "  Medium::close(session);\n");
    // Finally, shut down our client:
    delete ourAuthenticator;
    delete authDBForREGISTER;
    Medium::close(ourClient);
    ALOG(TX_LOG_INFO, TAG, "  Medium::close(ourClient);\n");
    // Adios...
    // exit(shutdownExitCode);
    return;
}

void signalHandlerShutdown(int /*sig*/) {
    ALOG(TX_LOG_INFO, TAG, " signalHandlerShutdown ,,Got shutdown signal\n");
  *env << "Got shutdown signal\n";
  waitForResponseToTEARDOWN = False; // to ensure that we end, even if the server does not respond to our TEARDOWN
  shutdown(0);
}

void checkForPacketArrival(void* /*clientData*/) {
  if (!notifyOnPacketArrival) return; // we're not checking

  // Check each subsession, to see whether it has received data packets:
  unsigned numSubsessionsChecked = 0;
  unsigned numSubsessionsWithReceivedData = 0;
  unsigned numSubsessionsThatHaveBeenSynced = 0;

  MediaSubsessionIterator iter(*session);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    RTPSource* src = subsession->rtpSource();
    if (src == NULL) continue;
    ++numSubsessionsChecked;

    if (src->receptionStatsDB().numActiveSourcesSinceLastReset() > 0) {
      // At least one data packet has arrived
      ++numSubsessionsWithReceivedData;
    }
    if (src->hasBeenSynchronizedUsingRTCP()) {
      ++numSubsessionsThatHaveBeenSynced;
    }
  }

  unsigned numSubsessionsToCheck = numSubsessionsChecked;
  // Special case for "QuickTimeFileSink"s and "AVIFileSink"s:
  // They might not use all of the input sources:
  if (qtOut != NULL) {
    numSubsessionsToCheck = qtOut->numActiveSubsessions();
  } else if (aviOut != NULL) {
    numSubsessionsToCheck = aviOut->numActiveSubsessions();
  }

  Boolean notifyTheUser;
  if (!syncStreams) {
    notifyTheUser = numSubsessionsWithReceivedData > 0; // easy case
  } else {
    notifyTheUser = numSubsessionsWithReceivedData >= numSubsessionsToCheck
      && numSubsessionsThatHaveBeenSynced == numSubsessionsChecked;
    // Note: A subsession with no active sources is considered to be synced
  }
  if (notifyTheUser) {
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
	char timestampStr[100];
	sprintf(timestampStr, "%ld%03ld", timeNow.tv_sec, (long)(timeNow.tv_usec/1000));
    *env << (syncStreams ? "Synchronized d" : "D")
		<< "ata packets have begun arriving [" << timestampStr << "]\007\n";
    return;
  }

  // No luck, so reschedule this check again, after a delay:
  int uSecsToDelay = 100000; // 100 ms
  arrivalCheckTimerTask
    = env->taskScheduler().scheduleDelayedTask(uSecsToDelay,
			       (TaskFunc*)checkForPacketArrival, NULL);
}

void checkInterPacketGaps(void* /*clientData*/) {
  if (interPacketGapMaxTime == 0) return; // we're not checking

  // Check each subsession, counting up how many packets have been received:
  unsigned newTotNumPacketsReceived = 0;

  MediaSubsessionIterator iter(*session);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    RTPSource* src = subsession->rtpSource();
    if (src == NULL) continue;
    newTotNumPacketsReceived += src->receptionStatsDB().totNumPacketsReceived();
  }

  if (newTotNumPacketsReceived == totNumPacketsReceived) {
    // No additional packets have been received since the last time we
    // checked, so end this stream:
    *env << "Closing session, because we stopped receiving packets.\n";
    interPacketGapCheckTimerTask = NULL;
    sessionAfterPlaying();
  } else {
    totNumPacketsReceived = newTotNumPacketsReceived;
    // Check again, after the specified delay:
    interPacketGapCheckTimerTask
      = env->taskScheduler().scheduleDelayedTask(interPacketGapMaxTime*1000000,
				 (TaskFunc*)checkInterPacketGaps, NULL);
  }
}
