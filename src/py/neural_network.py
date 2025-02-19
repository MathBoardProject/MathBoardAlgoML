import tensorflow as tf
import tensorflow_datasets as tfds

def normalize_img(image, label):
  """Normalizes images: `uint8` -> `float32`."""
  return tf.cast(image, tf.float32) / 255., label

# Load the EMNIST dataset
(emnist_train, emnist_test), ds_info = tfds.load(
  'emnist',
  split=['train', 'test'],
  shuffle_files=True,
  as_supervised=True,
  with_info=True,
)

# Normalize and prepare the training dataset
emnist_train = emnist_train.map(normalize_img, num_parallel_calls=tf.data.AUTOTUNE)
emnist_train = emnist_train.cache()
emnist_train = emnist_train.shuffle(ds_info.splits['train'].num_examples)
emnist_train = emnist_train.batch(128)
emnist_train = emnist_train.prefetch(tf.data.AUTOTUNE)

# Normalize and prepare the test dataset
emnist_test = emnist_test.map(normalize_img, num_parallel_calls=tf.data.AUTOTUNE)
emnist_test = emnist_test.batch(128)
emnist_test = emnist_test.cache()
emnist_test = emnist_test.prefetch(tf.data.AUTOTUNE)

# Build the model
model = tf.keras.models.Sequential([
  tf.keras.layers.Flatten(input_shape=(28, 28)),
  tf.keras.layers.Dense(128, activation='relu'),
  tf.keras.layers.Dense(10)
])

model.compile(
  optimizer=tf.keras.optimizers.Adam(0.001),
  loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
  metrics=[tf.keras.metrics.SparseCategoricalAccuracy()],
)

# Train the model
model.fit(
    emnist_train,
    epochs=6,
    validation_data=emnist_test,
)

# Convert the model to TFLite
conv = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = conv.convert()

# Save the TFLite model
with open('../../models/emnist.tflite', 'wb') as f:
  f.write(tflite_model)
