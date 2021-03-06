/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieVector_DEFINED
#define SkottieVector_DEFINED

#include "modules/skottie/src/animator/Keyframe.h"

#include <vector>

namespace skottie {
namespace internal {

class VectorKeyframeAnimatorBuilder final : public KeyframeAnimatorBuilder {
public:
    using VectorLenParser  = bool(*)(const skjson::Value&, size_t*);
    using VectorDataParser = bool(*)(const skjson::Value&, size_t, float*);

    VectorKeyframeAnimatorBuilder(VectorLenParser, VectorDataParser);

    sk_sp<KeyframeAnimatorBase> make(const AnimationBuilder&,
                                     const skjson::ArrayValue&,
                                     void*) override;

private:
    bool parseValue(const AnimationBuilder&,
                    const skjson::Value&,
                    void*) const override;

    bool parseKFValue(const AnimationBuilder&,
                      const skjson::ObjectValue&,
                      const skjson::Value&,
                      Keyframe::Value*) override;

    const VectorLenParser  fParseLen;
    const VectorDataParser fParseData;

    std::vector<float>     fStorage;
    size_t                 fVecLen,         // size of individual vector values we store
                           fCurrentVec = 0; // vector value index being parsed (corresponding
                                            // storage offset is fCurrentVec * fVecLen)
};

}} // namespace skottie::internal

#endif // SkottieVector_DEFINED
