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
#include <ddsketch.hpp>
#include <collapsing_lowest_dense_store.hpp>
#include <logarithmic_mapping.hpp>
namespace datasketches {

template<typename T>
tdigest_merge_timing_profile<T>::tdigest_merge_timing_profile():
generator(std::chrono::system_clock::now().time_since_epoch().count()),
distribution(0.0, 1.0)
{}

template<typename T>
void tdigest_merge_timing_profile<T>::run() {
  const size_t lg_min_stream_len(0);
  const size_t lg_max_stream_len(23);
  const size_t ppo(16);

  const size_t lg_max_trials(16);
  const size_t lg_min_trials(6);

  const size_t num_sketches(32);

  std::cout << "Stream\tTrials\tBuild\tUpdate\tMerge\tSize" << std::endl;

  std::vector<T> values(1ULL << lg_max_stream_len, 0);

  size_t stream_length(1 << lg_min_stream_len);
  while (stream_length <= (1 << lg_max_stream_len)) {

    long long build_sum = 0;
    long long update_sum = 0;
    long long merge_sum = 0;
    size_t size_bytes_sum = 0;

    const size_t num_trials = get_num_trials(stream_length, lg_min_stream_len, lg_max_stream_len, lg_min_trials, lg_max_trials);
    auto start = std::chrono::high_resolution_clock::now();
    const uint64_t seed_base = static_cast<uint64_t>(start.time_since_epoch().count());
    #pragma omp parallel reduction(+:build_sum, update_sum, merge_sum, size_bytes_sum)
    {
      std::mt19937 rng(seed_base + static_cast<uint64_t>(omp_get_thread_num()));
      std::uniform_real_distribution<T> dist(0.0, 1.0);
      std::vector<T> local_values(stream_length);
      #pragma omp for
      for (size_t t = 0; t < num_trials; ++t) {
        std::generate(local_values.begin(), local_values.end(), [&] { return dist(rng); });

        auto start_build(std::chrono::high_resolution_clock::now());
        std::vector<DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>> sketches;
        sketches.reserve(num_sketches);
        for (size_t i = 0; i < num_sketches; ++i) sketches.emplace_back(0.01);
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
        DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping> merge_sketch(0.01);
        for (size_t i = 0; i < num_sketches; ++i) merge_sketch.merge(sketches[i]);
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

    stream_length = pwr_2_law_next(ppo, stream_length);
  }
}

template<typename T>
T tdigest_merge_timing_profile<T>::sample() {
  return distribution(generator);
}

}

#endif
