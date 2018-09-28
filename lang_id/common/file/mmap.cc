/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lang_id/common/file/mmap.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lang_id/common/lite_base/logging.h"
#include "lang_id/common/lite_base/macros.h"

namespace libtextclassifier3 {
namespace mobile {

namespace {
inline string GetLastSystemError() {
  return string(strerror(errno));
}

inline MmapHandle GetErrorMmapHandle() {
  return MmapHandle(nullptr, 0);
}

class FileCloser {
 public:
  explicit FileCloser(int fd) : fd_(fd) {}
  ~FileCloser() {
    int result = close(fd_);
    if (result != 0) {
      const string last_error = GetLastSystemError();
      SAFTM_LOG(ERROR) << "Error closing file descriptor: " << last_error;
    }
  }
 private:
  const int fd_;

  SAFTM_DISALLOW_COPY_AND_ASSIGN(FileCloser);
};
}  // namespace

MmapHandle MmapFile(const string &filename) {
  int fd = open(filename.c_str(), O_RDONLY);

  if (fd < 0) {
    const string last_error = GetLastSystemError();
    SAFTM_LOG(ERROR) << "Error opening " << filename << ": " << last_error;
    return GetErrorMmapHandle();
  }

  // Make sure we close fd no matter how we exit this function.  As the man page
  // for mmap clearly states: "closing the file descriptor does not unmap the
  // region."  Hence, we can close fd as soon as we return from here.
  FileCloser file_closer(fd);

  return MmapFile(fd);
}

MmapHandle MmapFile(int fd) {
  // Get file stats to obtain file size.
  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    const string last_error = GetLastSystemError();
    SAFTM_LOG(ERROR) << "Unable to stat fd: " << last_error;
    return GetErrorMmapHandle();
  }
  size_t file_size_in_bytes = static_cast<size_t>(sb.st_size);

  // Perform actual mmap.
  void *mmap_addr = mmap(

      // Let system pick address for mmapp-ed data.
      nullptr,

      // Mmap all bytes from the file.
      file_size_in_bytes,

      // One can read / write the mapped data (but see MAP_PRIVATE below).
      // Normally, we expect only to read it, but in the future, we may want to
      // write it, to fix e.g., endianness differences.
      PROT_READ | PROT_WRITE,

      // Updates to mmaped data are *not* propagated to actual file.
      // AFAIK(salcianu) that's anyway not possible on Android.
      MAP_PRIVATE,

      // Descriptor of file to mmap.
      fd,

      // Map bytes right from the beginning of the file.  This, and
      // file_size_in_bytes (2nd argument) means we map all bytes from the file.
      0);
  if (mmap_addr == MAP_FAILED) {
    const string last_error = GetLastSystemError();
    SAFTM_LOG(ERROR) << "Error while mmapping: " << last_error;
    return GetErrorMmapHandle();
  }

  return MmapHandle(mmap_addr, file_size_in_bytes);
}

bool Unmap(MmapHandle mmap_handle) {
  if (!mmap_handle.ok()) {
    // Unmapping something that hasn't been mapped is trivially successful.
    return true;
  }
  if (munmap(mmap_handle.start(), mmap_handle.num_bytes()) != 0) {
    const string last_error = GetLastSystemError();
    SAFTM_LOG(ERROR) << "Error during Unmap / munmap: " << last_error;
    return false;
  }
  return true;
}

}  // namespace mobile
}  // namespace nlp_saft
