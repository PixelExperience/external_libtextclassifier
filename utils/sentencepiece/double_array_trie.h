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

#ifndef LIBTEXTCLASSIFIER_UTILS_SENTENCEPIECE_DOUBLE_ARRAY_TRIE_H_
#define LIBTEXTCLASSIFIER_UTILS_SENTENCEPIECE_DOUBLE_ARRAY_TRIE_H_

#include <functional>
#include <vector>

#include "utils/sentencepiece/match.h"
#include "utils/strings/stringpiece.h"

namespace libtextclassifier3 {

// A trie node specifies a node in the tree, either an intermediate node or
// a leaf node.
// A leaf node contains the id as an int of the string match. This id is encoded
// in the lower 30 bits, thus the number of distinct ids is 2^30.
// An intermediate node has an associated label and an offset to it's children.
// The label is encoded in the least significant byte and must match the input
// character during matching.
typedef unsigned int TrieNode;

// A memory mappable trie, compatible with Darts::DoubleArray.
class DoubleArrayTrie {
 public:
  // nodes and nodes_length specify the array of the nodes of the trie.
  DoubleArrayTrie(const TrieNode* nodes, const int nodes_length)
      : nodes_(nodes), nodes_length_(nodes_length) {}

  // Find matches that are prefixes of a string.
  std::vector<TrieMatch> FindAllPrefixMatches(StringPiece input) const;

  // Find the longest prefix match of a string.
  TrieMatch LongestPrefixMatch(StringPiece input) const;

 private:
  // Returns whether a node as a leaf as a child.
  bool has_leaf(int i) const { return nodes_[i] & 0x100; }

  // Available when a node is a leaf.
  int value(int i) const { return static_cast<int>(nodes_[i] & 0x7fffffff); }

  // Label associated with a node.
  // A leaf node will have the MSB set and thus return an invalid label.
  unsigned int label(int i) const { return nodes_[i] & 0x800000ff; }

  // Returns offset to children.
  unsigned int offset(int i) const {
    return (nodes_[i] >> 10) << ((nodes_[i] & 0x200) >> 6);
  }

  void GatherPrefixMatches(
      StringPiece input, const std::function<void(TrieMatch)>& update_fn) const;

  const TrieNode* nodes_;
  const int nodes_length_;
};

}  // namespace libtextclassifier3

#endif  // LIBTEXTCLASSIFIER_UTILS_SENTENCEPIECE_DOUBLE_ARRAY_TRIE_H_