# Patchwing Engine 多版本编译指南

> 本文档以 3.44.0 为模板，详细说明各版本编译的差异和处理方式。

## 一、版本分类

| 版本范围 | 类型 | 源码仓库 | 目录结构 | Dart SDK |
|---------|------|---------|---------|----------|
| 3.44.0+ | Monorepo (新) | `szyijia/flutter` | `repo/engine/src/flutter/` | 公开上游 |
| 3.29~3.41.8 | Monorepo (旧) | `szyijia/flutter` | `repo/engine/src/flutter/` | 公开上游 |
| 3.10~3.27 | Legacy | `szyijia/engine` | `repo/src/flutter/` | 公开上游 (已修改) |

## 二、3.44.0 编译流程（模板/参考）

### 2.1 源码结构

```
repo/                          ← .gclient 所在目录
├── .gclient
├── DEPS                       ← 顶层 DEPS（引用 engine/src/flutter）
├── engine/
│   └── src/
│       ├── flutter/           ← engine 源码（GN 根目录 = engine/src/）
│       │   ├── tools/gn       ← GN 配置脚本
│       │   ├── shell/
│       │   │   ├── common/shorebird/
│       │   │   │   ├── BUILD.gn
│       │   │   │   ├── build_rust_updater.gni  ← ✅ 3.44.0 新增！
│       │   │   │   ├── build_rust_updater.py   ← ✅ 3.44.0 新增！
│       │   │   │   ├── shorebird.cc
│       │   │   │   └── updater.cc
│       │   │   └── platform/android/BUILD.gn
│       │   ├── runtime/shorebird/
│       │   │   ├── BUILD.gn
│       │   │   ├── shorebird_dart_stubs.cc     ← Patchwing 添加
│       │   │   └── shorebird_dart_stubs.h
│       │   └── third_party/updater/            ← Rust updater 源码
│       │       ├── Cargo.toml
│       │       └── library/
│       └── out/
│           ├── android_release_arm64/
│           └── host_release/
```

### 2.2 关键特性

1. **Rust updater 自动编译**：`build_rust_updater.gni` + `build_rust_updater.py` 集成到 GN 中
   - GN action `build_rust_updater` 在 ninja 编译时自动调用 `cargo build`
   - 输出路径：`$root_out_dir/cargo_target/<rust_target>/release/libupdater.a`
   - 不再需要手动 `cargo build`

2. **Dart SDK**：使用公开上游 `https://dart.googlesource.com/sdk.git`
   - DEPS 中 `dart_sdk_git` 已改为公开地址
   - 添加了 `shorebird_dart_stubs.cc` 作为备用（当前未被 BUILD.gn 引用）

3. **updater 路径**：
   - DEPS: `engine/src/flutter/third_party/updater`
   - BUILD.gn 引用: `//flutter/third_party/updater/target/...`（GN `//` = `engine/src/`）
   - 实际由 GN action 编译到 `$root_out_dir/cargo_target/` 下

4. **编译命令**：
   ```bash
   cd repo/engine/src/flutter
   python3 tools/gn --android --android-cpu=arm64 --runtime-mode=release --no-goma
   ninja -C ../out/android_release_arm64 flutter/lib/snapshot:generate_snapshot_bin flutter.jar
   
   python3 tools/gn --runtime-mode=release --no-goma
   ninja -C ../out/host_release gen_snapshot
   ```

### 2.3 Patchwing 对 3.44.0 的修改（相对于 shorebirdtech/flutter）

| 文件 | 修改内容 |
|------|---------|
| `DEPS` | `dart_sdk_git`: `git@github.com:shorebirdtech/dart-sdk.git` → `https://dart.googlesource.com/sdk.git` |
| `DEPS` | `dart_sdk_revision`: 更新为对应的公开上游 commit |
| `engine/src/flutter/runtime/shorebird/shorebird_dart_stubs.cc` | 新增：stub 实现（备用） |
| `engine/src/flutter/runtime/shorebird/shorebird_dart_stubs.h` | 新增：stub 声明（备用） |

---

## 三、3.29~3.41.8 编译流程（Monorepo 旧版本）

### 3.1 与 3.44.0 的差异

| 对比项 | 3.44.0 | 3.29~3.41.8 |
|--------|--------|-------------|
| Rust updater 编译 | GN 自动（`build_rust_updater.gni`） | **需手动 `cargo build`** |
| updater 输出路径 | `$root_out_dir/cargo_target/` | `third_party/updater/target/` (源码树内) |
| BUILD.gn 引用方式 | 通过 GN action 依赖 | 硬编码路径 `//flutter/third_party/updater/target/...` |
| Dart SDK | 公开上游 | 公开上游（已修改） |

### 3.2 源码结构

与 3.44.0 相同的 monorepo 结构，但**没有** `build_rust_updater.gni` 和 `build_rust_updater.py`。

