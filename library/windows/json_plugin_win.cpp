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
#include "json_plugin_win.h"

#include "json_method_codec_win.h"

namespace flutter_desktop_embedding {

JsonPluginWin::JsonPluginWin(const std::string &channel, bool input_blocking)
    : Plugin(channel, input_blocking) {}

JsonPluginWin::~JsonPluginWin() {}

const MethodCodec &JsonPluginWin::GetCodec() const {
  return JsonMethodCodecWin::GetInstance();
}

void JsonPluginWin::HandleMethodCall(const MethodCall &method_call,
                                  std::unique_ptr<MethodResult> result) {
  HandleJsonMethodCall(dynamic_cast<const JsonMethodCallWin &>(method_call),
                       std::move(result));
}

void JsonPluginWin::InvokeMethod(const std::string &method,
                              const JsonObject &arguments) {
  InvokeMethodCall(JsonMethodCallWin(method, arguments));
}

}  // namespace flutter_desktop_embedding
