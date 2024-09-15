"""
Module for starfield image registration using triangle-based invariants.

This module provides functions to estimate and apply geometric transformations between
two sets of starfield images. The core technique relies on computing invariant features 
(triangle invariants) from sets of nearest neighbors of stars in both images. The 
transformation between the two images is then estimated using these invariant features.

The key functionalities include:
- Estimating transformation between two images
- Applying the estimated transformation to align images
- Extracting source positions (stars) from images
- RANSAC algorithm for robust model estimation

Author: Max Qian
Version: 2.6.0 (lithium)
"""

__version__ = "2.6.0"

__all__ = [
    "MIN_MATCHES_FRACTION",
    "MaxIterError",
    "NUM_NEAREST_NEIGHBORS",
    "PIXEL_TOL",
    "apply_transform",
    "estimate_transform",
    "find_transform",
    "matrix_transform",
    "register",
]

from collections import Counter
from functools import partial
from itertools import combinations
from typing import Any, Tuple, Union

import sep_pjw as sep
import numpy as np
from numpy.typing import NDArray
from scipy.spatial import KDTree
from skimage.transform import estimate_transform, matrix_transform, warp

try:
    import bottleneck as bn
except ImportError:
    HAS_BOTTLENECK = False
else:
    HAS_BOTTLENECK = True

PIXEL_TOL = 2
"""The pixel distance tolerance to assume two invariant points are the same.

Default: 2
"""

MIN_MATCHES_FRACTION = 0.8
"""The minimum fraction of triangle matches to accept a transformation.

If the minimum fraction yields more than 10 triangles, 10 is used instead.

Default: 0.8
"""

NUM_NEAREST_NEIGHBORS = 5
"""
The number of nearest neighbors of a given star (including itself) to construct
the triangle invariants.

Default: 5
"""

_default_median = bn.nanmedian if HAS_BOTTLENECK else np.nanmedian  # pragma: no cover
"""
Default median function when/if optional bottleneck is available
"""

_default_average = bn.nanmean if HAS_BOTTLENECK else np.nanmean  # pragma: no cover
"""
Default mean function when/if optional bottleneck is available
"""


def _invariantfeatures(x1: NDArray, x2: NDArray, x3: NDArray) -> list[float]:
    """
    Given 3 points x1, x2, x3, return the invariant features for the set.

    Invariant features are ratios of side lengths of the triangles formed by these points,
    sorted by size. These features are scale-invariant and can be used to compare star
    positions between images.

    Args:
        x1, x2, x3: 2D coordinates of points in the source image.

    Returns:
        List containing two invariant feature values derived from the triangle side ratios.
    """
    sides = np.sort(
        [np.linalg.norm(x1 - x2), np.linalg.norm(x2 - x3), np.linalg.norm(x1 - x3)])
    return [sides[2] / sides[1], sides[1] / sides[0]]


def _arrangetriplet(sources: NDArray, vertex_indices: tuple[int, int, int]) -> NDArray:
    """
    Reorder the given triplet of vertex indices according to the length of their sides.

    This function returns the indices in a consistent order based on the triangle's side
    lengths. It ensures that the triangle invariants are consistently computed.

    Args:
        sources: Array of source star positions.
        vertex_indices: Indices of the three vertices that form the triangle.

    Returns:
        Reordered array of vertex indices based on side lengths.
    """
    ind1, ind2, ind3 = vertex_indices
    x1, x2, x3 = sources[vertex_indices]

    side_ind = np.array([(ind1, ind2), (ind2, ind3), (ind3, ind1)])
    side_lengths = [np.linalg.norm(
        x1 - x2), np.linalg.norm(x2 - x3), np.linalg.norm(x3 - x1)]
    l1_ind, l2_ind, l3_ind = np.argsort(side_lengths)

    count = Counter(side_ind[[l1_ind, l2_ind]].flatten())
    a = count.most_common(1)[0][0]
    count = Counter(side_ind[[l2_ind, l3_ind]].flatten())
    b = count.most_common(1)[0][0]
    count = Counter(side_ind[[l3_ind, l1_ind]].flatten())
    c = count.most_common(1)[0][0]

    return np.array([a, b, c])


