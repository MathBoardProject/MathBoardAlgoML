#pragma once

// opencv
#include <opencv2/core.hpp>

// std
#include <fstream>
#include <iostream>
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
                                std::size_t epochs, int mini_batch_size,
                                float learning_rate,
                                const std::vector<Sample> &test_data = {});

private:
  // Iterate over all training samples in `mini_batch`, applying small updates
  // to gradient descent of cost function. Adjusts weights and biases based on
  // these updates. `learning_rate` indicates how big this changes of weights
  // and biases are.
  void UpdateMiniBatch(const std::vector<Sample> &mini_batch,
                       float learning_rate);

  // Resolve each training set in `test_data` and retruns ratio of
  // good answers to all samples
  double Evaluate(const std::vector<Sample> &test_data);

  // Iterates over `training_set` to compute the error for individual neurons.
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
                                            uint32_t partition_size) {
  if (partition_size == 0) {
    std::ofstream debug_output("debug_output.txt", std::ios::app);

    debug_output
        << "[PartitionVector] Error: partition_size can't be equel to 0\n";

    debug_output.close();
    std::exit(EXIT_FAILURE);
  }
  if (partition_size >= input.size()) {
    std::ofstream debug_output("debug_output.txt", std::ios::app);

    debug_output << "[PartitionVector] Error: partition_size can't be bigger "
                    "than input size\n";

    debug_output.close();
    std::exit(EXIT_FAILURE);
  }
  std::vector<std::vector<Sample>> partitions;
  const u_int32_t partition_number = std::ceil(
      static_cast<float>(input.size()) / static_cast<float>(partition_size));
  partitions.resize(partition_number);
  for (std::size_t i = 0; i < partitions.size(); i++) {
    const size_t start_index = i * partition_size;
    const size_t end_index =
        std::min(start_index + partition_size, input.size());

    partitions[i] = std::vector<Sample>(input.begin() + start_index,
                                        input.begin() + end_index);
  }
  return partitions;
}

// Finds the index of the highest value in `input`.
template <typename T> std::size_t FindMaxIndex(const std::vector<T> &input) {
  std::size_t max_val_index = 0;
  for (std::size_t i = 0; i < input.size(); i++) {
    if (input[i] > input[max_val_index]) {
      max_val_index = i;
    }
  }
  return max_val_index;
}

cv::Mat Sigmoid(const cv::Mat &weighted_input);
cv::Mat SigmoidDerivative(const cv::Mat &weighted_input);