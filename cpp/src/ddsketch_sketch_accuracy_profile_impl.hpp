/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_IMPL_HPP
#define CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_IMPL_HPP

#include <algorithm>
#include <cmath>

#include "ddsketch_sketch_accuracy_profile.hpp"
#include "ddsketch.hpp"
#include "collapsing_highest_dense_store.hpp"
#include "collapsing_lowest_dense_store.hpp"
#include "logarithmic_mapping.hpp"
#include "true_rank.hpp"

#include "tdigest.hpp"
#include "req_sketch.hpp"

namespace datasketches {

template<typename T>
void ddsketch_sketch_accuracy_profile<T>::run_trial(
  std::vector<T>& values, size_t stream_length,
  const std::vector<double>& ranks,
  std::vector<std::vector<double>>& errors, const size_t t) {

  // === Sketch: uncomment ONE ===
  // DDSketch (collapsing lowest dense store, alpha=0.01)
  DDSketch<CollapsingLowestDenseStore<2048, std::allocator<T>>, LogarithmicMapping> sketch(0.01);
  // DDSketch (collapsing highest dense store, alpha=0.01)
  // DDSketch<CollapsingHighestDenseStore<2048, std::allocator<T>>, LogarithmicMapping> sketch(0.01);
  // t-digest (k=200)
  // tdigest<T> sketch(200);
  // REQ (HRA, k=30)
  // req_sketch<T> sketch(30, true);
  // REQ (LRA, k=30)
  // req_sketch<T> sketch(30, false);
  for (size_t i = 0; i < stream_length; ++i) sketch.update(values[i]);

  std::sort(values.begin(), values.begin() + stream_length);

  for (size_t j = 0; j < ranks.size(); ++j) {
    const T true_q = get_quantile(values, stream_length, ranks[j]);
    const T est_q = sketch.get_quantile(ranks[j]);
    errors[j][t] = std::abs(est_q - true_q) / true_q;
  }
}

}

#endif //CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_IMPL_HPP
