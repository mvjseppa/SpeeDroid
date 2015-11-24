# SpeeDroid
Android speed sign detector based on OpenCV.

This app uses OpenCV to turn the camera of an Android phone to a real time speed limit sign detector.

Use at your own risk, author takes absolutely no responsibility. See LICENSE.md for licensing details.

Currently supported features are:
-Identifying speed signs in the image
-Displaying 3 latest results on screen
-Ability to set region of interest to exclude areas where signs cannot be detected

The detection is based on color filtering and a RANSAC circle detection algorithm.
Due to limitations in mobile phone cameras the detection works best in good lightning conditions and on low driving speeds.

Possible future features and improvements:
-Optimize the detection algorithm to improve framerate
-Improve camera focus and exposure settings
-Sign classification based on a feature detection algorithm




