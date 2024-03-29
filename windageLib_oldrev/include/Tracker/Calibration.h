/* ========================================================================
 * PROJECT: windage Library
 * ========================================================================
 * This work is based on the original windage Library developed by
 *   Woonhyuk Baek
 *   Woontack Woo
 *   U-VR Lab, GIST of Gwangju in Korea.
 *   http://windage.googlecode.com/
 *   http://uvr.gist.ac.kr/
 *
 * Copyright of the derived and new portions of this work
 *     (C) 2009 GIST U-VR Lab.
 *
 * This framework is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For further information please contact 
 *   Woonhyuk Baek
 *   <windage@live.com>
 *   GIST U-VR Lab.
 *   Department of Information and Communication
 *   Gwangju Institute of Science and Technology
 *   1, Oryong-dong, Buk-gu, Gwangju
 *   South Korea
 * ========================================================================
 ** @author   Woonhyuk Baek
 * ======================================================================== */

#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <cv.h>

#include "base.h"

namespace windage
{
	/**
	 * @brief
	 *		Class for Camera Parameter
	 * @author
	 *		windage
	 */
	class DLLEXPORT Calibration
	{
	private:
		double parameter[8];

		CvMat* intrinsicMatrix;			///< intrinsic parameter (Camera coordinate to Image coordinate)
		CvMat* distortionCoefficients;	///< distortion coefficient
		CvMat* extrinsicMatrix;			///< extrinsic parameter (World coordinate to Camera coordinate)

		IplImage* dstMapX;				///< pre-calculated data storage for map-based undistortion method
		IplImage* dstMapY;				///< pre-calculated data storage for map-based undistortion method

		void Release();
		
	public:
		Calibration();
		~Calibration();
		
		inline double* GetParameters(){return this->parameter;};
		inline CvMat* GetIntrinsicMatrix(){return intrinsicMatrix;};
		inline CvMat* GetDistortionCoefficients(){return distortionCoefficients;};
		inline CvMat* GetExtrinsicMatrix(){return extrinsicMatrix;};

		/**
		 * @brief
		 *		initialize camera paramter (intrinsic parameter)
		 * @remark
		 *		it is not necessary function 
		 *		can alternate SetIntrinsicMatirx and SetDistortionCoefficients functions but recommended
		 */
		void Initialize(
						double fx, 				///< intrinsic parameter x focal length
						double fy, 				///< intrinsic parameter y focal length
						double cx, 				///< intrinsic parameter x principle point
						double cy, 				///< intrinsic parameter y principle point
						double d1, 				///< intrinsic parameter distortion factor1
						double d2, 				///< intrinsic parameter distortion factor2
						double d3, 				///< intrinsic parameter distortion factor3
						double d4 				///< intrinsic parameter distortion factor4
						);

		void SetIntrinsicMatirx(double fx, double fy, double cx, double cy);
		void SetDistortionCoefficients(double d1=0.0, double d2=0.0, double d3=0.0, double d4=0.0);
		void SetExtrinsicMatrix(CvMat* matrix);
		void SetExtrinsicMatrix(float* matrix);
		void SetExtrinsicMatrix(double* matrix);

		void ConvertExtrinsicParameter(CvMat* rotationVector, CvMat* translationVector);

		CvScalar GetCameraPosition();

		/**
		 * @defgroup CoordinateConvertor Coordinate Convertor
		 * @brief
		 *		Coordinate Convertor
		 * @remark
		 *		Coordinate Convert World, Camera, Image coordinate
		 * @addtogroup CoordinateConvertor
		 * @{
		 */
		int ConvertWorld2Camera(CvMat* output4vector, CvMat* input4vector);
		int ConvertCamera2Image(CvMat* output3vector, CvMat* input3vector);
		int ConvertWorld2Image(CvMat* output3vector, CvMat* input4vector);
		CvPoint ConvertWorld2Image(double x, double y, double z);
		int ConvertCamera2World(CvMat* output3vector, CvMat* input3vector);
		int ConvertImage2Camera(CvMat* output3vector, CvMat* input3vector, double z);
		int ConvertImage2World(CvMat* output3vector, CvMat* input3vector, double z);
		CvPoint2D64f ConvertImage2World(double ix, double iy, double wz=0.0);
		/** @} */

		/**
		 * @brief
		 *		initialize remapping data for map-based undistortion method
		 * @remark
		 *		pre-calculate undistortion reampping data using distortino coefficents
		 */
		void InitUndistortionMap(
									int width,	///< input image width size
									int height	///< input image height size
								);
		/**
		 * @brief
		 *		undistortion method
		 * @remark
		 *		if after initialized undistortion map then undistort input image using pre-calculated data (faster)
		 *		else real-time calculate undisotrtion map
		 */
		void Undistortion(IplImage* input, IplImage* output);

		void DrawInfomation(IplImage* colorImage, double size = 10.0);
	};

}
#endif