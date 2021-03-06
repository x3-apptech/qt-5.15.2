// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/execution/protectors.h"

#include "src/execution/isolate-inl.h"
#include "src/execution/protectors-inl.h"
#include "src/handles/handles-inl.h"
#include "src/objects/contexts.h"
#include "src/objects/property-cell.h"
#include "src/objects/smi.h"
#include "src/tracing/trace-event.h"
#include "src/utils/utils.h"

namespace v8 {
namespace internal {

namespace {
void TraceProtectorInvalidation(const char* protector_name) {
  DCHECK(FLAG_trace_protector_invalidation);
  static constexpr char kInvalidateProtectorTracingCategory[] =
      "V8.InvalidateProtector";
  static constexpr char kInvalidateProtectorTracingArg[] = "protector-name";

  DCHECK(FLAG_trace_protector_invalidation);

  // TODO(jgruber): Remove the PrintF once tracing can output to stdout.
  i::PrintF("Invalidating protector cell %s", protector_name);
  TRACE_EVENT_INSTANT1("v8", kInvalidateProtectorTracingCategory,
                       TRACE_EVENT_SCOPE_THREAD, kInvalidateProtectorTracingArg,
                       protector_name);
}
}  // namespace

#define INVALIDATE_PROTECTOR_ON_ISOLATE_DEFINITION(name, unused_index, cell) \
  void Protectors::Invalidate##name(Isolate* isolate) {                      \
    DCHECK(isolate->factory()->cell()->value().IsSmi());                     \
    DCHECK(Is##name##Intact(isolate));                                       \
    if (FLAG_trace_protector_invalidation) {                                 \
      TraceProtectorInvalidation(#name);                                     \
    }                                                                        \
    PropertyCell::SetValueWithInvalidation(                                  \
        isolate, #cell, isolate->factory()->cell(),                          \
        handle(Smi::FromInt(kProtectorInvalid), isolate));                   \
    DCHECK(!Is##name##Intact(isolate));                                      \
  }
DECLARED_PROTECTORS_ON_ISOLATE(INVALIDATE_PROTECTOR_ON_ISOLATE_DEFINITION)
#undef INVALIDATE_PROTECTOR_ON_ISOLATE_DEFINITION

}  // namespace internal
}  // namespace v8
