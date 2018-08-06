/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef LIBTEXTCLASSIFIER_UTIL_HASH_HASH_H_
#define LIBTEXTCLASSIFIER_UTIL_HASH_HASH_H_

#include <string>

#include "util/base/integral_types.h"

namespace libtextclassifier2 {

uint32 Hash32(const char *data, size_t n, uint32 seed);

static inline uint32 Hash32WithDefaultSeed(const char *data, size_t n) {
  return Hash32(data, n, 0xBEEF);
}

static inline uint32 Hash32WithDefaultSeed(const std::string &input) {
  return Hash32WithDefaultSeed(input.data(), input.size());
}

}  // namespace libtextclassifier2

#endif  // LIBTEXTCLASSIFIER_UTIL_HASH_HASH_H_
