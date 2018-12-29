package com.sohu.tv.bee.util;

import android.hardware.Camera;
import android.util.Log;

public class CameraPermissionCheckUtils {
    private static final String TAG = "CameraPermissionCheckUt";

    public static boolean checkCameraPermission() {
        boolean canUse = true;
        Camera mCamera = null;
        try {
            mCamera = Camera.open(0);
            mCamera.setDisplayOrientation(90);
        } catch (Exception e) {
            Log.e(TAG, Log.getStackTraceString(e));
            canUse = false;
        }
        if (mCamera != null) {
            mCamera.release();
        }
        return canUse;
    }
}
