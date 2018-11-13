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
#include "json_method_codec_win.h"

#include <iostream>

#include "json_method_call_win.h"

namespace flutter_desktop_embedding {

namespace {
// Keys used in MethodCall encoding.
constexpr char kMessageMethodKey[] = "method";
constexpr char kMessageArgumentsKey[] = "args";
}  // namespace

// static
const JsonMethodCodecWin &JsonMethodCodecWin::GetInstance() {
  static JsonMethodCodecWin sInstance;
  return sInstance;
}

std::unique_ptr<MethodCall> JsonMethodCodecWin::DecodeMethodCallInternal(
    const uint8_t *message, const size_t message_size) const {
  auto raw_message = reinterpret_cast<const char *>(message);

  JsonObject json_message{nullptr};

  std::string sizedBuffer;
  sizedBuffer.append(raw_message, message_size);

  auto raw = to_hstring(sizedBuffer);
  if (!JsonObject::TryParse(raw, json_message)) {
    std::cerr << "Unable to parse JSON method call." << std::endl;
    return nullptr;
  }

  auto method = json_message.GetNamedString(to_hstring(kMessageMethodKey));
  IJsonValue arguments;
  
  arguments = json_message.GetNamedValue(to_hstring(kMessageArgumentsKey));

  return std::make_unique<JsonMethodCallWin>(winrt::to_string(method),
                                             arguments);
}

std::unique_ptr<std::vector<uint8_t>>
JsonMethodCodecWin::EncodeMethodCallInternal(
    const MethodCall &method_call) const {
  JsonObject message;
  auto methodName = JsonValue::CreateStringValue(to_hstring(method_call.method_name().c_str()));

  message.SetNamedValue(to_hstring(kMessageMethodKey), methodName);
  message.SetNamedValue(to_hstring(kMessageArgumentsKey),
      *static_cast<const IJsonValue *> (method_call.arguments()));

  return EncodeJsonObject(message);

  /*Json::Value message(Json::objectValue);
  message[kMessageMethodKey] = method_call.method_name();
  message[kMessageArgumentsKey] =
      *static_cast<const Json::Value *>(method_call.arguments());

  return EncodeJsonObject(message);*/
}

std::unique_ptr<std::vector<uint8_t>>
JsonMethodCodecWin::EncodeSuccessEnvelopeInternal(const void *result) const {
  /* Json::Value envelope(Json::arrayValue);
   envelope.append(result == nullptr
                       ? Json::Value()
                       : *static_cast<const Json::Value *>(result));
                                           */
  JsonArray envelope;
  //envelope.Append(empty);

  return EncodeJsonObject(envelope);
}

std::unique_ptr<std::vector<uint8_t>>
JsonMethodCodecWin::EncodeErrorEnvelopeInternal(
    const std::string &error_code, const std::string &error_message,
    const void *error_details) const {
  /*Json::Value envelope(Json::arrayValue);
  envelope.append(error_code);
  envelope.append(error_message.empty() ? Json::Value() : error_message);
  envelope.append(error_details == nullptr
                      ? Json::Value()
                      : *static_cast<const Json::Value *>(error_details));
  return EncodeJsonObject(envelope);*/
  throw new std::exception();
}

std::unique_ptr<std::vector<uint8_t>> JsonMethodCodecWin::EncodeJsonObject(
    const IJsonValue &json) const {
  auto result = json.Stringify();
  
  std::string serialization;

  if (result.size() == 2) {
    serialization = "[null]";
  } else {
    serialization = to_string(result);
  }

   return std::make_unique<std::vector<uint8_t>>(serialization.begin(),
                                                    serialization.end());
}

}  // namespace flutter_desktop_embedding
