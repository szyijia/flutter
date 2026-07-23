// Copyright 2024 Patchwing Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Stub implementations for Patchwing private dart-sdk symbols.
// These satisfy the linker when building with upstream dart-sdk.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

extern "C" int Patchwing_ReadLinkHeader(const uint8_t* /*data*/,
                                        size_t /*size*/) {
  fprintf(stderr, "[patchwing] FATAL: Patchwing_ReadLinkHeader stub invoked. "
                  "This should never happen in Patchwing builds.\n");
  abort();
  return 0;
}

extern "C" void Patchwing_SetBaseSnapshots(
    const uint8_t* /*isolate_snapshot_data*/,
    const uint8_t* /*isolate_snapshot_instructions*/,
    const uint8_t* /*vm_snapshot_data*/,
    const uint8_t* /*vm_snapshot_instructions*/) {
  fprintf(stderr, "[patchwing] FATAL: Patchwing_SetBaseSnapshots stub invoked. "
                  "This should never happen in Patchwing builds.\n");
  abort();
}

