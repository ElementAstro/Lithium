import numpy as np
from sklearn.cluster import DBSCAN
from typing import List, Tuple, Optional
from pathlib import Path
from loguru import logger
import matplotlib.pyplot as plt
import json


class StarClustering:
    def __init__(self, eps: float = 0.5, min_samples: int = 5, algorithm: str = 'auto') -> None:
        """
        Initialize the StarClustering with DBSCAN parameters.

        :param eps: The maximum distance between two samples for them to be considered as in the same neighborhood.
        :param min_samples: The number of samples in a neighborhood for a point to be considered as a core point.
        :param algorithm: The algorithm to be used by DBSCAN. Default is 'auto'.
        """
        self.eps = eps
        self.min_samples = min_samples
        self.algorithm = algorithm
        self.clusters: Optional[List[List[Tuple[int, int]]]] = None
        self.centroids: Optional[List[Tuple[int, int]]] = None

        # Configure Loguru logger
        logger.remove()  # Remove default logger
        logger.add("clustering.log", rotation="10 MB", retention="10 days",
                   level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")
        logger.debug(
            f"Initialized StarClustering with eps={self.eps}, min_samples={self.min_samples}, algorithm='{self.algorithm}'")

    def cluster_stars(self, stars: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
        """
        Cluster stars using the DBSCAN algorithm and compute centroids of each cluster.

        :param stars: List of star positions as (x, y) tuples.
        :return: List of clustered star centroids as (x, y) tuples.
        """
        logger.info("Starting clustering process")
        if not stars:
            logger.warning(
                "Empty star list provided. Returning empty centroid list.")
            return []

        try:
            clustering = DBSCAN(
                eps=self.eps, min_samples=self.min_samples, algorithm=self.algorithm)
            labels = clustering.fit_predict(stars)
            logger.debug(f"DBSCAN labels: {labels}")

            unique_labels = set(labels)
            self.clusters = []
            self.centroids = []

            for label in unique_labels:
                if label == -1:
                    logger.debug("Skipping noise points")
                    continue
                class_members = [stars[i]
                                 for i in range(len(stars)) if labels[i] == label]
                centroid = tuple(np.mean(class_members, axis=0).astype(int))
                self.clusters.append(class_members)
                self.centroids.append(centroid)
                logger.debug(f"Cluster {label}: Centroid at {centroid}")

            logger.info(
                f"Clustering completed with {len(self.centroids)} clusters found")
            return self.centroids
        except Exception as e:
            logger.error(f"Error during clustering: {e}")
            raise

    def visualize_clusters(self, stars: List[Tuple[int, int]], save_path: Optional[Path] = None) -> None:
        """
        Visualize the clustered stars and their centroids.

        :param stars: List of star positions as (x, y) tuples.
        :param save_path: Optional Path to save the plot image.
        """
        if self.centroids is None:
            logger.error(
                "No clusters to visualize. Please run cluster_stars first.")
            raise ValueError("No clusters to visualize.")

        logger.info("Visualizing clusters")
        plt.figure(figsize=(10, 8))
        colors = plt.cm.get_cmap('viridis', len(self.centroids))

        for idx, centroid in enumerate(self.centroids):
            cluster = self.clusters[idx]
            cluster_np = np.array(cluster)
            plt.scatter(cluster_np[:, 0], cluster_np[:, 1],
                        s=30, color=colors(idx), label=f'Cluster {idx}')
            plt.scatter(centroid[0], centroid[1],
                        s=100, color='red', marker='X')

        # Plot noise points
        noise = [star for star, label in zip(stars, DBSCAN(
            eps=self.eps, min_samples=self.min_samples, algorithm=self.algorithm).fit_predict(stars)) if label == -1]
        if noise:
            noise_np = np.array(noise)
            plt.scatter(noise_np[:, 0], noise_np[:, 1],
                        s=30, color='grey', label='Noise')

        plt.title('Star Clusters')
        plt.xlabel('X Coordinate')
        plt.ylabel('Y Coordinate')
        plt.legend()
        plt.grid(True)
        if save_path:
            plt.savefig(save_path)
            logger.info(f"Cluster visualization saved to {save_path}")
        plt.show()
        logger.debug("Clusters visualized successfully")

    def save_clusters(self, filepath: Union[str, Path]) -> None:
        """
        Save the clustered centroids to a JSON file.

        :param filepath: Path to save the JSON file.
        """
        if self.centroids is None:
            logger.error(
                "No clusters to save. Please run cluster_stars first.")
            raise ValueError("No clusters to save.")

        logger.info(f"Saving clusters to {filepath}")
        try:
            with open(filepath, 'w') as file:
                json.dump(self.centroids, file, indent=4)
            logger.debug("Clusters saved successfully")
        except Exception as e:
            logger.error(f"Failed to save clusters: {e}")
            raise

    def load_clusters(self, filepath: Union[str, Path]) -> List[Tuple[int, int]]:
        """
        Load clustered centroids from a JSON file.

        :param filepath: Path to load the JSON file from.
        :return: List of clustered star centroids as (x, y) tuples.
        """
        logger.info(f"Loading clusters from {filepath}")
        try:
            with open(filepath, 'r') as file:
                self.centroids = json.load(file)
            logger.debug(f"Clusters loaded: {self.centroids}")
            return self.centroids
        except Exception as e:
            logger.error(f"Failed to load clusters: {e}")
            raise

    def update_parameters(self, eps: Optional[float] = None, min_samples: Optional[int] = None, algorithm: Optional[str] = None) -> None:
        """
        Update DBSCAN parameters.

        :param eps: New epsilon value.
        :param min_samples: New minimum samples value.
        :param algorithm: New algorithm to use.
        """
        if eps is not None:
            self.eps = eps
            logger.debug(f"Updated eps to {self.eps}")
        if min_samples is not None:
            self.min_samples = min_samples
            logger.debug(f"Updated min_samples to {self.min_samples}")
        if algorithm is not None:
            self.algorithm = algorithm
            logger.debug(f"Updated algorithm to {self.algorithm}")


# Example Usage
if __name__ == "__main__":
    import sys

    # Initialize StarClustering object
    star_cluster = StarClustering(eps=30, min_samples=5)

    # Sample star positions
    stars = [
        (10, 10), (12, 11), (11, 13),
        (50, 50), (51, 52), (49, 51),
        (90, 90), (91, 89), (89, 92)
    ]

    # Perform clustering
    centroids = star_cluster.cluster_stars(stars)
    print(f"Cluster Centroids: {centroids}")

    # Visualize clusters
    star_cluster.visualize_clusters(stars, save_path=Path("clusters.png"))

    # Save clusters to JSON
    star_cluster.save_clusters("clusters.json")

    # Load clusters from JSON
    loaded_centroids = star_cluster.load_clusters("clusters.json")
    print(f"Loaded Cluster Centroids: {loaded_centroids}")
