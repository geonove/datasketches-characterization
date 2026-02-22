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

#ifndef TDIGEST_MERGE_TIMING_PROFILE_IMPL_HPP_
#define TDIGEST_MERGE_TIMING_PROFILE_IMPL_HPP_

#include <iostream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <vector>
#include <omp.h>

#include <tdigest.hpp>
#include <req_sketch.hpp>
#include <ddsketch.hpp>
#include <collapsing_lowest_dense_store.hpp>
#include <collapsing_highest_dense_store.hpp>
#include <logarithmic_mapping.hpp>
namespace datasketches {

template<typename T>
tdigest_merge_timing_profile<T>::tdigest_merge_timing_profile():
generator(std::chrono::system_clock::now().time_since_epoch().count()),
distribution(0.0, 1.0)
{}

template<typename T>
void tdigest_merge_timing_profile<T>::run() {
  const std::vector<size_t> points = {1000, 10000, 100000, 1000000, 10000000, 100000000};
  const size_t num_trials_fixed = 256;

  const size_t num_sketches(32);

  // === Sketch type: uncomment ONE block ===
  // tdigest (k=200)
  using sketch_t = tdigest<T>;
  auto make_sketch = []() { return sketch_t(200); };
  // req_sketch (HRA, k=30)
  // using sketch_t = req_sketch<T>;
  // auto make_sketch = []() { return sketch_t(30, true); };
  // req_sketch (LRA, k=30)
  // using sketch_t = req_sketch<T>;
  // auto make_sketch = []() { return sketch_t(30, false); };
  // DDSketch (Collapsing Lowest Dense Store, alpha=0.01)
  // using sketch_t = DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>;
  // auto make_sketch = []() { return sketch_t(0.01); };
  // DDSketch (Collapsing Highest Dense Store, alpha=0.01)
  // using sketch_t = DDSketch<CollapsingHighestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>;
  // auto make_sketch = []() { return sketch_t(0.01); };

  std::cout << "Stream\tTrials\tBuild\tUpdate\tMerge\tSize" << std::endl;

  for (size_t pi = 0; pi < points.size(); ++pi) {
    const size_t stream_length = points[pi];

    long long build_sum = 0;
    long long update_sum = 0;
    long long merge_sum = 0;
    size_t size_bytes_sum = 0;

    const size_t num_trials = num_trials_fixed;
    const uint64_t seed_base = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    #pragma omp parallel reduction(+:build_sum, update_sum, merge_sum, size_bytes_sum)
    {
      std::mt19937 rng(seed_base + static_cast<uint64_t>(omp_get_thread_num()));

      // === Distribution: uncomment ONE ===
      // Uniform(0, 1)
      // auto sample = [&rng]() { static thread_local std::uniform_real_distribution<T> d(0.0, 1.0); return d(rng); };
      // Exponential(lambda=1.5)
      // auto sample = [&rng]() { static thread_local std::exponential_distribution<T> d(1.5); return d(rng); };
      // Pareto(alpha=1.5, x_m=1.0)
      auto sample = [&rng]() { static thread_local std::uniform_real_distribution<T> d(0.0, 1.0); return std::pow(d(rng), -1.0 / 1.5); };

      std::vector<T> local_values(stream_length);
      #pragma omp for
      for (size_t t = 0; t < num_trials; ++t) {
        std::generate(local_values.begin(), local_values.end(), [&] { return sample(); });

        auto start_build(std::chrono::high_resolution_clock::now());
        std::vector<sketch_t> sketches;
        sketches.reserve(num_sketches);
        for (size_t i = 0; i < num_sketches; ++i) sketches.push_back(make_sketch());
        auto finish_build(std::chrono::high_resolution_clock::now());
        build_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_build - start_build).count();

        auto start_update(std::chrono::high_resolution_clock::now());
        size_t s = 0;
        for (size_t i = 0; i < stream_length; ++i) {
          sketches[s].update(local_values[i]);
          ++s;
          if (s == num_sketches) s = 0;
        }
        auto finish_update(std::chrono::high_resolution_clock::now());
        update_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_update - start_update).count();

        auto start_merge(std::chrono::high_resolution_clock::now());
        sketch_t merge_sketch = make_sketch();
        // For tdigest / DDSketch:
        for (size_t i = 0; i < num_sketches; ++i) merge_sketch.merge(sketches[i]);
        // For req_sketch (needs move):
        // for (size_t i = 0; i < num_sketches; ++i) merge_sketch.merge(std::move(sketches[i]));
        auto finish_merge(std::chrono::high_resolution_clock::now());
        merge_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_merge - start_merge).count();

        size_bytes_sum += merge_sketch.get_serialized_size_bytes();
      }
    }
    std::cout << stream_length << "\t"
        << num_trials << "\t"
        << (double) build_sum / num_trials / num_sketches << "\t"
        << (double) update_sum / num_trials / stream_length / num_sketches << "\t"
        << (double) merge_sum / num_trials / num_sketches << "\t"
        << (double) size_bytes_sum / num_trials << "\n";
  }
}

template<typename T>
T tdigest_merge_timing_profile<T>::sample() {
  return distribution(generator);
}

}

#endif