def _generate_invariants(sources: NDArray) -> Tuple[NDArray, NDArray]:
    """
    Generate invariant features and the corresponding triangles from a set of source points.

    This function constructs triangles from the nearest neighbors of each source point and
    calculates their invariant features. The invariants are used for matching between images.

    Args:
        sources: Array of source star positions.

    Returns:
        A tuple containing the unique invariant features and the corresponding triangle vertices.
    """
    arrange = partial(_arrangetriplet, sources=sources)

    inv = []
    triang_vrtx = []
    coordtree = KDTree(sources)
    knn = min(len(sources), NUM_NEAREST_NEIGHBORS)
    for asrc in sources:
        _, indx = coordtree.query(asrc, knn)

        all_asterism_triang = [arrange(vertex_indices=list(cmb))
                               for cmb in combinations(indx, 3)]
        triang_vrtx.extend(all_asterism_triang)

        inv.extend([_invariantfeatures(*sources[triplet])
                   for triplet in all_asterism_triang])

    uniq_ind = [pos for (pos, elem) in enumerate(inv)
                if elem not in inv[pos + 1:]]
    inv_uniq = np.array(inv)[uniq_ind]
    triang_vrtx_uniq = np.array(triang_vrtx)[uniq_ind]

    return inv_uniq, triang_vrtx_uniq


class _MatchTransform:
    """
    A class to manage the fitting of a geometric transformation using matched invariant points.

    This class handles the estimation of the 2D similarity transformation between
    two sets of points, and computes errors between estimated and actual points.
    """

    def __init__(self, source: NDArray, target: NDArray):
        """
        Initialize the transformation model with source and target control points.

        Args:
            source: Source control points.
            target: Target control points.
        """
        self.source = source
        self.target = target

    def fit(self, data: NDArray) -> Any:
        """
        Estimate the best 2D similarity transformation from the matched points in data.

        Args:
            data: Matched point pairs from source and target.

        Returns:
            A similarity transform object.
        """
        d1, d2, d3 = data.shape
        s, d = data.reshape(d1 * d2, d3).T
        return estimate_transform("similarity", self.source[s], self.target[d])

    def get_error(self, data: NDArray, approx_t: Any) -> NDArray:
        """
        Calculate the maximum residual error for the matched points given the estimated transform.

        Args:
            data: Matched point pairs.
            approx_t: Estimated transformation model.

        Returns:
            Maximum residual error for each set of matched points.
        """
        d1, d2, d3 = data.shape
        s, d = data.reshape(d1 * d2, d3).T
        resid = approx_t.residuals(
            self.source[s], self.target[d]).reshape(d1, d2)
        return resid.max(axis=1)


def _data(image: Union[NDArray, Any]) -> NDArray:
    """
    Retrieve the underlying 2D pixel data from the image.

    Args:
        image: The input image.

    Returns:
        The pixel data as a 2D NumPy array.
    """
    if hasattr(image, "data") and isinstance(image.data, np.ndarray):
        return image.data
    return np.asarray(image)


def _mask(image: Union[NDArray, Any]) -> Union[NDArray, None]:
    """
    Retrieve the mask from the image, if available.

    Args:
        image: The input image.

    Returns:
        The mask as a 2D NumPy array, or None if no mask is present.
    """
    if hasattr(image, "mask"):
        thenp_mask = np.asarray(image.mask)
        return thenp_mask if thenp_mask.ndim == 2 else np.logical_or.reduce(thenp_mask, axis=-1)
    return None


def _bw(image: NDArray) -> NDArray:
    """
    Convert the input image to a 2D grayscale image.

    Args:
        image: Input image, possibly with multiple channels.

    Returns:
        Grayscale 2D NumPy array.
    """
    return image if image.ndim == 2 else _default_average(image, axis=-1)


def _shape(image: NDArray) -> tuple[int, int]:
    """
    Get the shape of the image, ignoring channels.

    Args:
        image: Input image.

    Returns:
        A tuple representing the 2D shape (height, width) of the image.
    """
    return image.shape if image.ndim == 2 else image.shape[:2]


