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

// [M3] 把 vmcode 中的 LinkTable 拷贝进 VM 全局状态（在 ReadLinkHeader
// 之后、isolate 创建前调用；data 为整个 vmcode 映射）。成功返回 0。
int Patchwing_SetupLinkTables(const uint8_t* data, size_t size);

// Snapshot blob 的 size 访问（原版 dart 无此 API，engine 侧按 snapshot 头
// 格式实现：magic(int32) + length(int64 LE) + kind(int64)，length 为含头总长）。
// 对应 runtime/vm/snapshot.h 的 kMagicSize/kLengthOffset 布局。
size_t Dart_SnapshotDataSize(const uint8_t* data);
size_t Dart_SnapshotInstrSize(const uint8_t* data);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // FLUTTER_RUNTIME_PATCHWING_DART_STUBS_H_

