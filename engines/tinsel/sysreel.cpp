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
 * System Reel handling.
 */

#include "tinsel/sysreel.h"

#include "common/scummsys.h"

#include "tinsel/cursor.h"
#include "tinsel/pid.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

/**
 * Returns the handle to the sysreel at the given index.
 *
 * @param index reel to get the handle to
 */
SCNHANDLE SystemReel::Get(int32 index) {
	assert(index >= 0 && index < MAX_SYSREELS);

    return reels[index];
}

/**
 * Stores a reel at an index and if the index is a cursor
 *
 * @param index where to store the reel
 * @param reel handle to the reel
 */
void SystemReel::Set(int32 index, SCNHANDLE reel) {

	assert(index >= 0 && index < MAX_SYSREELS);

    if (index == SYSREEL_LOADSCREEN) {
		if (CoroScheduler.getCurrentPID() != PID_SCENE) {
			return;
		}
	}

    reels[index] = reel;

    // Noir actually calls a function specifically for doing DwInitCursor on
    // system reel 11.
	if (index == SYSREEL_CURSOR && reel != 0) {
		_vm->_cursor->DwInitCursor(reel);
	}
}

} // End of namespace Tinsel