def find_transform(
    source: Union[NDArray, Any],
    target: Union[NDArray, Any],
    max_control_points: int = 50,
    detection_sigma: int = 5,
    min_area: int = 5
) -> Tuple[Any, Tuple[NDArray, NDArray]]:
    """
    Estimate the geometric transformation between the source and target images.

    This function identifies control points (stars) in both the source and target images,
    computes their triangle-based invariant features, and finds the best transformation
    to align the source to the target using a RANSAC-based method.

    Args:
        source: The source image to be transformed.
        target: The target image to match the source to.
        max_control_points: Maximum number of control points to use for transformation.
        detection_sigma: Sigma threshold for detecting control points.
        min_area: Minimum area for detecting sources in the image.

    Returns:
        A tuple containing the estimated transformation object and a tuple of matched control points
        from the source and target images.

    Raises:
        ValueError: If fewer than 3 control points are found in either image.
        TypeError: If the input type of source or target is unsupported.
    """
    try:
        source_controlp = (np.array(source)[:max_control_points] if len(_data(source)[0]) == 2
                           else _find_sources(_bw(_data(source)), detection_sigma, min_area, _mask(source))[:max_control_points])
    except Exception:
        raise TypeError("Input type for source not supported.")

    try:
        target_controlp = (np.array(target)[:max_control_points] if len(_data(target)[0]) == 2
                           else _find_sources(_bw(_data(target)), detection_sigma, min_area, _mask(target))[:max_control_points])
    except Exception:
        raise TypeError("Input type for target not supported.")

    if len(source_controlp) < 3 or len(target_controlp) < 3:
        raise ValueError(
            "Reference stars in source or target image are less than the minimum value (3).")

    source_invariants, source_asterisms = _generate_invariants(source_controlp)
    target_invariants, target_asterisms = _generate_invariants(target_controlp)

    source_tree = KDTree(source_invariants)
    target_tree = KDTree(target_invariants)

    matches_list = source_tree.query_ball_tree(target_tree, r=0.1)

    matches = [list(zip(t1, t2)) for t1, t2_list in zip(
        source_asterisms, matches_list) for t2 in target_asterisms[t2_list]]
    matches = np.array(matches)

    inv_model = _MatchTransform(source_controlp, target_controlp)
    n_invariants = len(matches)
    min_matches = max(1, min(10, int(n_invariants * MIN_MATCHES_FRACTION)))

    if (len(source_controlp) == 3 or len(target_controlp) == 3) and len(matches) == 1:
        best_t = inv_model.fit(matches)
        inlier_ind = np.arange(len(matches))
    else:
        best_t, inlier_ind = _ransac(
            matches, inv_model, PIXEL_TOL, min_matches)

    triangle_inliers = matches[inlier_ind]
    inl_arr = triangle_inliers.reshape(-1, 2)
    inl_unique = set(map(tuple, inl_arr))

    inl_dict = {}
    for s_i, t_i in inl_unique:
        s_vertex = source_controlp[s_i]
        t_vertex = target_controlp[t_i]
        t_vertex_pred = matrix_transform(s_vertex, best_t.params)
        error = np.linalg.norm(t_vertex_pred - t_vertex)

        if s_i not in inl_dict or error < inl_dict[s_i][1]:
            inl_dict[s_i] = (t_i, error)

    inl_arr_unique = np.array([[s_i, t_i]
                              for s_i, (t_i, _) in inl_dict.items()])
    s, d = inl_arr_unique.T

    return best_t, (source_controlp[s], target_controlp[d])


def apply_transform(
    transform: Any,
    source: Union[NDArray, Any],
    target: Union[NDArray, Any],
    fill_value: Union[float, None] = None,
    propagate_mask: bool = False
) -> Tuple[NDArray, NDArray]:
    """
    Apply the estimated transformation to align the source image to the target image.

    The transformation is applied to the source image, and an optional mask is propagated
    if requested. The function returns the aligned source image and a binary footprint
    of the transformed region.

    Args:
        transform: The transformation to apply.
        source: The source image to be transformed.
        target: The target image to align the source to.
        fill_value: Value to fill in regions outside the source image after transformation.
        propagate_mask: Whether to propagate the source mask after transformation.

    Returns:
        A tuple containing the aligned source image and the transformation footprint.
    """
    source_data = _data(source)
    target_shape = _data(target).shape

    aligned_image = warp(
        source_data,
        inverse_map=transform.inverse,
        output_shape=target_shape,
        order=3,
        mode="constant",
        cval=_default_median(source_data),
        clip=True,
        preserve_range=True,
    )

    footprint = warp(
        np.zeros(_shape(source_data), dtype="float32"),
        inverse_map=transform.inverse,
        output_shape=target_shape,
        cval=1.0,
    )
    footprint = footprint > 0.4

    source_mask = _mask(source)
    if source_mask is not None and propagate_mask:
        if source_mask.shape == source_data.shape:
            source_mask_rot = warp(
                source_mask.astype("float32"),
                inverse_map=transform.inverse,
                output_shape=target_shape,
                cval=1.0,
            )
            source_mask_rot = source_mask_rot > 0.4
            footprint |= source_mask_rot

    if fill_value is not None:
        aligned_image[footprint] = fill_value

    return aligned_image, footprint


