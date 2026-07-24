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
#include <cstring>

// [M3] Patchwing_ReadLinkHeader / Patchwing_SetBaseSnapshots /
// Patchwing_SetupLinkTables 的真实实现已迁入 dart-sdk fork
// （runtime/vm/patchwing/patchwing_runtime.cc），engine 构建随 fork 链接，
// 此处不再提供 stub（避免与 fork 侧符号重复定义）。

namespace {
// snapshot blob 头布局（与 runtime/vm/snapshot.h 一致）：
//   offset 0: int32  magic
//   offset 4: int64  length（含头的整个 blob 长度，小端）
//   offset 12: int64 kind
size_t ReadSnapshotLength(const uint8_t* data) {
  if (data == nullptr) {
    return 0;
  }
  int64_t length = 0;
  memcpy(&length, data + 4, sizeof(int64_t));  // 小端读取（目标平台均小端）
  return length > 0 ? static_cast<size_t>(length) : 0;
}
}  // namespace

extern "C" size_t Dart_SnapshotDataSize(const uint8_t* data) {
  return ReadSnapshotLength(data);
}

extern "C" size_t Dart_SnapshotInstrSize(const uint8_t* data) {
  return ReadSnapshotLength(data);
}

