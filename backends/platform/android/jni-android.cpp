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

#include "backends/platform/android/jni-android.h"

#include "backends/platform/android/asset-archive.h"
#include "backends/platform/android/events.h"
#include "backends/platform/android/system.h"

#include "base/main.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/encoding.h"
#include "engines/engine.h"


const JNINativeMethod JNI::_natives[] = {
	{ "pushEvent", "(II)V",
		(void *)JNI::pushEvent },
	{ "setPause", "(Z)V",
		(void *)JNI::setPause }
};

JNI::JNI(OSystem_Android *system, ANativeActivity *activity):
	_pause(false),
	_system(system),
	_activity(activity),
	_instance(_activity->clazz),
	_assets(nullptr),
	_jobj_audio_track(nullptr),
	_MID_getDPI(nullptr),
	_MID_displayMessageOnOSD(nullptr),
	_MID_openUrl(nullptr),
	_MID_hasTextInClipboard(nullptr),
	_MID_getTextFromClipboard(nullptr),
	_MID_setTextInClipboard(nullptr),
	_MID_isConnectionLimited(nullptr),
	_MID_setWindowCaption(nullptr),
	_MID_showVirtualKeyboard(nullptr),
	_MID_showKeyboardControl(nullptr),
	_MID_convertEncoding(nullptr),
	_MID_getAllStorageLocations(nullptr),
	_MID_finish(nullptr),
	_MID_stringFromKeyCode(nullptr),
	_MID_AudioTrack_flush(nullptr),
	_MID_AudioTrack_pause(nullptr),
	_MID_AudioTrack_play(nullptr),
	_MID_AudioTrack_stop(nullptr),
	_MID_AudioTrack_write(nullptr)
 {
	// should be called from main thread

	// initial value of zero!
	sem_init(&_pause_sem, 0, 0);

	JNIEnv *env = _activity->env;

	jclass clazz = env->GetObjectClass(_instance);

	env->RegisterNatives(clazz, _natives, ARRAYSIZE(_natives));

#define FIND_METHOD(prefix, name, signature) do {							\
		_MID_ ## prefix ## name = env->GetMethodID(clazz, #name, signature);	\
		if (_MID_ ## prefix ## name == 0)									\
			return;															\
	} while (0)

	FIND_METHOD(, setWindowCaption, "(Ljava/lang/String;)V");
	FIND_METHOD(, getDPI, "([F)V");
	FIND_METHOD(, displayMessageOnOSD, "(Ljava/lang/String;)V");
	FIND_METHOD(, openUrl, "(Ljava/lang/String;)V");
	FIND_METHOD(, hasTextInClipboard, "()Z");
	FIND_METHOD(, getTextFromClipboard, "()Ljava/lang/String;");
	FIND_METHOD(, setTextInClipboard, "(Ljava/lang/String;)Z");
	FIND_METHOD(, isConnectionLimited, "()Z");
	FIND_METHOD(, showVirtualKeyboard, "(Z)V");
	FIND_METHOD(, showKeyboardControl, "(Z)V");
	FIND_METHOD(, getAllStorageLocations, "()[Ljava/lang/String;");
	FIND_METHOD(, convertEncoding, "(Ljava/lang/String;Ljava/lang/String;[B)[B");
	FIND_METHOD(, finish, "()V");
	FIND_METHOD(, stringFromKeyCode, "(JJIIIIIIII)Ljava/lang/String;");
	FIND_METHOD(, getAssets, "()Landroid/content/res/AssetManager;");

	// _jobj_audio_track = env->NewGlobalRef(at);

	// clazz = env->GetObjectClass(_jobj_audio_track);

	// FIND_METHOD(AudioTrack_, flush, "()V");
	// FIND_METHOD(AudioTrack_, pause, "()V");
	// FIND_METHOD(AudioTrack_, play, "()V");
	// FIND_METHOD(AudioTrack_, stop, "()V");
	// FIND_METHOD(AudioTrack_, write, "([BII)I");

#undef FIND_METHOD
}

