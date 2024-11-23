#include "base64.hpp"
#include "bmp.hpp"
#include "centroid.hpp"
#include "convolve.hpp"
#include "debayer.hpp"
#include "fitsio.hpp"
#include "fitskeyword.hpp"
#include "fwhm.hpp"
#include "hfr.hpp"
#include "hist.hpp"
#include "imgio.hpp"
#include "imgutils.hpp"
#include "ndarray_converter.hpp"
#include "stretch.hpp"
#include "thumbhash.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

/**
 * @brief Convert std::vector<std::byte> to Python bytes object
 *
 * @param vec std::vector<std::byte> object
 * @return py::bytes Python bytes object
 */
auto vectorToBytes(const std::vector<std::byte>& vec) -> py::bytes {
    return {reinterpret_cast<const char*>(vec.data()), vec.size()};
}

/**
 * @brief Convert Python bytes object to std::vector<std::byte>
 *
 * @param bytes Python bytes object
 * @return std::vector<std::byte> std::vector<std::byte> object
 */
auto bytesToVector(const py::bytes& bytes) -> std::vector<std::byte> {
    std::string str = bytes;
    return {reinterpret_cast<const std::byte*>(str.data()),
            reinterpret_cast<const std::byte*>(str.data() + str.size())};
}

/**
 * @brief Convert OpenCV cv::Mat to NumPy array
 *
 * @param mat OpenCV cv::Mat object
 * @return py::array_t<uint8_t> NumPy array
 */
auto matToNumpy(const cv::Mat& mat) -> py::array_t<uint8_t> {
    py::array_t<uint8_t> array({mat.rows, mat.cols, mat.channels()}, mat.data);
    return array;
}

/**
 * @brief Convert NumPy array to OpenCV cv::Mat
 *
 * @param array NumPy array
 * @return cv::Mat OpenCV cv::Mat object
 */
auto numpyToMat(const py::array_t<uint8_t>& array) -> cv::Mat {
    py::buffer_info info = array.request();
    int height = info.shape[0];
    int width = info.shape[1];
    int channels =
        (info.ndim == 2) ? 1 : info.shape[2];  // Support grayscale images
    return {height, width, CV_8UC(channels), info.ptr};
}

cv::Mat numpyToCvMat(py::array_t<uint8_t>& input) {
    py::buffer_info buf = input.request();
    int rows = buf.shape[0];
    int cols = buf.shape[1];
    return cv::Mat(rows, cols, CV_8UC1, buf.ptr);
}

py::tuple calcSubPixelCenterWrapper(
    const py::array_t<uint8_t>& roi, const std::pair<float, float>& initCenter,
    float epsilon = DEFAULT_EPSILON,
    int maxIterations = MAX_ITERATIONS_DEFAULT) {
    // Convert numpy.ndarray to cv::Mat
    cv::Mat roiMat = numpyToCvMat(const_cast<py::array_t<uint8_t>&>(roi));

    // Convert Python tuple (x, y) to cv::Point2f
    cv::Point2f center(initCenter.first, initCenter.second);

    // Call the actual C++ implementation
    cv::Point2f result = StarCentroid::calcSubPixelCenter(
        roiMat, std::move(center), epsilon, maxIterations);

    // Return as a Python tuple
    return py::make_tuple(result.x, result.y);
}

