//
// Created by Andrea Novellini on 26/12/2025.
//

#ifndef CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_HPP
#define CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_HPP

#include "ddsketch_accuracy_profile.hpp"

namespace datasketches {

template<typename T>
class ddsketch_sketch_accuracy_profile: public ddsketch_accuracy_profile<T> {
public:
  void run_trial(std::vector<T>& values, size_t stream_length,
    const std::vector<double>& ranks, std::vector<std::vector<double>>& errors, const size_t t);
};

}

#include "ddsketch_sketch_accuracy_profile_impl.hpp"

#endif //CHARACTERIZATION_DDSKETCH_SKETCH_ACCURACY_PROFILE_HPP