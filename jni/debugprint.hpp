/*
 * degugprint.hpp
 *
 *  Created on: 10 Sep 2015
 *      Author: Mikko
 */

#include <jni.h>
#include <android/log.h>

#ifndef DEGUGPRINT_HPP_
#define DEGUGPRINT_HPP_


#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"mvs.speedroid",__VA_ARGS__)


#endif /* DEGUGPRINT_HPP_ */
