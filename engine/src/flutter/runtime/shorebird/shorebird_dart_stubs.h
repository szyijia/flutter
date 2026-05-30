// Copyright 2024 Patchwing Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Patchwing W5' (v3.0): The Shorebird private dart-sdk fork exports two
// C-linkage symbols that vanilla upstream dart-sdk does not provide:
//
//   * Shorebird_SetBaseSnapshots — registers the base (unpatched) snapshot
//     mappings with the Shorebird-patched Dart VM so the VM can fall back
//     to them when interpreter-mode patches are loaded. iOS-only — guarded
//     by SHOREBIRD_USE_INTERPRETER at the call site.
//
//   * Shorebird_ReadLinkHeader — parses the Shorebird linker header that
//     prefixes a .vmcode patch file and returns the offset of the embedded
//     ELF blob inside the file.
//
// In the Patchwing v3.0 architecture we replace the Shorebird private
// dart-sdk fork with stock upstream dart-sdk (see vendor/flutter/DEPS) and
// drive patches via bsdiff full-replacement instead of dart-aware diff /
// .vmcode loading. Therefore these two functions are NEVER called at
// runtime on the Patchwing release path:
//
//   * Shorebird_SetBaseSnapshots: iOS interpreter-mode is disabled.
//   * Shorebird_ReadLinkHeader:   patch_cache::TryLoadFromPatch() bails
//                                 out early when the path does not end
//                                 in ".vmcode" (bsdiff path replaces the
//                                 whole libapp.so on Android, never goes
//                                 through .vmcode loading).
//
// However the symbols MUST exist at link time, otherwise libflutter.so
// fails to link. This header + shorebird_dart_stubs.cc provide the
// minimum declarations and weak no-op definitions to satisfy the linker
// while making accidental runtime invocation loud (FML_LOG(FATAL)).

#ifndef FLUTTER_RUNTIME_SHOREBIRD_SHOREBIRD_DART_STUBS_H_
#define FLUTTER_RUNTIME_SHOREBIRD_SHOREBIRD_DART_STUBS_H_

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Reads the Shorebird-specific linker header at the start of a .vmcode
// patch file and returns the byte offset of the embedded ELF image
// within |data|.
//
// Patchwing stub: returns 0 (i.e. "ELF starts at offset 0"); this is
// safe because Patchwing never invokes this function at runtime — the
// .vmcode loading path is unreachable when bsdiff full-replacement is
// the patch format.
int Shorebird_ReadLinkHeader(const uint8_t* data, size_t size);

// Registers the base (unpatched) isolate / VM snapshot mappings with
// the Shorebird-patched Dart VM. iOS-only; guarded at the call site by
// SHOREBIRD_USE_INTERPRETER, which is 0 on Android / Patchwing host
// builds.
//
// Patchwing stub: no-op; never invoked because SHOREBIRD_USE_INTERPRETER
// is disabled on every Patchwing target.
void Shorebird_SetBaseSnapshots(const uint8_t* isolate_snapshot_data,
                                const uint8_t* isolate_snapshot_instructions,
                                const uint8_t* vm_snapshot_data,
                                const uint8_t* vm_snapshot_instructions);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // FLUTTER_RUNTIME_SHOREBIRD_SHOREBIRD_DART_STUBS_H_
