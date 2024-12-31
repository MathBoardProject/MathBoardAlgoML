// local
#include "network.hpp"

// std
#include <fstream>
#include <iostream>

Network::Network(const std::vector<uint32_t> &layers_sizes)
    : m_LayersSizes(layers_sizes) {
  for (std::size_t l = 1; l < m_LayersSizes.size(); l++) {

    cv::Mat biases = cv::Mat::zeros(layers_sizes[l], 1, CV_64F);
    cv::randu(biases, 0.0, 1.0);
    m_Biases.push_back(biases);

    cv::Mat weights =
        cv::Mat::zeros(layers_sizes[l], layers_sizes[l - 1], CV_64F);
    const double limit =
        std::sqrt(1.0 / (layers_sizes[l] + layers_sizes[l - 1]));
    cv::randu(weights, 0, limit);
    m_Weights.push_back(weights);
  }
}

std::vector<double>
Network::FeedForward(const std::vector<double> &input) const {
  if (input.size() != m_LayersSizes[0]) {
    spdlog::error("[Network::FeedForward]: input size doesn't match "
                  "m_LayersSizes[0] size\n");
  }

  cv::Mat activation = cv::Mat(input).reshape(1, input.size());

  for (std::size_t i = 0; i < m_Weights.size(); i++) {
    cv::Mat weighted_input;
    cv::gemm(m_Weights[i], activation, 1.0f, m_Biases[i], 1.0f, weighted_input);
    activation = Sigmoid(weighted_input).clone();
  }

  std::vector<double> output;
  activation.reshape(1, 1).copyTo(output);
  return output;
}

void Network::SochasticGradientDescent(std::vector<Sample> &training_data,
                                       std::size_t epochs,
                                       std::size_t mini_batch_size,
                                       double learning_rate,
                                       double regularization_parameter,
                                       const std::vector<Sample> &test_data) {

  if (regularization_parameter < 0.0) {
    spdlog::error("[Network::SochasticGradientDescent]: "
                  "regularization_parameter have to non negative number\n");
  }
  if (learning_rate <= 0.0) {
    spdlog::error("[Network::SochasticGradientDescent]: "
                  "learning_rate have to be a positive number\n");
  }

  const double weight_decay_factor =
      (1.0 - (learning_rate * regularization_parameter / training_data.size()));

  for (std::size_t i = 0; i < epochs; i++) {
    std::vector<std::vector<Sample>> mini_batches = PartitionVector<Sample>(
        ShuffleVector<Sample>(training_data), mini_batch_size);
    for (std::size_t j = 0; j < mini_batches.size(); j++) {
      UpdateMiniBatch(mini_batches[j], learning_rate, weight_decay_factor);
    }
    if (test_data.size() != 0) {
      std::cout << "Epoch: " << i << " " << Evaluate(test_data) * 100.0
                << "%\n";
    }
  }
}

void Network::UpdateMiniBatch(const std::vector<Sample> &mini_batch,
                              double learning_rate,
                              double weight_decay_factor) {
  // partial derrivative of cost function with respect to biases
  std::vector<cv::Mat> bias_gradient;
  bias_gradient.resize(m_Biases.size());
  for (std::size_t i = 0; i < bias_gradient.size(); i++) {
    bias_gradient[i] = cv::Mat::zeros(m_Biases[i].size(), CV_64F);
  }

  // partial derrivative of cost function with respect to weights
  std::vector<cv::Mat> weight_gradient;
  weight_gradient.resize(m_Weights.size());
  for (std::size_t i = 0; i < weight_gradient.size(); i++) {
    weight_gradient[i] = cv::Mat::zeros(m_Weights[i].size(), CV_64F);
  }

  for (std::size_t i = 0; i < mini_batch.size(); i++) {
    std::vector<cv::Mat> delta_bias_gradient;
    std::vector<cv::Mat> delta_weight_gradient;
    Backpropagation(mini_batch[i], delta_bias_gradient, delta_weight_gradient);

    for (std::size_t j = 0; j < bias_gradient.size(); j++) {
      bias_gradient[j] += delta_bias_gradient[j];
    }
    for (std::size_t j = 0; j < weight_gradient.size(); j++) {
      weight_gradient[j] += delta_weight_gradient[j];
    }
  }

  for (std::size_t i = 0; i < m_Biases.size(); i++) {
    m_Biases[i] -= (learning_rate / mini_batch.size()) * bias_gradient[i];
  }

  for (std::size_t i = 0; i < m_Weights.size(); i++) {
    // m_Weights[i] -= (learning_rate / mini_batch.size()) * weight_gradient[i];
    m_Weights[i] = weight_decay_factor * m_Weights[i] -
                   (learning_rate / mini_batch.size()) * weight_gradient[i];
  }
}

double Network::Evaluate(const std::vector<Sample> &test_data) {
  double correct_guesses = 0.0;
  for (std::size_t i = 0; i < test_data.size(); i++) {
    std::vector<double> output = FeedForward(test_data[i].input);
    if (FindMaxIndex(output.begin(), output.end()) ==
        FindMaxIndex(test_data[i].expectedOutput.begin(),
                     test_data[i].expectedOutput.end())) {
      correct_guesses++;
    }
  }
  return correct_guesses / static_cast<double>(test_data.size());
}

void Network::Backpropagation(const Sample &training_data,
                              std::vector<cv::Mat> &delta_bias_gradient,
                              std::vector<cv::Mat> &delta_weight_gradient) {
  delta_bias_gradient.resize(m_Biases.size());
  delta_weight_gradient.resize(m_Weights.size());
  cv::Mat activation =
      cv::Mat(training_data.input).reshape(1, training_data.input.size());
  std::vector<cv::Mat> activations{activation.clone()};
  std::vector<cv::Mat> weighted_inputs;

  // Feed forward
  for (std::size_t i = 0; i < m_Weights.size(); i++) {
    cv::Mat weighted_input;
    cv::gemm(m_Weights[i], activation, 1.0f, m_Biases[i], 1.0f, weighted_input);
    weighted_inputs.push_back(weighted_input);
    activation = Sigmoid(weighted_input).clone();
    activations.push_back(activation);
  }

  // make column vector from expected output
  const cv::Mat expected_output =
      cv::Mat(training_data.expectedOutput)
          .reshape(1, training_data.expectedOutput.size());

  // cross entropy function
  cv::Mat error = (activations.back() - expected_output);

  // error of output layer
  delta_bias_gradient.back() = error;
  delta_weight_gradient.back() =
      error * activations[activations.size() - 2].t();

  // error for hidden layers
  for (std::size_t i = activations.size() - 2; i > 0; i--) {
    error = (m_Weights[i].t() * error)
                .mul(SigmoidDerivative(weighted_inputs[i - 1]));
    delta_bias_gradient[i - 1] = error;
    delta_weight_gradient[i - 1] = error * activations[i - 1].t();
  }
}

cv::Mat Sigmoid(const cv::Mat &weighted_input) {
  cv::Mat output = cv::Mat::zeros(weighted_input.size(), CV_64F);
  cv::exp(-weighted_input, output);
  output = 1.0 / (1.0 + output);
  return output;
}

cv::Mat SigmoidDerivative(const cv::Mat &weighted_input) {
  cv::Mat sigmoid = Sigmoid(weighted_input);
  return sigmoid.mul(1.0 - sigmoid);
}