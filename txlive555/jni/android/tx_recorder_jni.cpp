#include <android/log.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <assert.h>
#include "loghelp.h"
#include "tx_recorder_jni_define.h"
#include "recorder.hh"

/*搜寻设备*/
JNIEXPORT jint JNICALL _startRecoderRTSPStream(JNIEnv *env, jclass clazz, jstring filepath, jstring rtsppath)
{
	ALOG(TX_LOG_INFO, TAG, "filepath = %s, rtsppath = %s\n", (char *)env->GetStringUTFChars(filepath, NULL), (char *)env->GetStringUTFChars(rtsppath, NULL));
	int ret = recordRTSPStream((char *)env->GetStringUTFChars(filepath, NULL), (char *)env->GetStringUTFChars(rtsppath, NULL));
	ALOG(TX_LOG_INFO, TAG, "recordRTSPStream ret = %d\n" , ret);
		return ret;
}

JNIEXPORT void JNICALL _stopRecordRTSPStream(JNIEnv *env, jclass clazz)
{
    ALOG(TX_LOG_INFO, TAG, "stopRecordRTSPStream\n");
    stopRecordingRtspStream();
}

/* 虚拟机 */
static JNINativeMethod gMethods[] = {

    {"_startRecoderRTSPStream", "(Ljava/lang/String;Ljava/lang/String;)I", (void*)_startRecoderRTSPStream},
    {"_stopRecordRTSPStream", "()V", (void*)_stopRecordRTSPStream},

};

static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, JNIREG_CLASS, gMethods,
        sizeof(gMethods) / sizeof(gMethods[0])))
        return JNI_FALSE;

    return JNI_TRUE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
//    JNIEnv* env = NULL;
//    jint result = -1;
//
//    if (vm->GetEnv( (void**) &env, JNI_VERSION_1_1) != JNI_OK) {
//        __android_log_print(ANDROID_LOG_ERROR, "tag", "load library error 1");
//        return JNI_ERR;
//    }
//    assert(env != NULL);
//
//    if (!registerNatives(env)) {
//        ALOG(ANDROID_LOG_ERROR, "tag", "load library error 2");
//        return JNI_ERR;
//    }
//    result = JNI_VERSION_1_1;
//    ALOG(ANDROID_LOG_ERROR, "tag", "load library success: %d", result);
//    return result;
	JNIEnv* env = NULL;
	jint result = -1;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {//从VM中取得JNIEnv的指针
		ALOG(ANDROID_LOG_ERROR, "tag","ERROR: GetEnv failed\n");
		return JNI_ERR;
	}
	assert(env != NULL);
	if (!registerNatives(env)) {//调用动态注册的方法
		ALOG(ANDROID_LOG_ERROR, "tag","ERROR: MediaPlayer native registration failed\n");
		return JNI_ERR;
	}
	 result = JNI_VERSION_1_4;
	 ALOG(ANDROID_LOG_ERROR, "tag", "load library success: %d", result);
   return result;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    ALOG(ANDROID_LOG_ERROR, "tag", "library was unload");
}
