# Patchwing Engine 本地编译指南（macOS）

> 本文档说明如何在本机 macOS 上编译 `vendor/flutter` 的 engine，生成 `pw release` / `pw patch` 所需的产物。

## 一、前置条件

### 1.1 目录结构

```
/Volumes/data/git/flutter_update/
├── vendor/flutter/                    ← 魔改后的 Flutter SDK（含 engine 源码）
│   ├── engine/src/                    ← engine 编译根目录
│   │   ├── flutter/                   ← engine 源码（GN 根 = engine/src/）
│   │   │   ├── tools/gn              ← GN 配置脚本
│   │   │   ├── shell/platform/android/io/flutter/embedding/engine/FlutterJNI.java
│   │   │   └── third_party/
│   │   │       ├── android_tools/     ← Android SDK + NDK
│   │   │       └── updater/           ← Rust updater 源码
│   │   └── out/
│   │       ├── android_release_arm64/ ← Android arm64 编译产物
│   │       └── host_release/          ← macOS host 编译产物
│   └── packages/flutter_tools/        ← Flutter 构建工具（Dart）
├── new/shorebird/                     ← Patchwing CLI 源码
└── cli_legacy/testapp/                ← 测试应用
```

### 1.2 工具依赖

| 工具 | 来源 | 说明 |
|------|------|------|
| ninja | `engine/src/flutter/third_party/android_tools/sdk/cmake/3.22.1/bin/ninja` | 编译系统 |
| python3 | 系统自带 | GN 配置脚本 |
| javac | 系统 JDK | 编译 Java embedding |
| rustup + cargo | 系统安装 | 编译 Rust updater |

### 1.3 环境变量（可选）

```bash
# 方便后续命令使用
export ENGINE_SRC=/Volumes/data/git/flutter_update/vendor/flutter/engine/src
export NINJA=$ENGINE_SRC/flutter/third_party/android_tools/sdk/cmake/3.22.1/bin/ninja
```

---

## 二、编译步骤

### 2.1 完整编译（首次或 GN 配置变更后）

```bash
cd $ENGINE_SRC/flutter

# 1. 配置 GN（Android arm64 release）
python3 tools/gn --android --android-cpu=arm64 --runtime-mode=release --no-goma

# 2. 编译完整 engine（耗时较长，约 30-60 分钟）
$NINJA -C ../out/android_release_arm64 flutter/lib/snapshot:generate_snapshot_bin flutter.jar

# 3. 配置 GN（Host release，用于 gen_snapshot）
python3 tools/gn --runtime-mode=release --no-goma

# 4. 编译 host gen_snapshot
$NINJA -C ../out/host_release gen_snapshot
```

### 2.2 增量编译（仅修改 Java 源码后）

当只修改了 `FlutterJNI.java` 等 Java 文件时，只需增量编译 embedding jar：

```bash
cd $ENGINE_SRC

# 1. 编译 Java 源码
$NINJA -C out/android_release_arm64 flutter/shell/platform/android:flutter_shell_java

# 2. 打包 embedding jar（含 pom 和 metadata）
$NINJA -C out/android_release_arm64 flutter/shell/platform/android:embedding_jars
```

**耗时**：约 5-10 秒（增量编译）

### 2.3 增量编译（仅修改 C++ 源码后）

当修改了 `flutter_main.cc`、`shorebird.cc` 等 native 代码时：

```bash
cd $ENGINE_SRC

# 重新编译 libflutter.so 和打包
$NINJA -C out/android_release_arm64 flutter/shell/platform/android:flutter
```

---

## 三、编译产物说明

### 3.1 关键产物路径

| 产物 | 路径 | 用途 |
|------|------|------|
| `flutter_embedding_release.jar` | `out/android_release_arm64/` | Java embedding 层（含 FlutterJNI） |
| `flutter_embedding_release.pom` | `out/android_release_arm64/` | Maven POM 文件 |
| `arm64_v8a_release.jar` | `out/android_release_arm64/` | libflutter.so 的 Maven 包装 |
| `libflutter.so` | `out/android_release_arm64/lib.stripped/` | stripped 版本的 engine .so |
| `gen_snapshot` | `out/host_release/` | AOT 编译器（macOS host） |
| `gen_snapshot` | `out/android_release_arm64/clang_arm64/` | AOT 编译器（cross-compile 版） |

