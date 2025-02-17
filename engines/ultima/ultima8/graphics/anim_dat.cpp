/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/anim_dat.h"

#include "ultima/ultima8/world/actors/actor_anim.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

AnimDat::AnimDat() {
}

AnimDat::~AnimDat() {
	for (unsigned int i = 0; i < _anims.size(); i++)
		delete _anims[i];
}

const ActorAnim *AnimDat::getAnim(uint32 shape) const {
	if (shape >= _anims.size())
		return nullptr;

	return _anims[shape];
}

const AnimAction *AnimDat::getAnim(uint32 shape, uint32 action) const {
	if (shape >= _anims.size())
		return nullptr;
	if (_anims[shape] == 0)
		return nullptr;

	return _anims[shape]->getAction(action);
}

uint32 AnimDat::getActionNumberForSequence(Animation::Sequence action, const Actor *actor) {
	if (GAME_IS_U8) {
		return static_cast<uint32>(action);
	} else {
		bool smallwpn = true;
		bool altfire = false;
		bool isavatar = (actor && actor->getShape() == 1);
		if (isavatar && actor->getActiveWeapon()) {
			const Item *wpn = getItem(actor->getActiveWeapon());
			const ShapeInfo *shapeinfo = (wpn ? wpn->getShapeInfo() : nullptr);
			const WeaponInfo *wpninfo = (shapeinfo ? shapeinfo->_weaponInfo : nullptr);
			smallwpn = (wpninfo && wpninfo->_small);
			altfire = (wpninfo && (wpninfo->_overlayShape == 0x36e || wpninfo->_overlayShape == 0x33b));
		}

		//
		// For crusader the actions have different IDs.  Rather than
		// rewrite everything, we just translate them here for all the ones
		// we want to use programmatically.  There are more, but they are
		// called from usecode so don't need translation.
		//
		// We also translate based on weapon.  See the function at 1128:2104
		//
		// First, if the animation includes the Animation::crusaderAbsoluteAnimFlag
		// bitmask then it's from the usecode - use directly and don't translate.
		//
		const uint32 action_int = static_cast<uint32>(action);
		if (action_int & Animation::crusaderAbsoluteAnimFlag)
			return action_int - Animation::crusaderAbsoluteAnimFlag;

		switch (action) {
		case Animation::stand:
			return Animation::standCru;
		case Animation::step:
			return Animation::walkCru; // Same as walk in crusader.
		case Animation::walk:
			return Animation::walkCru;
		case Animation::retreat:
			return (smallwpn ? Animation::retreatSmallWeapon : Animation::retreatLargeWeapon);
		case Animation::reloadSmallWeapon:
			return (smallwpn ? Animation::reloadSmallWeapon : Animation::reloadLargeWeapon);
		case Animation::run:
			return Animation::runCru;
		case Animation::combatRunSmallWeapon:
			return (smallwpn ? Animation::combatRunSmallWeapon : Animation::combatRunLargeWeapon);
		case Animation::combatStand:
			return (smallwpn ? Animation::combatStandSmallWeapon : Animation::combatStandLargeWeapon);
		case Animation::unreadyWeapon:
				return (smallwpn ? Animation::unreadySmallWeapon :  Animation::unreadyLargeWeapon);
		case Animation::readyWeapon:
			return (smallwpn ? Animation::readySmallWeapon : Animation::readyLargeWeapon);
		case Animation::attack: {
			if (smallwpn)
				return Animation::fireSmallWeapon;
			return (altfire ? Animation::brightFireLargeWpn : Animation::fireLargeWeapon);
		}
		// Note: 14, 17, 21, 22, 29 == nothing for avatar
		case Animation::fallBackwards:
			return Animation::fallBackwardsCru;
		case Animation::die:
			return Animation::fallBackwardsCru; // by default fall over backwards.
		case Animation::advance:
			return (smallwpn ? Animation::advanceSmallWeapon : Animation::advanceLargeWeapon);
		// Kneel start/end never used in code - mover process uses correct ones.
		/*case Animation::startKneeling:
			return Animation::kneelStartCru;
		case Animation::stopKneeling:
			return Animation::kneelEndCru;*/
		case Animation::kneel:
			return (smallwpn ? Animation::kneelingWithSmallWeapon : Animation::kneelingWithLargeWeapon);
		case Animation::kneelAndFire: {
			if (smallwpn)
				return Animation::kneelAndFireSmallWeapon;
			return (altfire ? Animation::brightKneelAndFireLargeWeapon : Animation::kneelAndFireLargeWeapon);
		}
		case Animation::lookLeft:
			return 0;
		case Animation::lookRight:
			return 0;
		case Animation::jump:
			return Animation::quickJumpCru;
		case Animation::startRunLargeWeapon:
			return (smallwpn ? Animation::startRunSmallWeapon : Animation::startRunLargeWeapon2);
		case Animation::stopRunningAndDrawSmallWeapon:
			return (smallwpn ? Animation::stopRunningAndDrawSmallWeapon : Animation::stopRunningAndDrawLargeWeapon);
		default:
			return action_int;
		}
	}
}

