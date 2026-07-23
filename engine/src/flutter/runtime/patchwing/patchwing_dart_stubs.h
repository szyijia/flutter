// Copyright 2024 Patchwing Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Stub declarations for Patchwing private dart-sdk symbols.

#ifndef FLUTTER_RUNTIME_PATCHWING_DART_STUBS_H_
#define FLUTTER_RUNTIME_PATCHWING_DART_STUBS_H_

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

int Patchwing_ReadLinkHeader(const uint8_t* data, size_t size);

void Patchwing_SetBaseSnapshots(const uint8_t* isolate_snapshot_data,
                                const uint8_t* isolate_snapshot_instructions,
                                const uint8_t* vm_snapshot_data,
                                const uint8_t* vm_snapshot_instructions);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // FLUTTER_RUNTIME_PATCHWING_DART_STUBS_H_

