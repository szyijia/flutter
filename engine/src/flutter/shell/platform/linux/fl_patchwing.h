#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PATCHWING_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PATCHWING_H_

#include <glib-object.h>
#include <string>

namespace flutter {

gboolean SetUpPatchwing(const char* assets_path, std::string& patch_path);

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PATCHWING_H_
