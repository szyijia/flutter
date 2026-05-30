// Copyright 2024 Patchwing Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Stub declarations for Shorebird private dart-sdk symbols.

#ifndef FLUTTER_RUNTIME_SHOREBIRD_DART_STUBS_H_
#define FLUTTER_RUNTIME_SHOREBIRD_DART_STUBS_H_

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

int Shorebird_ReadLinkHeader(const uint8_t* data, size_t size);

void Shorebird_SetBaseSnapshots(const uint8_t* isolate_snapshot_data,
                                const uint8_t* isolate_snapshot_instructions,
                                const uint8_t* vm_snapshot_data,
                                const uint8_t* vm_snapshot_instructions);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // FLUTTER_RUNTIME_SHOREBIRD_DART_STUBS_H_

