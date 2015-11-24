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

import android.os.Bundle;
import android.preference.PreferenceFragment;

public class SpeeDroidSettingsFragment extends PreferenceFragment {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preferences);
    }
    
}
