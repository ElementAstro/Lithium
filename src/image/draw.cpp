/**
 * Star recognizer using the CImg library and CCFits.
 *
 * Copyright (C) 2015 Carsten Schmitt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <assert.h>
#include "cimg/CImg.h"
#include <tuple>
#include <iomanip>
#include <functional>
#include <list>
#include <set>
#include <array>
#include <vector>
#include <gsl/gsl_multifit_nlin.h>
using namespace std;
using namespace cimg_library;
typedef tuple<int
			  /*x*/
			  ,
			  int
			  /*y*/
			  >
	PixelPosT;
typedef set<PixelPosT> PixelPosSetT;
typedef list<PixelPosT> PixelPosListT;
typedef tuple<float, float> PixSubPosT;
typedef tuple<float
			  /*x1*/
			  ,
			  float
			  /*y1*/
			  ,
			  float
			  /*x2*/
			  ,
			  float
			  /*y2*/
			  >
	FrameT;
struct StarInfoT
{
	FrameT clusterFrame;
	FrameT cogFrame;
	FrameT hfdFrame;
	PixSubPosT cogCentroid;
	PixSubPosT subPixelInterpCentroid;
	float hfd;
	float fwhmHorz;
	float fwhmVert;
	float maxPixValue;
	bool saturated;
};
typedef list<StarInfoT> StarInfoListT;
/**
 * Get all pixels inside a radius: https://stackoverflow.com/questions/14487322/get-all-pixel-array-inside-circle
 * Algorithm: https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 */
bool insideCircle(float inX
				  /*pos of x*/
				  ,
				  float inY
				  /*pos of y*/
				  ,
				  float inCenterX, float inCenterY, float inRadius)
{
	return (pow(inX - inCenterX, 2.0) + pow(inY - inCenterY, 2.0) <= pow(inRadius, 2.0));
}
#include <fitsio.h>
void readFile(CImg<float> &inImg, const string &inFilename, int *outBitPix = 0)
{
	int status = 0;
	fitsfile *fptr;
	// Open the FITS file
	fits_open_file(&fptr, inFilename.c_str(), READONLY, &status);
	if (status != 0)
	{
		throw std::runtime_error("Error opening file: " + inFilename);
	}
	// Get the image dimensions and bitpix
	int naxis;
	long naxes[2];
	fits_get_img_param(fptr, 2, outBitPix, &naxis, naxes, &status);
	if (status != 0)
	{
		throw std::runtime_error("Error getting image parameters");
	}
	// Allocate memory for the image data
	float *imgData = new float[naxes[0] * naxes[1]];
	// Read the image data
	int anyNull;
	long fpixel[2] = {
		1, 1};
	fits_read_pix(fptr, TFLOAT, fpixel, naxes[0] * naxes[1], NULL, imgData, &anyNull, &status);
	if (status != 0)
	{
		throw std::runtime_error("Error reading image data");
	}
	// Copy the image data to the CImg object
	inImg.resize(naxes[0], naxes[1], 1, 1);
	cimg_forXY(inImg, x, y)
	{
		inImg(x, inImg.height() - y - 1) = imgData[x + y * naxes[0]];
	}
	// Free memory used for image data
	delete[] imgData;
	// Close the FITS file
	fits_close_file(fptr, &status);
	if (status != 0)
	{
		throw std::runtime_error("Error closing file: " + inFilename);
	}
}
void thresholdOtsu(const CImg<float> &inImg, long inBitPix, CImg<float> *outBinImg)
{
	CImg<> hist = inImg.get_histogram(pow(2.0, inBitPix));
	float sum = 0;
	cimg_forX(hist, pos)
	{
		sum += pos * hist[pos];
	}
	float numPixels = inImg.width() * inImg.height();
	float sumB = 0, wB = 0, max = 0.0;
	float threshold1 = 0.0, threshold2 = 0.0;
	cimg_forX(hist, i)
	{
		wB += hist[i];
		if (!wB)
		{
			continue;
		}
		float wF = numPixels - wB;
		if (!wF)
		{
			break;
		}
		sumB += i * hist[i];
		float mF = (sum - sumB) / wF;
		float mB = sumB / wB;
		float diff = mB - mF;
		float bw = wB * wF * pow(diff, 2.0);
		if (bw >= max)
		{
			threshold1 = i;
			if (bw > max)
			{
				threshold2 = i;
			}
			max = bw;
		}
	}
	// end loop
	float th = (threshold1 + threshold2) / 2.0;
	*outBinImg = inImg;
	// Create a copy
	outBinImg->threshold(th);
}
/**
 * Removes all white neighbours arond pixel from whitePixels
 * if they exist and adds them to pixelsToBeProcessed.
 */
