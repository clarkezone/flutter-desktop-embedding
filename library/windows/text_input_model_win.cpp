// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "text_input_model_win.h"

#include <iostream>

// TODO(awdavies): Need to fix this regarding issue #47.
static constexpr char kComposingBaseKey[] = "composingBase";

static constexpr char kComposingExtentKey[] = "composingExtent";

static constexpr char kSelectionAffinityKey[] = "selectionAffinity";
static constexpr char kAffinityDownstream[] = "TextAffinity.downstream";

static constexpr char kSelectionBaseKey[] = "selectionBase";
static constexpr char kSelectionExtentKey[] = "selectionExtent";

static constexpr char kSelectionIsDirectionalKey[] = "selectionIsDirectional";

static constexpr char kTextKey[] = "text";

static constexpr char kClientId[] = "clientid";

static constexpr char kEditingState[] = "editingState";


namespace flutter_desktop_embedding {

TextInputModelWin::TextInputModelWin(int client_id)
    : text_(L""),
      client_id_(client_id),
      selection_base_(text_.begin()),
      selection_extent_(text_.begin()) {}

TextInputModelWin::~TextInputModelWin() {}

bool TextInputModelWin::SetEditingState(size_t selection_base,
                                     size_t selection_extent,
                                     const std::wstring_view &text) {
  if (selection_base > selection_extent) {
    return false;
  }
  // Only checks extent since it is implicitly greater-than-or-equal-to base.
  if (selection_extent > text.size()) {
    return false;
  }
  text_ = std::wstring(text);
  selection_base_ = text_.begin() + selection_base;
  selection_extent_ = text_.begin() + selection_extent;
  return true;
}

void TextInputModelWin::DeleteSelected() {
  selection_base_ = text_.erase(selection_base_, selection_extent_);
  // Moves extent back to base, so that it is a single cursor placement again.
  selection_extent_ = selection_base_;
}

void TextInputModelWin::AddCharacter(char c) {
  if (selection_base_ != selection_extent_) {
    DeleteSelected();
  }
  selection_extent_ = text_.insert(selection_extent_, c);
  selection_extent_++;
  selection_base_ = selection_extent_;
}

bool TextInputModelWin::Backspace() {
  if (selection_base_ != selection_extent_) {
    DeleteSelected();
    return true;
  }
  if (selection_base_ != text_.begin()) {
    selection_base_ = text_.erase(selection_base_ - 1, selection_base_);
    selection_extent_ = selection_base_;
    return true;
  }
  return false;  // No edits happened.
}

bool TextInputModelWin::Delete() {
  if (selection_base_ != selection_extent_) {
    DeleteSelected();
    return true;
  }
  if (selection_base_ != text_.end()) {
    selection_base_ = text_.erase(selection_base_, selection_base_ + 1);
    selection_extent_ = selection_base_;
    return true;
  }
  return false;
}

void TextInputModelWin::MoveCursorToBeginning() {
  selection_base_ = text_.begin();
  selection_extent_ = text_.begin();
}

void TextInputModelWin::MoveCursorToEnd() {
  selection_base_ = text_.end();
  selection_extent_ = text_.end();
}

bool TextInputModelWin::MoveCursorForward() {
  // If about to move set to the end of the highlight (when not selecting).
  if (selection_base_ != selection_extent_) {
    selection_base_ = selection_extent_;
    return true;
  }
  // If not at the end, move the extent forward.
  if (selection_extent_ != text_.end()) {
    selection_extent_++;
    selection_base_++;
    return true;
  }
  return false;
}

bool TextInputModelWin::MoveCursorBack() {
  // If about to move set to the beginning of the highlight
  // (when not selecting).
  if (selection_base_ != selection_extent_) {
    selection_extent_ = selection_base_;
    return true;
  }
  // If not at the start, move the beginning backward.
  if (selection_base_ != text_.begin()) {
    selection_base_--;
    selection_extent_--;
    return true;
  }
  return false;
}

//TODO: can i make this an ijsonvalue?
IJsonValue TextInputModelWin::GetState() const {
  // TODO(awdavies): Most of these are hard-coded for now.
  JsonArray returnVal;
  JsonObject editing_state;
  editing_state.SetNamedValue(to_hstring(kComposingBaseKey), // to_hastring required as kComposingBaseKey is 8 bit char
                              JsonValue::CreateNumberValue(-1));
  editing_state.SetNamedValue(to_hstring(kComposingExtentKey),
                              JsonValue::CreateNumberValue(-1));
  editing_state.SetNamedValue(to_hstring(kSelectionAffinityKey),
      JsonValue::CreateStringValue(to_hstring(kAffinityDownstream)));
  editing_state.SetNamedValue(
      to_hstring(kSelectionBaseKey),
                              JsonValue::CreateNumberValue(static_cast<int>(
                                  selection_base_ - text_.begin())));
  editing_state.SetNamedValue(to_hstring(kSelectionExtentKey),
                              JsonValue::CreateNumberValue(static_cast<int>(
                                  selection_extent_ - text_.begin())));
  editing_state.SetNamedValue(to_hstring(kSelectionIsDirectionalKey),
                              JsonValue::CreateBooleanValue(false));
  editing_state.SetNamedValue(
      to_hstring(kTextKey),
                              JsonValue::CreateStringValue(text_));
 
  // TODO(stuartmorgan): Move client_id out up to the plugin so that this
  // function just returns the editing state.

  returnVal.Append(JsonValue::CreateNumberValue(client_id_));
  returnVal.Append(editing_state);
  return returnVal;
}

}  // namespace flutter_desktop_embedding
