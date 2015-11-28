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
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;

import mvs.speedroid.R;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;

public class SpeeDroidActivity extends Activity implements CvCameraViewListener2 {
	
	private static final String TAG = "mvs.speedroid";
	private Mat mLastFrame;
	private SpeeDroidCameraView mOpenCvCameraView;
	private SharedPreferences mSpeeDroidPrefs;

	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				Log.i(TAG, "OpenCV loaded successfully");

				// Load native library after(!) OpenCV initialization
				System.loadLibrary("speedroid");

				InitJniPart();

				mOpenCvCameraView.enableFpsMeter();
				mOpenCvCameraView.enableView();
				mOpenCvCameraView.setCameraParams();
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
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

		mSpeeDroidPrefs = PreferenceManager.getDefaultSharedPreferences(this);

		mOpenCvCameraView = (SpeeDroidCameraView) findViewById(R.id.speedroid_surface_view);
		mOpenCvCameraView.setCvCameraViewListener(this);
	}

	@Override
	public void onPause() {
		super.onPause();
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
		DestroyJniPart();
	}

	@Override
	public void onResume() {
		super.onResume();
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this,
				mLoaderCallback);
	}

	public void onDestroy() {
		super.onDestroy();
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
		DestroyJniPart();
	}

	public void onCameraViewStarted(int width, int height) {
		mLastFrame = new Mat(height, width, CvType.CV_8UC4);
	}

	public void onCameraViewStopped() {
		mLastFrame.release();
	}

	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {

		// input frame has RGBA format
		mLastFrame = inputFrame.rgba();
		Core.flip(mLastFrame, mLastFrame, -1);

		int roiWidth = mSpeeDroidPrefs.getInt("roiWidth", 0);
		int roiHeight = mSpeeDroidPrefs.getInt("roiHeight", 0);

		// Log.i(TAG, "ROI size in prefs: " + Integer.toString(roiWidth) + "x" +
		// Integer.toString(roiHeight));

		ProcessImage(mLastFrame.getNativeObjAddr(), roiWidth, roiHeight);

		return mLastFrame;
	}

	@Override
	public boolean onKeyDown(int keycode, KeyEvent e) {
		switch (keycode) {
		case KeyEvent.KEYCODE_MENU:
			// Hardware menu button calls the onClick for on-screen settings
			// button.
			findViewById(R.id.imageButtonSettings).callOnClick();
			return true;
		}

		return super.onKeyDown(keycode, e);
	}

	public void settingsButtonClicked(View view) {
		Log.i(TAG, "Settings button clicked!");

		Intent i = new Intent(this, SpeeDroidSettingsActivity.class);
		startActivity(i);

	}

	public native void ProcessImage(long matAddrRgba, int roiWidth, int roiHeight);

	public native void InitJniPart();

	public native void DestroyJniPart();
}
