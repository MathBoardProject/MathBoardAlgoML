import tensorflow as tf

mnist = tf.keras.datasets.mnist

(x_train, y_train), (x_test, y_test) = mnist.load_data()
x_train, x_test = x_train / 255.0, x_test / 255.0



CNNmodel = tf.keras.models.Sequential([
  tf.keras.layers.Convolution2D(16, 4, activation="relu", 
                                kernel_regularizer=tf.keras.regularizers.l2(),
                                input_shape=(28, 28, 1)),
  tf.keras.layers.MaxPool2D(),
  tf.keras.layers.Convolution2D(32, 4, activation="relu"),
  tf.keras.layers.Flatten(),
  tf.keras.layers.Dense(128, activation="relu"),
  tf.keras.layers.Dense(10, activation="softmax")
])

loss_fn = tf.keras.losses.SparseCategoricalCrossentropy(from_logits=False)

CNNmodel.compile(optimizer='adam',
              loss=loss_fn,
              metrics=['accuracy'])

CNNmodel.fit(x_train, y_train, epochs=10, validation_data=(x_test, y_test))

converter = tf.lite.TFLiteConverter.from_keras_model(CNNmodel) 
tflite_model = converter.convert()

with open('models/converted_model.tflite', 'wb') as f:     
  f.write(tflite_model)


