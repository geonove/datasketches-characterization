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

#ifndef TDIGEST_TIMING_PROFILE_IMPL_HPP_
#define TDIGEST_TIMING_PROFILE_IMPL_HPP_

#include <iostream>
#include <algorithm>
#include <chrono>
#include <atomic>
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
tdigest_timing_profile<T>::tdigest_timing_profile():
generator(std::chrono::system_clock::now().time_since_epoch().count()),
distribution(0.0, 1.0)
{}

template<typename T>
void tdigest_timing_profile<T>::run() {
  const std::vector<size_t> points = {1000, 10000, 100000, 1000000, 10000000, 100000000};
  const size_t num_trials_fixed = 256;

  const size_t num_queries(20);

  const uint16_t k = 200;

  std::cout << "Stream\tTrials\tBuild\tUpdate\tQuantile\tRank\tSerialize\tDeserialize\tSize" << std::endl;

  std::vector<T> rank_query_values(num_queries, 0);
  for (size_t i = 0; i < num_queries; i++) rank_query_values[i] = sample();

  double quantile_query_values[num_queries];
  for (size_t i = 0; i < num_queries; i++) quantile_query_values[i] = distribution(generator);

  for (size_t pi = 0; pi < points.size(); ++pi) {
    const size_t stream_length = points[pi];
    long long build_sum = 0;
    long long update_sum = 0;
    long long get_quantile_sum = 0;
    long long get_rank_sum = 0;
    long long serialize_sum = 0;
    long long deserialize_sum = 0;
    size_t size_bytes_sum = 0;

    const size_t num_trials = num_trials_fixed;
    const uint64_t seed_base = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    #pragma omp parallel reduction(+:build_sum, update_sum, get_quantile_sum, get_rank_sum, serialize_sum, deserialize_sum, size_bytes_sum)
    {
      std::mt19937 rng(seed_base + static_cast<uint64_t>(omp_get_thread_num()));

      // === Distribution: uncomment ONE ===
      // Uniform(0, 1)
      // auto sample = [&rng]() { static thread_local std::uniform_real_distribution<T> d(0.0, 1.0); return d(rng); };
      // Exponential(lambda=1.5)
      auto sample = [&rng]() { static thread_local std::exponential_distribution<T> d(1.5); return d(rng); };
      // Pareto(alpha=1.5, x_m=1.0)
      // auto sample = [&rng]() { static thread_local std::uniform_real_distribution<T> d(0.0, 1.0); return std::pow(d(rng), -1.0 / 1.5); };

      std::vector<T> values(stream_length);
      #pragma omp for
      for (size_t t = 0; t < num_trials; ++t) {
        std::generate(values.begin(), values.end(), [&] { return sample(); });

        // === Sketch: uncomment ONE ===
        // tdigest (k passed from above)
        auto start_build(std::chrono::high_resolution_clock::now());
        tdigest<T> sketch(k);
        // req_sketch (HRA, k=30)
        // req_sketch<T> sketch(30, true);
        // req_sketch (LRA, k=30)
        // req_sketch<T> sketch(30, false);
        // DDSketch (Collapsing Lowest Dense Store, alpha=0.01)
        // DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping> sketch(0.01);
        // DDSketch (Collapsing Highest Dense Store, alpha=0.01)
        // DDSketch<CollapsingHighestDenseStore<2048, std::allocator<double>>, LogarithmicMapping> sketch(0.01);
        auto finish_build(std::chrono::high_resolution_clock::now());
        build_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_build - start_build).count();

        auto start_update(std::chrono::high_resolution_clock::now());
        for (size_t i = 0; i < stream_length; ++i) sketch.update(values[i]);
        auto finish_update(std::chrono::high_resolution_clock::now());
        update_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_update - start_update).count();

        auto start_get_quantile(std::chrono::high_resolution_clock::now());
        for (size_t i = 0; i < num_queries; i++) {
          volatile T q = sketch.get_quantile(quantile_query_values[i]);
        }
        auto finish_get_quantile(std::chrono::high_resolution_clock::now());
        get_quantile_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_quantile - start_get_quantile).count();

        auto start_get_rank(std::chrono::high_resolution_clock::now());
        for (size_t i = 0; i < num_queries; i++) {
          volatile double rank = sketch.get_rank(rank_query_values[i]);
        }
        auto finish_get_rank(std::chrono::high_resolution_clock::now());
        get_rank_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_rank - start_get_rank).count();

        auto start_serialize(std::chrono::high_resolution_clock::now());
        auto bytes = sketch.serialize();
        auto finish_serialize(std::chrono::high_resolution_clock::now());
        serialize_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_serialize - start_serialize).count();

        size_bytes_sum += bytes.size();

        // Note: deserialize API differs per sketch. Adjust the call below as needed.
        auto start_deserialize(std::chrono::high_resolution_clock::now());
        auto deserialized = tdigest<T>::deserialize(bytes.data(), bytes.size());
        // auto deserialized = req_sketch<T>::deserialize(bytes.data(), bytes.size());
        // auto deserialized = DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>::deserialize(bytes.data(), bytes.size());
        // auto deserialized = DDSketch<CollapsingHighestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>::deserialize(bytes.data(), bytes.size());
        auto finish_deserialize(std::chrono::high_resolution_clock::now());
        deserialize_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_deserialize - start_deserialize).count();
      }
    }
    std::cout << stream_length << "\t"
        << num_trials << "\t"
        << (double) build_sum / num_trials << "\t"
        << (double) update_sum / num_trials / stream_length << "\t"
        << (double) get_quantile_sum / num_trials / num_queries << "\t"
        << (double) get_rank_sum / num_trials / num_queries << "\t"
        << (double) serialize_sum / num_trials << "\t"
        << (double) deserialize_sum / num_trials << "\t"
        << (double) size_bytes_sum / num_trials << "\n";
  }
}

template<typename T>
T tdigest_timing_profile<T>::sample() {
  return distribution(generator);
}

}

#endif
