// Copyright 2024 Patchwing Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// See shorebird_dart_stubs.h for the rationale behind this file.

#include "flutter/runtime/shorebird/shorebird_dart_stubs.h"

#include "flutter/fml/logging.h"

extern "C" int Shorebird_ReadLinkHeader(const uint8_t* /*data*/,
                                        size_t /*size*/) {
  // Should never be reached on the Patchwing release path: bsdiff
  // full-replacement replaces libapp.so wholesale and never produces
  // .vmcode files, so patch_cache::TryLoadFromPatch() returns nullptr
  // before this function is called. Log loudly if invoked, then return
  // 0 (best-effort: treat the whole input buffer as raw ELF).
  FML_LOG(FATAL) << "[patchwing] Shorebird_ReadLinkHeader stub invoked: "
                    ".vmcode loading is not supported in the Patchwing "
                    "build (use bsdiff full-replacement instead).";
  return 0;
}

extern "C" void Shorebird_SetBaseSnapshots(
    const uint8_t* /*isolate_snapshot_data*/,
    const uint8_t* /*isolate_snapshot_instructions*/,
    const uint8_t* /*vm_snapshot_data*/,
    const uint8_t* /*vm_snapshot_instructions*/) {
  // Should never be reached: this symbol is only referenced from
  // shell/common/shorebird/shorebird.cc inside an
  // `#if SHOREBIRD_USE_INTERPRETER` block, and Patchwing never enables
  // SHOREBIRD_USE_INTERPRETER. Provided purely to satisfy the linker
  // for any unforeseen callers.
  FML_LOG(FATAL) << "[patchwing] Shorebird_SetBaseSnapshots stub invoked: "
                    "iOS interpreter-mode patches are not supported in "
                    "the Patchwing build.";
}