PYBIND11_MODULE(base64, m) {
    m.doc() = "Base64 encoding and decoding module";

    m.def(
        "base64Encode",
        [](const std::string& input) -> std::string {
            return base64Encode(
                reinterpret_cast<const unsigned char*>(input.c_str()),
                input.length());
        },
        py::arg("input"), "Encodes a string to Base64 format");

    m.def("base64Decode", &base64Decode, py::arg("encoded_string"),
          "Decodes a Base64 encoded string");

    py::class_<ByteUnion>(m, "ByteUnion")
        .def(py::init<>())
        .def_readwrite("value", &ByteUnion::value)
        .def_readwrite("bytes", &ByteUnion::bytes);

    // Bind Image struct
    py::class_<Image>(m, "Image")
        .def(py::init<>())
        .def_readwrite("data", &Image::data)
        .def_readwrite("greyData", &Image::greyData)
        .def_readwrite("sizeX", &Image::sizeX)
        .def_readwrite("sizeY", &Image::sizeY);

    // Bind littleToNative function
    m.def("littleToNative", py::overload_cast<uint32_t>(&littleToNative),
          "Convert little-endian uint32_t to native-endian");
    m.def("littleToNative", py::overload_cast<uint16_t>(&littleToNative),
          "Convert little-endian uint16_t to native-endian");

    // Bind readEndianInt function
    m.def("readEndianInt", &readEndianInt,
          "Read a little-endian uint32_t from a file");

    // Bind readEndianShort function
    m.def("readEndianShort", &readEndianShort,
          "Read a little-endian uint16_t from a file");

    // Bind loadBMPImage function
    m.def("loadBMPImage", &loadBMPImage, "Load a BMP image from a file");

    // Bind saveGrayImage function
    m.def("saveGrayImage", &saveGrayImage, "Save a grayscale image to a file");

    py::class_<StarCentroid::CentroidResult>(m, "CentroidResult")
        .def(py::init<>())
        .def_readwrite("weightedCenter",
                       &StarCentroid::CentroidResult::weightedCenter)
        .def_readwrite("subPixelCenter",
                       &StarCentroid::CentroidResult::subPixelCenter)
        .def_readwrite("roundedCenter",
                       &StarCentroid::CentroidResult::roundedCenter);

    // Bind StarCentroid class
    py::class_<StarCentroid>(m, "StarCentroid")
        .def_static(
            "readFits",
            [](const std::string& filename) {
                return StarCentroid::readFits(filename);
            },
            R"pbdoc(
            Read a FITS file and return the image matrix
            Parameters:
                filename (str): Path to the FITS file
            Returns:
                numpy.ndarray: Image matrix
        )pbdoc")
        .def_static("calcIntensityWeightedCenter",
                    &StarCentroid::calcIntensityWeightedCenter, R"pbdoc(
            Calculate the intensity-weighted center of the image
            Parameters:
                image (numpy.ndarray): Input image
            Returns:
                tuple: Coordinates of the intensity-weighted center (x, y)
        )pbdoc")
        .def_static("calcSubPixelCenter", &calcSubPixelCenterWrapper,
                    R"pbdoc(
            Calculate the sub-pixel center
            Parameters:
                roi (numpy.ndarray): Region of interest
                initCenter (tuple): Initial center coordinates (x, y)
                epsilon (float): Threshold for stopping the iteration
                maxIterations (int): Maximum number of iterations
            Returns:
                tuple: Coordinates of the sub-pixel center (x, y)
        )pbdoc",
                    py::arg("roi"), py::arg("initCenter"),
                    py::arg("epsilon") = DEFAULT_EPSILON,
                    py::arg("maxIterations") = MAX_ITERATIONS_DEFAULT)
        .def_static("findCentroid", &StarCentroid::findCentroid, R"pbdoc(
            Find the centroid of the image
            Parameters:
                image (numpy.ndarray): Input image
            Returns:
                CentroidResult: Centroid result
        )pbdoc",
                    py::arg("image"))
        .def_static("visualizeResults", &StarCentroid::visualizeResults,
                    R"pbdoc(
            Visualize the centroid results
            Parameters:
                image (numpy.ndarray): Input image
                result (CentroidResult): Centroid result
        )pbdoc");

    m.def(
        "convolve",
        [](const py::array_t<uint8_t>& input,
           const py::array_t<uint8_t>& kernel) {
            cv::Mat inputMat = numpyToMat(input);
            cv::Mat kernelMat = numpyToMat(kernel);
            cv::Mat outputMat;
            convolve(inputMat, kernelMat, outputMat);
            return matToNumpy(outputMat);
        },
        R"pbdoc(
        Perform convolution on the input image
        Parameters:
            input (numpy.ndarray): Input image
            kernel (numpy.ndarray): Convolution kernel
        Returns:
            numpy.ndarray: Convolved image
    )pbdoc");

    // Bind dftConvolve function
    m.def(
        "dftConvolve",
        [](const py::array_t<uint8_t>& input,
           const py::array_t<uint8_t>& kernel) {
            cv::Mat inputMat = numpyToMat(input);
            cv::Mat kernelMat = numpyToMat(kernel);
            cv::Mat outputMat;
            dftConvolve(inputMat, kernelMat, outputMat);
            return matToNumpy(outputMat);
        },
        R"pbdoc(
        Perform convolution on the input image using Discrete Fourier Transform
        Parameters:
            input (numpy.ndarray): Input image
            kernel (numpy.ndarray): Convolution kernel
        Returns:
            numpy.ndarray: Convolved image
    )pbdoc");

    // Bind deconvolve function
    m.def(
        "deconvolve",
        [](const py::array_t<uint8_t>& input,
           const py::array_t<uint8_t>& kernel) {
            cv::Mat inputMat = numpyToMat(input);
            cv::Mat kernelMat = numpyToMat(kernel);
            cv::Mat outputMat;
            deconvolve(inputMat, kernelMat, outputMat);
            return matToNumpy(outputMat);
        },
        R"pbdoc(
        Perform deconvolution on the input image
        Parameters:
            input (numpy.ndarray): Input image
            kernel (numpy.ndarray): Convolution kernel
        Returns:
            numpy.ndarray: Deconvolved image
    )pbdoc");

    // Bind separableConvolve function
    m.def(
        "separableConvolve",
        [](const py::array_t<uint8_t>& input,
           const py::array_t<uint8_t>& kernelX,
           const py::array_t<uint8_t>& kernelY) {
            cv::Mat inputMat = numpyToMat(input);
            cv::Mat kernelXMat = numpyToMat(kernelX);
            cv::Mat kernelYMat = numpyToMat(kernelY);
            cv::Mat outputMat;
            separableConvolve(inputMat, kernelXMat, kernelYMat, outputMat);
            return matToNumpy(outputMat);
        },
        R"pbdoc(
        Perform separable convolution on the input image
        Parameters:
            input (numpy.ndarray): Input image
            kernelX (numpy.ndarray): Horizontal convolution kernel
            kernelY (numpy.ndarray): Vertical convolution kernel
        Returns:
            numpy.ndarray: Convolved image
    )pbdoc");

    // Bind DebayerResult struct
    py::class_<DebayerResult>(m, "DebayerResult")
        .def(py::init<>())
        .def_readwrite("debayeredImage", &DebayerResult::debayeredImage)
        .def_readwrite("continueProcessing", &DebayerResult::continueProcessing)
        .def_readwrite("header", &DebayerResult::header);

    // Bind readFits function
    m.def(
        "readFits",
        [](const std::string& filepath,
           std::map<std::string, std::string>& header) {
            return matToNumpy(readFits(filepath, header));
        },
        R"pbdoc(
        Read a FITS file and return the image matrix
        Parameters:
            filepath (str): Path to the FITS file
            header (dict): Dictionary to store FITS header information
        Returns:
            numpy.ndarray: Image matrix
    )pbdoc");

    // Bind debayer function
    m.def(
        "debayer",
        [](const std::string& filepath) {
            DebayerResult result = debayer(filepath);
            py::dict header;
            for (const auto& [key, value] : result.header) {
                header[py::str(key)] = py::str(value);
            }
            return py::make_tuple(matToNumpy(result.debayeredImage),
                                  result.continueProcessing, header);
        },
        R"pbdoc(
        Perform debayering process
        Parameters:
            filepath (str): Path to the image file
        Returns:
            tuple: A tuple containing the debayered image, a boolean indicating whether to continue processing, and a dictionary of header information
    )pbdoc");

    // Bind FitsResult struct
    py::class_<FitsResult>(m, "FitsResult")
        .def(py::init<>())
        .def_readwrite("image", &FitsResult::image)
        .def_readwrite("header", &FitsResult::header);

    // Bind readFits function
    m.def(
        "readFits",
        [](const std::string& filepath) {
            FitsResult result = readFits(filepath);
            py::dict header;
            for (const auto& [key, value] : result.header) {
                header[py::str(key)] = py::str(value);
            }
            return py::make_tuple(matToNumpy(result.image), header);
        },
        R"pbdoc(
        Read a FITS file and return the image matrix and header information
        Parameters:
            filepath (str): Path to the FITS file
        Returns:
            tuple: A tuple containing the image matrix (numpy.ndarray) and header information (dict)
    )pbdoc");

    // Bind writeMatToFits function
    m.def(
        "writeMatToFits",
        [](py::array_t<uint8_t> image, const std::string& filepath) {
            cv::Mat mat = numpyToMat(image);
            writeMatToFits(mat, filepath);
        },
        R"pbdoc(
        Write cv::Mat image data to a FITS file
        Parameters:
            image (numpy.ndarray): Image data to write
            filepath (str): Path to the target FITS file
    )pbdoc");

    // Bind matToBase64 function
    m.def(
        "matToBase64",
        [](py::array_t<uint8_t> image, const std::string& imgFormat) {
            cv::Mat mat = numpyToMat(image);
            return matToBase64(mat, imgFormat);
        },
        R"pbdoc(
        Convert cv::Mat image data to a Base64 string
        Parameters:
            image (numpy.ndarray): Image data to convert
            imgFormat (str): Image encoding format (e.g., ".png", ".jpg")
        Returns:
            str: Base64 encoded string
    )pbdoc");

    // Bind fitsToBase64 function
    m.def("fitsToBase64", &fitsToBase64, R"pbdoc(
        Convert a FITS file to a Base64 string
        Parameters:
            filepath (str): Path to the FITS file
        Returns:
            str: Base64 encoded string
    )pbdoc");

    // Bind readFitsDeviceName function
    m.def("readFitsDeviceName", &readFitsDeviceName, R"pbdoc(
        Read the device name from a FITS file
        Parameters:
            filepath (str): Path to the FITS file
        Returns:
            Optional[str]: Device name if present, otherwise None
    )pbdoc");

    // Bind DataPoint struct
    py::class_<DataPoint>(m, "DataPoint")
        .def(py::init<double, double>(), py::arg("x") = 0, py::arg("y") = 0)
        .def_readwrite("x", &DataPoint::x)
        .def_readwrite("y", &DataPoint::y);

    // Bind GaussianParams struct
    py::class_<GaussianParams>(m, "GaussianParams")
        .def(py::init<>())
        .def_readwrite("base", &GaussianParams::base)
        .def_readwrite("peak", &GaussianParams::peak)
        .def_readwrite("center", &GaussianParams::center)
        .def_readwrite("width", &GaussianParams::width);

    // Bind GaussianFit class
    py::class_<GaussianFit>(m, "GaussianFit")
        .def_static("fit", &GaussianFit::fit, R"pbdoc(
            Fit a Gaussian model to the given data points
            Parameters:
                points (List[DataPoint]): List of data points
                eps (float): Convergence threshold
                maxIter (int): Maximum number of iterations
            Returns:
                Optional[GaussianParams]: Fitted Gaussian parameters if successful, otherwise None
        )pbdoc",
                    py::arg("points"), py::arg("eps") = 1e-6,
                    py::arg("maxIter") = 100)
        .def_static("evaluate", &GaussianFit::evaluate, R"pbdoc(
            Evaluate the Gaussian model at a given x value
            Parameters:
                params (GaussianParams): Gaussian parameters
                x (float): x value
            Returns:
                float: Evaluated y value
        )pbdoc",
                    py::arg("params"), py::arg("x"))
        .def_static("visualize", &GaussianFit::visualize, R"pbdoc(
            Visualize the Gaussian fit
            Parameters:
                points (List[DataPoint]): List of data points
                params (GaussianParams): Gaussian parameters
        )pbdoc",
                    py::arg("points"), py::arg("params"));

    // Bind FITSRecord::Type enum
    py::enum_<FITSRecord::Type>(m, "Type")
        .value("VOID", FITSRecord::Type::VOID)
        .value("COMMENT", FITSRecord::Type::COMMENT)
        .value("STRING", FITSRecord::Type::STRING)
        .value("LONGLONG", FITSRecord::Type::LONGLONG)
        .value("DOUBLE", FITSRecord::Type::DOUBLE)
        .export_values();

    // Bind FITSRecord class
    py::class_<FITSRecord>(m, "FITSRecord")
        .def(py::init<>())
        .def(py::init<std::string_view, std::string_view, std::string_view>(),
             py::arg("key"), py::arg("value"), py::arg("comment") = "")
        .def(py::init<std::string_view, int64_t, std::string_view>(),
             py::arg("key"), py::arg("value"), py::arg("comment") = "")
        .def(py::init<std::string_view, double, int, std::string_view>(),
             py::arg("key"), py::arg("value"), py::arg("decimal") = 6,
             py::arg("comment") = "")
        .def(py::init<std::string_view>(), py::arg("comment"))
        .def("type", &FITSRecord::type, R"pbdoc(
            Get the type of the FITS record
            Returns:
                Type: The type of the FITS record
        )pbdoc")
        .def("key", &FITSRecord::key, R"pbdoc(
            Get the key of the FITS record
            Returns:
                str: The key of the FITS record
        )pbdoc")
        .def("comment", &FITSRecord::comment, R"pbdoc(
            Get the comment of the FITS record
            Returns:
                str: The comment of the FITS record
        )pbdoc")
        .def("decimal", &FITSRecord::decimal, R"pbdoc(
            Get the decimal precision of the FITS record
            Returns:
                int: The decimal precision of the FITS record
        )pbdoc")
        .def("valueString", &FITSRecord::valueString, R"pbdoc(
            Get the string value of the FITS record
            Returns:
                str: The string value of the FITS record
            Raises:
                RuntimeError: If the value is not a string
        )pbdoc")
        .def("valueInt", &FITSRecord::valueInt, R"pbdoc(
            Get the integer value of the FITS record
            Returns:
                int: The integer value of the FITS record
            Raises:
                RuntimeError: If the value is not an integer
        )pbdoc")
        .def("valueDouble", &FITSRecord::valueDouble, R"pbdoc(
            Get the double value of the FITS record
            Returns:
                float: The double value of the FITS record
            Raises:
                RuntimeError: If the value is not a double
        )pbdoc")
        .def(
            "setValue",
            [](FITSRecord& self, const std::string& value) {
                self.setValue(value);
            },
            R"pbdoc(
            Set the value of the FITS record to a string
            Parameters:
                value (str): The string value to set
        )pbdoc")
        .def(
            "setValue",
            [](FITSRecord& self, int64_t value) { self.setValue(value); },
            R"pbdoc(
            Set the value of the FITS record to an integer
            Parameters:
                value (int): The integer value to set
        )pbdoc")
        .def(
            "setValue",
            [](FITSRecord& self, double value) { self.setValue(value); },
            R"pbdoc(
            Set the value of the FITS record to a double
            Parameters:
                value (float): The double value to set
        )pbdoc");

    // Bind fits_literals namespace
    m.def(
        "operator"
        "_fits_comment",
        [](const char* str, size_t) { return FITSRecord(str); }, R"pbdoc(
        Create a FITSRecord with a comment
        Parameters:
            str (str): The comment string
        Returns:
            FITSRecord: The FITSRecord with the comment
    )pbdoc");

    // Bind calcHfr function
    m.def(
        "calcHfr",
        [](const py::array_t<uint8_t>& inImage, float radius) {
            cv::Mat mat = numpyToMat(inImage);
            return calcHfr(mat, radius);
        },
        R"pbdoc(
        Calculate the Half Flux Radius (HFR) of an image
        Parameters:
            inImage (numpy.ndarray): Input image
            radius (float): Radius for HFR calculation
        Returns:
            float: Calculated HFR value
    )pbdoc");

    // Bind caldim function
    m.def(
        "caldim",
        [](const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            return caldim(mat);
        },
        R"pbdoc(
        Calculate the dimension of an image
        Parameters:
            img (numpy.ndarray): Input image
        Returns:
            bool: True if the calculation is successful, otherwise False
    )pbdoc");

    // Bind preprocessImage function
    m.def(
        "preprocessImage",
        [](const py::array_t<uint8_t>& img, py::array_t<uint8_t>& grayimg,
           py::array_t<uint8_t>& rgbImg, py::array_t<uint8_t>& mark_img) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat grayMat;
            cv::Mat rgbMat;
            cv::Mat markMat;
            preprocessImage(mat, grayMat, rgbMat, markMat);
            grayimg = matToNumpy(grayMat);
            rgbImg = matToNumpy(rgbMat);
            mark_img = matToNumpy(markMat);
        },
        R"pbdoc(
        Preprocess the input image
        Parameters:
            img (numpy.ndarray): Input image
            grayimg (numpy.ndarray): Output grayscale image
            rgbImg (numpy.ndarray): Output RGB image
            mark_img (numpy.ndarray): Output marked image
    )pbdoc");

    // Bind removeNoise function
    m.def(
        "removeNoise",
        [](py::array_t<uint8_t>& map, bool if_removehotpixel,
           bool if_noiseremoval) {
            cv::Mat mat = numpyToMat(map);
            removeNoise(mat, if_removehotpixel, if_noiseremoval);
            map = matToNumpy(mat);
        },
        R"pbdoc(
        Remove noise from the input image
        Parameters:
            map (numpy.ndarray): Input image
            if_removehotpixel (bool): Flag to remove hot pixels
            if_noiseremoval (bool): Flag to remove noise
    )pbdoc");

    // Bind calculateMeanAndStd function
    m.def(
        "calculateMeanAndStd",
        [](const py::array_t<uint8_t>& map, bool down_sample_mean_std) {
            cv::Mat mat = numpyToMat(map);
            double medianVal;
            double stdDev;
            calculateMeanAndStd(mat, down_sample_mean_std, medianVal, stdDev);
            return py::make_tuple(medianVal, stdDev);
        },
        R"pbdoc(
        Calculate the mean and standard deviation of the input image
        Parameters:
            map (numpy.ndarray): Input image
            down_sample_mean_std (bool): Flag to downsample for mean and std calculation
        Returns:
            tuple: A tuple containing the median value and standard deviation
    )pbdoc");

    // Bind processContours function
    m.def(
        "processContours",
        [](const py::array_t<uint8_t>& grayimg,
           const py::array_t<uint8_t>& rgbImg, py::array_t<uint8_t>& mark_img,
           const std::vector<std::vector<cv::Point>>& contours,
           bool do_star_mark) {
            cv::Mat grayMat = numpyToMat(grayimg);
            cv::Mat rgbMat = numpyToMat(rgbImg);
            cv::Mat markMat;
            auto result = processContours(grayMat, rgbMat, markMat, contours,
                                          do_star_mark);
            mark_img = matToNumpy(markMat);
            return result;
        },
        R"pbdoc(
        Process contours in the input image
        Parameters:
            grayimg (numpy.ndarray): Input grayscale image
            rgbImg (numpy.ndarray): Input RGB image
            mark_img (numpy.ndarray): Output marked image
            contours (List[List[cv::Point]]): List of contours
            do_star_mark (bool): Flag to mark stars
        Returns:
            tuple: A tuple containing the number of stars, HFR value, list of HFR values, and list of star sizes
    )pbdoc");

    // Bind starDetectAndHfr function
    m.def(
        "starDetectAndHfr",
        [](const py::array_t<uint8_t>& img, bool if_removehotpixel,
           bool if_noiseremoval, bool do_star_mark, bool down_sample_mean_std,
           py::array_t<uint8_t>& mark_img) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat markMat;
            auto result =
                starDetectAndHfr(mat, if_removehotpixel, if_noiseremoval,
                                 do_star_mark, down_sample_mean_std, markMat);
            mark_img = matToNumpy(std::get<0>(result));
            return py::make_tuple(mark_img, std::get<1>(result),
                                  std::get<2>(result), std::get<3>(result));
        },
        R"pbdoc(
        Detect stars and calculate HFR in the input image
        Parameters:
            img (numpy.ndarray): Input image
            if_removehotpixel (bool): Flag to remove hot pixels
            if_noiseremoval (bool): Flag to remove noise
            do_star_mark (bool): Flag to mark stars
            down_sample_mean_std (bool): Flag to downsample for mean and std calculation
            mark_img (numpy.ndarray): Output marked image
        Returns:
            tuple: A tuple containing the marked image, number of stars, HFR value, and JSON object with additional information
    )pbdoc");

    // Bind calculateHist function
    m.def(
        "calculateHist",
        [](const py::array_t<uint8_t>& img, int histSize = DEFAULT_HIST_SIZE,
           bool normalize = false) {
            cv::Mat mat = numpyToMat(img);
            std::vector<cv::Mat> hist = calculateHist(mat, histSize, normalize);
            std::vector<py::array_t<uint8_t>> result;
            result.reserve(hist.size());
            for (const auto& h : hist) {
                result.push_back(matToNumpy(h));
            }
            return result;
        },
        R"pbdoc(
        Calculate the histogram of an image
        Parameters:
            img (numpy.ndarray): Input image
            histSize (int): Size of the histogram
            normalize (bool): Flag to normalize the histogram
        Returns:
            List[numpy.ndarray]: List of histograms for each channel
    )pbdoc",
        py::arg("img"), py::arg("histSize") = DEFAULT_HIST_SIZE,
        py::arg("normalize") = false);

    // Bind calculateGrayHist function
    m.def(
        "calculateGrayHist",
        [](const py::array_t<uint8_t>& img, int histSize = DEFAULT_HIST_SIZE,
           bool normalize = false) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat hist = calculateGrayHist(mat, histSize, normalize);
            return matToNumpy(hist);
        },
        R"pbdoc(
        Calculate the grayscale histogram of an image
        Parameters:
            img (numpy.ndarray): Input image
            histSize (int): Size of the histogram
            normalize (bool): Flag to normalize the histogram
        Returns:
            numpy.ndarray: Grayscale histogram
    )pbdoc",
        py::arg("img"), py::arg("histSize") = DEFAULT_HIST_SIZE,
        py::arg("normalize") = false);

    // Bind calculateCDF function
    m.def(
        "calculateCDF",
        [](const py::array_t<uint8_t>& hist) {
            cv::Mat mat = numpyToMat(hist);
            cv::Mat cdf = calculateCDF(mat);
            return matToNumpy(cdf);
        },
        R"pbdoc(
        Calculate the cumulative distribution function (CDF) of a histogram
        Parameters:
            hist (numpy.ndarray): Input histogram
        Returns:
            numpy.ndarray: CDF of the histogram
    )pbdoc");

    // Bind equalizeHistogram function
    m.def(
        "equalizeHistogram",
        [](const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat equalized = equalizeHistogram(mat);
            return matToNumpy(equalized);
        },
        R"pbdoc(
        Equalize the histogram of an image
        Parameters:
            img (numpy.ndarray): Input image
        Returns:
            numpy.ndarray: Image with equalized histogram
    )pbdoc");

    // Bind drawHistogram function
    m.def(
        "drawHistogram",
        [](const py::array_t<uint8_t>& hist, int histSize = DEFAULT_HIST_SIZE,
           int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT) {
            cv::Mat mat = numpyToMat(hist);
            cv::Mat histImage = drawHistogram(mat, histSize, width, height);
            return matToNumpy(histImage);
        },
        R"pbdoc(
        Draw the histogram of an image
        Parameters:
            hist (numpy.ndarray): Input histogram
            histSize (int): Size of the histogram
            width (int): Width of the histogram image
            height (int): Height of the histogram image
        Returns:
            numpy.ndarray: Image of the histogram
    )pbdoc",
        py::arg("hist"), py::arg("histSize") = DEFAULT_HIST_SIZE,
        py::arg("width") = DEFAULT_WIDTH, py::arg("height") = DEFAULT_HEIGHT);

    // Bind StretchParams struct
    py::class_<StretchParams>(m, "StretchParams")
        .def(py::init<>())
        .def_readwrite("shadows", &StretchParams::shadows)
        .def_readwrite("tones", &StretchParams::tones)
        .def_readwrite("highlights", &StretchParams::highlights);

    // Bind stretchWhiteBalance function
    m.def(
        "stretchWhiteBalance",
        [](const std::vector<py::array_t<uint8_t>>& hists,
           const std::vector<py::array_t<uint8_t>>& bgrPlanes) {
            std::vector<cv::Mat> histMats;
            std::vector<cv::Mat> bgrMats;
            histMats.reserve(hists.size());
            for (const auto& h : hists) {
                histMats.push_back(numpyToMat(h));
            }
            bgrMats.reserve(bgrPlanes.size());
            for (const auto& b : bgrPlanes) {
                bgrMats.push_back(numpyToMat(b));
            }
            cv::Mat result = stretchWhiteBalance(histMats, bgrMats);
            return matToNumpy(result);
        },
        R"pbdoc(
        Perform white balance stretching on an image
        Parameters:
            hists (List[numpy.ndarray]): Vector of histograms for each color channel
            bgrPlanes (List[numpy.ndarray]): Vector of BGR color planes
        Returns:
            numpy.ndarray: The white-balanced image
        Raises:
            ValueError: If input vectors don't contain exactly 3 channels
    )pbdoc");

    // Bind stretchGray function
    m.def(
        "stretchGray",
        [](const py::array_t<uint8_t>& hist, py::array_t<uint8_t>& plane) {
            cv::Mat histMat = numpyToMat(hist);
            cv::Mat planeMat = numpyToMat(plane);
            cv::Mat result = stretchGray(histMat, planeMat);
            plane = matToNumpy(planeMat);
            return matToNumpy(result);
        },
        R"pbdoc(
        Stretch a grayscale image using its histogram
        Parameters:
            hist (numpy.ndarray): Histogram of the grayscale image
            plane (numpy.ndarray): Input grayscale image plane
        Returns:
            numpy.ndarray: The stretched grayscale image
        Raises:
            ValueError: If input histogram or plane is empty
    )pbdoc");

    // Bind grayStretch function
    m.def(
        "grayStretch",
        [](const py::array_t<uint8_t>& img,
           double blackClip = DEFAULT_BLACK_CLIP,
           double targetBkg = DEFAULT_TARGET_BKG) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat result = grayStretch(mat, blackClip, targetBkg);
            return matToNumpy(result);
        },
        R"pbdoc(
        Perform gray level stretching with configurable parameters
        Parameters:
            img (numpy.ndarray): Input grayscale image
            blackClip (float): Black clipping factor
            targetBkg (float): Target background value
        Returns:
            numpy.ndarray: The stretched grayscale image
    )pbdoc",
        py::arg("img"), py::arg("blackClip") = DEFAULT_BLACK_CLIP,
        py::arg("targetBkg") = DEFAULT_TARGET_BKG);

    // Bind stretchOneChannel function
    m.def(
        "stretchOneChannel",
        [](const py::array_t<uint8_t>& normalizedImg,
           const StretchParams& params) {
            cv::Mat mat = numpyToMat(normalizedImg);
            cv::Mat result = stretchOneChannel(mat, params);
            return matToNumpy(result);
        },
        R"pbdoc(
        Stretch a single channel using provided parameters
        Parameters:
            normalizedImg (numpy.ndarray): Normalized input image (0.0 to 1.0)
            params (StretchParams): Stretching parameters
        Returns:
            numpy.ndarray: The stretched channel
    )pbdoc");

    // Bind stretchThreeChannels function
    m.def(
        "stretchThreeChannels",
        [](const py::array_t<uint8_t>& img, const std::vector<double>& shadows,
           const std::vector<double>& midtones,
           const std::vector<double>& highlights, int inputRange,
           bool doJpg = false) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat result = stretchThreeChannels(
                mat, shadows, midtones, highlights, inputRange, doJpg);
            return matToNumpy(result);
        },
        R"pbdoc(
        Stretch all three channels of an image independently
        Parameters:
            img (numpy.ndarray): Input BGR image
            shadows (List[float]): Shadow levels for each channel
            midtones (List[float]): Midtone levels for each channel
            highlights (List[float]): Highlight levels for each channel
            inputRange (int): Input image range (e.g., 255 for 8-bit)
            doJpg (bool): Whether to output 8-bit (true) or 16-bit (false)
        Returns:
            numpy.ndarray: The stretched color image
        Raises:
            ValueError: If input vectors don't match channel count
    )pbdoc",
        py::arg("img"), py::arg("shadows"), py::arg("midtones"),
        py::arg("highlights"), py::arg("inputRange"), py::arg("doJpg") = false);

    // Bind calculateStretchParameters function
    m.def(
        "calculateStretchParameters",
        [](const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            auto result = calculateStretchParameters(mat);
            return py::make_tuple(std::get<0>(result), std::get<1>(result),
                                  std::get<2>(result));
        },
        R"pbdoc(
        Calculate optimal stretch parameters for an image
        Parameters:
            img (numpy.ndarray): Input image
        Returns:
            tuple: A tuple containing shadows, midtones, and highlights
    )pbdoc");

    // Bind autoStretch function
    m.def(
        "autoStretch",
        [](const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat result = autoStretch(mat);
            return matToNumpy(result);
        },
        R"pbdoc(
        Perform automatic stretching based on image content
        Parameters:
            img (numpy.ndarray): Input image (grayscale or color)
        Returns:
            numpy.ndarray: The automatically stretched image
    )pbdoc");

    // Bind adaptiveStretch function
    m.def(
        "adaptiveStretch",
        [](const py::array_t<uint8_t>& img, int blockSize = 16) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat result = adaptiveStretch(mat, blockSize);
            return matToNumpy(result);
        },
        R"pbdoc(
        Perform adaptive local stretching using block processing
        Parameters:
            img (numpy.ndarray): Input image
            blockSize (int): Size of local processing blocks
        Returns:
            numpy.ndarray: The adaptively stretched image
        Raises:
            ValueError: If blockSize is less than 1
    )pbdoc",
        py::arg("img"), py::arg("blockSize") = 16);

    // Bind YCbCr struct
    py::class_<YCbCr>(m, "YCbCr")
        .def(py::init<>())
        .def_readwrite("y", &YCbCr::y)
        .def_readwrite("cb", &YCbCr::cb)
        .def_readwrite("cr", &YCbCr::cr);

    // Bind dct function
    m.def(
        "dct",
        [](const py::array_t<uint8_t>& input, py::array_t<uint8_t>& output) {
            cv::Mat input_mat = numpyToMat(input);
            cv::Mat output_mat;
            dct(input_mat, output_mat);
            output = matToNumpy(output_mat);
        },
        R"pbdoc(
        Perform Discrete Cosine Transform (DCT) on the input image
        Parameters:
            input (numpy.ndarray): Input image matrix
            output (numpy.ndarray): Output matrix to store the DCT result
    )pbdoc");

    // Bind rgbToYCbCr function
    m.def(
        "rgbToYCbCr",
        [](const py::array_t<uint8_t>& rgb) {
            cv::Vec<unsigned char, 3> rgb_vec =
                *reinterpret_cast<const cv::Vec<unsigned char, 3>*>(rgb.data());
            return rgbToYCbCr(rgb_vec);
        },
        R"pbdoc(
        Convert an RGB color to YCbCr color space
        Parameters:
            rgb (numpy.ndarray): Input RGB color
        Returns:
            YCbCr: The YCbCr color space values
    )pbdoc");

    // Bind encodeThumbHash function
    m.def(
        "encodeThumbHash",
        [](const py::array_t<uint8_t>& image) {
            cv::Mat mat = numpyToMat(image);
            return encodeThumbHash(mat);
        },
        R"pbdoc(
        Encode an image into a ThumbHash
        Parameters:
            image (numpy.ndarray): Input image to be encoded
        Returns:
            List[float]: Encoded ThumbHash
    )pbdoc");

    // Bind decodeThumbHash function
    m.def(
        "decodeThumbHash",
        [](const std::vector<double>& thumbHash, int width, int height) {
            cv::Mat result = decodeThumbHash(thumbHash, width, height);
            return matToNumpy(result);
        },
        R"pbdoc(
        Decode a ThumbHash into an image
        Parameters:
            thumbHash (List[float]): Encoded ThumbHash data
            width (int): Width of the output thumbnail image
            height (int): Height of the output thumbnail image
        Returns:
            numpy.ndarray: Decoded thumbnail image
    )pbdoc");

    // Bind insideCircle function
    m.def("insideCircle", &insideCircle, R"pbdoc(
        Check if a point is inside a circle
        Parameters:
            xCoord (int): X coordinate of the point
            yCoord (int): Y coordinate of the point
            centerX (int): X coordinate of the circle center
            centerY (int): Y coordinate of the circle center
            radius (float): Radius of the circle
        Returns:
            bool: True if the point is inside the circle, otherwise False
    )pbdoc");

    // Bind checkElongated function
    m.def("checkElongated", &checkElongated, R"pbdoc(
        Check if a rectangle is elongated
        Parameters:
            width (int): Width of the rectangle
            height (int): Height of the rectangle
        Returns:
            bool: True if the rectangle is elongated, otherwise False
    )pbdoc");

    // Bind checkWhitePixel function
    m.def(
        "checkWhitePixel",
        [](const py::array_t<uint8_t>& rect_contour, int x_coord, int y_coord) {
            cv::Mat mat = numpyToMat(rect_contour);
            return checkWhitePixel(mat, x_coord, y_coord);
        },
        R"pbdoc(
        Check if a pixel is white
        Parameters:
            rect_contour (numpy.ndarray): Input image
            x_coord (int): X coordinate of the pixel
            y_coord (int): Y coordinate of the pixel
        Returns:
            int: 1 if the pixel is white, otherwise 0
    )pbdoc");

    // Bind checkEightSymmetryCircle function
    m.def(
        "checkEightSymmetryCircle",
        [](const py::array_t<uint8_t>& rect_contour, const cv::Point& center,
           int x_p, int y_p) {
            cv::Mat mat = numpyToMat(rect_contour);
            return checkEightSymmetryCircle(mat, center, x_p, y_p);
        },
        R"pbdoc(
        Check eight symmetry of a circle
        Parameters:
            rect_contour (numpy.ndarray): Input image
            center (cv::Point): Center of the circle
            x_p (int): X coordinate of the point
            y_p (int): Y coordinate of the point
        Returns:
            int: Symmetry score
    )pbdoc");

    // Bind checkFourSymmetryCircle function
    m.def(
        "checkFourSymmetryCircle",
        [](const py::array_t<uint8_t>& rect_contour, const cv::Point& center,
           float radius) {
            cv::Mat mat = numpyToMat(rect_contour);
            return checkFourSymmetryCircle(mat, center, radius);
        },
        R"pbdoc(
        Check four symmetry of a circle
        Parameters:
            rect_contour (numpy.ndarray): Input image
            center (cv::Point): Center of the circle
            radius (float): Radius of the circle
        Returns:
            int: Symmetry score
    )pbdoc");

    // Bind defineNarrowRadius function
    m.def("defineNarrowRadius", &defineNarrowRadius, R"pbdoc(
        Define narrow radius
        Parameters:
            min_area (int): Minimum area
            max_area (float): Maximum area
            area (float): Area
            scale (float): Scale
        Returns:
            tuple: A tuple containing the radius, a vector of radii, and a vector of scales
    )pbdoc");

    // Bind checkBresenhamCircle function
    m.def(
        "checkBresenhamCircle",
        [](const py::array_t<uint8_t>& rect_contour, float radius,
           float pixel_ratio, bool if_debug = false) {
            cv::Mat mat = numpyToMat(rect_contour);
            return checkBresenhamCircle(mat, radius, pixel_ratio, if_debug);
        },
        R"pbdoc(
        Check Bresenham circle
        Parameters:
            rect_contour (numpy.ndarray): Input image
            radius (float): Radius of the circle
            pixel_ratio (float): Pixel ratio
            if_debug (bool): Debug flag
        Returns:
            bool: True if the circle is valid, otherwise False
    )pbdoc");

    // Bind calculateAverageDeviation function
    m.def(
        "calculateAverageDeviation",
        [](double mid, const py::array_t<uint8_t>& norm_img) {
            cv::Mat mat = numpyToMat(norm_img);
            return calculateAverageDeviation(mid, mat);
        },
        R"pbdoc(
        Calculate average deviation
        Parameters:
            mid (float): Mid value
            norm_img (numpy.ndarray): Normalized image
        Returns:
            float: Average deviation
    )pbdoc");

    // Bind calculateMTF function
    m.def(
        "calculateMTF",
        [](double magnitude, const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat result = calculateMTF(magnitude, mat);
            return matToNumpy(result);
        },
        R"pbdoc(
        Calculate MTF
        Parameters:
            magnitude (float): Magnitude
            img (numpy.ndarray): Input image
        Returns:
            numpy.ndarray: MTF image
    )pbdoc");

    // Bind calculateScale function
    m.def(
        "calculateScale",
        [](const py::array_t<uint8_t>& img, int resize_size = 1552) {
            cv::Mat mat = numpyToMat(img);
            return calculateScale(mat, resize_size);
        },
        R"pbdoc(
        Calculate scale
        Parameters:
            img (numpy.ndarray): Input image
            resize_size (int): Resize size
        Returns:
            float: Scale
    )pbdoc");

    // Bind calculateMedianDeviation function
    m.def(
        "calculateMedianDeviation",
        [](double mid, const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            return calculateMedianDeviation(mid, mat);
        },
        R"pbdoc(
        Calculate median deviation
        Parameters:
            mid (float): Mid value
            img (numpy.ndarray): Input image
        Returns:
            float: Median deviation
    )pbdoc");

    // Bind computeParamsOneChannel function
    m.def(
        "computeParamsOneChannel",
        [](const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            auto result = computeParamsOneChannel(mat);
            return py::make_tuple(std::get<0>(result), std::get<1>(result),
                                  std::get<2>(result));
        },
        R"pbdoc(
        Compute parameters for one channel
        Parameters:
            img (numpy.ndarray): Input image
        Returns:
            tuple: A tuple containing the parameters
    )pbdoc");

    // Bind autoWhiteBalance function
    m.def(
        "autoWhiteBalance",
        [](const py::array_t<uint8_t>& img) {
            cv::Mat mat = numpyToMat(img);
            cv::Mat result = autoWhiteBalance(mat);
            return matToNumpy(result);
        },
        R"pbdoc(
        Perform automatic white balance
        Parameters:
            img (numpy.ndarray): Input image
        Returns:
            numpy.ndarray: White-balanced image
    )pbdoc");

    // Bind loadImage function
    m.def(
        "loadImage",
        [](const std::string& filename, int flags = 1) {
            cv::Mat mat = loadImage(filename, flags);
            return matToNumpy(mat);
        },
        R"pbdoc(
        Load a single image
        Parameters:
            filename (str): Path to the image file
            flags (int): Flags for image loading
        Returns:
            numpy.ndarray: Loaded image
    )pbdoc",
        py::arg("filename"), py::arg("flags") = 1);

    // Bind loadImages function
    m.def(
        "loadImages",
        [](const std::string& folder,
           const std::vector<std::string>& filenames = {}, int flags = 1) {
            std::vector<std::pair<std::string, cv::Mat>> images =
                loadImages(folder, filenames, flags);
            std::vector<std::pair<std::string, py::array_t<uint8_t>>> result;
            result.reserve(images.size());
            for (const auto& [name, mat] : images) {
                result.emplace_back(name, matToNumpy(mat));
            }
            return result;
        },
        R"pbdoc(
        Load all images from a folder
        Parameters:
            folder (str): Path to the folder
            filenames (List[str]): List of filenames to load
            flags (int): Flags for image loading
        Returns:
            List[Tuple[str, numpy.ndarray]]: List of loaded images with their filenames
    )pbdoc",
        py::arg("folder"), py::arg("filenames") = std::vector<std::string>{},
        py::arg("flags") = 1);

    // Bind saveImage function
    m.def("saveImage", &saveImage, R"pbdoc(
        Save an image to a file
        Parameters:
            filename (str): Path to the output file
            image (numpy.ndarray): Image to save
        Returns:
            bool: True if the image was saved successfully, otherwise False
    )pbdoc");

    // Bind saveMatTo8BitJpg function
    m.def("saveMatTo8BitJpg", &saveMatTo8BitJpg, R"pbdoc(
        Save a cv::Mat image to an 8-bit JPG file
        Parameters:
            image (numpy.ndarray): Image to save
            output_path (str): Path to the output file
        Returns:
            bool: True if the image was saved successfully, otherwise False
    )pbdoc");

    // Bind saveMatTo16BitPng function
    m.def("saveMatTo16BitPng", &saveMatTo16BitPng, R"pbdoc(
        Save a cv::Mat image to a 16-bit PNG file
        Parameters:
            image (numpy.ndarray): Image to save
            output_path (str): Path to the output file
        Returns:
            bool: True if the image was saved successfully, otherwise False
    )pbdoc");

    // Bind saveMatToFits function
    m.def("saveMatToFits", &saveMatToFits, R"pbdoc(
        Save a cv::Mat image to a FITS file
        Parameters:
            image (numpy.ndarray): Image to save
            output_path (str): Path to the output file
        Returns:
            bool: True if the image was saved successfully, otherwise False
    )pbdoc");
}