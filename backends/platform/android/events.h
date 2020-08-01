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

#ifndef PLATFORM_ANDROID_EVENTS_H
#define PLATFORM_ANDROID_EVENTS_H

#include "backends/platform/android/common.h"

#include "common/events.h"

#include <android/looper.h>
#include <android/input.h>

class OSystem_Android;

class AndroidEventSource : public Common::EventSource
{
private:
	OSystem_Android *_system;
	ALooper *_looper;
	AInputQueue* _queue;
	int _msgR;
	int _msgW;

public:
	AndroidEventSource(OSystem_Android *system);
	virtual ~AndroidEventSource() override;

	void pushEvent(Common::Event &event);
	virtual bool pollEvent(Common::Event &event) override;

	void inputQueueCreated(AInputQueue* queue);
	void inputQueueDestroyed(AInputQueue* queue);
};

#endif
