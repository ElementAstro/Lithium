from dataclasses import dataclass, field
from typing import Any
import logging


@dataclass
class CalibrationParams:
    wavelength: float  # Effective filter wavelength in nm
    transmissivity: float  # Filter transmissivity in the range (0,1)
    filter_width: float  # Filter bandwidth in nm
    aperture: float  # Telescope aperture diameter in mm
    obstruction: float  # Telescope central obstruction diameter in mm
    exposure_time: float  # Exposure time in seconds
    extinction: float  # Atmospheric extinction in the range [0,1)
    gain: float  # Sensor gain in e-/ADU
    quantum_efficiency: float  # Sensor quantum efficiency in the range (0,1)

    def __post_init__(self):
        """
        Validate calibration parameters after initialization.
        """
        if not (0 < self.transmissivity < 1):
            raise ValueError("Transmissivity must be between 0 and 1.")
        if not (0 <= self.extinction < 1):
            raise ValueError("Extinction must be in the range [0, 1).")
        if not (0 < self.quantum_efficiency < 1):
            raise ValueError("Quantum efficiency must be between 0 and 1.")
        if self.obstruction >= self.aperture:
            raise ValueError(
                "Obstruction diameter must be smaller than aperture diameter.")
