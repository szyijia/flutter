// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Patchwing W5'-3 (v3.0): Stubbed implementation.
//
// Upstream code path here calls Shorebird-private Dart C APIs that do
// NOT exist in the vanilla upstream dart-sdk:
//
//   * Dart_SnapshotDataSize(const uint8_t*)
//   * Dart_SnapshotInstrSize(const uint8_t*)
//
// In the Patchwing v3.0 architecture we drive patches via bsdiff
// full-replacement of libapp.so on Android, so the .vmcode loading
// path (PatchCacheEntry::Create / PatchMapping::Create*) is NEVER
// reached at runtime — TryLoadFromPatch() in patch_cache.cc bails out
// early when the path does not end in ".vmcode".
//
// We therefore stub both factory methods with FML_LOG(FATAL) to satisfy
// the linker while making any accidental runtime invocation loud. See
// docs/W5_NOTES.md §"Future work: real .vmcode support" for the plan
// to restore the real implementation.

#include "flutter/runtime/shorebird/patch_mapping.h"

#include "flutter/fml/logging.h"

namespace flutter {

std::shared_ptr<PatchMapping> PatchMapping::CreateIsolateData(
    std::shared_ptr<PatchCacheEntry> /*entry*/) {
  FML_LOG(FATAL) << "[patchwing] PatchMapping::CreateIsolateData stub "
                    "invoked: .vmcode loading is not supported in the "
                    "Patchwing v3.0 build (use bsdiff full-replacement "
                    "instead). See docs/W5_NOTES.md.";
  return nullptr;
}

std::shared_ptr<PatchMapping> PatchMapping::CreateIsolateInstructions(
    std::shared_ptr<PatchCacheEntry> /*entry*/) {
  FML_LOG(FATAL) << "[patchwing] PatchMapping::CreateIsolateInstructions "
                    "stub invoked: .vmcode loading is not supported in "
                    "the Patchwing v3.0 build (use bsdiff full-"
                    "replacement instead). See docs/W5_NOTES.md.";
  return nullptr;
}

PatchMapping::PatchMapping(std::shared_ptr<PatchCacheEntry> entry,
                           const uint8_t* data,
                           size_t size)
    : cache_entry_(std::move(entry)), data_(data), size_(size) {}

PatchMapping::~PatchMapping() = default;

size_t PatchMapping::GetSize() const {
  return size_;
}

const uint8_t* PatchMapping::GetMapping() const {
  return data_;
}

bool PatchMapping::IsDontNeedSafe() const {
  // Patch mappings are file-backed and safe for madvise(DONTNEED).
  return true;
}

}  // namespace flutter
