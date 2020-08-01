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

#ifndef PLATFORM_ANDROID_SYSTEM_H
#define PLATFORM_ANDROID_SYSTEM_H

#include "backends/platform/android/common.h"

#include "audio/mixer_intern.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/modular-backend.h"
#include "backends/platform/android/portdefs.h"
#include "backends/plugins/posix/posix-provider.h"
#include "common/archive.h"
#include "common/fs.h"

#include <pthread.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>

class AndroidGraphicsManager;
class AndroidEventSource;
class JNI;

class OSystem_Android : public ModularMutexBackend, public ModularGraphicsBackend {
public:
	JNI* _jni;
	ANativeActivity* _nativeActivity;
	AndroidEventSource *_eventSource;

private:
	ANativeWindow *_waitingNativeWindow;
	AInputQueue *_waitingInputQueue;

	// passed from the dark side
	int _audio_sample_rate;
	int _audio_buffer_size;

	bool _mainThreadRunning;
	pthread_t _mainThread;
	static void *mainThreadFunc(void *arg);

	bool _timer_thread_exit;
	pthread_t _timer_thread;
	static void *timerThreadFunc(void *arg);

	bool _audio_thread_exit;
	pthread_t _audio_thread;
	static void *audioThreadFunc(void *arg);

	bool _virtkeybd_on;

	Audio::MixerImpl *_mixer;
	timeval _startTime;

	Common::String getSystemProperty(const char *name) const;

public:
	OSystem_Android(ANativeActivity* nativeActivity, int audio_sample_rate, int audio_buffer_size);
	virtual ~OSystem_Android();

	virtual void initBackend() override;

	virtual bool hasFeature(OSystem::Feature f) override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) override;

private:
	Common::Point _touch_pt_down, _touch_pt_scroll, _touch_pt_dt;
	int _eventScaleX;
	int _eventScaleY;
	bool _touchpad_mode;
	int _touchpad_scale;
	int _trackball_scale;
	int _dpad_scale;
	int _joystick_scale;
	int _fingersDown;

	void pushEvent(const Common::Event &event);

public:
	virtual Common::KeymapperDefaultBindings *getKeymapperDefaultBindings() override;

	virtual uint32 getMillis(bool skipRecord = false) override;
	virtual void delayMillis(uint msecs) override;

	virtual void quit() override;

	virtual void setWindowCaption(const char *caption) override;

	virtual Audio::Mixer *getMixer() override;
	virtual void getTimeAndDate(TimeDate &t) const override;
	virtual void logMessage(LogMessageType::Type type, const char *message) override;
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	virtual bool openUrl(const Common::String &url) override;
	virtual bool hasTextInClipboard() override;
	virtual Common::U32String getTextFromClipboard() override;
	virtual bool setTextInClipboard(const Common::U32String &text) override;
	virtual bool isConnectionLimited() override;
	virtual Common::String getSystemLanguage() const override;
	virtual char *convertEncoding(const char *to, const char *from, const char *string, size_t length) override;

	AndroidGraphicsManager *getAndroidGraphicsManager() const;
	void nativeWindowCreated(ANativeWindow* window);
	void nativeWindowDestroyed(ANativeWindow* window);
	void inputQueueCreated(AInputQueue* queue);
	void inputQueueDestroyed(AInputQueue* queue);
};

#endif
