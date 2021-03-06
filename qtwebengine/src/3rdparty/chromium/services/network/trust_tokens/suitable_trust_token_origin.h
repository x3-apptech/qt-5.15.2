// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_TRUST_TOKENS_SUITABLE_TRUST_TOKEN_ORIGIN_H_
#define SERVICES_NETWORK_TRUST_TOKENS_SUITABLE_TRUST_TOKEN_ORIGIN_H_

#include "base/util/type_safety/pass_key.h"
#include "url/origin.h"

namespace network {

// Class SuitableTrustTokenOrigin is a thin wrapper over url::Origin enforcing
// invariants required of all origins suitable for keying persistent Trust
// Tokens state (https://github.com/wicg/trust-token-api). These origins must
// be:
// - potentially trustworthy, in the sense of
// network::IsOriginPotentiallyTrustworthy (this is a security requirement); and
// - either HTTP or HTTPS (this is so that the origins have unique
// serializations).
class SuitableTrustTokenOrigin {
 public:
  SuitableTrustTokenOrigin() = delete;
  ~SuitableTrustTokenOrigin();

  SuitableTrustTokenOrigin(const SuitableTrustTokenOrigin& rhs);
  SuitableTrustTokenOrigin& operator=(const SuitableTrustTokenOrigin& rhs);
  SuitableTrustTokenOrigin(SuitableTrustTokenOrigin&& rhs);
  SuitableTrustTokenOrigin& operator=(SuitableTrustTokenOrigin&& rhs);

  // Returns nullopt if |origin| (or |url|) is unsuitable for keying Trust
  // Tokens persistent state. Otherwise, returns a new SuitableTrustTokenOrigin
  // wrapping |origin| (or |url|).
  static base::Optional<SuitableTrustTokenOrigin> Create(url::Origin origin);
  static base::Optional<SuitableTrustTokenOrigin> Create(const GURL& url);

  std::string Serialize() const;
  const url::Origin& origin() const { return origin_; }

  // Constructs a SuitableTrustTokenOrigin from the given origin. Public only as
  // an implementation detail; clients should use |Create|.
  SuitableTrustTokenOrigin(util::PassKey<SuitableTrustTokenOrigin>,
                           url::Origin&& origin);

 private:
  friend bool operator==(const SuitableTrustTokenOrigin& lhs,
                         const SuitableTrustTokenOrigin& rhs);
  friend bool operator<(const SuitableTrustTokenOrigin& lhs,
                        const SuitableTrustTokenOrigin& rhs);
  url::Origin origin_;
};

inline bool operator==(const SuitableTrustTokenOrigin& lhs,
                       const SuitableTrustTokenOrigin& rhs) {
  return lhs.origin_ == rhs.origin_;
}

inline bool operator<(const SuitableTrustTokenOrigin& lhs,
                      const SuitableTrustTokenOrigin& rhs) {
  return lhs.origin_ < rhs.origin_;
}

}  // namespace network

#endif  // SERVICES_NETWORK_TRUST_TOKENS_SUITABLE_TRUST_TOKEN_ORIGIN_H_
