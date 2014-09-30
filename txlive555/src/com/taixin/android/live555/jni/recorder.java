package com.taixin.android.live555.jni;

public class recorder implements SimpleRecorder{
	@Override
	public int startRecordRTSP(String filePath, String rtspPath) {
		return _startRecoderRTSPStream(filePath, rtspPath);
	}
	
	@Override
	public void stopRecordRTSP() {
		_stopRecordRTSPStream();
	}
	
	public native int _startRecoderRTSPStream(String filePath, String rtspPath);
	public native void _stopRecordRTSPStream();
	static{
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("txlive555");
		System.loadLibrary("txlive555-jni");
	}

}