### 3.3 关键差异：updater 路径

```
DEPS 中: engine/src/flutter/third_party/updater
BUILD.gn 引用: //flutter/third_party/updater/target/<rust_target>/release/libupdater.a
```

GN 的 `//` = `engine/src/`，所以：
- `//flutter/third_party/updater/target/aarch64-linux-android/release/libupdater.a`
- = `engine/src/flutter/third_party/updater/target/aarch64-linux-android/release/libupdater.a`

### 3.4 编译步骤

```bash
# 1. 手动编译 Rust updater for Android
cd repo/engine/src/flutter/third_party/updater

# 配置 NDK 环境
export CC_aarch64_linux_android="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang"
export AR_aarch64_linux_android="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar"
export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER="$CC_aarch64_linux_android"

rustup target add aarch64-linux-android
cargo build --release --target aarch64-linux-android -p updater

# 验证输出
ls target/aarch64-linux-android/release/libupdater.a

# 2. 编译 Android engine
cd repo/engine/src/flutter
python3 tools/gn --android --android-cpu=arm64 --runtime-mode=release --no-goma
ninja -C ../out/android_release_arm64 flutter/lib/snapshot:generate_snapshot_bin flutter.jar

# 3. 手动编译 Rust updater for host
cd repo/engine/src/flutter/third_party/updater
rustup target add x86_64-unknown-linux-gnu  # 或 aarch64-apple-darwin
cargo build --release --target x86_64-unknown-linux-gnu -p updater

# 4. 编译 host gen_snapshot
cd repo/engine/src/flutter
python3 tools/gn --runtime-mode=release --no-goma
ninja -C ../out/host_release gen_snapshot
```

### 3.5 Patchwing 对 3.29~3.41.8 的修改

| 文件 | 修改内容 |
|------|---------|
| `DEPS` | `dart_sdk_git`: 改为公开上游 |
| `DEPS` | `dart_sdk_revision`: 更新为对应的公开上游 commit |

> 注意：3.29+ 版本的 shorebirdtech/flutter 中 `dart_sdk_git` 可能已经是公开的，
> 需要逐版本确认。

---

## 四、3.10~3.27 编译流程（Legacy 独立 engine 仓库）

### 4.1 与 3.44.0 的差异

| 对比项 | 3.44.0 | 3.10~3.27 |
|--------|--------|-----------|
| 源码仓库 | `szyijia/flutter` (monorepo) | `szyijia/engine` (独立) |
| .gclient name | `.` | `src/flutter` |
| engine 源码路径 | `repo/engine/src/flutter/` | `repo/src/flutter/` |
| GN 根目录 (`//`) | `engine/src/` | `src/` |
| updater DEPS 路径 | `engine/src/flutter/third_party/updater` | `src/third_party/updater` |
| updater BUILD.gn 引用 | `//flutter/third_party/updater/target/...` | `//third_party/updater/target/...` |
| Rust updater 编译 | GN 自动 | **需手动 `cargo build`** |
| Dart SDK | 公开上游 | 公开上游（已修改） |

### 4.2 源码结构

```
repo/                          ← .gclient 所在目录
├── .gclient
├── src/
│   ├── flutter/               ← engine 源码（checkout 自 szyijia/engine）
│   │   ├── DEPS              ← engine 的 DEPS
│   │   ├── tools/gn
│   │   ├── shell/
│   │   │   ├── common/shorebird/
│   │   │   │   ├── BUILD.gn  ← 无 build_rust_updater.gni
│   │   │   │   └── shorebird.cc
│   │   │   └── platform/android/BUILD.gn
│   │   └── updater/          ← C 头文件目录（include_dirs）
│   ├── third_party/
│   │   └── updater/           ← ⚠️ Rust updater 源码（注意：在 src/ 下，不在 src/flutter/ 下！）
│   │       ├── Cargo.toml
│   │       └── library/
│   └── out/
│       ├── android_release_arm64/
│       └── host_release/
```

### 4.3 关键差异：updater 路径

```
DEPS 中: src/third_party/updater
BUILD.gn 引用: //third_party/updater/target/<rust_target>/release/libupdater.a
```

GN 的 `//` = `src/`，所以：
- `//third_party/updater/target/aarch64-linux-android/release/libupdater.a`
- = `src/third_party/updater/target/aarch64-linux-android/release/libupdater.a`

⚠️ **这是之前 3.24.5 编译失败的根因**：workflow 在 `src/flutter/` 下查找 `third_party/updater`（不存在），
实际路径是 `src/third_party/updater`。

### 4.4 .gclient 配置

```python
solutions = [
  {
    "name": "src/flutter",
    "url": "https://github.com/szyijia/engine.git@flutter_release/3.24.5",
    "managed": False,
    "custom_deps": {},
    "custom_vars": {
      "download_emsdk": False,
      "release_candidate": False,
      "download_android_deps": True,
    },
  },
]
```

