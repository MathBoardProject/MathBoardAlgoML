import tensorflow as tf
import tensorflow_datasets as tfds
import datetime
import os


def normalize_img(image, label):
    """Normalizes images: `uint8` -> `float32`."""
    return tf.cast(image, tf.float32) / 255.0, label


builder = tfds.builder("bhmsds")
builder.info.set_file_format(
    file_format="tfrecord",
    override=True,
    override_if_initialized=True,
)
builder.download_and_prepare()

bhmsds_train, bhmsds_test = builder.as_dataset(
    split=["train", "test"],
    shuffle_files=True,
    as_supervised=True,
)

ds_info = builder.info

# Normalize and prepare the train dataset
bhmsds_train = bhmsds_train.map(normalize_img, num_parallel_calls=tf.data.AUTOTUNE)
bhmsds_train = bhmsds_train.cache()
bhmsds_train = bhmsds_train.shuffle(ds_info.splits["train"].num_examples)
bhmsds_train = bhmsds_train.batch(64)
bhmsds_train = bhmsds_train.prefetch(tf.data.AUTOTUNE)

# Normalize and prepare the test dataset
bhmsds_test = bhmsds_test.map(normalize_img, num_parallel_calls=tf.data.AUTOTUNE)
bhmsds_test = bhmsds_test.batch(64)
bhmsds_test = bhmsds_test.cache()
bhmsds_test = bhmsds_test.prefetch(tf.data.AUTOTUNE)

# Build the model
model = tf.keras.models.Sequential(
    [
        tf.keras.layers.Conv2D(filters=16, kernel_size=(4, 4), activation="relu"),
        tf.keras.layers.MaxPooling2D(),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(18),
    ]
)

model.compile(
    optimizer=tf.keras.optimizers.Adam(0.001),
    loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
    metrics=[tf.keras.metrics.SparseCategoricalAccuracy()],
)

# define the what info would be logged
log_dir = "logs/fit" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
tensorboard_callback = tf.keras.callbacks.TensorBoard(
    log_dir=log_dir,
    histogram_freq=1,
    embeddings_freq=1,
    update_freq="epoch",
)

# Train the model
model.fit(
    bhmsds_train,
    epochs=10,
    callbacks=[tensorboard_callback],
    validation_data=bhmsds_test,
)

# Convert the model to TFLite
conv = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = conv.convert()

# Save the TFLite model
with open("../../models/bhmsds_model.tflite", "wb") as f:
    f.write(tflite_model)