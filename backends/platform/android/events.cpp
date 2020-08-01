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

static const Common::KeyCode keymap[] = {
	Common::KEYCODE_INVALID, // AKEYCODE_UNKNOWN
	Common::KEYCODE_LEFTSOFT, // AKEYCODE_SOFT_LEFT
	Common::KEYCODE_RIGHTSOFT, // AKEYCODE_SOFT_RIGHT
	Common::KEYCODE_AC_HOME, // AKEYCODE_HOME
	Common::KEYCODE_AC_BACK, // AKEYCODE_BACK
	Common::KEYCODE_CALL, // AKEYCODE_CALL
	Common::KEYCODE_HANGUP, // AKEYCODE_ENDCALL
	Common::KEYCODE_0, // AKEYCODE_0
	Common::KEYCODE_1, // AKEYCODE_1
	Common::KEYCODE_2, // AKEYCODE_2
	Common::KEYCODE_3, // AKEYCODE_3
	Common::KEYCODE_4, // AKEYCODE_4
	Common::KEYCODE_5, // AKEYCODE_5
	Common::KEYCODE_6, // AKEYCODE_6
	Common::KEYCODE_7, // AKEYCODE_7
	Common::KEYCODE_8, // AKEYCODE_8
	Common::KEYCODE_9, // AKEYCODE_9
	Common::KEYCODE_ASTERISK, // AKEYCODE_STAR
	Common::KEYCODE_HASH, // AKEYCODE_POUND
	Common::KEYCODE_UP, // AKEYCODE_DPAD_UP
	Common::KEYCODE_DOWN, // AKEYCODE_DPAD_DOWN
	Common::KEYCODE_LEFT, // AKEYCODE_DPAD_LEFT
	Common::KEYCODE_RIGHT, // AKEYCODE_DPAD_RIGHT
	Common::KEYCODE_SELECT, // AKEYCODE_DPAD_CENTER
	Common::KEYCODE_VOLUMEUP, // AKEYCODE_VOLUME_UP
	Common::KEYCODE_VOLUMEDOWN, // AKEYCODE_VOLUME_DOWN
	Common::KEYCODE_POWER, // AKEYCODE_POWER
	Common::KEYCODE_CAMERA, // AKEYCODE_CAMERA
	Common::KEYCODE_CLEAR, // AKEYCODE_CLEAR
	Common::KEYCODE_a, // AKEYCODE_A
	Common::KEYCODE_b, // AKEYCODE_B
	Common::KEYCODE_c, // AKEYCODE_C
	Common::KEYCODE_d, // AKEYCODE_D
	Common::KEYCODE_e, // AKEYCODE_E
	Common::KEYCODE_f, // AKEYCODE_F
	Common::KEYCODE_g, // AKEYCODE_G
	Common::KEYCODE_h, // AKEYCODE_H
	Common::KEYCODE_i, // AKEYCODE_I
	Common::KEYCODE_j, // AKEYCODE_J
	Common::KEYCODE_k, // AKEYCODE_K
	Common::KEYCODE_l, // AKEYCODE_L
	Common::KEYCODE_m, // AKEYCODE_M
	Common::KEYCODE_n, // AKEYCODE_N
	Common::KEYCODE_o, // AKEYCODE_O
	Common::KEYCODE_p, // AKEYCODE_P
	Common::KEYCODE_q, // AKEYCODE_Q
	Common::KEYCODE_r, // AKEYCODE_R
	Common::KEYCODE_s, // AKEYCODE_S
	Common::KEYCODE_t, // AKEYCODE_T
	Common::KEYCODE_u, // AKEYCODE_U
	Common::KEYCODE_v, // AKEYCODE_V
	Common::KEYCODE_w, // AKEYCODE_W
	Common::KEYCODE_x, // AKEYCODE_X
	Common::KEYCODE_y, // AKEYCODE_Y
	Common::KEYCODE_z, // AKEYCODE_Z
	Common::KEYCODE_COMMA, // AKEYCODE_COMMA
	Common::KEYCODE_PERIOD, // AKEYCODE_PERIOD
	Common::KEYCODE_LALT, // AKEYCODE_ALT_LEFT
	Common::KEYCODE_RALT, // AKEYCODE_ALT_RIGHT
	Common::KEYCODE_LSHIFT, // AKEYCODE_SHIFT_LEFT
	Common::KEYCODE_RSHIFT, // AKEYCODE_SHIFT_RIGHT
	Common::KEYCODE_TAB, // AKEYCODE_TAB
	Common::KEYCODE_SPACE, // AKEYCODE_SPACE
	Common::KEYCODE_LCTRL, // AKEYCODE_SYM
	Common::KEYCODE_WWW, // AKEYCODE_EXPLORER
	Common::KEYCODE_MAIL, // AKEYCODE_ENVELOPE
	Common::KEYCODE_RETURN, // AKEYCODE_ENTER
	Common::KEYCODE_BACKSPACE, // AKEYCODE_DEL
	Common::KEYCODE_BACKQUOTE, // AKEYCODE_GRAVE
	Common::KEYCODE_MINUS, // AKEYCODE_MINUS
	Common::KEYCODE_EQUALS, // AKEYCODE_EQUALS
	Common::KEYCODE_LEFTPAREN, // AKEYCODE_LEFT_BRACKET
	Common::KEYCODE_RIGHTPAREN, // AKEYCODE_RIGHT_BRACKET
	Common::KEYCODE_BACKSLASH, // AKEYCODE_BACKSLASH
	Common::KEYCODE_SEMICOLON, // AKEYCODE_SEMICOLON
	Common::KEYCODE_QUOTE, // AKEYCODE_APOSTROPHE
	Common::KEYCODE_SLASH, // AKEYCODE_SLASH
	Common::KEYCODE_AT, // AKEYCODE_AT
	Common::KEYCODE_INVALID, // AKEYCODE_NUM
	Common::KEYCODE_INVALID, // AKEYCODE_HEADSETHOOK
	Common::KEYCODE_INVALID, // AKEYCODE_FOCUS
	Common::KEYCODE_PLUS, // AKEYCODE_PLUS
	Common::KEYCODE_MENU, // AKEYCODE_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_NOTIFICATION
	Common::KEYCODE_AC_SEARCH, // AKEYCODE_SEARCH
	Common::KEYCODE_AUDIOPLAYPAUSE, // AKEYCODE_MEDIA_PLAY_PAUSE
	Common::KEYCODE_AUDIOSTOP, // AKEYCODE_MEDIA_STOP
	Common::KEYCODE_AUDIONEXT, // AKEYCODE_MEDIA_NEXT
	Common::KEYCODE_AUDIOPREV, // AKEYCODE_MEDIA_PREVIOUS
	Common::KEYCODE_AUDIOREWIND, // AKEYCODE_MEDIA_REWIND
	Common::KEYCODE_AUDIOFASTFORWARD, // AKEYCODE_MEDIA_FAST_FORWARD
	Common::KEYCODE_MUTE, // AKEYCODE_MUTE
	Common::KEYCODE_PAGEUP, // AKEYCODE_PAGE_UP
	Common::KEYCODE_PAGEDOWN, // AKEYCODE_PAGE_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_PICTSYMBOLS
	Common::KEYCODE_INVALID, // AKEYCODE_SWITCH_CHARSET
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_A
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_B
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_C
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_X
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_Y
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_Z
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_L1
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_R1
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_L2
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_R2
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_THUMBL
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_THUMBR
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_START
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_SELECT
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_MODE
	Common::KEYCODE_ESCAPE, // AKEYCODE_ESCAPE
	Common::KEYCODE_DELETE, // AKEYCODE_FORWARD_DEL
	Common::KEYCODE_LCTRL, // AKEYCODE_CTRL_LEFT
	Common::KEYCODE_RCTRL, // AKEYCODE_CTRL_RIGHT
	Common::KEYCODE_CAPSLOCK, // AKEYCODE_CAPS_LOCK
	Common::KEYCODE_SCROLLOCK, // AKEYCODE_SCROLL_LOCK
	Common::KEYCODE_LSUPER, // AKEYCODE_META_LEFT
	Common::KEYCODE_RSUPER, // AKEYCODE_META_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_FUNCTION
	Common::KEYCODE_SYSREQ, // AKEYCODE_SYSRQ
	Common::KEYCODE_BREAK, // AKEYCODE_BREAK
	Common::KEYCODE_HOME, // AKEYCODE_MOVE_HOME
	Common::KEYCODE_END, // AKEYCODE_MOVE_END
	Common::KEYCODE_INSERT, // AKEYCODE_INSERT
	Common::KEYCODE_AC_FORWARD, // AKEYCODE_FORWARD
	Common::KEYCODE_AUDIOPLAY, // AKEYCODE_MEDIA_PLAY
	Common::KEYCODE_AUDIOPAUSE, // AKEYCODE_MEDIA_PAUSE
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_CLOSE
	Common::KEYCODE_EJECT, // AKEYCODE_MEDIA_EJECT
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_RECORD
	Common::KEYCODE_F1, // AKEYCODE_F1
	Common::KEYCODE_F2, // AKEYCODE_F2
	Common::KEYCODE_F3, // AKEYCODE_F3
	Common::KEYCODE_F4, // AKEYCODE_F4
	Common::KEYCODE_F5, // AKEYCODE_F5
	Common::KEYCODE_F6, // AKEYCODE_F6
	Common::KEYCODE_F7, // AKEYCODE_F7
	Common::KEYCODE_F8, // AKEYCODE_F8
	Common::KEYCODE_F9, // AKEYCODE_F9
	Common::KEYCODE_F10, // AKEYCODE_F10
	Common::KEYCODE_F11, // AKEYCODE_F11
	Common::KEYCODE_F12, // AKEYCODE_F12
	Common::KEYCODE_NUMLOCK, // AKEYCODE_NUM_LOCK
	Common::KEYCODE_KP0, // AKEYCODE_NUMPAD_0
	Common::KEYCODE_KP1, // AKEYCODE_NUMPAD_1
	Common::KEYCODE_KP2, // AKEYCODE_NUMPAD_2
	Common::KEYCODE_KP3, // AKEYCODE_NUMPAD_3
	Common::KEYCODE_KP4, // AKEYCODE_NUMPAD_4
	Common::KEYCODE_KP5, // AKEYCODE_NUMPAD_5
	Common::KEYCODE_KP6, // AKEYCODE_NUMPAD_6
	Common::KEYCODE_KP7, // AKEYCODE_NUMPAD_7
	Common::KEYCODE_KP8, // AKEYCODE_NUMPAD_8
	Common::KEYCODE_KP9, // AKEYCODE_NUMPAD_9
	Common::KEYCODE_KP_DIVIDE, // AKEYCODE_NUMPAD_DIVIDE
	Common::KEYCODE_KP_MULTIPLY, // AKEYCODE_NUMPAD_MULTIPLY
	Common::KEYCODE_KP_MINUS, // AKEYCODE_NUMPAD_SUBTRACT
	Common::KEYCODE_KP_PLUS, // AKEYCODE_NUMPAD_ADD
	Common::KEYCODE_KP_PERIOD, // AKEYCODE_NUMPAD_DOT
	Common::KEYCODE_INVALID, // AKEYCODE_NUMPAD_COMMA
	Common::KEYCODE_KP_ENTER, // AKEYCODE_NUMPAD_ENTER
	Common::KEYCODE_KP_EQUALS, // AKEYCODE_NUMPAD_EQUALS
	Common::KEYCODE_INVALID, // AKEYCODE_NUMPAD_LEFT_PAREN
	Common::KEYCODE_INVALID, // AKEYCODE_NUMPAD_RIGHT_PAREN
	Common::KEYCODE_INVALID, // AKEYCODE_VOLUME_MUTE
	Common::KEYCODE_INVALID, // AKEYCODE_INFO
	Common::KEYCODE_INVALID, // AKEYCODE_CHANNEL_UP
	Common::KEYCODE_INVALID, // AKEYCODE_CHANNEL_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_ZOOM_IN
	Common::KEYCODE_INVALID, // AKEYCODE_ZOOM_OUT
	Common::KEYCODE_INVALID, // AKEYCODE_TV
	Common::KEYCODE_INVALID, // AKEYCODE_WINDOW
	Common::KEYCODE_INVALID, // AKEYCODE_GUIDE
	Common::KEYCODE_INVALID, // AKEYCODE_DVR
	Common::KEYCODE_AC_BOOKMARKS, // AKEYCODE_BOOKMARK
	Common::KEYCODE_INVALID, // AKEYCODE_CAPTIONS
	Common::KEYCODE_INVALID, // AKEYCODE_SETTINGS
	Common::KEYCODE_INVALID, // AKEYCODE_TV_POWER
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT
	Common::KEYCODE_INVALID, // AKEYCODE_STB_POWER
	Common::KEYCODE_INVALID, // AKEYCODE_STB_INPUT
	Common::KEYCODE_INVALID, // AKEYCODE_AVR_POWER
	Common::KEYCODE_INVALID, // AKEYCODE_AVR_INPUT
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_RED
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_GREEN
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_YELLOW
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_BLUE
	Common::KEYCODE_INVALID, // AKEYCODE_APP_SWITCH
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_1
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_2
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_3
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_4
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_5
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_6
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_7
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_8
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_9
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_10
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_11
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_12
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_13
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_14
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_15
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_16
	Common::KEYCODE_INVALID, // AKEYCODE_LANGUAGE_SWITCH
	Common::KEYCODE_INVALID, // AKEYCODE_MANNER_MODE
	Common::KEYCODE_INVALID, // AKEYCODE_3D_MODE
	Common::KEYCODE_INVALID, // AKEYCODE_CONTACTS
	Common::KEYCODE_INVALID, // AKEYCODE_CALENDAR
	Common::KEYCODE_INVALID, // AKEYCODE_MUSIC
	Common::KEYCODE_CALCULATOR, // AKEYCODE_CALCULATOR
	Common::KEYCODE_INVALID, // AKEYCODE_ZENKAKU_HANKAKU
	Common::KEYCODE_INVALID, // AKEYCODE_EISU
	Common::KEYCODE_INVALID, // AKEYCODE_MUHENKAN
	Common::KEYCODE_INVALID, // AKEYCODE_HENKAN
	Common::KEYCODE_INVALID, // AKEYCODE_KATAKANA_HIRAGANA
	Common::KEYCODE_INVALID, // AKEYCODE_YEN
	Common::KEYCODE_INVALID, // AKEYCODE_RO
	Common::KEYCODE_INVALID, // AKEYCODE_KANA
	Common::KEYCODE_INVALID, // AKEYCODE_ASSIST
	Common::KEYCODE_INVALID, // AKEYCODE_BRIGHTNESS_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_BRIGHTNESS_UP
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_AUDIO_TRACK
	Common::KEYCODE_SLEEP, // AKEYCODE_SLEEP
	Common::KEYCODE_INVALID, // AKEYCODE_WAKEUP
	Common::KEYCODE_INVALID, // AKEYCODE_PAIRING
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_TOP_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_11
	Common::KEYCODE_INVALID, // AKEYCODE_12
	Common::KEYCODE_INVALID, // AKEYCODE_LAST_CHANNEL
	Common::KEYCODE_INVALID, // AKEYCODE_TV_DATA_SERVICE
	Common::KEYCODE_INVALID, // AKEYCODE_VOICE_ASSIST
	Common::KEYCODE_INVALID, // AKEYCODE_TV_RADIO_SERVICE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TELETEXT
	Common::KEYCODE_INVALID, // AKEYCODE_TV_NUMBER_ENTRY
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TERRESTRIAL_ANALOG
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TERRESTRIAL_DIGITAL
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE_BS
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE_CS
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE_SERVICE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_NETWORK
	Common::KEYCODE_INVALID, // AKEYCODE_TV_ANTENNA_CABLE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_2
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_3
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_4
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPOSITE_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPOSITE_2
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPONENT_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPONENT_2
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_VGA_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_AUDIO_DESCRIPTION
	Common::KEYCODE_INVALID, // AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP
	Common::KEYCODE_INVALID, // AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_TV_ZOOM_MODE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_CONTENTS_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_TV_MEDIA_CONTEXT_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TIMER_PROGRAMMING
	Common::KEYCODE_HELP, // AKEYCODE_HELP
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_PREVIOUS
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_NEXT
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_IN
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_OUT
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_PRIMARY
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_1
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_2
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_3
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_UP_LEFT
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_DOWN_LEFT
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_UP_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_DOWN_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_SKIP_FORWARD
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_SKIP_BACKWARD
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_STEP_FORWARD
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_STEP_BACKWARD
	Common::KEYCODE_INVALID, // AKEYCODE_SOFT_SLEEP
	Common::KEYCODE_CUT, // AKEYCODE_CUT
	Common::KEYCODE_COPY, // AKEYCODE_COPY
	Common::KEYCODE_PASTE, // AKEYCODE_PASTE
	Common::KEYCODE_INVALID, // AKEYCODE_SYSTEM_NAVIGATION_UP
	Common::KEYCODE_INVALID, // AKEYCODE_SYSTEM_NAVIGATION_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_SYSTEM_NAVIGATION_LEFT
	Common::KEYCODE_INVALID, // AKEYCODE_SYSTEM_NAVIGATION_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_ALL_APPS
	Common::KEYCODE_INVALID, // AKEYCODE_REFRESH
	Common::KEYCODE_INVALID, // AKEYCODE_THUMBS_UP
	Common::KEYCODE_INVALID, // AKEYCODE_THUMBS_DOWN
	Common::KEYCODE_INVALID // AKEYCODE_PROFILE_SWITCH
};

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
			if (eventType == AINPUT_EVENT_TYPE_KEY) {
				result = processKeyEvent(e, event);
			} else if  (eventType == AINPUT_EVENT_TYPE_MOTION) {
				result = processMotionEvent(e, event);
			} else {
				// TODO process other types of events
			}
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

