LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
#APP_ABI := armeabi-v7a-hard
 
LOCAL_MODULE := txlive555-jni
 
LOCAL_ARM_MODE := arm
 
LOCAL_PRELINK_MODULE := false
 
LOCAL_CPPFLAGS := \
	-DNULL=0 -DSOCKLEN_T=socklen_t -DNO_SSTREAM -DBSD=1 -DNO_SSTREAM -fexceptions -DANDROID -DXLOCALE_NOT_USED
 
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../live555/BasicUsageEnvironment/include \
	$(LOCAL_PATH)/../../../live555/BasicUsageEnvironment \
	$(LOCAL_PATH)/../../../live555/UsageEnvironment/include \
	$(LOCAL_PATH)/../../../live555/UsageEnvironment \
	$(LOCAL_PATH)/../../../live555/groupsock/include \
	$(LOCAL_PATH)/../../../live555/groupsock \
	$(LOCAL_PATH)/../../../live555/liveMedia/include \
	$(LOCAL_PATH)/../../../live555/liveMedia \
 
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += openRTSP.cpp  
LOCAL_SRC_FILES += recorder.cpp  
LOCAL_SRC_FILES += tx_recorder_jni.cpp  
            
LOCAL_SHARED_LIBRARIES := txlive555     
LOCAL_LDLIBS := -llog               
 
include $(BUILD_SHARED_LIBRARY)