JNI::~JNI() {
	// should be called from main thread
	if (_assets != nullptr)	{
		JNIEnv *env = _activity->env;
		env->DeleteGlobalRef(_assets);
	}
}

JNIEnv *JNI::getEnv() {
	JNIEnv *env = nullptr;
	jint res = _activity->vm->GetEnv((void **)&env, JNI_VERSION_1_6);

	if (res != JNI_OK) {
		LOGE("GetEnv() failed: %d", res);
		abort();
	}

	return env;
}

void JNI::attachThread(const char* threadName) {
	JavaVMAttachArgs args;
	args.version = JNI_VERSION_1_6;
	args.name = threadName;
	args.group = nullptr;

	JNIEnv *env = nullptr;
	_activity->vm->AttachCurrentThread(&env, &args);
}

void JNI::detachThread() {
	_activity->vm->DetachCurrentThread();
}

void JNI::throwByName(JNIEnv *env, const char *name, const char *msg) {
	jclass cls = env->FindClass(name);

	// if cls is 0, an exception has already been thrown
	if (cls != 0)
		env->ThrowNew(cls, msg);

	env->DeleteLocalRef(cls);
}

void JNI::throwRuntimeException(JNIEnv *env, const char *msg) {
	throwByName(env, "java/lang/RuntimeException", msg);
}

// calls to the dark side

void JNI::getDPI(float *values) {
	values[0] = 0.0;
	values[1] = 0.0;

	JNIEnv *env = getEnv();

	jfloatArray array = env->NewFloatArray(2);

	env->CallVoidMethod(_instance, _MID_getDPI, array);

	if (env->ExceptionCheck()) {
		LOGE("Failed to get DPIs");

		env->ExceptionDescribe();
		env->ExceptionClear();
	} else {
		jfloat *res = env->GetFloatArrayElements(array, 0);

		if (res) {
			values[0] = res[0];
			values[1] = res[1];

			env->ReleaseFloatArrayElements(array, res, 0);
		}
	}

	env->DeleteLocalRef(array);
}