def register(
    source: Union[NDArray, Any],
    target: Union[NDArray, Any],
    fill_value: Union[float, None] = None,
    propagate_mask: bool = False,
    max_control_points: int = 50,
    detection_sigma: int = 5,
    min_area: int = 5
) -> Tuple[NDArray, NDArray]:
    """
    Register and align the source image to the target image using triangle invariants.

    This function estimates the transformation between the source and target images,
    applies the transformation, and returns the aligned source image along with
    the transformation footprint.

    Args:
        source: The source image to be aligned.
        target: The target image for alignment.
        fill_value: Value to fill in regions outside the source image after transformation.
        propagate_mask: Whether to propagate the source mask after transformation.
        max_control_points: Maximum number of control points to use for transformation.
        detection_sigma: Sigma threshold for detecting control points.
        min_area: Minimum area for detecting sources in the image.

    Returns:
        A tuple containing the aligned source image and the transformation footprint.
    """
    t, _ = find_transform(
        source=source,
        target=target,
        max_control_points=max_control_points,
        detection_sigma=detection_sigma,
        min_area=min_area,
    )
    return apply_transform(t, source, target, fill_value, propagate_mask)


def _find_sources(img: NDArray, detection_sigma: int = 5, min_area: int = 5, mask: Union[NDArray, None] = None) -> NDArray:
    """
    Detect bright sources (e.g., stars) in the image using SEP (Source Extractor).

    This function returns the coordinates of sources sorted by brightness.

    Args:
        img: The input image in which to detect sources.
        detection_sigma: Sigma threshold for source detection.
        min_area: Minimum area for detecting sources.
        mask: Optional mask for ignoring certain parts of the image.

    Returns:
        A NumPy array of detected source coordinates (x, y), sorted by brightness.
    """
    image = img.astype("float32")
    bkg = sep.Background(image, mask=mask)
    thresh = detection_sigma * bkg.globalrms
    sources = sep.extract(image - bkg.back(), thresh,
                          minarea=min_area, mask=mask)
    sources.sort(order="flux")
    return np.array([[asrc["x"], asrc["y"]] for asrc in sources[::-1]])


class MaxIterError(RuntimeError):
    """
    Custom error raised if the maximum number of iterations is reached during the RANSAC process.

    This exception indicates that the RANSAC algorithm has exhausted all possible
    matching triangles without finding an acceptable transformation.
    """
    pass


def _ransac(data: NDArray, model: Any, thresh: float, min_matches: int) -> Tuple[Any, NDArray]:
    """
    Fit a model to data using the RANSAC (Random Sample Consensus) algorithm.

    This robust method estimates the transformation model by iteratively fitting
    to subsets of data and discarding outliers.

    Args:
        data: Matched point pairs.
        model: The transformation model to fit.
        thresh: Error threshold to consider a data point as an inlier.
        min_matches: Minimum number of inliers required to accept the model.

    Returns:
        A tuple containing the best-fit model and the indices of inliers.

    Raises:
        MaxIterError: If the maximum number of iterations is reached without finding
                      an acceptable transformation.
    """
    n_data = data.shape[0]
    all_idxs = np.arange(n_data)
    np.random.default_rng().shuffle(all_idxs)

    for iter_i in range(n_data):
        maybe_idxs = all_idxs[iter_i:iter_i + 1]
        test_idxs = np.concatenate([all_idxs[:iter_i], all_idxs[iter_i + 1:]])
        maybeinliers = data[maybe_idxs, :]
        test_points = data[test_idxs, :]
        maybemodel = model.fit(maybeinliers)
        test_err = model.get_error(test_points, maybemodel)
        also_idxs = test_idxs[test_err < thresh]
        alsoinliers = data[also_idxs, :]
        if len(alsoinliers) >= min_matches:
            good_data = np.concatenate((maybeinliers, alsoinliers))
            good_fit = model.fit(good_data)
            break
    else:
        raise MaxIterError(
            "List of matching triangles exhausted before an acceptable transformation was found")

    better_fit = good_fit
    for _ in range(3):
        test_err = model.get_error(data, better_fit)
        better_inlier_idxs = np.arange(n_data)[test_err < thresh]
        better_data = data[better_inlier_idxs]
        better_fit = model.fit(better_data)

    return better_fit, better_inlier_idxs
