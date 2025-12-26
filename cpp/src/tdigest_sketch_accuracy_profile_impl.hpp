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
#include <ddsketch.hpp>
#include <sparse_store.hpp>
#include <logarithmic_mapping.hpp>

#include "true_rank.hpp"

namespace datasketches {

template<typename T>
void tdigest_sketch_accuracy_profile<T>::run_trial(
    std::vector<T>& values, size_t stream_length, uint16_t k,
    const std::vector<double>& ranks,
    std::vector<std::vector<double>>& errors, const size_t t) {

  // tdigest<T> sketch(k);
  DDSketch<SparseStore<std::allocator<T>>, LogarithmicMapping> sketch(0.01);
  for (size_t i = 0; i < stream_length; ++i) sketch.update(values[i]);

  std::sort(values.begin(), values.begin() + stream_length);

  for (size_t j = 0; j < ranks.size(); ++j) {
    const double r = ranks[j];

    // Match DDSketch's rank -> quantile convention: target_rank = r * (n - 1)
    // and the first element whose cumulative count exceeds target_rank is returned.
    // That corresponds to taking the floor of r * (n - 1) on the sorted data.
    size_t idx = static_cast<size_t>(r * (stream_length - 1));

    // Use the empirical quantile from the generated sample as ground truth.
    // Comparing against the theoretical distribution quantile exaggerates error,
    // especially for tiny sample sizes where sampling variance dominates.
    const T true_q = values[idx];
    const T est_q  = sketch.get_quantile(r); // replace for DDsketch

    // symmetric relative error (stable near 0)
    // const double denom = std::max(std::abs((double)true_q), std::abs((double)est_q));
    // const double rel_err = (denom == 0.0) ? 0.0 : std::abs((double)est_q - (double)true_q) / denom;
    //
    // errors[j][t] = rel_err;

    double mul_err = 0.0;
    if (true_q > 0 && est_q > 0) {
      const double ratio = (double)est_q / (double)true_q;
      mul_err = std::max(ratio, 1.0 / ratio) - 1.0;
    }
    errors[j][t] = mul_err;
  }
}

}

#endif
