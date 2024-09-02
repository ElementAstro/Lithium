from dataclasses import dataclass
import numpy as np
from typing import Optional

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
