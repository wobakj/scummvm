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

#include "backends/platform/android/system.h"

#include <android/native_activity.h>

static void onDestroy(ANativeActivity* activity) {
    ENTER();
    OSystem_Android *android_system = (OSystem_Android *)activity->instance;
    android_system->destroy();
}

static void onStart(ANativeActivity* activity) {
    ENTER();
    // android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_START);
}

static void onResume(ANativeActivity* activity) {
    ENTER();
    // android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_RESUME);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
    ENTER();
    // struct android_app* android_app = (struct android_app*)activity->instance;
    void* savedState = NULL;

    // pthread_mutex_lock(&android_app->mutex);
    // android_app->stateSaved = 0;
    // android_app_write_cmd(android_app, APP_CMD_SAVE_STATE);
    // while (!android_app->stateSaved) {
    //     pthread_cond_wait(&android_app->cond, &android_app->mutex);
    // }

    // if (android_app->savedState != NULL) {
    //     savedState = android_app->savedState;
    //     *outLen = android_app->savedStateSize;
    //     android_app->savedState = NULL;
    //     android_app->savedStateSize = 0;
    // }

    // pthread_mutex_unlock(&android_app->mutex);

    return savedState;
}

static void onPause(ANativeActivity* activity) {
    ENTER();
    // android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity) {
    ENTER();
    // android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged(ANativeActivity* activity) {
    ENTER();
    // android_app_write_cmd(android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity) {
    ENTER();
    // android_app_write_cmd(android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
    ENTER();
    // android_app_write_cmd((struct android_app*)activity->instance,
    //         focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    ENTER();
    OSystem_Android *android_system = (OSystem_Android *)activity->instance;
    android_system->nativeWindowCreated(window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    ENTER();
    OSystem_Android *android_system = (OSystem_Android *)activity->instance;
    android_system->nativeWindowDestroyed(window);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    ENTER();
    OSystem_Android *android_system = (OSystem_Android *)activity->instance;
    android_system->inputQueueCreated(queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    ENTER();
    OSystem_Android *android_system = (OSystem_Android *)activity->instance;
    android_system->inputQueueDestroyed(queue);
}

JNIEXPORT
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    ENTER();

    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

    OSystem_Android *android_system = new OSystem_Android(activity);
    g_system = android_system;
    activity->instance = android_system;
}
