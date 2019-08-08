#include "Filesystem.h"

// Unfortunately we cannot use path::replace_filename since this is not
// implemented in boost::filesystem
fs::path add_prefix_to_filename(
    const std::string& prefix, const fs::path& file) {
    std::string newFilename {prefix};
    newFilename.append(file.filename().string());
    return file.parent_path() /= newFilename;
}