void JNI::displayMessageOnOSD(const Common::U32String &msg) {
	// called from common/osd_message_queue, method: OSDMessageQueue::pollEvent()
	JNIEnv *env = getEnv();

	jstring java_msg = convertToJString(env, msg.encode(), "UTF-8");
	if (java_msg == nullptr) {
		// Show a placeholder indicative of the translation error instead of silent failing
		java_msg = env->NewStringUTF("?");
		LOGE("Failed to convert message to UTF-8 for OSD!");
	}

	env->CallVoidMethod(_instance, _MID_displayMessageOnOSD, java_msg);

	if (env->ExceptionCheck()) {
		LOGE("Failed to display OSD message");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(java_msg);
}

bool JNI::openUrl(const Common::String &url) {
	bool success = true;
	JNIEnv *env = getEnv();
	jstring javaUrl = env->NewStringUTF(url.c_str());

	env->CallVoidMethod(_instance, _MID_openUrl, javaUrl);

	if (env->ExceptionCheck()) {
		LOGE("Failed to open URL");

		env->ExceptionDescribe();
		env->ExceptionClear();
		success = false;
	}

	env->DeleteLocalRef(javaUrl);
	return success;
}

bool JNI::hasTextInClipboard() {
	JNIEnv *env = getEnv();
	bool hasText = env->CallBooleanMethod(_instance, _MID_hasTextInClipboard);

	if (env->ExceptionCheck()) {
		LOGE("Failed to check the contents of the clipboard");

		env->ExceptionDescribe();
		env->ExceptionClear();
		hasText = true;
	}

	return hasText;
}

Common::U32String JNI::getTextFromClipboard() {
	JNIEnv *env = getEnv();

	jstring javaText = (jstring)env->CallObjectMethod(_instance, _MID_getTextFromClipboard);

	if (env->ExceptionCheck()) {
		LOGE("Failed to retrieve text from the clipboard");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return Common::U32String();
	}

	Common::String text = convertFromJString(env, javaText, "UTF-8");
	env->DeleteLocalRef(javaText);

	return text.decode();
}

bool JNI::setTextInClipboard(const Common::U32String &text) {
	JNIEnv *env = getEnv();
	jstring javaText = convertToJString(env, text.encode(), "UTF-8");

	bool success = env->CallBooleanMethod(_instance, _MID_setTextInClipboard, javaText);

	if (env->ExceptionCheck()) {
		LOGE("Failed to add text to the clipboard");

		env->ExceptionDescribe();
		env->ExceptionClear();
		success = false;
	}

	env->DeleteLocalRef(javaText);
	return success;
}

bool JNI::isConnectionLimited() {
	JNIEnv *env = getEnv();
	bool limited = env->CallBooleanMethod(_instance, _MID_isConnectionLimited);

	if (env->ExceptionCheck()) {
		LOGE("Failed to check whether connection's limited");

		env->ExceptionDescribe();
		env->ExceptionClear();
		limited = true;
	}

	return limited;
}

void JNI::setWindowCaption(const Common::String &caption) {
	JNIEnv *env = getEnv();
	jstring java_caption = convertToJString(env, caption, "ISO-8859-1");

	env->CallVoidMethod(_instance, _MID_setWindowCaption, java_caption);

	if (env->ExceptionCheck()) {
		LOGE("Failed to set window caption");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(java_caption);
}

void JNI::showVirtualKeyboard(bool enable) {
	JNIEnv *env = getEnv();

	env->CallVoidMethod(_instance, _MID_showVirtualKeyboard, enable);

	if (env->ExceptionCheck()) {
		LOGE("Error trying to show virtual keyboard");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::showKeyboardControl(bool enable) {
	JNIEnv *env = getEnv();

	env->CallVoidMethod(_instance, _MID_showKeyboardControl, enable);

	if (env->ExceptionCheck()) {
		LOGE("Error trying to show virtual keyboard control");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

char *JNI::convertEncoding(const char *to, const char *from, const char *string, size_t length) {
	JNIEnv *env = getEnv();

	jstring javaTo = env->NewStringUTF(to);
	jstring javaFrom = env->NewStringUTF(from);
	jbyteArray javaString = env->NewByteArray(length);
	env->SetByteArrayRegion(javaString, 0, length, reinterpret_cast<const jbyte*>(string));

	jbyteArray javaOut = (jbyteArray)env->CallObjectMethod(_instance, _MID_convertEncoding, javaTo, javaFrom, javaString);

	if (!javaOut || env->ExceptionCheck()) {
		LOGE("Failed to convert text from %s to %s", from, to);

		env->ExceptionDescribe();
		env->ExceptionClear();

		return nullptr;
	}

	int outLength = env->GetArrayLength(javaOut);
	char *buf = (char *)malloc(outLength + 1);
	if (!buf)
		return nullptr;

	env->GetByteArrayRegion(javaOut, 0, outLength, reinterpret_cast<jbyte *>(buf));
	buf[outLength] = 0;

	return buf;
}

void JNI::setAudioPause() {
	JNIEnv *env = getEnv();

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_flush);

	if (env->ExceptionCheck()) {
		LOGE("Error flushing AudioTrack");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_pause);

	if (env->ExceptionCheck()) {
		LOGE("Error setting AudioTrack: pause");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::setAudioPlay() {
	JNIEnv *env = getEnv();

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_play);

	if (env->ExceptionCheck()) {
		LOGE("Error setting AudioTrack: play");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::setAudioStop() {
	JNIEnv *env = getEnv();

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_stop);

	if (env->ExceptionCheck()) {
		LOGE("Error setting AudioTrack: stop");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

// natives for the dark side

void JNI::pushEvent(int type, int customType) {
	OSystem_Android *system = dynamic_cast<OSystem_Android *>(g_system);
	assert(system);

	Common::Event event;
	event.type = (Common::EventType)type;
	event.customType = (Common::CustomEventType)customType;
	system->_eventSource->pushEvent(event);
}

void JNI::setPause(JNIEnv *env, jobject self, jboolean value) {
	OSystem_Android *system = dynamic_cast<OSystem_Android *>(g_system);
	assert(system);

	if (g_engine) {
		LOGD("pauseEngine: %d", value);

		if (value)
			system->_jni->_pauseToken = g_engine->pauseEngine();
		else
			system->_jni->_pauseToken.clear();

		/*if (value &&
				g_engine->hasFeature(Engine::kSupportsSavingDuringRuntime) &&
				g_engine->canSaveGameStateCurrently())
			g_engine->saveGameState(0, "Android parachute");*/
	}

	system->_jni->_pause = value;

	if (!system->_jni->_pause) {
		// wake up all threads
		for (uint i = 0; i < 3; ++i)
			sem_post(&system->_jni->_pause_sem);
	}
}

jstring JNI::convertToJString(JNIEnv *env, const Common::String &str, const Common::String &from) {
	Common::Encoding converter("UTF-8", from.c_str());
	char *utf8Str = converter.convert(str.c_str(), converter.stringLength(str.c_str(), from));
	if (utf8Str == nullptr)
		return nullptr;

	jstring jstr = env->NewStringUTF(utf8Str);
	free(utf8Str);

	return jstr;
}

Common::String JNI::convertFromJString(JNIEnv *env, const jstring &jstr, const Common::String &to) {
	const char *utf8Str = env->GetStringUTFChars(jstr, 0);
	if (!utf8Str)
		return Common::String();

	Common::Encoding converter(to.c_str(), "UTF-8");
	char *asciiStr = converter.convert(utf8Str, env->GetStringUTFLength(jstr));
	env->ReleaseStringUTFChars(jstr, utf8Str);

	Common::String str(asciiStr);
	free(asciiStr);

	return str;
}

Common::Array<Common::String> JNI::getAllStorageLocations() {
	Common::Array<Common::String> *res = new Common::Array<Common::String>();

	JNIEnv *env = getEnv();

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_instance, _MID_getAllStorageLocations);

	if (env->ExceptionCheck()) {
		LOGE("Error finding system archive path");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return *res;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);
		const char *path = env->GetStringUTFChars(path_obj, 0);

		if (path != 0) {
			res->push_back(path);
			env->ReleaseStringUTFChars(path_obj, path);
		}

		env->DeleteLocalRef(path_obj);
	}

	return *res;
}

void JNI::finish() {
	JNIEnv *env = getEnv();
	env->CallVoidMethod(_instance, _MID_finish);
}

Common::String JNI::stringFromKeyCode(AInputEvent* event) {
    JNIEnv *env = JNI::getEnv();

	jstring jKeyCodeString = (jstring)env->CallObjectMethod(_instance, _MID_stringFromKeyCode, AKeyEvent_getDownTime(event), AKeyEvent_getEventTime(event), AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event), AKeyEvent_getRepeatCount(event), AKeyEvent_getMetaState(event), AInputEvent_getDeviceId(event), AKeyEvent_getScanCode(event), AKeyEvent_getFlags(event), AInputEvent_getSource(event));

	if (jKeyCodeString == nullptr) {
		return Common::String();
	}

	const char* keyCodeString = env->GetStringUTFChars(jKeyCodeString, nullptr);
	Common::String strReturn (keyCodeString);
	env->ReleaseStringUTFChars(jKeyCodeString, keyCodeString);

	return strReturn;
}

jobject JNI::getAssets() {
	if (_assets == nullptr) {
		JNIEnv *env = getEnv();
		jobject assets = env->CallObjectMethod(_instance, _MID_getAssets);
		_assets = env->NewGlobalRef(assets);
	}
	return _assets;
}
