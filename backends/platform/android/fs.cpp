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

#include "backends/platform/android/fs.h"

#include "backends/platform/android/jni-android.h"
#include "backends/platform/android/system.h"


AndroidFilesystemNode::AndroidFilesystemNode(OSystem_Android *system)
	: POSIXFilesystemNode()
	, _system(system)
{
}

POSIXFilesystemNode *AndroidFilesystemNode::makeNode(const Common::String &path) const {
	return new AndroidFilesystemNode(_system, path);
}

AndroidFilesystemNode::AndroidFilesystemNode(OSystem_Android *system, const Common::String &path)
	: POSIXFilesystemNode(path)
	, _system(system)
{
}

bool AndroidFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	if (_path == "/") {
		Common::Array<Common::String> list;
		_system->_jni->getAllStorageLocations(list);
		for (Common::Array<Common::String>::const_iterator it = list.begin(), end = list.end(); it != end; ++it) {
			AndroidFilesystemNode *entry = new AndroidFilesystemNode(_system);

			entry->_isDirectory = true;
			entry->_isValid = true;
			entry->_displayName = *it;
			// ++it;
			entry->_path = *it;
			myList.push_back(entry);
		}
		return true;
	} else {
		return POSIXFilesystemNode::getChildren(myList, mode, hidden);
	}
}

AbstractFSNode *AndroidFilesystemNode::getParent() const {
	AbstractFSNode *parent = POSIXFilesystemNode::getParent();

	Common::Array<Common::String> list;
	_system->_jni->getAllStorageLocations(list);

	for (Common::Array<Common::String>::const_iterator it = list.begin(), end = list.end(); it != end; ++it) {
		if (getPath().equals(*it)) {
			return makeNode("/");
		}
	}
	return parent;
}


AndroidFilesystemFactory::AndroidFilesystemFactory(OSystem_Android *system)
	: POSIXFilesystemFactory()
	, _system(system)
{
}

AbstractFSNode *AndroidFilesystemFactory::makeRootFileNode() const {
	return new AndroidFilesystemNode(_system, "/");
}

AbstractFSNode *AndroidFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new AndroidFilesystemNode(_system, "/");
}

AbstractFSNode *AndroidFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	// AppAssert(!path.empty());
	return new AndroidFilesystemNode(_system, path);
}
