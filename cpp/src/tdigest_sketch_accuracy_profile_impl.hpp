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

#ifndef TDIGEST_SKETCH_ACCURACY_PROFILE_IMPL_HPP_
#define TDIGEST_SKETCH_ACCURACY_PROFILE_IMPL_HPP_

#include <tdigest.hpp>
#include <req_sketch.hpp>
#include <ddsketch.hpp>
#include <collapsing_lowest_dense_store.hpp>
#include <collapsing_highest_dense_store.hpp>
#include <logarithmic_mapping.hpp>

#include "true_rank.hpp"

namespace datasketches {

template<typename T>
void tdigest_sketch_accuracy_profile<T>::run_trial(std::vector<T>& values, size_t stream_length, uint16_t k,
    const std::vector<double>& ranks, std::vector<std::vector<double>>& rank_errors, const size_t t) {

  // === Sketch: uncomment ONE ===
  // tdigest (k passed from base class, default 200)
  tdigest<T> sketch(k);
  // req_sketch (HRA, k=30)
  // req_sketch<T> sketch(30, true);
  // req_sketch (LRA, k=30)
  // req_sketch<T> sketch(30, false);
  // DDSketch (Collapsing Lowest Dense Store, alpha=0.01)
  // DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping> sketch(0.01);
  // DDSketch (Collapsing Highest Dense Store, alpha=0.01)
  // DDSketch<CollapsingHighestDenseStore<2048, std::allocator<double>>, LogarithmicMapping> sketch(0.01);
  for (size_t i = 0; i < stream_length; ++i) sketch.update(values[i]);

  std::sort(values.begin(), values.begin() + stream_length);
  unsigned j = 0;
  for (const double rank: ranks) {
    const T quantile = get_quantile(values, stream_length, rank);
    const double true_rank = get_rank(values, stream_length, quantile, MIDPOINT);
    rank_errors[j++][t] = (std::abs(sketch.get_rank(quantile) - true_rank));
  }
}

}

#endif
