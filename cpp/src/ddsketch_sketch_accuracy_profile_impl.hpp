//
// Created by Andrea Novellini on 03/01/2026.
//

#ifndef CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_IMPL_HPP
#define CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_IMPL_HPP

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

  // DDSketch<CollapsingHighestDenseStore<2048, std::allocator<T>>, LogarithmicMapping> sketch(0.01);
  // DDSketch<CollapsingLowestDenseStore<2048, std::allocator<T>>, LogarithmicMapping> sketch(0.01);
  // tdigest<double> sketch(100);
  // tdigest<double> sketch(200);
  // req_sketch<double> sketch(10, true);
  req_sketch<double> sketch(40, true);
  for (size_t i = 0; i < stream_length; ++i) sketch.update(values[i]);

  std::sort(values.begin(), values.begin() + stream_length);

  for (size_t j = 0; j < ranks.size(); ++j) {
    const double rank = ranks[j];


    // Use the empirical quantile from the generated sample as ground truth.
    const T true_q = get_quantile(values, stream_length, rank);
    const T est_q  = sketch.get_quantile(rank);

    // double mul_err = 0.0;
    // if (true_q > 0 && est_q > 0) {
    //   const double ratio = (double)est_q / (double)true_q;
    //   mul_err = std::max(ratio, 1.0 / ratio) - 1.0;
    // }
    errors[j][t] = std::abs(est_q - true_q) / true_q;
  }
}

}

#endif //CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_IMPL_HPP