#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#

import os
import sys
import shutil
import ctypes
import argparse
from datetime import datetime
import struct
from os import path
# for pack
from struct import *
from subtlsVersTextResource import *



pathToParent = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
pathToCommon = os.path.join(pathToParent, "common")
sys.path.append(pathToCommon)


COMPANY_EMAIL = "classic.adventures.in.greek@gmail.com"
APP_VERSION = "1.50"
APP_NAME = "packBladeRunnerMIXFromPCTLKXLS"
APP_WRAPPER_NAME = "mixResourceCreator.py"
APP_NAME_SPACED = "Blade Runner MIX Resource Creator"
APP_SHORT_DESC = "Make a Text Resource file for spoken in-game quotes and pack Text Resources with Fonts into a SUBTITLES.MIX file."

# strFileName should be the full file name (including extension)
def calculateFoldHash(strFileName):
	i = 0
	hash = 0
	strParam = strFileName.upper()
	lenFileName = len(strParam)
	while i < lenFileName and i < 12:
		groupSum = 0
		# work in groups of 4 bytes
		for j in range(0, 4):
			# LSB first, so the four letters in the string are re-arranged (first letter goes to lower place)
			groupSum >>= 8
			if (i < lenFileName):
				groupSum |= (ord(strParam[i]) << 24)
				i += 1
			else: # if	i >= lenFileName  but still haven't completed the four byte loop add 0s
				groupSum |= 0
		hash = ((hash << 1) | ((hash >> 31) & 1)) + groupSum
	hash &= 0xFFFFFFFF	   # mask here!
	if gTraceModeEnabled:
		print '[Debug] ', (strParam +': '  +''.join('{:08X}'.format(hash)))
	return hash

#
# aux - sort by first object in list of tuples
def getSortMixFilesKey(item):
	keyTmp = item[0] & 0xFFFFFFFF

	signedKeyTmp = ctypes.c_long(keyTmp).value
	return signedKeyTmp

