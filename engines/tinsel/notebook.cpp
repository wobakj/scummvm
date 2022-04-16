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
 * Notebook subsystem
 */

#include "tinsel/notebook.h"

#include "tinsel/dialogs.h"

namespace Tinsel {

void Notebook::AddHyperlink(int32 id_1, int32 id_2) {
	INV_OBJECT *invObject = _vm->_dialogs->GetInvObject(id_1);
	if (invObject->title != 0) {
		error("A clue can only be hyperlinked if it only has one title!");
		return;
	}

	invObject = _vm->_dialogs->GetInvObject(id_2);
	if (invObject->title != 0) {
		error("A clue can only be hyperlinked if it only has one title!");
		return;
	}

	uint32 i;
	for (i = 0; i < MAX_HYPERS; ++i) {
		int32 curr_id1 = _hyperlinks[i].id_1;
		if (curr_id1 == 0)
			break;
		if ((curr_id1 == id_1) || (id_1 == _hyperlinks[i].id_2)) {
			if ((curr_id1 != id_2) && (id_2 != _hyperlinks[i].id_2)) {
				error("A clue/title can only be hyperlinked to one other clue/title!");
			}
			return;
		}
	}

	if (i < MAX_HYPERS) {
		_hyperlinks[i].id_1 = id_1;
		_hyperlinks[i].id_2 = id_2;
	} else {
		error("Need to increase MAX_HYPERS");
	}
}

} // End of namespace Tinsel
