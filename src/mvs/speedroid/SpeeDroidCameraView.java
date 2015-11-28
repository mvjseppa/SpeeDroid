package mvs.speedroid;

import java.util.List;

import org.opencv.android.JavaCameraView;

import android.content.Context;
import android.hardware.Camera;
import android.util.AttributeSet;

public class SpeeDroidCameraView extends JavaCameraView {

	public SpeeDroidCameraView(Context context, int cameraId) {
		super(context, cameraId);
	}

	public SpeeDroidCameraView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public void setCameraParams() {
		Camera.Parameters params = mCamera.getParameters();

		List<String> FocusModes = params.getSupportedFocusModes();
		if (FocusModes != null
				&& FocusModes.contains(Camera.Parameters.FOCUS_MODE_INFINITY)) {
			params.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY);
		}

		params.setExposureCompensation(params.getMinExposureCompensation());

		mCamera.setParameters(params);

	}

}