### 4.5 编译步骤

```bash
# 1. 手动编译 Rust updater for Android
cd repo/src/third_party/updater  # ⚠️ 注意：是 src/third_party/，不是 src/flutter/third_party/

# 配置 NDK 环境
export CC_aarch64_linux_android="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang"
export AR_aarch64_linux_android="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar"
export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER="$CC_aarch64_linux_android"

rustup target add aarch64-linux-android
cargo build --release --target aarch64-linux-android -p updater

# 验证输出
ls target/aarch64-linux-android/release/libupdater.a

# 2. 编译 Android engine
cd repo/src/flutter
python3 tools/gn --android --android-cpu=arm64 --runtime-mode=release --no-goma
ninja -C ../out/android_release_arm64 flutter/lib/snapshot:generate_snapshot_bin flutter.jar

# 3. 手动编译 Rust updater for host
cd repo/src/third_party/updater
rustup target add x86_64-unknown-linux-gnu  # 或 aarch64-apple-darwin
cargo build --release --target x86_64-unknown-linux-gnu -p updater

# 4. 编译 host gen_snapshot
cd repo/src/flutter
python3 tools/gn --runtime-mode=release --no-goma
ninja -C ../out/host_release gen_snapshot
```

### 4.6 Patchwing 对 3.10~3.27 的修改（在 szyijia/engine 仓库中）

| 文件 | 修改内容 |
|------|---------|
| `DEPS` | `dart_sdk_git`: `git@github.com:shorebirdtech/dart-sdk.git` → `https://dart.googlesource.com/sdk.git` |
| `DEPS` | `dart_sdk_revision`: 更新为对应的公开上游 commit |
| `DEPS` | **perfetto URL 修复（3.10~3.19）**：`fuchsia_git + "/third_party/android.googlesource.com/platform/external/perfetto"` → `flutter_git + "/third_party/perfetto"`（commit `b8da0709...` 保持不变，flutter mirror 中可获取） |
| `BUILD.gn` / `lib/snapshot/BUILD.gn` | 将 `analyze_snapshot` 的引用包裹在 `if (host_os == "linux")` 中（修复 macOS host_release_arm64 GN 错误） |
| `shell/common/shorebird/snapshots_data_handle.cc` | 将 `DataMapping` / `InstructionsMapping` 替换为 `FML_LOG(FATAL)` stub（消除对 `Dart_SnapshotDataSize` / `Dart_SnapshotInstrSize` 的依赖） |
| `shell/common/shorebird/shorebird.cc` 等 | 添加 `shorebird_dart_stubs` include / stub 实现（消除 `Shorebird_SetBaseSnapshots` 等私有符号未定义） |

> 修复脚本：`scripts/fix_perfetto_deps.py`、`scripts/fix_analyze_snapshot_legacy.py`、`scripts/fix_snapshots_data_handle.py`

---

## 五、Workflow 统一目录结构

为了让 `build-all-versions.yml` 能统一处理打包逻辑，legacy 版本会创建软链接：

```bash
# Legacy 结构: repo/src/flutter/ → 创建软链接
mkdir -p repo/engine
ln -sf "$(pwd)/repo/src" repo/engine/src
```

这样打包时可以统一使用 `repo/engine/src/out/` 或 `repo/src/out/` 路径。

---

## 六、各版本 Dart SDK 处理

### 问题背景

Shorebirdtech 原始仓库使用私有 dart-sdk（`git@github.com:shorebirdtech/dart-sdk.git`），
其中添加了一些私有 API（如 `Shorebird_ReadLinkHeader`）。

Patchwing 使用公开上游 dart-sdk，需要：
1. 修改 DEPS 中的 `dart_sdk_git` 为公开地址
2. 找到对应的公开 commit（`dart_sdk_revision`）
3. 如果有链接错误，添加 stub 实现

### 各版本处理方式

| 版本 | 原始 dart_sdk_git | 是否需要修改 | 是否需要 stubs |
|------|------------------|-------------|---------------|
| 3.44.0 | `git@github.com:shorebirdtech/dart-sdk.git` | ✅ 已修改 | 已添加（备用） |
| 3.29~3.41.8 | `https://dart.googlesource.com/sdk.git` | ❌ 已是公开 | 不需要 |
| 3.10~3.27 | `git@github.com:shorebirdtech/dart-sdk.git` | ✅ 已修改 | 不需要（符号由 updater 库提供） |

---

## 七、产物说明

每个版本编译后产出：

```
patchwing-engine-<version>-<platform>.zip
├── android_release_arm64/
│   └── flutter.jar            ← Android arm64 release engine
├── host_release/
│   └── gen_snapshot           ← 宿主机 AOT 编译器
└── patchwing.json             ← 元数据（版本、hash、时间等）
```