void AnimDat::load(Common::SeekableReadStream *rs) {
	AnimFrame f;

	// CONSTANT !
	_anims.resize(2048);

	unsigned int actioncount = 64;

	if (GAME_IS_CRUSADER) {
		actioncount = 256;
	}

	for (unsigned int shape = 0; shape < _anims.size(); shape++) {
		rs->seek(4 * shape);
		uint32 offset = rs->readUint32LE();

		if (offset == 0) {
			_anims[shape] = nullptr;
			continue;
		}

		ActorAnim *a = new ActorAnim();

		// CONSTANT !
		a->_actions.resize(actioncount);

		for (unsigned int action = 0; action < actioncount; action++) {
			rs->seek(offset + action * 4);
			uint32 actionoffset = rs->readUint32LE();

			if (actionoffset == 0) {
				a->_actions[action] = 0;
				continue;
			}

			a->_actions[action] = new AnimAction();

			a->_actions[action]->_shapeNum = shape;
			a->_actions[action]->_action = action;

			rs->seek(actionoffset);
			// byte 0: action size
			uint32 actionsize = rs->readByte();
			a->_actions[action]->_size = actionsize;
			// byte 1: flags low byte
			uint32 rawflags = rs->readByte();
			// byte 2: frame repeat and rotated flag
			byte repeatAndRotateFlag = rs->readByte();
			a->_actions[action]->_frameRepeat = repeatAndRotateFlag & 0xf;
			if (GAME_IS_U8 && (repeatAndRotateFlag & 0xf0)) {
				// This should never happen..
				error("Anim data: frame repeat byte should never be > 0xf");
			} else if (GAME_IS_CRUSADER) {
				// WORKAROUND: In Crusader, the process wait semantics changed so
				// wait of 1 frame was the same as no wait.  The frame repeat
				// is implemented as a process wait, so this effectively reduced
				// all frame repeats by 1.
				if (a->_actions[action]->_frameRepeat)
					a->_actions[action]->_frameRepeat--;
			}
			// byte 3: flags high byte
			rawflags |= rs->readByte() << 8;

			// Only one flag in this byte in crusader.. the "rotate" flag.
			rawflags |= (repeatAndRotateFlag & 0xf0) << 12;

			a->_actions[action]->_flags = AnimAction::loadAnimActionFlags(rawflags);

			unsigned int dirCount = 8;
			if (a->_actions[action]->hasFlags(AnimAction::AAF_16DIRS)) {
				dirCount = 16;
			}

			/*
			if (a->_actions[action]->_flags & AnimAction::AAF_UNKFLAGS) {
				warning("AnimFlags: shape %d action %d has unknown flags %04X", shape, action,
					  a->_actions[action]->_flags & AnimAction::AAF_UNKFLAGS);
			}
			*/

			a->_actions[action]->_dirCount = dirCount;

			for (unsigned int dir = 0; dir < dirCount; dir++) {
				a->_actions[action]->_frames[dir].clear();

				for (unsigned int j = 0; j < actionsize; j++) {
					if (GAME_IS_U8) {
						f._frame = rs->readByte(); // & 0x7FF;
						uint8 x = rs->readByte();
						f._frame += (x & 0x7) << 8;
						f._deltaZ = rs->readSByte();
						f._sfx = rs->readByte();
						f._deltaDir = rs->readSByte();
						f._flags = rs->readByte();
						f._flags += (x & 0xF8) << 8;
					} else if (GAME_IS_CRUSADER) {
						// byte 0: low byte of frame
						f._frame = rs->readByte();
						// byte 1: low nibble is high part of frame, high nibble is flags (used later)
						const uint8 x = rs->readByte();
						f._frame += (x & 0xF) << 8;
						// byte 2: delta z
						f._deltaZ = rs->readSByte();
						// byte 3: sfx
						f._sfx = rs->readByte();
						// byte 4: deltadir (signed) - convert to pixels
						f._deltaDir = rs->readSByte();
						// byte 5: flags.  The lowest bit is actually a sign bit for
						// deltaDir, which is technically 9 bits long.  There are no
						// animations in the game exceeding 8-bit signed, the 9th bit
						// is there so that multiple frames can be stacked in the same
						// struct by the original game when it's skipping frames.
						// We have more memory and don't frame-skip, so just ignore it.
						f._flags = rs->readByte() & 0xFE;
						f._flags += (x & 0xF0) << 8;
						// bytes 6, 7: more flags
						f._flags += rs->readUint16LE() << 16;

						/*if (f._flags & AnimFrame::AFF_UNKNOWN) {
							warning("AnimFlags: shape %d action %d dir %d frame %d has unknown flags %08X", shape, action, dir, j,
									f._flags & AnimFrame::AFF_UNKNOWN);
						}*/
					}
					a->_actions[action]->_frames[dir].push_back(f);
				}
			}
		}

		_anims[shape] = a;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
