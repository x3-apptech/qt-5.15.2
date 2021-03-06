// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "headless/lib/headless_content_client.h"

#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

namespace headless {

HeadlessContentClient::HeadlessContentClient() = default;

HeadlessContentClient::~HeadlessContentClient() = default;

base::string16 HeadlessContentClient::GetLocalizedString(int message_id) {
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece HeadlessContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) {
  return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedMemory* HeadlessContentClient::GetDataResourceBytes(
    int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
      resource_id);
}

gfx::Image& HeadlessContentClient::GetNativeImageNamed(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(
      resource_id);
}

blink::OriginTrialPolicy* HeadlessContentClient::GetOriginTrialPolicy() {
  // Prevent initialization race (see crbug.com/721144). There may be a
  // race when the policy is needed for worker startup (which happens on a
  // separate worker thread).
  base::AutoLock auto_lock(origin_trial_policy_lock_);
  if (!origin_trial_policy_)
    origin_trial_policy_ = std::make_unique<HeadlessOriginTrialPolicy>();
  return origin_trial_policy_.get();
}

}  // namespace headless
