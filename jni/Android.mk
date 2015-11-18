LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include D:\NVPACK\OpenCV-2.4.8.2-Tegra-sdk\sdk\native\jni\OpenCV-tegra3.mk

LOCAL_MODULE    := speedroid
LOCAL_SRC_FILES := jni_part.cpp SimpleTimer.cpp CircleRANSAC.cpp
LOCAL_LDLIBS +=  -llog -ldl

include $(BUILD_SHARED_LIBRARY)
