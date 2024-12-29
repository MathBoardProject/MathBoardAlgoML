#pragma once

// opencv
#include <opencv2/core.hpp>

// spdlog
#include <spdlog/spdlog.h>

// std
#include <algorithm>
#include <random>
#include <vector>

// tuple holding information about input of a training data and expected output
// out of it
struct Sample {
public:
  Sample() = default;
  Sample(const std::vector<double> &input,
         const std::vector<double> &expected_output)
      : input(input), expectedOutput(expected_output) {}

public:
  std::vector<double> input;
  std::vector<double> expectedOutput;
};

// Represents simple feed forward Neural Network.
// As a cost function we use cross entropy and neuron type is sigmoid neurons.
// It supports learning by SGD and simple L2 regularization technique.
class Network {
public:
  // Initializes network layers, biases, and weights using the given layer
  // sizes. Weights are initialized with Xavier initialization for training
  // stability.
  Network(const std::vector<u_int32_t> &layers_sizes);

  // Returns a vector where the index of the highest value represents the
  // predicted answer. The `input` size must match the size of the network's
  // input layer.
  std::vector<double> FeedForward(const std::vector<double> &input) const;

  // Divides `training_data` into smaller mini-batches of size
  // `mini_batch_size`. Trains the network over `epochs` iterations. If
  // `test_data` is provided, outputs the model's performance after each epoch
  // (this significantly slows the process).
  void SochasticGradientDescent(std::vector<Sample> &training_data,
                                std::size_t epochs, std::size_t mini_batch_size,
                                double learning_rate,
                                double regularization_parameter,
                                const std::vector<Sample> &test_data = {});

private:
  // Iterate over all training samples in `mini_batch`, applying small updates
  // to gradient descent of cost function. Adjusts weights and biases based on
  // these updates. `learning_rate` indicates how big this changes of weights
  // and biases are and 'regularization_parameter' favours smaller weights.
  void UpdateMiniBatch(const std::vector<Sample> &mini_batch,
                       double learning_rate, double weight_decay_factor);

  // Resolve each training set in `test_data` and retruns ratio of
  // good answers to all samples
  double Evaluate(const std::vector<Sample> &test_data);

  // Iterates over `training_set` to compute the error for individual layers.
  // Based on this error, calculates how biases and weights should be adjusted,
  // and stores these adjustments in `delta_bias_gradient` and
  // `delta_weight_gradient`. This function implements the learning step of the
  // network.
  void Backpropagation(const Sample &training_data,
                       std::vector<cv::Mat> &delta_bias_gradient,
                       std::vector<cv::Mat> &delta_weight_gradient);

private:
  std::vector<u_int32_t> m_LayersSizes;
  std::vector<cv::Mat> m_Biases;
  std::vector<cv::Mat> m_Weights;
};

template <typename T>
std::vector<std::vector<T>> PartitionVector(const std::vector<T> &input,
                                            std::size_t partition_size) {
  if (input.size() == 0) {
    spdlog::error("[PartitionVector] Error: input vector is empty\n");
  }
  if (partition_size == 0 || partition_size > input.size()) {
    spdlog::error("[PartitionVector] Error: partition_size must be in range "
                  "( 0, input.size() ) and yours is {}",
                  partition_size);
  }
  std::vector<std::vector<T>> partitions;
  const u_int32_t partition_number = std::ceil(
      static_cast<float>(input.size()) / static_cast<float>(partition_size));
  partitions.resize(partition_number);
  for (std::size_t i = 0; i < partitions.size(); i++) {
    const size_t start_index = i * partition_size;
    const size_t end_index =
        std::min(start_index + partition_size, input.size());

    partitions[i] =
        std::vector<T>(input.begin() + start_index, input.begin() + end_index);
  }
  return partitions;
}

// Finds the index of the highest value in `input`.
template <typename Iterator>
std::size_t FindMaxIndex(Iterator begin, Iterator end) {
  auto max_iter = std::max_element(begin, end);
  return std::distance(begin, max_iter);
}

template <typename T> std::vector<T> ShuffleVector(const std::vector<T> vec) {
  std::vector<T> rand_vector = vec;
  std::random_device rd;
  std::mt19937 rng(rd());
  std::shuffle(rand_vector.begin(), rand_vector.end(), rng);
  return rand_vector;
}