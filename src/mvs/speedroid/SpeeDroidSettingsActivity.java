package mvs.speedroid;

import android.app.Activity;
import android.os.Bundle;

public class SpeeDroidSettingsActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Display the fragment as the main content.
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SpeeDroidSettingsFragment())
                .commit();
    }
	
}
