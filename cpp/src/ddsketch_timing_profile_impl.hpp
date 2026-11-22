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

#ifndef DDSKETCH_TIMING_PROFILE_IMPL_HPP_
#define DDSKETCH_TIMING_PROFILE_IMPL_HPP_

#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
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
void ddsketch_timing_profile<T>::run() {
  const std::vector<size_t> points = {1000, 10000, 100000, 1000000, 10000000, 100000000};
  const size_t num_trials = 256;
  const size_t num_queries(20);

  // === Sketch: uncomment ONE block ===
  // t-digest (k=200)
  // using sketch_t = tdigest<T>;
  // auto make_sketch = []() { return sketch_t(200); };
  // REQ (HRA, k=30)
  // using sketch_t = req_sketch<T>;
  // auto make_sketch = []() { return sketch_t(30, true); };
  // REQ (LRA, k=30)
  // using sketch_t = req_sketch<T>;
  // auto make_sketch = []() { return sketch_t(30, false); };
  // DDSketch (collapsing lowest dense store, alpha=0.01)
  using sketch_t = DDSketch<CollapsingLowestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>;
  auto make_sketch = []() { return sketch_t(0.01); };
  // DDSketch (collapsing highest dense store, alpha=0.01)
  // using sketch_t = DDSketch<CollapsingHighestDenseStore<2048, std::allocator<double>>, LogarithmicMapping>;
  // auto make_sketch = []() { return sketch_t(0.01); };

  std::cout << "Stream\tTrials\tBuild\tUpdate\tQuantile\tRank\tSerialize\tDeserialize\tSize" << std::endl;

  for (const size_t stream_length: points) {
    long long build_sum = 0;
    long long update_sum = 0;
    long long get_quantile_sum = 0;
    long long get_rank_sum = 0;
    long long serialize_sum = 0;
    long long deserialize_sum = 0;
    size_t size_bytes_sum = 0;

    const uint64_t seed_base = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    #pragma omp parallel reduction(+:build_sum, update_sum, get_quantile_sum, get_rank_sum, serialize_sum, deserialize_sum, size_bytes_sum)
    {
      std::mt19937 rng(seed_base + static_cast<uint64_t>(omp_get_thread_num()));

      // === Distribution: uncomment ONE ===
      // Uniform(0, 1)
      // std::uniform_real_distribution<T> dist(0, 1.0);
      // auto sample = [&] { return dist(rng); };
      // Exponential(lambda=1.5)
      std::exponential_distribution<T> dist(1.5);
      auto sample = [&] { return dist(rng); };
      // Pareto(alpha=1.5, x_m=1)
      // std::uniform_real_distribution<T> dist(0, 1.0);
      // auto sample = [&] { return std::pow(1.0 - dist(rng), -1.0 / 1.5); };

      std::uniform_real_distribution<double> rank_dist(0.0, 1.0);
      std::vector<T> rank_query_values(num_queries);
      std::vector<double> quantile_query_ranks(num_queries);
      for (size_t i = 0; i < num_queries; i++) {
        rank_query_values[i] = sample();
        quantile_query_ranks[i] = rank_dist(rng);
      }

      volatile double sink = 0; // prevents the queries from being optimized away
      std::vector<T> values(stream_length);
      #pragma omp for
      for (size_t t = 0; t < num_trials; ++t) {
        std::generate(values.begin(), values.end(), sample);

        auto start_build(std::chrono::high_resolution_clock::now());
        sketch_t sketch = make_sketch();
        auto finish_build(std::chrono::high_resolution_clock::now());
        build_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_build - start_build).count();

        auto start_update(std::chrono::high_resolution_clock::now());
        for (size_t i = 0; i < stream_length; ++i) sketch.update(values[i]);
        auto finish_update(std::chrono::high_resolution_clock::now());
        update_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_update - start_update).count();

        auto start_get_quantile(std::chrono::high_resolution_clock::now());
        for (size_t i = 0; i < num_queries; i++) {
          sink = sketch.get_quantile(quantile_query_ranks[i]);
        }
        auto finish_get_quantile(std::chrono::high_resolution_clock::now());
        get_quantile_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_quantile - start_get_quantile).count();

        auto start_get_rank(std::chrono::high_resolution_clock::now());
        for (size_t i = 0; i < num_queries; i++) {
          sink = sketch.get_rank(rank_query_values[i]);
        }
        auto finish_get_rank(std::chrono::high_resolution_clock::now());
        get_rank_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_rank - start_get_rank).count();

        auto start_serialize(std::chrono::high_resolution_clock::now());
        std::ostringstream os;
        sketch.serialize(os);
        const std::string bytes = os.str();
        auto finish_serialize(std::chrono::high_resolution_clock::now());
        serialize_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_serialize - start_serialize).count();
        size_bytes_sum += bytes.size();

        std::istringstream is(bytes);
        auto start_deserialize(std::chrono::high_resolution_clock::now());
        auto deserialized = sketch_t::deserialize(is);
        auto finish_deserialize(std::chrono::high_resolution_clock::now());
        deserialize_sum += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_deserialize - start_deserialize).count();
      }
      (void) sink;
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

}

#endif
