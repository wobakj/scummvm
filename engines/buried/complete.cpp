/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/complete.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/video_window.h"

#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

#define CHECK_PUZZLE_FLAG(flag) \
	if (_globalFlags.flag != 0) \
		puzzlesSolved++

#define CHECK_RESEARCH_FLAG(flag) \
	if (_globalFlags.flag != 0) \
		researchBonusRaw++

#define CHECK_CRITICAL_EVIDENCE(flag) \
	if (_globalFlags.evcapBaseID[i] == flag) \
		criticalEvidence++

#define CHECK_SUPPORTING_EVIDENCE(flag) \
	if (_globalFlags.evcapBaseID[i] == flag) \
		supportingEvidence++

CompletionWindow::CompletionWindow(BuriedEngine *vm, Window *parent, GlobalFlags &globalFlags) : Window(vm, parent), _globalFlags(globalFlags) {
	_vm->_sound->setAmbientSound();

	_status = 0;
	_background = nullptr;
	_currentSoundEffectID = -1;
	_gageVideo = nullptr;

	_rect = Common::Rect(0, 0, 640, 480);

	_timer = setTimer(1000);

	_fontHeightA = (_vm->getLanguage() == Common::JA_JPN) ? 12 : 14;
	_textFontA = _vm->_gfx->createFont(_fontHeightA);

	_fontHeightB = 20;
	_textFontB = _vm->_gfx->createFont(_fontHeightB, true);

	_walkthroughMode = _globalFlags.generalWalkthroughMode != 0;

	int puzzlesSolved = 0;
	CHECK_PUZZLE_FLAG(scoreGotTranslateBioChip);
	CHECK_PUZZLE_FLAG(scoreEnteredSpaceStation);
	CHECK_PUZZLE_FLAG(scoreDownloadedArthur);
	CHECK_PUZZLE_FLAG(scoreFoundSculptureDiagram);
	CHECK_PUZZLE_FLAG(scoreEnteredKeep);
	CHECK_PUZZLE_FLAG(scoreGotKeyFromSmithy);
	CHECK_PUZZLE_FLAG(scoreEnteredTreasureRoom);
	CHECK_PUZZLE_FLAG(scoreFoundSwordDiamond);
	CHECK_PUZZLE_FLAG(scoreMadeSiegeCycle);
	CHECK_PUZZLE_FLAG(scoreEnteredCodexTower);
	CHECK_PUZZLE_FLAG(scoreLoggedCodexEvidence);
	CHECK_PUZZLE_FLAG(scoreEnteredMainCavern);
	CHECK_PUZZLE_FLAG(scoreGotWealthGodPiece);
	CHECK_PUZZLE_FLAG(scoreGotRainGodPiece);
	CHECK_PUZZLE_FLAG(scoreGotWarGodPiece);
	CHECK_PUZZLE_FLAG(scoreCompletedDeathGod);
	CHECK_PUZZLE_FLAG(scoreEliminatedAgent3);
	CHECK_PUZZLE_FLAG(scoreTransportToKrynn);
	CHECK_PUZZLE_FLAG(scoreGotKrynnArtifacts);
	CHECK_PUZZLE_FLAG(scoreDefeatedIcarus);

	int researchBonusRaw = 0;
	CHECK_RESEARCH_FLAG(scoreResearchINNLouvreReport); // > v1.01
	CHECK_RESEARCH_FLAG(scoreResearchINNHighBidder);
	CHECK_RESEARCH_FLAG(scoreResearchINNAppeal);
	CHECK_RESEARCH_FLAG(scoreResearchINNUpdate);
	CHECK_RESEARCH_FLAG(scoreResearchINNJumpsuit);
	CHECK_RESEARCH_FLAG(scoreResearchBCJumpsuit);
	CHECK_RESEARCH_FLAG(scoreResearchMichelle);
	CHECK_RESEARCH_FLAG(scoreResearchMichelleBkg);
	CHECK_RESEARCH_FLAG(scoreResearchLensFilter);
	CHECK_RESEARCH_FLAG(scoreResearchCastleFootprint);
	CHECK_RESEARCH_FLAG(scoreResearchDaVinciFootprint);
	CHECK_RESEARCH_FLAG(scoreResearchMorphSculpture);
	CHECK_RESEARCH_FLAG(scoreResearchEnvironCart);
	CHECK_RESEARCH_FLAG(scoreResearchAgent3Note);
	CHECK_RESEARCH_FLAG(scoreResearchAgent3DaVinci);

	int criticalEvidence = 0;
	int supportingEvidence = 0;
	for (int i = 0; i < _globalFlags.evcapNumCaptured; i++) {
		CHECK_CRITICAL_EVIDENCE(CASTLE_EVIDENCE_SWORD);
		CHECK_CRITICAL_EVIDENCE(MAYAN_EVIDENCE_ENVIRON_CART);
		CHECK_CRITICAL_EVIDENCE(DAVINCI_EVIDENCE_CODEX);
		CHECK_CRITICAL_EVIDENCE(AI_EVIDENCE_SCULPTURE);

		CHECK_SUPPORTING_EVIDENCE(CASTLE_EVIDENCE_FOOTPRINT);
		CHECK_SUPPORTING_EVIDENCE(MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID);
		CHECK_SUPPORTING_EVIDENCE(MAYAN_EVIDENCE_PHONY_BLOOD);
		CHECK_SUPPORTING_EVIDENCE(CASTLE_EVIDENCE_AGENT3);
		CHECK_SUPPORTING_EVIDENCE(DAVINCI_EVIDENCE_FOOTPRINT);
		CHECK_SUPPORTING_EVIDENCE(DAVINCI_EVIDENCE_AGENT3);
		CHECK_SUPPORTING_EVIDENCE(DAVINCI_EVIDENCE_LENS_FILTER);
	}

	int hints = _globalFlags.scoreHintsTotal;
	int finalCriticalEvidenceScore = criticalEvidence * 1000;
	int finalSupportingEvidenceScore = supportingEvidence * 500;
	int finalPuzzleScore = puzzlesSolved * 200;
	int finalResearchScore = researchBonusRaw * 100;
	int hintsScore = hints * 50;
	int completionScore = 2000;
	int totalScore = finalCriticalEvidenceScore + finalSupportingEvidenceScore + finalPuzzleScore + finalResearchScore + completionScore;

	// Build the string buffers
	if (_walkthroughMode) {
		if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0)) {
			// HACK HACK HACK: Oh god. This is horrid.
			Common::String stringResource = _vm->getString(IDS_COMPL_WALK_SCORE_DESC_TEMPL);
			_scoringTextDescriptions = Common::String::format(stringResource.c_str(), criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw);
			stringResource = _vm->getString(IDS_COMPL_WALK_SCORE_AMT_TEMPL);
			_scoringTextScores = Common::String::format(stringResource.c_str(), finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore);
		} else {
			_scoringTextDescriptions = Common::String::format("Critical Evidence: %d / 4 x 1000\nSupporting Evidence: %d / 7 x 500\nPuzzles Solved: %d / 19 x 200\nResearch Bonus: %d / 15 x 100\nCompletion Bonus:",
					criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw);
			_scoringTextScores = Common::String::format("%d\n%d\n%d\n%d\n%d", finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore);
		}
	} else {
		totalScore -= hintsScore;

		if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0)) {
			// HACK HACK HACK: Again, horrid.
			Common::String stringResource = _vm->getString(IDS_COMPL_SCORE_DESC_TEMPL);
			_scoringTextDescriptions = Common::String::format(stringResource.c_str(), criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw, hints);
			stringResource = _vm->getString(IDS_COMPL_SCORE_AMT_TEMPL);
			_scoringTextScores = Common::String::format(stringResource.c_str(), finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore, -hintsScore);
		} else {
			_scoringTextDescriptions = Common::String::format("Critical Evidence: %d / 4 x 1000\nSupporting Evidence: %d / 7 x 500\nPuzzles Solved: %d / 20 x 200\nResearch Bonus: %d / 15 x 100\nCompletion Bonus:\n\nHints: %d @ -50 ea.",
					criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw, hints);
			_scoringTextScores = Common::String::format("%d\n%d\n%d\n%d\n%d\n\n%d", finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore, -hintsScore);
		}
	}

	// This would be a hack, but since it's just printing one number, I'm not
	// loading that damned string too.
	_scoringTextFinalScore = Common::String::format("%d", totalScore);

	_vm->_sound->setAmbientSound();
}

