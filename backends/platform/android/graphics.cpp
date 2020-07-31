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

#include "backends/platform/android/graphics.h"

#include "backends/platform/android/jni-android.h"
#include "backends/platform/android/system.h"

#include "common/array.h"

AndroidGraphicsManager::AndroidGraphicsManager(OSystem_Android *system, ANativeWindow* nativeWindow)
	: _system(system)
	, _nativeWindowPending(nativeWindow)
	, _nativeWindowCurrent(nativeWindow)
	, _eglDisplay(EGL_NO_DISPLAY)
	, _eglContext(EGL_NO_CONTEXT)
	, _eglSurface(EGL_NO_SURFACE)
	, _eglConfig(nullptr)
{
	ENTER();

	if (_nativeWindowCurrent != nullptr) {
		initEGLContext();
		initEGLSurface(_nativeWindowCurrent);
	}
}

AndroidGraphicsManager::~AndroidGraphicsManager() {
	ENTER();

	destroyEGLSurface();
	destroyEGLContext();
}

void AndroidGraphicsManager::setNativeWindow(ANativeWindow *nativeWindow) {
	// this is called from a different thread, just mark that we need a new context
	_nativeWindowPending = nativeWindow;
}

void AndroidGraphicsManager::updateScreen() {
	// mutex or local copy of _nativeWindowPending
	if (_nativeWindowPending != _nativeWindowCurrent) {
		if (_nativeWindowCurrent != nullptr) {
			destroyEGLSurface();
		}
		if (_nativeWindowPending != nullptr) {
			initEGLSurface(_nativeWindowPending);
		}
		_nativeWindowCurrent = _nativeWindowPending;
	}

	if (_eglSurface == EGL_NO_SURFACE) {
		return;
	}

	OpenGLGraphicsManager::updateScreen();
}

void AndroidGraphicsManager::displayMessageOnOSD(const Common::U32String &msg) {
	ENTER("%s", msg.encode().c_str());

	JNI::displayMessageOnOSD(msg);
}

bool AndroidGraphicsManager::loadVideoMode(uint requestedWidth, uint requestedHeight, const Graphics::PixelFormat &format) {
	ENTER("%d, %d, %s", requestedWidth, requestedHeight, format.toString().c_str());

	// We get this whenever a new resolution is requested. Since Android is
	// using a fixed output size we do nothing like that here.
	// TODO: Support screen rotation
	return true;
}

void AndroidGraphicsManager::refreshScreen() {
	//ENTER();

	if (!eglSwapBuffers(_eglDisplay, _eglSurface)) {
		EGLint err = eglGetError();
		if (err == EGL_BAD_SURFACE) {
			initEGLSurface(_nativeWindowCurrent);
			// Still consider glContext is valid
		} else if (err == EGL_CONTEXT_LOST || err == EGL_BAD_CONTEXT) {
			initEGLContext();
			initEGLSurface(_nativeWindowCurrent);
		}
	}
}

void *AndroidGraphicsManager::getProcAddress(const char *name) const {
	ENTER("%s", name);

	return (void*)eglGetProcAddress(name);
}

bool AndroidGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	mouse.x = CLIP<int16>(mouse.x, _activeArea.drawRect.left, _activeArea.drawRect.right);
	mouse.y = CLIP<int16>(mouse.y, _activeArea.drawRect.top, _activeArea.drawRect.bottom);

	setMousePosition(mouse.x, mouse.y);
	mouse = convertWindowToVirtual(mouse.x, mouse.y);

	return true;
}