### 3.2 产物如何被使用

`pw release` 调用 `flutter build appbundle --local-engine=...` 时：
1. Flutter 工具读取 `out/android_release_arm64/flutter_embedding_release.pom` 获取版本号
2. 创建临时 Maven 仓库，将 `flutter_embedding_release.jar` 和 `arm64_v8a_release.jar` 符号链接进去
3. Gradle 从该临时仓库解析 `io.flutter:flutter_embedding_release:$version` 依赖
4. 最终 APK 中包含正确的 `libflutter.so` 和 Java embedding 代码

---

## 四、验证编译结果

### 4.1 验证 embedding jar 中的 FlutterJNI

```bash
# 解压并反编译 FlutterJNI.class
cd /tmp && rm -rf verify && mkdir verify && cd verify
jar xf $ENGINE_SRC/out/android_release_arm64/flutter_embedding_release.jar \
    io/flutter/embedding/engine/FlutterJNI.class
javap -c -p io/flutter/embedding/engine/FlutterJNI.class | grep "yaml"
```

**期望输出**（应全部为 `patchwing.yaml`）：
```
92: ldc  #141  // String flutter_assets/patchwing.yaml
169: ldc  #175  // String patchwing.yaml:
192: ldc  #182  // String Failed to load patchwing.yaml
201: ldc  #184  // String Did you remember to include patchwing.yaml in your pubspec.yaml's assets?
```

### 4.2 验证 libflutter.so 中的 updater 符号

```bash
nm $ENGINE_SRC/out/android_release_arm64/libflutter.so | grep -i shorebird | head -10
```

**期望输出**：应包含 `shorebird_init`、`shorebird_check_for_downloadable_update` 等符号。

---

## 五、常见问题

### Q1: ninja 找不到

depot_tools 的 ninja 包装器可能找不到实际二进制。直接使用 Android SDK 中的 ninja：

```bash
export NINJA=$ENGINE_SRC/flutter/third_party/android_tools/sdk/cmake/3.22.1/bin/ninja
```

### Q2: 编译后 `pw release` 仍报 Gradle 错误

确保清理 Gradle 缓存中的旧 jar：

```bash
# 清理 testapp 的 build 缓存
cd /Volumes/data/git/flutter_update/cli_legacy/testapp
rm -rf build .gradle android/.gradle
rm -rf ~/.gradle/caches/transforms-*
```

### Q3: `flutter.jar` vs `flutter_embedding_release.jar`

- `flutter.jar`：包含 embedding + libflutter.so 的完整包（42MB+），编译耗时长
- `flutter_embedding_release.jar`：仅 Java embedding 层（1.5MB），编译快
- `pw release` 使用 `--local-engine` 时，Gradle 实际引用的是 `flutter_embedding_release.jar`

### Q4: 增量编译不生效

如果修改了 `.java` 文件但 ninja 显示 "no work to do"，可能是时间戳问题：

```bash
touch $ENGINE_SRC/flutter/shell/platform/android/io/flutter/embedding/engine/FlutterJNI.java
$NINJA -C $ENGINE_SRC/out/android_release_arm64 flutter/shell/platform/android:flutter_shell_java
$NINJA -C $ENGINE_SRC/out/android_release_arm64 flutter/shell/platform/android:embedding_jars
```

---

## 六、GN 参数参考

当前 `out/android_release_arm64/args.gn` 的关键参数：

```gn
target_os = "android"
target_cpu = "arm64"
flutter_runtime_mode = "release"
is_debug = false
is_clang = true
enable_lto = true
dart_use_compressed_pointers = true
```

如需重新生成 GN 配置：

```bash
cd $ENGINE_SRC/flutter
python3 tools/gn --android --android-cpu=arm64 --runtime-mode=release --no-goma
```
