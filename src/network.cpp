// local
#include "network.hpp"

// std
#include <fstream>

Network::Network(const std::vector<u_int32_t> &layers_sizes)
    : m_LayersSizes(layers_sizes) {
  for (std::size_t l = 1; l < m_LayersSizes.size(); l++) {

    cv::Mat biases = cv::Mat::zeros(layers_sizes[l], 1, CV_64F);
    cv::randu(biases, -1.0f, 1.0f);
    m_Biases.push_back(biases);

    cv::Mat weights =
        cv::Mat::zeros(layers_sizes[l], layers_sizes[l - 1], CV_64F);
    // Xavier initialization
    const float limit =
        std::sqrt(6.0f / (layers_sizes[l] + layers_sizes[l - 1]));
    cv::randu(weights, -limit, limit);
    m_Weights.push_back(weights);
  }
}

std::vector<double>
Network::FeedForward(const std::vector<double> &input) const {
  if (input.size() != m_LayersSizes[0]) {
    std::ofstream debug_output("debug_output.txt", std::ios::app);

    debug_output << "[Network::FeedForward] Error: input size doesn't match "
                    "m_LayersSizes[0] size\n";

    debug_output.close();
  }

  cv::Mat activation = cv::Mat(input).reshape(1, input.size());

  for (std::size_t i = 0; i < m_Weights.size(); i++) {
    cv::Mat weighted_input;
    cv::gemm(m_Weights[i], activation, 1.0f, m_Biases[i], 1.0f, weighted_input);
    activation = weighted_input.clone();
    activation = Sigmoid(weighted_input);
  }

  std::vector<double> output;
  activation.reshape(1, 1).copyTo(output);
  return output;
}

void Network::SochasticGradientDescent(std::vector<Sample> &training_data,
                                       std::size_t epochs, int mini_batch_size,
                                       float learning_rate,
                                       const std::vector<Sample> &test_data) {
  for (std::size_t i = 0; i < epochs; i++) {
    // TODO: random shuffle training data
    auto mini_batches = PartitionVector<Sample>(training_data, mini_batch_size);
    for (std::size_t j = 0; j < mini_batches.size(); j++) {
      UpdateMiniBatch(mini_batches[j], learning_rate);
    }
    if (test_data.size() != 0) {
      std::cout << "Epoch: " << i << " " << Evaluate(test_data) * 100.0f
                << "%\n";
    }
  }
}

void Network::UpdateMiniBatch(const std::vector<Sample> &mini_batch,
                              float learning_rate) {
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
    m_Weights[i] -= (learning_rate / mini_batch.size()) * weight_gradient[i];
  }
}

double Network::Evaluate(const std::vector<Sample> &test_data) {
  double correct_guesses = 0.0;
  for (std::size_t i = 0; i < test_data.size(); i++) {
    auto output = FeedForward(test_data[i].input);
    if (FindMaxIndex(output) == FindMaxIndex(test_data[i].expectedOutput)) {
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
    activation = weighted_input.clone();
    activation = Sigmoid(weighted_input);
    activations.push_back(activation);
  }

  // make column vector from expected output
  const cv::Mat expected_output =
      cv::Mat(training_data.expectedOutput)
          .reshape(1, training_data.expectedOutput.size());

  cv::Mat error = (activations.back() - expected_output)
                      .mul(SigmoidDerivative(weighted_inputs.back()));
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