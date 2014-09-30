package com.taixin.android.live555;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.taixin.android.live555.jni.SimpleRecorder;
import com.taixin.android.live555.jni.recorder;

public class MainActivity extends Activity {
	private Button btn ;
	private boolean isRecording = false;
	String rtsp = "rtsp://admin:12345@192.168.1.102:554/Streaming/Channels/1?transportmode=unicast&profile=Profile_1";
	SimpleRecorder recorder = new recorder();
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		initBTN();
	}
	
	public void initBTN(){
		btn = (Button)findViewById(R.id.button_first);
		btn.setText(R.string.start_record);
		btn.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
				if(!isRecording){
					isRecording = true;
					btn.setText(R.string.stop_record);
					recorder.startRecordRTSP("test.ts", rtsp);
				}
				else{
					isRecording = false;
					btn.setText(R.string.start_record);
					recorder.stopRecordRTSP();
				}	
			}
		});
	}
}
