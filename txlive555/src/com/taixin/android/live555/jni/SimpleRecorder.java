package com.taixin.android.live555.jni;

public interface SimpleRecorder {
	
	public int startRecordRTSP(String filePath, String rtspPath);
	
	public void stopRecordRTSP();
}
