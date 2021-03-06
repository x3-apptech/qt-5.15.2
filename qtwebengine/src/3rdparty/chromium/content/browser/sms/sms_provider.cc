// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/command_line.h"

#include "build/build_config.h"
#include "content/browser/sms/sms_provider.h"
#include "content/public/common/content_switches.h"
#include "url/gurl.h"
#include "url/origin.h"
#if defined(OS_ANDROID)
#include "content/browser/sms/sms_provider_gms_user_consent.h"
#include "content/browser/sms/sms_provider_gms_verification.h"
#endif

namespace content {

class RenderFrameHost;

SmsProvider::SmsProvider() = default;
SmsProvider::~SmsProvider() = default;

// static
std::unique_ptr<SmsProvider> SmsProvider::Create(
    base::WeakPtr<RenderFrameHost> rfh) {
#if defined(OS_ANDROID)
  if (base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kWebOtpBackend) ==
      switches::kWebOtpBackendSmsVerification) {
    return std::make_unique<SmsProviderGmsVerification>();
  }
  return std::make_unique<SmsProviderGmsUserConsent>(std::move(rfh));
#else
  return nullptr;
#endif
}

void SmsProvider::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void SmsProvider::RemoveObserver(const Observer* observer) {
  observers_.RemoveObserver(observer);
}

void SmsProvider::NotifyReceive(const std::string& sms) {
  base::Optional<SmsParser::Result> result = SmsParser::Parse(sms);
  if (result)
    NotifyReceive(result->origin, result->one_time_code);
}

void SmsProvider::NotifyReceive(const url::Origin& origin,
                                const std::string& one_time_code) {
  for (Observer& obs : observers_) {
    bool handled = obs.OnReceive(origin, one_time_code);
    if (handled) {
      break;
    }
  }
}

bool SmsProvider::HasObservers() {
  return observers_.might_have_observers();
}

}  // namespace content
