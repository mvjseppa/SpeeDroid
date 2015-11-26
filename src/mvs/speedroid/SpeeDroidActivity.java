/* 
 * SpeeDroid
 * Speed sign detector for Android
 * 
 * Copyright (c) 2015, Mikko Seppälä
 * All rights reserved.
 * 
 * See LICENSE.md file for licensing details.
 * 
 */

package mvs.speedroid;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;

import mvs.speedroid.R;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.support.v7.app.AppCompatActivity;


public class SpeeDroidActivity extends AppCompatActivity implements CvCameraViewListener2 {
    private static final String    TAG = "mvs.speedroid";

    private Mat                    lastFrame;

    private CameraBridgeViewBase   mOpenCvCameraView;
    private SharedPreferences	   speeDroidPrefs;

    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("speedroid");

                    InitJniPart();
                    
                    mOpenCvCameraView.enableFpsMeter();
                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    public SpeeDroidActivity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.speedroid_main_view);

        speeDroidPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        
        
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.speedroid_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
        DestroyJniPart();
    }

    public void onCameraViewStarted(int width, int height) {
        lastFrame = new Mat(height, width, CvType.CV_8UC4);
    }

    public void onCameraViewStopped() {
        lastFrame.release();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        
        // input frame has RGBA format
        lastFrame = inputFrame.rgba();
        Core.flip(lastFrame, lastFrame, -1);
        
        int roiWidth = speeDroidPrefs.getInt("roiWidth", 0);
        int roiHeight = speeDroidPrefs.getInt("roiHeight", 0);
        
        Log.i(TAG, "ROI size in prefs: " + Integer.toString(roiWidth) + "x" + Integer.toString(roiHeight));
        
        ProcessImage(lastFrame.getNativeObjAddr(), roiWidth, roiHeight);

        return lastFrame;
    }
    
    
    public void settingsButtonClicked(View view){
    	Log.i(TAG, "Settings button clicked!");
    	
    	Intent i = new Intent(this, SpeeDroidSettingsActivity.class);
    	startActivity(i);
    	
    }
    
    public native void ProcessImage(long matAddrRgba, int roiWidth, int roiHeight);
    public native void InitJniPart();
    public native void DestroyJniPart();
}