def packMIX(filesToPack, outputFile):
	print "[Info] Writing to output MIX file: %s..." % (outputFile)

	
	with open(outputFile, 'wb') as mix:
		



	except Exception as e:
		errorFound = True
		print '[Error] Unable to write to output MIX file. ' + str(e)
	if not errorFound:
		# Write header
		# 2 bytes: number of entries (NumFiles)
		# TODO 4 bytes: size of data segment
		# 12 * NumFiles bytes: Entry descriptors table
		#			4 bytes: ID (hash)
		#			4 bytes: Byte offset in Data Segment
		#			4 bytes: Byte length of entry data
		# *Data Segment* - contains the file data. Offset from Entry Descriptors does not include header segment byte length.
		#												Note that the offsets are relative to the start of the body so to find the
		#												actual offset in the MIX you have to add the size of the header which is
		#												(6 + (12 * NumFiles))

		#
		# ID column should in ascending order in MIX FILES (the engine uses binary sort to search for files)
		# so order the files based on ID hash
		# Create a list of 3-item tuples, first item is id, second item is filename
		# Then sort the list
		# Then write to entry table
		#
		# Also filenames should be 8 characters at most and 4 more for extension to conform with specs
		#	^^ this is done manually by making sure the filenames in the sheets of the excel as compliant
		# Based on observations from STARTUP.MIX:
		# 1) the hash ids can overflow and so lower numbers seem to appear down in the index table entries list
		#		 -- So we sort hash but we first translate the unsigned key to signed with ctypes
		# 2) the offsets are not necessarily sorted, meaning that the first entry in the index table won't necessarily have the 0x00000000 offset
		i = 0
		mixFileEntries = []
		totalFilesDataSize = 0
		currOffsetForDataSegment = 0 # we start after header and table of index entries, from 0, (but this means that when reading the offset we need to add 6 + numOfFiles * 12). This does not concern us though.

		mergedListOfSupportedSubtitleSheets = getSupportedSubtitleSheetsList()

		if gTraceModeEnabled:
			print "[Trace] mergedListOfSupportedSubtitleSheets="
			print mergedListOfSupportedSubtitleSheets

		for sheetDialogueName in mergedListOfSupportedSubtitleSheets:
			sheetDialogueNameTRx = sheetDialogueName
			if sheetDialogueNameTRx[-4:-1] != '.TR':
				tmpActiveLanguageTRxCode = sheetDialogueNameTRx[-5]
				sheetDialogueNameTRx = sheetDialogueName[:-4] + ('.TR%s' %(tmpActiveLanguageTRxCode))
			if os.path.isfile('./' + sheetDialogueNameTRx):
				entryID = calculateFoldHash(sheetDialogueNameTRx)
				mixEntryfileSizeBytes = os.path.getsize('./' + sheetDialogueNameTRx)
				mixFileEntries.append((entryID, sheetDialogueNameTRx, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes

		supportedTranslatedTrxFilenamesList = getSupportedTranslatedTrxFilenamesList()
		for translatedTRxFileName in supportedTranslatedTrxFilenamesList:
			if os.path.isfile('./' + translatedTRxFileName):
				entryID = calculateFoldHash(translatedTRxFileName)
				mixEntryfileSizeBytes = os.path.getsize('./' + translatedTRxFileName)
				mixFileEntries.append((entryID, translatedTRxFileName, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes

		for otherFileName in SUPPORTED_OTHER_FILES_FOR_MIX:
			if os.path.isfile('./' + otherFileName):
				entryID = calculateFoldHash(otherFileName)
				mixEntryfileSizeBytes = os.path.getsize('./' + otherFileName)
				mixFileEntries.append((entryID, otherFileName, mixEntryfileSizeBytes))
				totalFilesDataSize += mixEntryfileSizeBytes
		mixFileEntries.sort(key=getSortMixFilesKey)
		#
		# We write num of files here. After we verified they exist
		#
		numOfFiles = len(mixFileEntries)
		numOfFilesToWrite = pack('h',numOfFiles)  # short 2 bytes
		outMIXFile.write(numOfFilesToWrite)

		# This is just the data segment (after the entries index table). Adds up all the file sizes here
		totalFilesDataSizeToWrite = pack('I',totalFilesDataSize)  # unsigned integer 4 bytes
		outMIXFile.write(totalFilesDataSizeToWrite)

		if gTraceModeEnabled:
			print ("[Debug] Sorted Entries based on EntryId.")
		for mixFileEntry in mixFileEntries:
			if gTraceModeEnabled:
				print (''.join('{:08X}'.format(mixFileEntry[0])) + ': ' + mixFileEntry[1] + ' : ' + ''.join('{:08X}'.format(mixFileEntry[2])))
			entryID = mixFileEntry[0] & 0xFFFFFFFF
			entryIDToWrite = pack('I',entryID)	# unsigned integer 4 bytes
			outMIXFile.write(entryIDToWrite)
			entryOffset = currOffsetForDataSegment		# offsets have base after header and table of index entries
			entryOffsetToWrite = pack('I',entryOffset)	# unsigned integer 4 bytes
			outMIXFile.write(entryOffsetToWrite)
			entryByteLength =  mixFileEntry[2]	# File size
			entryByteLengthToWrite = pack('I',entryByteLength)	# unsigned integer 4 bytes
			outMIXFile.write(entryByteLengthToWrite)
			currOffsetForDataSegment += entryByteLength
		# Add data segments here
		errorReadingFound = False
		for mixFileEntry in mixFileEntries:
			try:
				inEntryMIXFile = open("./"+ mixFileEntry[1], 'rb')
			except:
				errorReadingFound = True
			if not errorReadingFound:
				outMIXFile.write(inEntryMIXFile.read())
				inEntryMIXFile.close()
			else:
				print ("[Error] Error while reading in ENTRY file")
				break

		outMIXFile.close()
		print "[Info] Total Resource files packed in %s: %d" % (DEFAULT_SUBTITLES_MIX_OUTPUT_NAME, numOfFiles)
		print "[Info] Done."
	return
#
# END FOR MIX FILE
#
#

def main(argsCL):
	parser = argparse.ArgumentParser(description='Process some integers.')
	parser.add_argument('integers', metavar='N', type=int, nargs='+', help='an integer for the accumulator')
	parser.add_argument('--sum', dest='accumulate', action='store_const', const=sum, default=max, help='sum the integers (default: find the max)')
	
if __name__ == "__main__":
	main(sys.argv[0:])
else:
	## debug
	#print '[Debug] %s was imported from another module' % (APP_WRAPPER_NAME)
	pass
