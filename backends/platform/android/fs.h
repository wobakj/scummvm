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

#ifndef PLATFORM_ANDROID_FS_H
#define PLATFORM_ANDROID_FS_H

#include "backends/platform/android/common.h"

#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/posix/posix-fs-factory.h"

class OSystem_Android;

class AndroidFilesystemNode : public POSIXFilesystemNode {
private:
	OSystem_Android *_system;

protected:
	AndroidFilesystemNode(OSystem_Android *system);

	virtual POSIXFilesystemNode *makeNode(const Common::String &path) const override;

public:
	AndroidFilesystemNode(OSystem_Android *system, const Common::String &path);

	virtual bool getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const override;
	virtual AbstractFSNode *getParent() const override;
};

class AndroidFilesystemFactory : public POSIXFilesystemFactory {
private:
	OSystem_Android *_system;

public:
	AndroidFilesystemFactory(OSystem_Android *system);

	AbstractFSNode *makeRootFileNode() const;
	AbstractFSNode *makeCurrentDirectoryFileNode() const;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const;
};

#endif