import tensorflow as tf

import numpy as np
import cv2

mnist = tf.keras.datasets.mnist
# Load MNIST dataset
(x_train, y_train), (x_test, y_test) = mnist.load_data()
x_train, x_test = x_train / 255.0, x_test / 255.0
x_train = x_train[..., tf.newaxis] 
x_test = x_test[..., tf.newaxis]   

rotate = tf.keras.Sequential([
  tf.keras.layers.RandomRotation(0.2)
])

train_ds = tf.data.Dataset.from_tensor_slices((x_train, y_train))
aug_ds = train_ds.map(
    lambda x, y: (tf.cast(rotate(x, training=True), tf.float64), y)
)
combined_ds = train_ds.concatenate(aug_ds)
combined_ds = combined_ds.shuffle(120000).batch(32)

model = tf.keras.models.Sequential([
  tf.keras.layers.Convolution2D(16, 4, activation="relu", 
                                kernel_regularizer=tf.keras.regularizers.l2(),
                                input_shape=(28, 28, 1)),
  tf.keras.layers.MaxPool2D(),
  tf.keras.layers.Convolution2D(32, 3, activation="relu", kernel_regularizer=tf.keras.regularizers.l2()),
  tf.keras.layers.Flatten(),
  tf.keras.layers.Dense(128, activation="relu"),
  tf.keras.layers.Dense(10, activation="softmax")
])

loss_fn = tf.keras.losses.SparseCategoricalCrossentropy(from_logits=False)

model.compile(optimizer='adam',
              loss=loss_fn,
              metrics=['accuracy'])

model.fit(combined_ds, epochs=10, validation_data=(x_test, y_test))

converter = tf.lite.TFLiteConverter.from_keras_model(model) 
tflite_model = converter.convert()

with open('models/converted_model.tflite', 'wb') as f:     
  f.write(tflite_model)