### 平台支持

| 平台 | Runner | host_out_dir | 用途 |
|------|--------|-------------|------|
| linux-x64 | ubuntu-latest | `host_release` | Linux 开发机 |
| darwin-arm64 | macos-14 | `host_release_arm64` | macOS Apple Silicon |

---

## 八、常见问题

### Q1: ninja 报错 `libupdater.a missing and no known rule to make it`

**原因**：旧版本（3.29~3.41.8 或 3.10~3.27）的 BUILD.gn 硬编码了 `libupdater.a` 路径，
但 GN/Ninja 不会自动编译 Rust 代码。

**解决**：在 `ninja` 之前手动执行 `cargo build`，确保 `libupdater.a` 存在于正确路径。

**路径对照**：
- Monorepo (3.29+): `repo/engine/src/flutter/third_party/updater/target/<rust_target>/release/libupdater.a`
- Legacy (3.10~3.27): `repo/src/third_party/updater/target/<rust_target>/release/libupdater.a`

### Q2: gclient sync 失败，提示 dart-sdk 无法访问

**原因**：DEPS 中 `dart_sdk_git` 仍指向 `git@github.com:shorebirdtech/dart-sdk.git`（私有仓库）。

**解决**：修改对应分支的 DEPS，将 `dart_sdk_git` 改为 `https://dart.googlesource.com/sdk.git`，
并更新 `dart_sdk_revision` 为对应的公开上游 commit。

### Q3: 链接错误 `undefined reference to Shorebird_ReadLinkHeader`

**原因**：使用公开 dart-sdk 后，某些 Shorebird 私有符号不存在。

**解决**：
- 如果 updater 库正常编译，这些符号由 `libupdater.a` 提供，不会出现此错误
- 如果确实缺失，添加 `shorebird_dart_stubs.cc` 并在 BUILD.gn 中引用

### Q4: 如何确定某个版本的 dart_sdk_revision？

1. 查看 shorebirdtech 原始 DEPS 中的 `dart_sdk_revision`
2. 在 shorebirdtech/dart-sdk 中找到该 commit
3. 查看该 commit 对应的上游 base commit（通常在 commit message 中有说明）
4. 或者使用 Flutter 官方 release 对应的 dart-sdk commit

### Q5: gclient sync 失败：`fatal: repository 'https://fuchsia.googlesource.com/third_party/android.googlesource.com/platform/external/perfetto/' not found`

**报错示例**：
```
fatal: repository 'https://fuchsia.googlesource.com/third_party/android.googlesource.com/platform/external/perfetto/' not found
Error: Command 'git ... fetch origin b8da07095979310818f0efde2ef3c69ea70d62c5 ...' returned non-zero exit status 128 in .../src/third_party/perfetto
```

**原因**：3.10.x ~ 3.19.x（legacy）的 DEPS 文件中 `src/third_party/perfetto` 使用了 `fuchsia_git + "/third_party/android.googlesource.com/platform/external/perfetto"`，该 Google 镜像已被删除（HTTP 404）。3.22.0+ 已迁移到 `flutter_git + "/third_party/perfetto"`。

**解决**：将 DEPS 中的 perfetto URL 改为 flutter mirror（commit hash 保持不变，`b8da0709...` 在 flutter mirror 中存在）：

```python
# 修复前
'src/third_party/perfetto':
  Var('fuchsia_git') + "/third_party/android.googlesource.com/platform/external/perfetto"
  + '@' + Var('dart_perfetto_rev'),

# 修复后
'src/third_party/perfetto':
  Var('flutter_git') + "/third_party/perfetto"
  + '@' + Var('dart_perfetto_rev'),
```

**修复脚本**：`scripts/fix_perfetto_deps.py`

**已修复版本**：`szyijia/engine` 的 `flutter_release/3.10.0` ~ `flutter_release/3.19.6`（含 `3.19.0-0.4.pre`），共 30 个分支。3.22.0+ 无需修复。

**额外说明**：如果 gclient 已经创建了 `_bad_scm/...perfetto<random>` 失败副本目录，需要在 runner 上清理或重置 workspace 后重跑。

---

## 九、版本检测逻辑

Workflow 中通过版本号的 minor 部分判断类型：

```bash
MINOR=$(echo "$FLUTTER_VERSION" | cut -d. -f2)
if [ "$MINOR" -ge 29 ]; then
  engine_type="monorepo"   # 3.29+
else
  engine_type="legacy"     # 3.10~3.27
fi
```

然后通过文件存在性判断是否需要手动编译 updater：

```bash
if [ ! -f "shell/common/shorebird/build_rust_updater.gni" ]; then
  # 旧版本，需要手动 cargo build
fi
```