void getAndRemoveNeighbours(PixelPosT inCurPixelPos, PixelPosSetT *inoutWhitePixels, PixelPosListT *inoutPixelsToBeProcessed)
{
	const size_t _numPixels = 8, _x = 0, _y = 1;
	const int offsets[_numPixels][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
	for (size_t p = 0; p < _numPixels; ++p)
	{
		PixelPosT curPixPos(std::get<0>(inCurPixelPos) + offsets[p][_x], std::get<1>(inCurPixelPos) + offsets[p][_y]);
		PixelPosSetT::iterator itPixPos = inoutWhitePixels->find(curPixPos);
		if (itPixPos != inoutWhitePixels->end())
		{
			const PixelPosT &curPixPos = *itPixPos;
			inoutPixelsToBeProcessed->push_back(curPixPos);
			inoutWhitePixels->erase(itPixPos);
			// Remove white pixel from "white set" since it has been now processed
		}
	}
	return;
}
template <typename T>
void clusterStars(const CImg<T> &inImg, StarInfoListT *outStarInfos)
{
	PixelPosSetT whitePixels;
	cimg_forXY(inImg, x, y)
	{
		if (inImg(x, y))
		{
			whitePixels.insert(whitePixels.end(), PixelPosT(x, y));
		}
	}
	// Iterate over white pixels as long as set is not empty
	while (whitePixels.size())
	{
		PixelPosListT pixelsToBeProcessed;
		PixelPosSetT::iterator itWhitePixPos = whitePixels.begin();
		pixelsToBeProcessed.push_back(*itWhitePixPos);
		whitePixels.erase(itWhitePixPos);
		FrameT frame(inImg.width(), inImg.height(), 0, 0);
		while (!pixelsToBeProcessed.empty())
		{
			PixelPosT curPixelPos = pixelsToBeProcessed.front();
			// Determine boundaries (min max in x and y directions)
			if (std::get<0>(curPixelPos)
				/*x*/
				< std::get<0>(frame)
				/*x1*/
			)
			{
				std::get<0>(frame) = std::get<0>(curPixelPos);
			}
			if (std::get<0>(curPixelPos)
				/*x*/
				> std::get<2>(frame)
				/*x2*/
			)
			{
				std::get<2>(frame) = std::get<0>(curPixelPos);
			}
			if (std::get<1>(curPixelPos)
				/*y*/
				< std::get<1>(frame)
				/*y1*/
			)
			{
				std::get<1>(frame) = std::get<1>(curPixelPos);
			}
			if (std::get<1>(curPixelPos)
				/*y*/
				> std::get<3>(frame)
				/*y2*/
			)
			{
				std::get<3>(frame) = std::get<1>(curPixelPos);
			}
			getAndRemoveNeighbours(curPixelPos, &whitePixels, &pixelsToBeProcessed);
			pixelsToBeProcessed.pop_front();
		}
		// Create new star-info and set cluster-frame.
		// NOTE: we may use new to avoid copy of StarInfoT...
		StarInfoT starInfo;
		starInfo.clusterFrame = frame;
		outStarInfos->push_back(starInfo);
	}
}
float calcIx2(const CImg<float> &img, int x)
{
	float Ix = 0;
	cimg_forY(img, y)
	{
		Ix += pow(img(x, y), 2.0) * (float)x;
	}
	return Ix;
}
float calcJy2(const CImg<float> &img, int y)
{
	float Iy = 0;
	cimg_forX(img, x)
	{
		Iy += pow(img(x, y), 2.0) * (float)y;
	}
	return Iy;
}
// Calculate Intensity Weighted Center (IWC)
void calcIntensityWeightedCenter(const CImg<float> &inImg, float *outX, float *outY)
{
	assert(outX && outY);
	// Determine weighted centroid - See https://cdn.intechopen.com/pdfs-wm/26716.pdf
	float Imean2 = 0, Jmean2 = 0, Ixy2 = 0;
	for (size_t i = 0; i < inImg.width(); ++i)
	{
		Imean2 += calcIx2(inImg, i);
		cimg_forY(inImg, y)
		{
			Ixy2 += pow(inImg(i, y), 2.0);
		}
	}
	for (size_t i = 0; i < inImg.height(); ++i)
	{
		Jmean2 += calcJy2(inImg, i);
	}
	*outX = Imean2 / Ixy2;
	*outY = Jmean2 / Ixy2;
}
void calcSubPixelCenter(const CImg<float> &inImg, float *outX, float *outY, size_t inNumIter = 10
						/*num iterations*/
)
{
	// Sub pixel interpolation
	float c, a1, a2, a3, a4, b1, b2, b3, b4;
	float a1n, a2n, a3n, a4n, b1n, b2n, b3n, b4n;
	assert(inImg.width() == 3 && inImg.height() == 3);
	b1 = inImg(0, 0);
	a2 = inImg(1, 0);
	b2 = inImg(2, 0);
	a1 = inImg(0, 1);
	c = inImg(1, 1);
	a3 = inImg(2, 1);
	b4 = inImg(0, 2);
	a4 = inImg(1, 2);
	b3 = inImg(2, 2);
	for (size_t i = 0; i < inNumIter; ++i)
	{
		float c2 = 2 * c;
		float sp1 = (a1 + a2 + c2) / 4;
		float sp2 = (a2 + a3 + c2) / 4;
		float sp3 = (a3 + a4 + c2) / 4;
		float sp4 = (a4 + a1 + c2) / 4;
		// New maximum is center
		float newC = std::max({sp1, sp2, sp3, sp4});
		// Calc position of new center
		float ad = pow(2.0, -((float)i + 1));
		if (newC == sp1)
		{
			*outX = *outX - ad;
			// to the left
			*outY = *outY - ad;
			// to the top
			// Calculate new sub pixel values
			b1n = (a1 + a2 + 2 * b1) / 4;
			b2n = (c + b2 + 2 * a2) / 4;
			b3n = sp3;
			b4n = (b4 + c + 2 * a1) / 4;
			a1n = (b1n + c + 2 * a1) / 4;
			a2n = (b1n + c + 2 * a2) / 4;
			a3n = sp2;
			a4n = sp4;
		}
		else if (newC == sp2)
		{
			*outX = *outX + ad;
			// to the right
			*outY = *outY - ad;
			// to the top
			// Calculate new sub pixel values
			b1n = (2 * a2 + b1 + c) / 4;
			b2n = (2 * b2 + a3 + a2) / 4;
			b3n = (2 * a3 + b3 + c) / 4;
			b4n = sp4;
			a1n = sp1;
			a2n = (b2n + c + 2 * a2) / 4;
			a3n = (b2n + c + 2 * a3) / 4;
			a4n = sp3;
		}
		else if (newC == sp3)
		{
			*outX = *outX + ad;
			// to the right
			*outY = *outY + ad;
			// to the bottom
			// Calculate new sub pixel values
			b1n = sp1;
			b2n = (b2 + 2 * a3 + c) / 4;
			b3n = (2 * b3 + a3 + a4) / 4;
			b4n = (2 * a4 + b4 + c) / 4;
			a1n = sp4;
			a2n = sp2;
			a3n = (b3n + 2 * a3 + c) / 4;
			a4n = (b3n + 2 * a4 + c) / 4;
		}
		else
		{
			*outX = *outX - ad;
			// to the left
			*outY = *outY + ad;
			// to the bottom
			// Calculate new sub pixel values
			b1n = (2 * a1 + b1 + c) / 4;
			b2n = sp2;
			b3n = (c + b3 + 2 * a4) / 4;
			b4n = (2 * b4 + a1 + a4) / 4;
			a1n = (b4n + 2 * a1 + c) / 4;
			a2n = sp1;
			a3n = sp3;
			a4n = (b4n + 2 * a4 + c) / 4;
		}
		c = newC;
		// Oi = Oi+1
		a1 = a1n;
		a2 = a2n;
		a3 = a3n;
		a4 = a4n;
		b1 = b1n;
		b2 = b2n;
		b3 = b3n;
		b4 = b4n;
	}
}
void calcCentroid(const CImg<float> &inImg, const FrameT &inFrame, PixSubPosT *outPixelPos, PixSubPosT *outSubPixelPos = 0, size_t inNumIterations = 10)
{
	// Get frame sub img
	CImg<float> subImg = inImg.get_crop(std::get<0>(inFrame), std::get<1>(inFrame), std::get<2>(inFrame), std::get<3>(inFrame));
	float &xc = std::get<0>(*outPixelPos);
	float &yc = std::get<1>(*outPixelPos);
	// 1. Calculate the IWC
	calcIntensityWeightedCenter(subImg, &xc, &yc);
	if (outSubPixelPos)
	{
		// 2. Round to nearest integer and then iteratively improve.
		int xi = floor(xc + 0.5);
		int yi = floor(yc + 0.5);
		CImg<float> img3x3 = inImg.get_crop(xi - 1
											/*x0*/
											,
											yi - 1
											/*y0*/
											,
											xi + 1
											/*x1*/
											,
											yi + 1
											/*y1*/
		);
		// 3. Interpolate using sub-pixel algorithm
		float xsc = xi, ysc = yi;
		calcSubPixelCenter(img3x3, &xsc, &ysc, inNumIterations);
		std::get<0>(*outSubPixelPos) = xsc;
		std::get<1>(*outSubPixelPos) = ysc;
	}
}
/**
 * Expects star centered in the middle of the image (in x and y) and mean background subtracted from image.
 *
 * HDF calculation: https://www005.upp.so-net.ne.jp/k_miyash/occ02/halffluxdiameter/halffluxdiameter_en.html
 *                  https://www.cyanogen.com/help/maximdl/Half-Flux.htm
 *
 * NOTE: Currently the accuracy is limited by the insideCircle function (-> sub-pixel accuracy).
 * NOTE: The HFD is estimated in case there is no flux (HFD ~ sqrt(2) * inOuterDiameter / 2).
 * NOTE: The outer diameter is usually a value which depends on the properties of the optical
 *       system and also on the seeing conditions. The HFD value calculated depends on this
 *       outer diameter value.
 */
float calcHfd(const CImg<float> &inImage, unsigned int inOuterDiameter)
{
	// Sum up all pixel values in whole circle
	float outerRadius = inOuterDiameter / 2;
	float sum = 0, sumDist = 0;
	int centerX = ceil(inImage.width() / 2.0);
	int centerY = ceil(inImage.height() / 2.0);
	cimg_forXY(inImage, x, y)
	{
		if (insideCircle(x, y, centerX, centerY, outerRadius))
		{
			sum += inImage(x, y);
			sumDist += inImage(x, y) * sqrt(pow((float)x - (float)centerX, 2.0f) + pow((float)y - (float)centerY, 2.0f));
		}
	}
	// NOTE: Multiplying with 2 is required since actually just the HFR is calculated above
	return (sum ? 2.0 * sumDist / sum : sqrt(2.0) * outerRadius);
}
/**********************************************************************
 * Helper classes
 **********************************************************************/
struct DataPointT
{
	float x;
	float y;
	DataPointT(float inX = 0, float inY = 0) : x(inX), y(inY)
	{
	}
};
typedef vector<DataPointT> DataPointsT;
struct GslMultiFitDataT
{
	float y;
	float sigma;
	DataPointT pt;
};
typedef vector<GslMultiFitDataT> GslMultiFitParmsT;
/**********************************************************************
 * Curve to fit to is supplied by traits.
 **********************************************************************/
template <class FitTraitsT>
class CurveFitTmplT
{
public:
	typedef typename FitTraitsT::CurveParamsT CurveParamsT;
	/**
	 * DataAccessor allows specifying how x,y data is accessed.
	 * See https://en.wikipedia.org/wiki/Approximation_error for expl. of rel and abs errors.
	 */
	template <typename DataAccessorT>
	static int
	fitGslLevenbergMarquart(const typename DataAccessorT::TypeT &inData, typename CurveParamsT::TypeT *outResults,
							double inEpsAbs, double inEpsRel, size_t inNumMaxIter = 500)
	{
		GslMultiFitParmsT gslMultiFitParms(inData.size());
		// Fill in the parameters
		for (typename DataAccessorT::TypeT::const_iterator it = inData.begin(); it != inData.end(); ++it)
		{
			size_t idx = std::distance(inData.begin(), it);
			const DataPointT &dataPoint = DataAccessorT::getDataPoint(idx, it);
			gslMultiFitParms[idx].y = dataPoint.y;
			gslMultiFitParms[idx].sigma = 0.1f;
			gslMultiFitParms[idx].pt = dataPoint;
		}
		// Fill in function info
		gsl_multifit_function_fdf f;
		f.f = FitTraitsT::gslFx;
		f.df = FitTraitsT::gslDfx;
		f.fdf = FitTraitsT::gslFdfx;
		f.n = inData.size();
		f.p = FitTraitsT::CurveParamsT::_Count;
		f.params = &gslMultiFitParms;
		gsl_vector *guess = gsl_vector_alloc(FitTraitsT::CurveParamsT::_Count);
		// Allocate the guess vector
		FitTraitsT::makeGuess(gslMultiFitParms, guess);
		// Make initial guesses based on the data
		// Create a Levenberg-Marquardt solver with n data points and m parameters
		gsl_multifit_fdfsolver *solver = gsl_multifit_fdfsolver_alloc(gsl_multifit_fdfsolver_lmsder,
																	  inData.size(), FitTraitsT::CurveParamsT::_Count);
		gsl_multifit_fdfsolver_set(solver, &f, guess);
		// Initialize the solver
		int status, i = 0;
		// Iterate to to find a result
		do
		{
			i++;
			status = gsl_multifit_fdfsolver_iterate(solver);
			// returns 0 in case of success
			if (status)
			{
				break;
			}
			status = gsl_multifit_test_delta(solver->dx, solver->x, inEpsAbs, inEpsRel);
		} while (status == GSL_CONTINUE && i < inNumMaxIter);
		// Store the results to be returned to the user (copy from gsl_vector to result structure)
		for (size_t i = 0; i < FitTraitsT::CurveParamsT::_Count; ++i)
		{
			typename FitTraitsT::CurveParamsT::TypeE idx = static_cast<typename FitTraitsT::CurveParamsT::TypeE>(i);
			(*outResults)[idx] = gsl_vector_get(solver->x, idx);
		}
		// Free GSL memory
		gsl_multifit_fdfsolver_free(solver);
		gsl_vector_free(guess);
		return status;
	}
};
/**********************************************************************
 * Gaussian fit traits
 **********************************************************************/
class GaussianFitTraitsT
{
private:
public:
	struct CurveParamsT
	{
		// b = base, p = peak, c = center in x, w = mean width (FWHM)
		enum TypeE
		{
			B_IDX = 0,
			P_IDX,
			C_IDX,
			W_IDX,
			_Count
		};
		struct TypeT : public std::array<float, TypeE::_Count>
		{
			TypeT(const gsl_vector *inVec = 0)
			{
				for (size_t i = 0; i < TypeE::_Count; ++i)
				{
					TypeE idx = static_cast<TypeE>(i);
					(*this)[i] = (inVec ? gsl_vector_get(inVec, idx) : 0);
				}
			}
		};
	};
	/* Makes a guess for b, p, c and w based on the supplied data */
	static void makeGuess(const GslMultiFitParmsT &inData, gsl_vector *guess)
	{
		size_t numDataPoints = inData.size();
		float y_mean = 0;
		float y_max = inData.at(0).pt.y;
		float c = inData.at(0).pt.x;
		for (size_t i = 0; i < numDataPoints; ++i)
		{
			const DataPointT &dataPoint = inData.at(i).pt;
			y_mean += dataPoint.y;
			if (y_max < dataPoint.y)
			{
				y_max = dataPoint.y;
				c = dataPoint.x;
			}
		}
		y_mean /= (float)numDataPoints;
		float w = (inData.at(numDataPoints - 1).pt.x - inData.at(0).pt.x) / 10.0;
		gsl_vector_set(guess, CurveParamsT::B_IDX, y_mean);
		gsl_vector_set(guess, CurveParamsT::P_IDX, y_max);
		gsl_vector_set(guess, CurveParamsT::C_IDX, c);
		gsl_vector_set(guess, CurveParamsT::W_IDX, w);
	}
	/* y = b + p * exp(-0.5f * ((t - c) / w) * ((t - c) / w)) */
	static float fx(float x, const CurveParamsT::TypeT &inParms)
	{
		float b = inParms[CurveParamsT::B_IDX];
		float p = inParms[CurveParamsT::P_IDX];
		float c = inParms[CurveParamsT::C_IDX];
		float w = inParms[CurveParamsT::W_IDX];
		float t = ((x - c) / w);
		t *= t;
		return (b + p * exp(-0.5f * t));
	}
	/* Calculates f(x) = b + p * e^[0.5*((x-c)/w)] for each data point. */
	static int gslFx(const gsl_vector *x, void *inGslParams, gsl_vector *outResultVec)
	{
		CurveParamsT::TypeT curveParams(x);
		// Store the current coefficient values
		const GslMultiFitParmsT *gslParams = ((GslMultiFitParmsT *)inGslParams);
		// Store parameter values
		// Execute Levenberg-Marquart on f(x)
		for (size_t i = 0; i < gslParams->size(); ++i)
		{
			const GslMultiFitDataT &gslData = gslParams->at(i);
			float yi = GaussianFitTraitsT::fx((float)gslData.pt.x, curveParams);
			gsl_vector_set(outResultVec, i, (yi - gslData.y) / gslData.sigma);
		}
		return GSL_SUCCESS;
	}
	/* Calculates the Jacobian (derivative) matrix of f(x) = b + p * e^[0.5*((x-c)/w)^2] for each data point */
	static int gslDfx(const gsl_vector *x, void *params, gsl_matrix *J)
	{
		// Store parameter values
		const GslMultiFitParmsT *gslParams = ((GslMultiFitParmsT *)params);
		// Store current coefficients
		float p = gsl_vector_get(x, CurveParamsT::P_IDX);
		float c = gsl_vector_get(x, CurveParamsT::C_IDX);
		float w = gsl_vector_get(x, CurveParamsT::W_IDX);
		// Store non-changing calculations
		float w2 = w * w;
		float w3 = w2 * w;
		for (size_t i = 0; i < gslParams->size(); ++i)
		{
			const GslMultiFitDataT &gslData = gslParams->at(i);
			float x_minus_c = (gslData.pt.x - c);
			float e = exp(-0.5f * (x_minus_c / w) * (x_minus_c / w));
			gsl_matrix_set(J, i, CurveParamsT::B_IDX, 1 / gslData.sigma);
			gsl_matrix_set(J, i, CurveParamsT::P_IDX, e / gslData.sigma);
			gsl_matrix_set(J, i, CurveParamsT::C_IDX, (p * e * x_minus_c) / (gslData.sigma * w2));
			gsl_matrix_set(J, i, CurveParamsT::W_IDX, (p * e * x_minus_c * x_minus_c) / (gslData.sigma * w3));
		}
		return GSL_SUCCESS;
	}
	/* Invokes f(x) and f'(x) */
	static int gslFdfx(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J)
	{
		gslFx(x, params, f);
		gslDfx(x, params, J);
		return GSL_SUCCESS;
	}
};
typedef list<PixSubPosT> MyDataContainerT;
class MyDataAccessorT
{
public:
	typedef MyDataContainerT TypeT;
	static DataPointT getDataPoint(size_t inIdx, TypeT::const_iterator inIt)
	{
		const PixSubPosT &pos = *inIt;
		DataPointT dp(get<0>(pos)
					  /*inIdx*/
					  ,
					  get<1>(pos)
					  /*y*/
		);
		return dp;
	}
};
FrameT
rectify(const FrameT &inFrame)
{
	float border = 3;
	float border2 = 2.0 * border;
	float width = fabs(std::get<0>(inFrame) - std::get<2>(inFrame)) + border2;
	float height = fabs(std::get<1>(inFrame) - std::get<3>(inFrame)) + border2;
	float L = max(width, height);
	float x0 = std::get<0>(inFrame) - (fabs(width - L) / 2.0) - border;
	float y0 = std::get<1>(inFrame) - (fabs(height - L) / 2.0) - border;
	return FrameT(x0, y0, x0 + L, y0 + L);
}
int main(int argc, char *argv[])
{
	/* outerHfdDiameter depends on pixel size and focal length (and seeing...).
	   Later we may calculate it automatically wihth goven focal length and pixel
	   size of the camera. For now it is a "best guess" value.
	*/
	const unsigned int outerHfdDiameter = 50;
	StarInfoListT starInfos;
	vector<list<StarInfoT *>> starBuckets;
	CImg<float> img;
	int bitPix = 0;
	// Read file to CImg
	try
	{
		cerr << "Opening file " << argv[1] << endl;
		readFile(img, argv[1], &bitPix);
	}
	catch (std::runtime_error &)
	{
		cerr << "Read FITS failed." << endl;
		return 1;
	}
	// Create RGB image from fits file to paint boundaries and centroids (just for visualization)
	CImg<unsigned char> rgbImg(img.width(), img.height(), 1
							   /*depth*/
							   ,
							   3
							   /*3 channels - RGB*/
	);
	float min = img.min(), mm = img.max() - min;
	cimg_forXY(img, x, y)
	{
		int value = 255.0 * (img(x, y) - min) / mm;
		rgbImg(x, y, 0
			   /*red*/
			   ) = value;
		rgbImg(x, y, 1
			   /*green*/
			   ) = value;
		rgbImg(x, y, 2
			   /*blue*/
			   ) = value;
	}
	// AD noise reduction --> In: Loaded image, Out: Noise reduced image
	// NOTE: This step takes a while for big images... too long for usage in a loop ->
	//       Should only be used on image segments, later...
	//
	// https://cimg.sourceforge.net/reference/structcimg__library_1_1CImg.html
	CImg<float> &aiImg = img.blur_anisotropic(30.0f,
											  /*amplitude*/
											  0.5f,
											  /*sharpness*/
											  0.3f,
											  /*anisotropy*/
											  0.6f,
											  /*alpha*/
											  1.1f,
											  /*sigma*/
											  0.8f,
											  /*dl*/
											  30,
											  /*da*/
											  2,
											  /*gauss_prec*/
											  0,
											  /*interpolation_type*/
											  false
											  /*fast_approx*/
	);
	// Thresholding (Otsu) --> In: Noise reduced image, Out: binary image
	CImg<float> binImg;
	thresholdOtsu(aiImg, bitPix, &binImg);
	// Clustering --> In: binary image from thresholding, Out: List of detected stars, subimg-boundaries (x1,y1,x2,y2) for each star
	clusterStars(binImg, &starInfos);
	cerr << "Recognized " << starInfos.size() << " stars..." << endl;
	// Calc brightness boundaries for possible focusing stars
	float maxPossiblePixValue = pow(2.0, bitPix) - 1;
	// For each star
	for (StarInfoListT::iterator it = starInfos.begin(); it != starInfos.end(); ++it)
	{
		const FrameT &frame = it->clusterFrame;
		FrameT &cogFrame = it->cogFrame;
		FrameT &hfdFrame = it->hfdFrame;
		PixSubPosT &cogCentroid = it->cogCentroid;
		PixSubPosT &subPixelInterpCentroid = it->subPixelInterpCentroid;
		float &hfd = it->hfd;
		float &fwhmHorz = it->fwhmHorz;
		float &fwhmVert = it->fwhmVert;
		float &maxPixValue = it->maxPixValue;
		bool &saturated = it->saturated;
		FrameT squareFrame = rectify(frame);
		// Centroid calculation --> In: Handle to full noise reduced image, subimg-boundaries (x1,y1,x2,y2), Out: (x,y) - abs. centroid coordinates
		calcCentroid(aiImg, squareFrame, &cogCentroid, &subPixelInterpCentroid, 10
					 /* num iterations */
		);
		std::get<0>(cogCentroid) += std::get<0>(squareFrame);
		std::get<1>(cogCentroid) += std::get<1>(squareFrame);
		std::get<0>(subPixelInterpCentroid) += std::get<0>(squareFrame);
		std::get<1>(subPixelInterpCentroid) += std::get<1>(squareFrame);
		// Calculate cog boundaries
		float maxClusterEdge = std::max(fabs(std::get<0>(frame) - std::get<2>(frame)), fabs(std::get<1>(frame) - std::get<3>(frame)));
		float cogHalfEdge = ceil(maxClusterEdge / 2.0);
		float cogX = std::get<0>(cogCentroid);
		float cogY = std::get<1>(cogCentroid);
		std::get<0>(cogFrame) = cogX - cogHalfEdge - 1;
		std::get<1>(cogFrame) = cogY - cogHalfEdge - 1;
		std::get<2>(cogFrame) = cogX + cogHalfEdge + 1;
		std::get<3>(cogFrame) = cogY + cogHalfEdge + 1;
		// HFD calculation --> In: image, Out: HFD value
		// Subtract mean value from image which is required for HFD calculation
		size_t hfdRectDist = floor(outerHfdDiameter / 2.0);
		std::get<0>(hfdFrame) = cogX - hfdRectDist;
		std::get<1>(hfdFrame) = cogY - hfdRectDist;
		std::get<2>(hfdFrame) = cogX + hfdRectDist;
		std::get<3>(hfdFrame) = cogY + hfdRectDist;
		CImg<float> hfdSubImg = aiImg.get_crop(std::get<0>(hfdFrame), std::get<1>(hfdFrame), std::get<2>(hfdFrame), std::get<3>(hfdFrame));
		maxPixValue = hfdSubImg.max();
		// saturated = (maxPixValue > lowerBound && maxPixValue < upperBound);
		saturated = (maxPixValue == maxPossiblePixValue);
		CImg<float> imgHfdSubMean(hfdSubImg);
		double mean = hfdSubImg.mean();
		cimg_forXY(hfdSubImg, x, y)
		{
			imgHfdSubMean(x, y) = (hfdSubImg(x, y) < mean ? 0 : hfdSubImg(x, y) - mean);
		}
		// Calc the HFD
		hfd = calcHfd(imgHfdSubMean, outerHfdDiameter
					  /*outer diameter in px*/
		);
		// FWHM calculation --> In: Handle to full noise reduced image, abs. centroid coordinates, Out: FWHM value
		MyDataContainerT vertDataPoints, horzDataPoints;
		cimg_forX(imgHfdSubMean, x)
		{
			horzDataPoints.push_back(make_pair(x, imgHfdSubMean(x, floor(imgHfdSubMean.height() / 2.0 + 0.5))));
		}
		cimg_forY(imgHfdSubMean, y)
		{
			vertDataPoints.push_back(make_pair(y, imgHfdSubMean(floor(imgHfdSubMean.width() / 2.0 + 0.5), y)));
		}
		// Do the LM fit
		typedef CurveFitTmplT<GaussianFitTraitsT> GaussMatcherT;
		typedef GaussMatcherT::CurveParamsT CurveParamsT;
		CurveParamsT::TypeT gaussCurveParmsHorz, gaussCurveParmsVert;
		GaussMatcherT::fitGslLevenbergMarquart<MyDataAccessorT>(horzDataPoints, &gaussCurveParmsHorz, 0.1f
																/*EpsAbs*/
																,
																0.1f
																/*EpsRel*/
		);
		fwhmHorz = gaussCurveParmsHorz[CurveParamsT::W_IDX];
		GaussMatcherT::fitGslLevenbergMarquart<MyDataAccessorT>(vertDataPoints, &gaussCurveParmsVert, 0.1f
																/*EpsAbs*/
																,
																0.1f
																/*EpsRel*/
		);
		fwhmVert = gaussCurveParmsVert[CurveParamsT::W_IDX];
	}
	// Create result image
	const int factor = 4;
	CImg<unsigned char> &rgbResized = rgbImg.resize(factor * rgbImg.width(), factor * rgbImg.height(),
													-100
													/*size_z*/
													,
													-100
													/*size_c*/
													,
													1
													/*interpolation_type*/
	);
	// Draw cluster boundaries and square cluster boundaries
	const unsigned char red[3] = {
		255, 0, 0},
						green[3] = {0, 255, 0}, yellow[3] = {255, 255, 0};
	const unsigned char black[3] = {
		0, 0, 0},
						blue[3] = {0, 0, 255}, white[3] = {255, 255, 255};
	const size_t cCrossSize = 3;
	// Mark all stars in RGB image
	for (StarInfoListT::iterator it = starInfos.begin(); it != starInfos.end(); ++it)
	{
		StarInfoT *curStarInfo = &(*it);
		PixSubPosT &cogCentroid = curStarInfo->cogCentroid;
		float &hfd = curStarInfo->hfd;
		float &fwhmHorz = curStarInfo->fwhmHorz;
		float &fwhmVert = curStarInfo->fwhmVert;
		float &maxPixValue = curStarInfo->maxPixValue;
		cerr << "cogCentroid=(" << setw(9) << std::get<0>(curStarInfo->cogCentroid)
			 << ", " << setw(9) << std::get<1>(curStarInfo->cogCentroid)
			 << "), " << setw(8) << ", maxPixValue: " << setw(8) << maxPixValue
			 << ", sat: " << curStarInfo->saturated << ", hfd: " << setw(10) << hfd
			 << ", fwhmHorz: " << setw(10) << fwhmHorz << ", fwhmVert: " << setw(10) << fwhmVert << endl;
		const FrameT &frame = curStarInfo->clusterFrame;
		FrameT squareFrame(rectify(frame));
		rgbResized.draw_rectangle(floor(factor * (std::get<0>(frame) - 1) + 0.5), floor(factor * (std::get<1>(frame) - 1) + 0.5),
								  floor(factor * (std::get<2>(frame) + 1) + 0.5), floor(factor * (std::get<3>(frame) + 1) + 0.5),
								  red, 1
								  /*opacity*/
								  ,
								  ~0
								  /*pattern*/
		);
		rgbResized.draw_rectangle(floor(factor * (std::get<0>(squareFrame) - 1) + 0.5), floor(factor * (std::get<1>(squareFrame) - 1) + 0.5),
								  floor(factor * (std::get<2>(squareFrame) + 1) + 0.5), floor(factor * (std::get<3>(squareFrame) + 1) + 0.5),
								  blue, 1
								  /*opacity*/
								  ,
								  ~0
								  /*pattern*/
		);
		// Draw centroid crosses and centroid boundaries
		const PixSubPosT &subPos = curStarInfo->cogCentroid;
		const FrameT &cogFrame = curStarInfo->cogFrame;
		const FrameT &hfdFrame = curStarInfo->hfdFrame;
		rgbResized.draw_line(floor(factor * (std::get<0>(subPos) - cCrossSize) + 0.5), floor(factor * std::get<1>(subPos) + 0.5),
							 floor(factor * (std::get<0>(subPos) + cCrossSize) + 0.5), floor(factor * std::get<1>(subPos) + 0.5), green, 1
							 /*opacity*/
		);
		rgbResized.draw_line(floor(factor * std::get<0>(subPos) + 0.5), floor(factor * (std::get<1>(subPos) - cCrossSize) + 0.5),
							 floor(factor * std::get<0>(subPos) + 0.5), floor(factor * (std::get<1>(subPos) + cCrossSize) + 0.5), green, 1
							 /*opacity*/
		);
		rgbResized.draw_rectangle(floor(factor * std::get<0>(cogFrame) + 0.5), floor(factor * std::get<1>(cogFrame) + 0.5),
								  floor(factor * std::get<2>(cogFrame) + 0.5), floor(factor * std::get<3>(cogFrame) + 0.5),
								  green, 1
								  /*opacity*/
								  ,
								  ~0
								  /*pattern*/
		);
		// Draw HFD
		rgbResized.draw_rectangle(floor(factor * std::get<0>(hfdFrame) + 0.5), floor(factor * std::get<1>(hfdFrame) + 0.5),
								  floor(factor * std::get<2>(hfdFrame) + 0.5), floor(factor * std::get<3>(hfdFrame) + 0.5),
								  yellow, 1
								  /*opacity*/
								  ,
								  ~0
								  /*pattern*/
		);
		rgbImg.draw_circle(floor(factor * std::get<0>(subPos) + 0.5), floor(factor * std::get<1>(subPos) + 0.5), factor * outerHfdDiameter / 2, yellow, 1
						   /*pattern*/
						   ,
						   1
						   /*opacity*/
		);
		rgbImg.draw_circle(floor(factor * std::get<0>(subPos) + 0.5), floor(factor * std::get<1>(subPos) + 0.5), factor * hfd / 2, yellow, 1
						   /*pattern*/
						   ,
						   1
						   /*opacity*/
		);
		// Draw text
		const bool &saturated = curStarInfo->saturated;
		ostringstream oss;
		oss.precision(4);
		oss << "HFD=" << hfd << endl
			<< "FWHM H=" << fwhmHorz << endl
			<< "FWHM V=" << fwhmVert << endl
			<< "MAX=" << (int)maxPixValue << endl
			<< "SAT=" << (saturated ? "Y" : "N");
		rgbImg.draw_text(floor(factor * std::get<0>(subPos) + 0.5), floor(factor * std::get<1>(subPos) + 0.5), oss.str().c_str(), white
						 /*fg color*/
						 ,
						 black
						 /*bg color*/
						 ,
						 0.7
						 /*opacity*/
						 ,
						 9
						 /*font-size*/
		);
	}
	rgbResized.save_bmp("out.bmp");
	return 0;
}