void AndroidGraphicsManager::initEGLContext() {
	ENTER();

	LOGD("Initializing EGL display");
	_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(_eglDisplay, nullptr, nullptr);

	LOGD("Initializing EGL context");

	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // Request GLES 2.0
		EGL_RED_SIZE, 8,  // We want RGB8888, should be supported everywhere
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT, // We want to render to window
		EGL_CONFIG_CAVEAT, EGL_NONE, // No slow config
		EGL_NONE
	};
	EGLint numConfigs;
	eglChooseConfig(_eglDisplay, attribs, nullptr, 0, &numConfigs);
	// There should be at least one context config that supports creating a window surface

	if (numConfigs == 0) {
		LOGF("No valid EGL config founds - GLES2, RGBA8888, window surface, no caveats");
		abort();
	}
	Common::Array<EGLConfig> supportedConfigs(numConfigs);
	eglChooseConfig(_eglDisplay, attribs, supportedConfigs.data(), numConfigs, &numConfigs);

	int i = 0;
	for (; i < numConfigs; i++) {
		EGLConfig& cfg = supportedConfigs[i];
		EGLint configID;
		eglGetConfigAttrib(_eglDisplay, cfg, EGL_CONFIG_ID, &configID);
		LOGD("Found EGL config %i", configID);

#define PRINT_EGL_CONFIG_ATTRIB(attrib) {                               \
			EGLint val;                                                 \
			eglGetConfigAttrib(_eglDisplay, cfg, attrib, &val); \
			LOGD("EGL config - %s = %i", #attrib, val);                 \
		}
		PRINT_EGL_CONFIG_ATTRIB(EGL_ALPHA_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_ALPHA_MASK_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_BIND_TO_TEXTURE_RGB);
		PRINT_EGL_CONFIG_ATTRIB(EGL_BIND_TO_TEXTURE_RGBA);
		PRINT_EGL_CONFIG_ATTRIB(EGL_BLUE_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_BUFFER_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_COLOR_BUFFER_TYPE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_CONFIG_CAVEAT);
		PRINT_EGL_CONFIG_ATTRIB(EGL_CONFIG_ID);
		PRINT_EGL_CONFIG_ATTRIB(EGL_CONFORMANT);
		PRINT_EGL_CONFIG_ATTRIB(EGL_DEPTH_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_GREEN_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_LEVEL);
		PRINT_EGL_CONFIG_ATTRIB(EGL_LUMINANCE_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_MAX_PBUFFER_WIDTH);
		PRINT_EGL_CONFIG_ATTRIB(EGL_MAX_PBUFFER_HEIGHT);
		PRINT_EGL_CONFIG_ATTRIB(EGL_MAX_PBUFFER_PIXELS);
		PRINT_EGL_CONFIG_ATTRIB(EGL_MAX_SWAP_INTERVAL);
		PRINT_EGL_CONFIG_ATTRIB(EGL_MIN_SWAP_INTERVAL);
		PRINT_EGL_CONFIG_ATTRIB(EGL_NATIVE_RENDERABLE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_NATIVE_VISUAL_ID);
		PRINT_EGL_CONFIG_ATTRIB(EGL_NATIVE_VISUAL_TYPE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_RED_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_RENDERABLE_TYPE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_SAMPLE_BUFFERS);
		PRINT_EGL_CONFIG_ATTRIB(EGL_SAMPLES);
		PRINT_EGL_CONFIG_ATTRIB(EGL_STENCIL_SIZE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_SURFACE_TYPE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_TRANSPARENT_TYPE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_TRANSPARENT_RED_VALUE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_TRANSPARENT_GREEN_VALUE);
		PRINT_EGL_CONFIG_ATTRIB(EGL_TRANSPARENT_BLUE_VALUE);
#undef PRINT_EGL_CONFIG_ATTRIB
	}
	if (i == numConfigs) {
		_eglConfig = supportedConfigs[0];
	}

	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2, // Request GLES 2.0
		EGL_NONE
	};
	_eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, context_attribs);
	assert(_eglContext != EGL_NO_CONTEXT);
}

void AndroidGraphicsManager::destroyEGLContext() {
	LOGD("Destroying EGL context");

	notifyContextDestroy();

	if (_eglDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (_eglContext != EGL_NO_CONTEXT) {
			eglDestroyContext(_eglDisplay, _eglContext);
		}

	}
	_eglContext = EGL_NO_CONTEXT;

	LOGD("Destroying EGL display");
	eglTerminate(_eglDisplay);
	_eglDisplay = EGL_NO_DISPLAY;
	_eglConfig = nullptr;
}

void AndroidGraphicsManager::initEGLSurface(ANativeWindow *nativeWindow) {
	ENTER();

	LOGD("Initializing EGL surface");

	_eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig, nativeWindow, nullptr);

	assert(_eglSurface != EGL_NO_SURFACE);

	if (eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
	}

	LOGI("Using EGL %s (%s); GL %s/%s (%s)", eglQueryString(_eglDisplay, EGL_VERSION), eglQueryString(_eglDisplay, EGL_VENDOR), glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));

	setContextType(OpenGL::kContextGLES2);

	// Notify the OpenGL code about our context. Choose a proper format based on endian, ARM is using bin endian, but emulator can run on x86 which is little endian
#ifdef SCUMM_LITTLE_ENDIAN
	notifyContextCreate(Graphics::PixelFormat(2, 8, 8, 8, 0, 0, 8, 16, 0), Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#else
	notifyContextCreate(Graphics::PixelFormat(2, 8, 8, 8, 0, 16, 8, 0, 0), Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#endif

	EGLint w, h;

	eglQuerySurface(_eglDisplay, _eglSurface, EGL_WIDTH, &w);
	eglQuerySurface(_eglDisplay, _eglSurface, EGL_HEIGHT, &h);
	// eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	float dpi[2];
	JNI::getDPI(dpi);
	handleResize(w, h, dpi[0], dpi[1]);
}

void AndroidGraphicsManager::destroyEGLSurface() {
	LOGD("Destroying EGL surface");

	if (_eglSurface != EGL_NO_SURFACE) {
		eglDestroySurface(_eglDisplay, _eglSurface);
	}

	_eglSurface = EGL_NO_SURFACE;
}