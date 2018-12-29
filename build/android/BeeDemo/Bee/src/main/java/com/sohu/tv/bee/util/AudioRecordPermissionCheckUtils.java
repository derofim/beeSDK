package com.sohu.tv.bee.util;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

public class AudioRecordPermissionCheckUtils {
    private static final String TAG = "RecordPermissionCheckUt";
    public static int audioSource = MediaRecorder.AudioSource.MIC;
    public static int sampleRateInHz = 44100;
    public static int channelConfig = AudioFormat.CHANNEL_IN_STEREO;
    public static int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
    public static int bufferSizeInBytes = 0;

    public static boolean checkAudioRecordPermission() {
        bufferSizeInBytes = AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
        AudioRecord audioRecord = null;
        try {
            audioRecord = new AudioRecord(audioSource, sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        try{
            audioRecord.startRecording();
        } catch (IllegalStateException e){
            e.printStackTrace();
            Log.e(TAG, Log.getStackTraceString(e));
        }
        audioRecord.stop();
        audioRecord.release();
        return true;
    }
}
