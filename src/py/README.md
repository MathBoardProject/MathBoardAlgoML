## Requirements:

* Python 3.12.9

* Tensorflow 2.18.0

* Tensorflow Datasets 4.9.7

  

## Install:

*  `python3.12 -m venv .venv`
*  `source .venv/bin/activate`
*  `pip install -r requirements.txt`

  

## Run:
Inside project directory
```
cd src/py/bhmsds
tfds build 
cd ..
python3 neural_network.py
```

## Monitor Training with TensorBoard:

### Using VScode:
1. **Download** the `'TensorBoard'` extension.
2. **Run** the program.
3. **Open TensorBoard** using `Ctrl + Shift + P` and select **`Python: Launch TensorBoard`**.
4. *(Optional)* **Open** [http://localhost:6006](http://localhost:6006).

### Using comand prompt:

1.  **Run** the program.
2.  **Use** Inside /path/to/project/src/py ```tensorboard --logdir logs/fit```
3.  **Open** [http://localhost:6006](http://localhost:6006).