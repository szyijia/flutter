#!/usr/bin/env bash

set -euo pipefail

readonly REPOSITORY_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly RUNTIME_PATHS=(
  "${REPOSITORY_ROOT}/bin/internal"
  "${REPOSITORY_ROOT}/packages/flutter_tools/lib"
  "${REPOSITORY_ROOT}/packages/flutter_tools/gradle"
  "${REPOSITORY_ROOT}/packages/flutter_tools/pubspec.yaml"
)

if rg -n -i 'shorebirdtech|shorebird\.dev' "${RUNTIME_PATHS[@]}"; then
  printf '%s\n' 'error: Flutter runtime/build sources reference Shorebird network hosts' >&2
  exit 1
fi

rg -q -F 'https://github.com/szyijia/patchwing.git' \
  "${REPOSITORY_ROOT}/packages/flutter_tools/pubspec.yaml"
rg -q -F 'https://cdn.patchwing.net/patchwing/branded-v2' \
  "${REPOSITORY_ROOT}/packages/flutter_tools/lib/src/cache.dart"
rg -q -F 'https://cdn.patchwing.net/patchwing/branded-v2' \
  "${REPOSITORY_ROOT}/packages/flutter_tools/gradle/src/main/kotlin/FlutterPluginConstants.kt"

printf '%s\n' 'Patchwing Flutter source verification passed.'