#undef CHECK_PUZZLE_FLAG
#undef CHECK_RESEARCH_FLAG
#undef CHECK_CRITICAL_EVIDENCE
#undef CHECK_SUPPORTING_EVIDENCE

CompletionWindow::~CompletionWindow() {
	delete _gageVideo;
	killTimer(_timer);

	delete _textFontA;
	delete _textFontB;

	if (_background) {
		_background->free();
		delete _background;
	}
}

void CompletionWindow::onPaint() {
	if (_background)
		_vm->_gfx->blit(_background, 0, 0);

	if (_status == 3) {
		// Draw the first line
		uint32 textColor = _vm->_gfx->getColor(102, 204, 153);
		Common::String firstBlockText = _vm->getString(2100);
		Common::Rect firstBlockRect(10, 54, 283, 86);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, firstBlockText, firstBlockRect.left, firstBlockRect.top, firstBlockRect.width(), firstBlockRect.height(), textColor, _fontHeightA);

		// Draw the cause text
		Common::String secondBlockText = _vm->getString(2102);
		Common::Rect secondBlockRect(10, 120, 283, 215);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, secondBlockText, secondBlockRect.left, secondBlockRect.top, secondBlockRect.width(), secondBlockRect.height(), textColor, _fontHeightA);

		// Description text
		Common::Rect scoringTextRect(10, 248, 283, 378);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _scoringTextDescriptions, scoringTextRect.left, scoringTextRect.top, scoringTextRect.width(), scoringTextRect.height(), textColor, _fontHeightA);

		// Scores
		textColor = _vm->_gfx->getColor(255, 255, 51);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _scoringTextScores, scoringTextRect.left, scoringTextRect.top, scoringTextRect.width(), scoringTextRect.height(), textColor, _fontHeightA, kTextAlignRight);

		// Total score
		Common::Rect finalScoreRect(122, 386, 283, 401);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontB, _scoringTextFinalScore, finalScoreRect.left, finalScoreRect.top, finalScoreRect.width(), finalScoreRect.height(), textColor, _fontHeightB, kTextAlignRight);
	}
}

