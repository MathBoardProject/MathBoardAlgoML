"""BHMSDS dataset."""

import tensorflow_datasets as tfds
import pathlib
import kagglehub
import os


def extract_label_from_filename(file_path):
    filename = pathlib.Path(file_path).stem
    label = filename.split("-")[0]
    return label


class BHMSDS(tfds.core.GeneratorBasedBuilder):
    VERSION = tfds.core.Version("1.0.0")
    RELEASE_NOTES = {"1.0.0": "Initial release."}

    def _info(self):
        return self.dataset_info_from_configs(
            description="Basic Handwritten Math Symbols Dataset",
            features=tfds.features.FeaturesDict(
                {
                    "image": tfds.features.Image(
                        shape=(28, 28, 1), doc="Grayscale image"
                    ),
                    "label": tfds.features.ClassLabel(
                        names=[
                            "0",
                            "1",
                            "2",
                            "3",
                            "4",
                            "5",
                            "6",
                            "7",
                            "8",
                            "9",
                            "dot",
                            "minus",
                            "plus",
                            "slash",
                            "w",
                            "x",
                            "y",
                            "z",
                        ],
                        doc="What is in the picture",
                    ),
                }
            ),
            supervised_keys=("image", "label"),
            homepage="https://github.com/wbachowski/bhmsds",
            disable_shuffling=False,
        )

    def _split_generators(self, dl_manager: tfds.download.DownloadManager):
        path = kagglehub.dataset_download(
            "c0wie08/basic-handwritten-math-symbols-dataset"
        )
        return {
            "train": self._generate_examples(os.path.join(path, "symbols", "train")),
            "test": self._generate_examples(os.path.join(path, "symbols", "test"))
        }

    def _generate_examples(self, path):
        """Yields examples."""
        i = 0
        for label in os.scandir(path):
            for img_path in os.scandir(label):
                i += 1
                yield i, {
                    "image": os.path.join(path, label, img_path),
                    "label": label.name,
                }