bool AndroidEventSource::processKeyEvent(AInputEvent *e, Common::Event &event) {
	int action = AKeyEvent_getAction(e);
	int keyCode = AKeyEvent_getKeyCode(e);
	int metaState = AKeyEvent_getMetaState(e);
	int repeatCount = AKeyEvent_getRepeatCount(e);

	if (action == AKEY_EVENT_ACTION_UP) {
		event.type = Common::EVENT_KEYUP;
	} else if (action == AKEY_EVENT_ACTION_DOWN) {
		event.type = Common::EVENT_KEYDOWN;
	} else if (action == AKEY_EVENT_ACTION_MULTIPLE) {
		LOGW("Multiple key action not implemented yet");
	}

	event.kbdRepeat = repeatCount > 1;

	if (metaState & (AMETA_SHIFT_ON  | AMETA_SHIFT_LEFT_ON | AMETA_SHIFT_RIGHT_ON)) {
		event.kbd.flags |= Common::KBD_SHIFT;
	}
	// JMETA_ALT is Fn on physical keyboards!
	// when mapping this to ALT - as we know it from PC keyboards - all
	// Fn combos will be broken (like Fn+q, which needs to end as 1 and
	// not ALT+1). Do not want.
	if (metaState & (AMETA_ALT_ON | AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON)) {
		event.kbd.flags |= Common::KBD_ALT;
	}
	if (metaState & (AMETA_CTRL_ON | AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON)) {
		event.kbd.flags |= Common::KBD_CTRL;
	}
	if (metaState & (AMETA_META_ON | AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON)) {
		event.kbd.flags |= Common::KBD_META;
	}
	if (metaState & (AMETA_CAPS_LOCK_ON)) {
		event.kbd.flags |= Common::KBD_CAPS;
	}
	if (metaState & (AMETA_NUM_LOCK_ON)) {
		event.kbd.flags |= Common::KBD_NUM;
	}
	if (metaState & (AMETA_SCROLL_LOCK_ON)) {
		event.kbd.flags |= Common::KBD_SCRL;
	}

	event.kbd.keycode = Common::KEYCODE_INVALID;
	if (keyCode <= AKEYCODE_PROFILE_SWITCH) {
		event.kbd.keycode = keymap[keyCode];
		event.kbd.ascii = 0;

		if (keyCode >= AKEYCODE_F1 && keyCode <= AKEYCODE_F12) {
			event.kbd.ascii = Common::ASCII_F1 + (keyCode - AKEYCODE_F1);
		}

		// Why is this function missing in NDK?
		Common::String str = _system->_jni->stringFromKeyCode(e);

		// Use only valid ASCII character (0..128)
		if (str.size() && str[0] > 0){
			event.kbd.ascii = str[0];
		}
	}

	LOGI("Key event code %i, ascii %c", event.kbd.keycode, event.kbd.ascii);
	return true;
}

bool AndroidEventSource::processMotionEvent(AInputEvent *e, Common::Event &event) {
	int action = AMotionEvent_getAction(e);
	float x = AMotionEvent_getX(e, 0);
	float y = AMotionEvent_getY(e, 0);

	event.mouse.x = x;
	event.mouse.y = y;
	if (action == AMOTION_EVENT_ACTION_DOWN) {
		event.type = Common::EVENT_LBUTTONDOWN;
	} else if (action == AMOTION_EVENT_ACTION_UP) {
		event.type = Common::EVENT_LBUTTONUP;
	} else if (action == AMOTION_EVENT_ACTION_MOVE) {
		event.type = Common::EVENT_MOUSEMOVE;
	}

	_system->getAndroidGraphicsManager()->notifyMousePosition(event.mouse);

	LOGI("Motion Event %i, %i %i", event.type, event.mouse.x, event.mouse.y);
	return true;
}