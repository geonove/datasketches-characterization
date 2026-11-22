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

#ifndef DDSKETCH_MEMORY_PROFILE_IMPL_HPP_
#define DDSKETCH_MEMORY_PROFILE_IMPL_HPP_

#include <cmath>
#include <random>

#include <tdigest.hpp>
#include <req_sketch.hpp>
#include <ddsketch.hpp>
#include <collapsing_lowest_dense_store.hpp>
#include <collapsing_highest_dense_store.hpp>
#include <logarithmic_mapping.hpp>

#include "counting_allocator.hpp"

namespace datasketches {

// this relies on a global variable to count total amount of allocated memory
extern thread_local long long int total_allocated_memory;

template<typename T>
void ddsketch_memory_profile<T>::run_trial(size_t lg_min_x, size_t num_points, size_t x_ppo) {
  total_allocated_memory = 0;

  thread_local std::mt19937 gen(std::random_device{}());

  // === Distribution: uncomment ONE ===
  // Uniform(0, 1)
  // std::uniform_real_distribution<T> dist(0, 1.0);
  // auto sample = [&] { return dist(gen); };
  // Exponential(lambda=1.5)
  std::exponential_distribution<T> dist(1.5);
  auto sample = [&] { return dist(gen); };
  // Pareto(alpha=1.5, x_m=1)
  // std::uniform_real_distribution<T> dist(0, 1.0);
  // auto sample = [&] { return std::pow(1.0 - dist(gen), -1.0 / 1.5); };

  // === Sketch: uncomment ONE block ===
  // t-digest (k=200)
  // using sketch_t = tdigest<T, counting_allocator<T>>;
  // auto make_sketch = []() { return sketch_t(200); };
  // REQ (HRA, k=30)
  // using sketch_t = req_sketch<T, std::less<T>, counting_allocator<T>>;
  // auto make_sketch = []() { return sketch_t(30, true); };
  // REQ (LRA, k=30)
  // using sketch_t = req_sketch<T, std::less<T>, counting_allocator<T>>;
  // auto make_sketch = []() { return sketch_t(30, false); };
  // DDSketch (collapsing lowest dense store, alpha=0.01)
  using sketch_t = DDSketch<CollapsingLowestDenseStore<2048, counting_allocator<double>>, LogarithmicMapping>;
  auto make_sketch = []() { return sketch_t(0.01); };
  // DDSketch (collapsing highest dense store, alpha=0.01)
  // using sketch_t = DDSketch<CollapsingHighestDenseStore<2048, counting_allocator<double>>, LogarithmicMapping>;
  // auto make_sketch = []() { return sketch_t(0.01); };

  sketch_t* s = new (counting_allocator<sketch_t>().allocate(1)) sketch_t(make_sketch());

  size_t count = 0;
  size_t p = 1ULL << lg_min_x;
  for (size_t i = 0; i < num_points; ++i) {
    const size_t delta = p - count;
    for (size_t j = 0; j < delta; ++j) s->update(sample());
    count += delta;
    #pragma omp critical(memory_usage_stats_update)
    {
      stats[i].update(total_allocated_memory);
    }
    p = pwr_2_law_next(x_ppo, p);
  }

  s->~sketch_t();
  counting_allocator<sketch_t>().deallocate(s, 1);
  if (total_allocated_memory != 0) throw std::runtime_error("total_allocated_memory != 0");
}

}

#endif
