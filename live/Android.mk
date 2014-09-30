LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
# NDK 7d支持硬浮点的直接编译
#APP_ABI := armeabi-v7a-hard
 
LOCAL_MODULE := txlive555
 
LOCAL_ARM_MODE := arm
 
LOCAL_PRELINK_MODULE := false
 
LOCAL_CPPFLAGS := \
	-DNULL=0 -DSOCKLEN_T=socklen_t -DNO_SSTREAM -DBSD=1 -DNO_SSTREAM -fexceptions -DANDROID -DXLOCALE_NOT_USED
 
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/BasicUsageEnvironment/include \
	$(LOCAL_PATH)/BasicUsageEnvironment \
	$(LOCAL_PATH)/UsageEnvironment/include \
	$(LOCAL_PATH)/UsageEnvironment \
	$(LOCAL_PATH)/groupsock/include \
	$(LOCAL_PATH)/groupsock \
	$(LOCAL_PATH)/liveMedia/include \
	$(LOCAL_PATH)/liveMedia \
 
LOCAL_MODULE_TAGS := optional
 
LOCAL_SRC_FILES := \
groupsock/IOHandlers.cpp                                       \
groupsock/Groupsock.cpp                                        \
groupsock/NetAddress.cpp                                       \
groupsock/GroupsockHelper.cpp                                  \
groupsock/NetInterface.cpp                                     \
groupsock/GroupEId.cpp                                         \
groupsock/inet.c                                               \
liveMedia/rtcp_from_spec.c                                     \
liveMedia/MPEG4GenericRTPSource.cpp                            \
liveMedia/MatroskaFileServerMediaSubsession.cpp                \
liveMedia/DVVideoRTPSink.cpp                                   \
liveMedia/H265VideoFileServerMediaSubsession.cpp               \
liveMedia/DarwinInjector.cpp                                   \
liveMedia/OggFile.cpp                                          \
liveMedia/MP3Transcoder.cpp                                    \
liveMedia/VorbisAudioRTPSink.cpp                               \
liveMedia/H265VideoStreamDiscreteFramer.cpp                    \
liveMedia/MPEG2TransportStreamFromESSource.cpp                 \
liveMedia/MPEG2TransportStreamIndexFile.cpp                    \
liveMedia/MediaSink.cpp                                        \
liveMedia/MPEG1or2Demux.cpp                                    \
liveMedia/QuickTimeFileSink.cpp                                \
liveMedia/MPEG1or2DemuxedServerMediaSubsession.cpp             \
liveMedia/MatroskaFileParser.cpp                               \
liveMedia/JPEGVideoSource.cpp                                  \
liveMedia/H264VideoStreamDiscreteFramer.cpp                    \
liveMedia/MP3ADURTPSink.cpp                                    \
liveMedia/MPEG4VideoStreamFramer.cpp                           \
liveMedia/MP3ADURTPSource.cpp                                  \
liveMedia/MP3InternalsHuffmanTable.cpp                         \
liveMedia/FramedFileSource.cpp                                 \
liveMedia/MPEG4GenericRTPSink.cpp                              \
liveMedia/MP3ADUTranscoder.cpp                                 \
liveMedia/FramedFilter.cpp                                     \
liveMedia/StreamParser.cpp                                     \
liveMedia/H265VideoRTPSink.cpp                                 \
liveMedia/TCPStreamSink.cpp                                    \
liveMedia/BasicUDPSource.cpp                                   \
liveMedia/MPEG1or2VideoFileServerMediaSubsession.cpp           \
liveMedia/MPEG4ESVideoRTPSource.cpp                            \
liveMedia/MatroskaFile.cpp                                     \
liveMedia/OggFileSink.cpp                                      \
liveMedia/MPEG1or2VideoRTPSource.cpp                           \
liveMedia/InputFile.cpp                                        \
liveMedia/BitVector.cpp                                        \
liveMedia/StreamReplicator.cpp                                 \
liveMedia/H264or5VideoStreamDiscreteFramer.cpp                 \
liveMedia/MP3FileSource.cpp                                    \
liveMedia/OggFileParser.cpp                                    \
liveMedia/EBMLNumber.cpp                                       \
liveMedia/H263plusVideoStreamParser.cpp                        \
liveMedia/VP8VideoRTPSink.cpp                                  \
liveMedia/VideoRTPSink.cpp                                     \
liveMedia/VorbisAudioRTPSource.cpp                             \
liveMedia/H263plusVideoStreamFramer.cpp                        \
liveMedia/DVVideoStreamFramer.cpp                              \
liveMedia/Base64.cpp                                           \
liveMedia/MPEG1or2AudioRTPSink.cpp                             \
liveMedia/ByteStreamFileSource.cpp                             \
liveMedia/H265VideoRTPSource.cpp                               \
liveMedia/AMRAudioFileServerMediaSubsession.cpp                \
liveMedia/H261VideoRTPSource.cpp                               \
liveMedia/RTSPRegisterSender.cpp                               \
liveMedia/MPEG1or2VideoRTPSink.cpp                             \
liveMedia/RTSPClient.cpp                                       \
liveMedia/RTPSource.cpp                                        \
liveMedia/MPEG2IndexFromTransportStream.cpp                    \
liveMedia/AMRAudioFileSink.cpp                                 \
liveMedia/AudioRTPSink.cpp                                     \
liveMedia/H264VideoFileSink.cpp                                \
liveMedia/AudioInputDevice.cpp                                 \
liveMedia/MPEG4VideoStreamDiscreteFramer.cpp                   \
liveMedia/H264or5VideoFileSink.cpp                             \
liveMedia/MP3ADUinterleaving.cpp                               \
liveMedia/AC3AudioStreamFramer.cpp                             \
liveMedia/MPEG2TransportStreamTrickModeFilter.cpp              \
liveMedia/MPEG1or2DemuxedElementaryStream.cpp                  \
liveMedia/MPEG4ESVideoRTPSink.cpp                              \
liveMedia/H264VideoFileServerMediaSubsession.cpp               \
liveMedia/OnDemandServerMediaSubsession.cpp                    \
liveMedia/OutputFile.cpp                                       \
liveMedia/FileSink.cpp                                         \
liveMedia/FileServerMediaSubsession.cpp                        \
liveMedia/MP3InternalsHuffman.cpp                              \
liveMedia/AMRAudioRTPSource.cpp                                \
liveMedia/MatroskaFileServerDemux.cpp                          \
liveMedia/ADTSAudioFileServerMediaSubsession.cpp               \
liveMedia/BasicUDPSink.cpp                                     \
liveMedia/MP3AudioMatroskaFileServerMediaSubsession.cpp        \
liveMedia/MPEG2TransportStreamFramer.cpp                       \
liveMedia/uLawAudioFilter.cpp                                  \
liveMedia/RTSPServer.cpp                                       \
liveMedia/DVVideoFileServerMediaSubsession.cpp                 \
liveMedia/H263plusVideoRTPSink.cpp                             \
liveMedia/H263plusVideoFileServerMediaSubsession.cpp           \
liveMedia/H265VideoStreamFramer.cpp                            \
liveMedia/JPEGVideoRTPSink.cpp                                 \
liveMedia/WAVAudioFileServerMediaSubsession.cpp                \
liveMedia/ByteStreamMultiFileSource.cpp                        \
liveMedia/TheoraVideoRTPSink.cpp                               \
liveMedia/ByteStreamMemoryBufferSource.cpp                     \
liveMedia/ourMD5.cpp                                           \
liveMedia/MPEG2TransportFileServerMediaSubsession.cpp          \
liveMedia/MPEG2TransportStreamMultiplexor.cpp                  \
liveMedia/MPEGVideoStreamParser.cpp                            \
liveMedia/MPEG1or2VideoStreamDiscreteFramer.cpp                \
liveMedia/VP8VideoRTPSource.cpp                                \
liveMedia/H264or5VideoStreamFramer.cpp                         \
liveMedia/AMRAudioRTPSink.cpp                                  \
liveMedia/RTPInterface.cpp                                     \
liveMedia/TheoraVideoRTPSource.cpp                             \
liveMedia/Locale.cpp                                           \
liveMedia/MPEGVideoStreamFramer.cpp                            \
liveMedia/QCELPAudioRTPSource.cpp                              \
liveMedia/ADTSAudioFileSource.cpp                              \
liveMedia/MediaSource.cpp                                      \
liveMedia/AC3AudioRTPSource.cpp                                \
liveMedia/WAVAudioFileSource.cpp                               \
liveMedia/MPEG4LATMAudioRTPSink.cpp                            \
liveMedia/MPEG1or2AudioRTPSource.cpp                           \
liveMedia/GSMAudioRTPSink.cpp                                  \
liveMedia/DigestAuthentication.cpp                             \
liveMedia/RTSPCommon.cpp                                       \
liveMedia/MP3ADU.cpp                                           \
liveMedia/Media.cpp                                            \
liveMedia/MPEG4VideoFileServerMediaSubsession.cpp              \
liveMedia/H264or5VideoRTPSink.cpp                              \
liveMedia/MPEG4LATMAudioRTPSource.cpp                          \
liveMedia/H264VideoRTPSink.cpp                                 \
liveMedia/MPEG1or2AudioStreamFramer.cpp                        \
liveMedia/AC3AudioRTPSink.cpp                                  \
liveMedia/H264VideoStreamFramer.cpp                            \
liveMedia/H265VideoFileSink.cpp                                \
liveMedia/T140TextRTPSink.cpp                                  \
liveMedia/OggFileServerMediaSubsession.cpp                     \
liveMedia/MP3Internals.cpp                                     \
liveMedia/MPEG1or2FileServerDemux.cpp                          \
liveMedia/RTPSink.cpp                                          \
liveMedia/AC3AudioFileServerMediaSubsession.cpp                \
liveMedia/H264VideoRTPSource.cpp                               \
liveMedia/ProxyServerMediaSession.cpp                          \
liveMedia/SimpleRTPSink.cpp                                    \
liveMedia/MultiFramedRTPSink.cpp                               \
liveMedia/H263plusVideoRTPSource.cpp                           \
liveMedia/RTCP.cpp                                             \
liveMedia/JPEGVideoRTPSource.cpp                               \
liveMedia/AMRAudioFileSource.cpp                               \
liveMedia/MultiFramedRTPSource.cpp                             \
liveMedia/AVIFileSink.cpp                                      \
liveMedia/TextRTPSink.cpp                                      \
liveMedia/AMRAudioSource.cpp                                   \
liveMedia/MatroskaDemuxedTrack.cpp                             \
liveMedia/MP3ADUdescriptor.cpp                                 \
liveMedia/QuickTimeGenericRTPSource.cpp                        \
liveMedia/ServerMediaSession.cpp                               \
liveMedia/FramedSource.cpp                                     \
liveMedia/MP3AudioFileServerMediaSubsession.cpp                \
liveMedia/SimpleRTPSource.cpp                                  \
liveMedia/OggDemuxedTrack.cpp                                  \
liveMedia/OggFileServerDemux.cpp                               \
liveMedia/MP3StreamState.cpp                                   \
liveMedia/MPEG1or2VideoStreamFramer.cpp                        \
liveMedia/MPEG2TransportStreamFromPESSource.cpp                \
liveMedia/PassiveServerMediaSubsession.cpp                     \
liveMedia/DeviceSource.cpp                                     \
liveMedia/RTSPServerSupportingHTTPStreaming.cpp                \
liveMedia/MediaSession.cpp                                     \
liveMedia/MPEG2TransportUDPServerMediaSubsession.cpp           \
liveMedia/DVVideoRTPSource.cpp                                 \
liveMedia/SIPClient.cpp                                        \
BasicUsageEnvironment/DelayQueue.cpp                           \
BasicUsageEnvironment/BasicTaskScheduler.cpp                   \
BasicUsageEnvironment/BasicUsageEnvironment0.cpp               \
BasicUsageEnvironment/BasicHashTable.cpp                       \
BasicUsageEnvironment/BasicUsageEnvironment.cpp                \
BasicUsageEnvironment/BasicTaskScheduler0.cpp                  \
UsageEnvironment/HashTable.cpp                                 \
UsageEnvironment/strDup.cpp                                    \
UsageEnvironment/UsageEnvironment.cpp                          
 
include $(BUILD_SHARED_LIBRARY)