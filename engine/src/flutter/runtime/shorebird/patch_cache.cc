// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Patchwing W5'-3 (v3.0): Stubbed implementation of PatchCacheEntry::Create.
//
// Upstream code path here calls:
//
//   * Shorebird_ReadLinkHeader(...)         — provided as a FATAL stub
//                                             in shorebird_dart_stubs.cc
//   * Dart_LoadELF(..., dart::bin::kReadOnly)
//                                           — Shorebird-private 9-arg
//                                             overload of the upstream
//                                             8-arg Dart_LoadELF; the
//                                             extra |mode| parameter
//                                             does not exist in vanilla
//                                             dart-sdk.
//
// Patchwing v3.0 drives patches via bsdiff full-replacement of
// libapp.so on Android, so the .vmcode loading path is NEVER reached
// at runtime — TryLoadFromPatch() below already bails out when the
// path does not end in ".vmcode". We therefore stub
// PatchCacheEntry::Create() with FML_LOG(FATAL) and remove the
// upstream Dart_LoadELF call altogether. See docs/W5_NOTES.md
// §"Future work: real .vmcode support" for the plan to restore the
// real implementation.

#include "flutter/runtime/shorebird/patch_cache.h"

#include <mutex>

#include "flutter/fml/logging.h"
#include "flutter/fml/mapping.h"
#include "flutter/runtime/shorebird/patch_mapping.h"

namespace flutter {

namespace {

// These symbol names match the constants in dart_snapshot.cc.
// We duplicate them here rather than extracting them into a header.
// They are actually defined down in Dart and will never change.
constexpr const char* kIsolateDataSymbol = "kDartIsolateSnapshotData";
constexpr const char* kIsolateInstructionsSymbol =
    "kDartIsolateSnapshotInstructions";

}  // namespace

// PatchCacheEntry implementation

std::shared_ptr<PatchCacheEntry> PatchCacheEntry::Create(
    const std::string& path) {
  // Patchwing v3.0 stub: see file header. The .vmcode loading code path
  // is unreachable on the release path because TryLoadFromPatch() bails
  // out for non-.vmcode inputs, and Patchwing always ships full
  // libapp.so via bsdiff (never .vmcode). If we ever land here it means
  // a caller bypassed TryLoadFromPatch(); fail loudly so it gets
  // noticed during dev rather than silently degrading.
  FML_LOG(FATAL) << "[patchwing] PatchCacheEntry::Create stub invoked for "
                 << path
                 << " — .vmcode loading is not supported in the "
                    "Patchwing v3.0 build (use bsdiff full-replacement "
                    "of libapp.so instead). See docs/W5_NOTES.md.";
  return nullptr;
}

PatchCacheEntry::PatchCacheEntry(const std::string& path,
                                 Dart_LoadedElf* elf,
                                 const uint8_t* isolate_data,
                                 const uint8_t* isolate_instrs)
    : path_(path),
      elf_(elf),
      isolate_data_(isolate_data),
      isolate_instrs_(isolate_instrs) {}

PatchCacheEntry::~PatchCacheEntry() {
  // In the Patchwing v3.0 stub build PatchCacheEntry instances are
  // never constructed (Create() always FATAL-aborts), so this dtor is
  // effectively unreachable. We still call Dart_UnloadELF for safety
  // in case a future change brings back real .vmcode loading.
  if (elf_ != nullptr) {
    FML_LOG(INFO) << "Unloading patch from " << path_;
    Dart_UnloadELF(elf_);
    elf_ = nullptr;
  }
}

PatchCache& PatchCache::Instance() {
  static PatchCache instance;
  return instance;
}

std::shared_ptr<PatchCacheEntry> PatchCache::GetOrLoad(
    const std::string& path) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Check if we have a cached entry that's still alive
  auto it = cache_.find(path);
  if (it != cache_.end()) {
    if (auto entry = it->second.lock()) {
      FML_LOG(INFO) << "PatchCache hit for " << path;
      return entry;
    }
    // Entry expired, remove it
    cache_.erase(it);
  }

  // Load a new entry
  auto entry = PatchCacheEntry::Create(path);
  if (entry) {
    cache_[path] = entry;  // Store weak_ptr
  }

  return entry;
}

void PatchCache::PruneExpired() {
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto it = cache_.begin(); it != cache_.end();) {
    if (it->second.expired()) {
      it = cache_.erase(it);
    } else {
      ++it;
    }
  }
}

std::shared_ptr<const fml::Mapping> TryLoadFromPatch(
    const std::vector<std::string>& native_library_paths,
    const char* symbol_name) {
  if (native_library_paths.empty()) {
    return nullptr;
  }

  // Check if the first path is a Shorebird patch (.vmcode file)
  const auto& patch_path = native_library_paths.front();
  bool is_patch = patch_path.find(".vmcode") != std::string::npos;
  if (!is_patch) {
    return nullptr;
  }

  // Patches only contain isolate data/instructions, not VM data/instructions.
  // Return nullptr for VM symbols to allow fallback to the base app.
  std::string symbol(symbol_name);
  if (symbol != kIsolateDataSymbol && symbol != kIsolateInstructionsSymbol) {
    return nullptr;
  }

  // Patchwing v3.0: reaching this point means a .vmcode file was
  // passed to the engine, which Patchwing does not support (we use
  // bsdiff full-replacement of libapp.so, not .vmcode). Fall through
  // into PatchCacheEntry::Create which will FML_LOG(FATAL).
  auto cache_entry = PatchCache::Instance().GetOrLoad(patch_path);
  if (!cache_entry) {
    FML_LOG(FATAL) << "Failed to load symbol from patch at " << patch_path;
    return nullptr;
  }

  FML_LOG(INFO) << "Loading symbol from patch: " << symbol_name;

  if (symbol == kIsolateDataSymbol) {
    return PatchMapping::CreateIsolateData(cache_entry);
  } else {
    FML_CHECK(symbol == kIsolateInstructionsSymbol);
    return PatchMapping::CreateIsolateInstructions(cache_entry);
  }
}

}  // namespace flutter