bool CompletionWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void CompletionWindow::onTimer(uint timer) {
	switch (_status) {
	case 0:
		_currentSoundEffectID = _vm->_sound->playSoundEffect("BITDATA/FUTAPT/FA_COURT.BTA");
		_status = 1;
		_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "BITDATA/FUTAPT/CONGRATS.BTS" : "BITDATA/FUTAPT/CONGRAT8.BTS");
		invalidateWindow(false);
		break;
	case 2:
		if (!_gageVideo || _gageVideo->getMode() == VideoWindow::kModeStopped) {
			delete _gageVideo;
			_gageVideo = nullptr;

			_status = 3;
			_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "BITDATA/FUTAPT/ENDING24.BTS" : "BITDATA/FUTAPT/ENDING8.BTS");
			invalidateWindow(false);
			_vm->_sound->setAmbientSound("BITDATA/FUTAPT/FA_FIN.BTA");
		}
		break;
	}
}

void CompletionWindow::onLButtonUp(const Common::Point &point, uint flags) {
	switch (_status) {
	case 1:
		_vm->_sound->stopSoundEffect(_currentSoundEffectID);
		_currentSoundEffectID = -1;
		_status = 2;

		if (_background) {
			_background->free();
			delete _background;
			_background = nullptr;
		}

		invalidateWindow(false);

		_gageVideo = new VideoWindow(_vm, this);

		if (!_gageVideo->openVideo("BITDATA/FUTAPT/FA_FIN.BTV"))
			error("Failed to load finale video");

		_gageVideo->setWindowPos(nullptr, 104, 145, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
		_gageVideo->enableWindow(false);
		_gageVideo->showWindow(kWindowShow);
		_gageVideo->playVideo();
		break;
	case 2:
		if (!_gageVideo || _gageVideo->getMode() == VideoWindow::kModeStopped) {
			delete _gageVideo;
			_gageVideo = nullptr;

			_status = 4;
			_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "BITDATA/FUTAPT/ENDING24.BTS" : "BITDATA/FUTAPT/ENDING8.BTS");
			invalidateWindow(false);
		}
		break;
	case 3:
		((FrameWindow *)_parent)->showCredits();
		break;
	}
}

} // End of namespace Buried
