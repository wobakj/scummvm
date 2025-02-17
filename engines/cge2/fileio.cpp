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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "common/system.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/memstream.h"
#include "cge2/cge2.h"
#include "cge2/fileio.h"

namespace CGE2 {

/*-----------------------------------------------------------------------
 * BtPage
 *-----------------------------------------------------------------------*/
void BtPage::readBTree(Common::ReadStream &s) {
	_header._count = s.readUint16LE();
	_header._down = s.readUint16LE();

	if (_header._down == kBtValNone) {
		// Leaf list
		for (int i = 0; i < kBtLeafCount; ++i) {
			s.read(_leaf[i]._key, kBtKeySize);
			_leaf[i]._pos = s.readUint32LE();
			_leaf[i]._size = s.readUint32LE();
		}
	} else {
		// Root index
		for (int i = 0; i < kBtInnerCount; ++i) {
			s.read(_inner[i]._key, kBtKeySize);
			_inner[i]._down = s.readUint16LE();
		}
	}
}

/*-----------------------------------------------------------------------
 * ResourceManager
 *-----------------------------------------------------------------------*/
ResourceManager::ResourceManager() {
	_datFile = new Common::File();
	_datFile->open(kDatName);

	_catFile = new Common::File();
	_catFile->open(kCatName);

	if (!_datFile->isOpen() || !_catFile->isOpen())
		error("Unable to open data files");

	for (int i = 0; i < kBtLevel; i++) {
		_buff[i]._page = new BtPage;
		_buff[i]._pageNo = kBtValNone;
		_buff[i]._index = -1;
		assert(_buff[i]._page != nullptr);
	}
}

ResourceManager::~ResourceManager() {
	_datFile->close();
	delete _datFile;

	_catFile->close();
	delete _catFile;

	for (int i = 0; i < kBtLevel; i++)
		delete _buff[i]._page;
}

void ResourceManager::xCrypt(byte *buf, uint16 length) {
	byte *b = buf;

	for (uint16 i = 0; i < length; i++)
		*b++ ^= kCryptSeed;
}

bool ResourceManager::seek(int32 offs, int whence) {
	return _datFile->seek(offs, whence);
}

uint16 ResourceManager::read(byte *buf, uint16 length) {
	if (!_datFile->isOpen())
		return 0;

	uint16 bytesRead = _datFile->read(buf, length);
	if (!bytesRead)
		error("Read %s - %d bytes", _datFile->getName(), length);
	xCrypt(buf, length);
	return bytesRead;
}

BtPage *ResourceManager::getPage(int level, uint16 pageId) {
	if (_buff[level]._pageNo != pageId) {
		int32 pos = pageId * kBtSize;
		_buff[level]._pageNo = pageId;

		if (_catFile->size() <= pos)
			return nullptr;

		// In the original, there was a check verifying if the
		// purpose was to write a new file. This should only be
		// to create a new file, thus it was removed.
		_catFile->seek(pageId * kBtSize, SEEK_SET);

		// Read in the page
		byte buffer[kBtSize];
		int bytesRead = catRead(buffer, kBtSize);

		// Unpack it into the page structure
		Common::MemoryReadStream stream(buffer, bytesRead, DisposeAfterUse::NO);
		_buff[level]._page->readBTree(stream);
		_buff[level]._index = -1;
	}
	return _buff[level]._page;
}

BtKeypack *ResourceManager::find(const char *key) {
	int lev = 0;
	uint16 nxt = kBtValRoot;
	while (!_catFile->eos()) {
		BtPage *pg = getPage(lev, nxt);
		if (!pg)
			return nullptr;

		// search
		if (pg->_header._down != kBtValNone) {
			int i;
			for (i = 0; i < pg->_header._count; i++) {
				// Does this work, or does it have to compare the entire buffer?
				if (scumm_strnicmp((const char *)key, (const char*)pg->_inner[i]._key, kBtKeySize) < 0)
					break;
			}
			nxt = (i) ? pg->_inner[i - 1]._down : pg->_header._down;
			_buff[lev]._index = i - 1;
			lev++;
		} else {
			int i;
			for (i = 0; i < pg->_header._count - 1; i++) {
				if (scumm_stricmp((const char *)key, (const char *)pg->_leaf[i]._key) <= 0)
					break;
			}

			// Hack to work around a mix between 24piram_ and 24pirami
			if (!strcmp(key, "24piram_.SPR") && (scumm_stricmp((const char *)key, (const char *)pg->_leaf[i]._key) < 0))
				++i;
			//

			_buff[lev]._index = i;
			return &pg->_leaf[i];
		}
	}
	return nullptr;
}

bool ResourceManager::exist(const char *name) {
	BtKeypack *result = find(name);
	if (!result)
		return false;

	return scumm_stricmp(result->_key, name) == 0;
}

uint16 ResourceManager::catRead(byte *buf, uint16 length) {
	if (!_catFile->isOpen())
		return 0;

	uint16 bytesRead = _catFile->read(buf, length);
	if (!bytesRead)
		error("Read %s - %d bytes", _catFile->getName(), length);
	xCrypt(buf, length);
	return bytesRead;
}

/*-----------------------------------------------------------------------
 * EncryptedStream
 *-----------------------------------------------------------------------*/
EncryptedStream::EncryptedStream(CGE2Engine *vm, const char *name) : _vm(vm), _lineCount(0) {
	_error = false;
	BtKeypack *kp = _vm->_resman->find(name);
	if (scumm_stricmp(kp->_key, name) != 0)
		_error = true;

	_vm->_resman->seek(kp->_pos);
	byte *dataBuffer;
	int bufSize;

	if ((strlen(name) > 4) && (scumm_stricmp(name + strlen(name) - 4, ".SPR") == 0)) {
		// SPR files have some inconsistencies. Some have extra 0x1A at the end, some others
		// do not have a carriage return at the end of the last line
		// Therefore, we remove this ending 0x1A and add extra new lines.
		// This fixes bug #6060
		dataBuffer = (byte *)malloc(kp->_size + 2);
		_vm->_resman->read(dataBuffer, kp->_size);
		if (dataBuffer[kp->_size - 1] == 0x1A)
			dataBuffer[kp->_size - 1] = '\n';
		dataBuffer[kp->_size] = '\n';
		dataBuffer[kp->_size + 1] = '\n';
		bufSize = kp->_size + 2;
	} else {
		dataBuffer = (byte *)malloc(kp->_size);
		_vm->_resman->read(dataBuffer, kp->_size);
		bufSize = kp->_size;
	}

	_readStream = new Common::MemoryReadStream(dataBuffer, bufSize, DisposeAfterUse::YES);
}

uint32 EncryptedStream::read(byte *dataPtr, uint32 dataSize) {
	return _readStream->read(dataPtr, dataSize);
}

int16 EncryptedStream::readSint16LE() {
	return _readStream->readSint16LE();
}

uint32 EncryptedStream::readUint32LE() {
	return _readStream->readUint32LE();
}

bool EncryptedStream::err() {
	return (_error || _readStream->err());
}

bool EncryptedStream::eos() {
	return _readStream->eos();
}

bool EncryptedStream::seek(int32 offset) {
	return _readStream->seek(offset);
}

Common::String EncryptedStream::readLine() {
	_lineCount++;
	Common::String line = _readStream->readLine();
	if (!line.empty() && (line[0] == ';' || line[0] == '.' || line[0] == '*'))
		line.clear(); // Returns an empty string, if the line is invalid.
	return line;
}

int32 EncryptedStream::size() {
	return _readStream->size();
}

int32 EncryptedStream::pos() {
	return _readStream->pos();
}

EncryptedStream::~EncryptedStream() {
	delete _readStream;
}

const char *EncryptedStream::kIdTab[] = {
	"[near]", "[mtake]", "[ftake]", "[phase]", "[seq]",
	"Name", "Type", "Front", "East",
	"Portable", "Transparent",
	nullptr
};

} // End of namespace CGE2
