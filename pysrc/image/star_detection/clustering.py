import numpy as np
from sklearn.cluster import DBSCAN
from typing import List, Tuple

def cluster_stars(stars: List[Tuple[int, int]], dbscan_eps: float, dbscan_min_samples: int) -> List[Tuple[int, int]]:
    """
    Cluster stars using the DBSCAN algorithm.

    Parameters:
    - stars: List of star positions as (x, y) tuples.
    - dbscan_eps: The maximum distance between two stars for them to be considered in the same neighborhood.
    - dbscan_min_samples: The number of stars in a neighborhood for a point to be considered a core point.

    Returns:
    - List of clustered star positions as (x, y) tuples.
    """
    if len(stars) == 0:
        return []

    clustering = DBSCAN(eps=dbscan_eps, min_samples=dbscan_min_samples).fit(stars)
    labels = clustering.labels_
    
    unique_labels = set(labels)
    clustered_stars = []
    for label in unique_labels:
        if label == -1:  # -1 indicates noise
            continue
        class_members = [stars[i] for i in range(len(stars)) if labels[i] == label]
        centroid = np.mean(class_members, axis=0).astype(int)
        clustered_stars.append(tuple(centroid))
    
    return clustered_stars
