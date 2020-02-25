/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PLATFORM_ANDROID_JNI_H
#define PLATFORM_ANDROID_JNI_H

#include "backends/platform/android/common.h"

#include "common/fs.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/ustr.h"
#include "engines/engine.h"

#include <jni.h>
#include <semaphore.h>
#include <android/native_activity.h>

class OSystem_Android;

class JNI {
public:
	JNI(OSystem_Android *system, ANativeActivity *activity);
	virtual ~JNI();

	bool _pause;
	sem_t _pause_sem;

	jint onLoad(JavaVM *vm);

	JNIEnv *getEnv();

	void attachThread(const char* threadName);
	void detachThread();

	void setWindowCaption(const Common::String &caption);
	void getDPI(float *values);
	void displayMessageOnOSD(const Common::U32String &msg);
	bool openUrl(const Common::String &url);
	bool hasTextInClipboard();
	Common::U32String getTextFromClipboard();
	bool setTextInClipboard(const Common::U32String &text);
	bool isConnectionLimited();
	void showVirtualKeyboard(bool enable);
	void showKeyboardControl(bool enable);
	char *convertEncoding(const char *to, const char *from, const char *string, size_t length);

	void setAudioPause();
	void setAudioPlay();
	void setAudioStop();

	inline int writeAudio(JNIEnv *env, jbyteArray &data, int offset,
									int size);

	Common::Array<Common::String> getAllStorageLocations();
	void finish();
	Common::String stringFromKeyCode(AInputEvent* pInputEvent);
	jobject getAssets();

private:
	OSystem_Android *_system;
	ANativeActivity *_activity;

	// back pointer to (java) peer instance
	jobject _instance;
	jobject _assets;

	jobject _jobj_audio_track;

	jmethodID _MID_getDPI;
	jmethodID _MID_displayMessageOnOSD;
	jmethodID _MID_openUrl;
	jmethodID _MID_hasTextInClipboard;
	jmethodID _MID_getTextFromClipboard;
	jmethodID _MID_setTextInClipboard;
	jmethodID _MID_isConnectionLimited;
	jmethodID _MID_setWindowCaption;
	jmethodID _MID_showVirtualKeyboard;
	jmethodID _MID_showKeyboardControl;
	jmethodID _MID_convertEncoding;
	jmethodID _MID_getAllStorageLocations;
	jmethodID _MID_finish;
	jmethodID _MID_stringFromKeyCode;
	jmethodID _MID_getAssets;

	jmethodID _MID_AudioTrack_flush;
	jmethodID _MID_AudioTrack_pause;
	jmethodID _MID_AudioTrack_play;
	jmethodID _MID_AudioTrack_stop;
	jmethodID _MID_AudioTrack_write;


	void throwByName(JNIEnv *env, const char *name, const char *msg);
	void throwRuntimeException(JNIEnv *env, const char *msg);

	// natives for the dark side

	static const JNINativeMethod _natives[];

	static void pushEvent(int type, int customType);
	static void setPause(JNIEnv *env, jobject self, jboolean value);

	jstring convertToJString(JNIEnv *env, const Common::String &str, const Common::String &from);
	Common::String convertFromJString(JNIEnv *env, const jstring &jstr, const Common::String &to);

	PauseToken _pauseToken;
};

inline int JNI::writeAudio(JNIEnv *env, jbyteArray &data, int offset, int size) {
	return env->CallIntMethod(_jobj_audio_track, _MID_AudioTrack_write, data,
								offset, size);
}

#endif
