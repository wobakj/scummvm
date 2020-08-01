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

#include "backends/platform/android/events.h"

#include "backends/platform/android/graphics.h"
#include "backends/platform/android/jni-android.h"
#include "backends/platform/android/system.h"

#include <unistd.h>
#include <errno.h>

#define LOOPER_IDENT_INPUT 1
#define LOOPER_IDENT_EVENT 2

AndroidEventSource::AndroidEventSource(OSystem_Android *system)
	: _system(system)
	, _queue(nullptr)
 {
	ENTER();

	_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

	int msgpipe[2];
	if (pipe(msgpipe)) {
		LOGE("could not create pipe: %s", strerror(errno));
		abort();
		}
	_msgR = msgpipe[0];
	_msgW = msgpipe[1];
	ALooper_addFd(_looper, _msgR, LOOPER_IDENT_EVENT, ALOOPER_EVENT_INPUT, nullptr, nullptr);
	}

AndroidEventSource::~AndroidEventSource() {
	ENTER();

	ALooper_removeFd(_looper, _msgR);
	_looper = nullptr;

	if (_queue != nullptr){
		AInputQueue_detachLooper(_queue);
		_queue = nullptr;
	}

	close(_msgR);
	close(_msgW);
}

void AndroidEventSource::pushEvent(Common::Event &event) {
	ENTER();

	write(_msgW, &event, sizeof(Common::Event));
}

bool AndroidEventSource::pollEvent(Common::Event &event) {
	if (_queue == nullptr) {
		return false;
	}

	int fd;
	int eventSource;
	void *data;

	int res = ALooper_pollOnce(0, &fd, &eventSource, &data);
	if (res == ALOOPER_POLL_TIMEOUT) {
		return false;
	}

	if (res == ALOOPER_POLL_ERROR) {
		LOGE("ALooper_pollOnce error");
		return false;
	}

	if (eventSource == LOOPER_IDENT_EVENT) {
		read(fd, &event, sizeof(event));
	return true;
	} else if (eventSource == LOOPER_IDENT_INPUT) {
		AInputEvent *e;
		if (AInputQueue_getEvent(_queue, &e) >= 0) {
			int eventType = AInputEvent_getType(e);
			bool result = false;
			// TODO process various types of events
			AInputQueue_finishEvent(_queue, e, 1);
			return result;
		} else {
			LOGW("Input event received but not?!");
		}
	}
	return false;
}

void AndroidEventSource::inputQueueCreated(AInputQueue* queue) {
	ENTER();

	_queue = queue;
	AInputQueue_attachLooper(queue, _looper, LOOPER_IDENT_INPUT, nullptr, nullptr);
}

void AndroidEventSource::inputQueueDestroyed(AInputQueue* queue) {
	ENTER();

	if (_queue == queue) {
		AInputQueue_detachLooper(queue);
		_queue = nullptr;
	}
}