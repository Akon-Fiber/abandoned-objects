//
//  video.hpp
//  abandoned-objects
//
// The code for video.hpp is largely based on the code from the book
// A practical guide to Computer Vision with OpenCV by Kenneth Dawson-Howe
//
//  Created by David Kelly on 08/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef video_hpp
#define video_hpp

#include <stdio.h>
#include "opencv2/core.hpp"

class MedianBackground
{
private:
    cv::Mat mMedianBackground;
    float**** mHistogram;
    float*** mLessThanMedian;
    float mAgingRate;
    float mCurrentAge;
    float mTotalAges;
    int mValuesPerBin;
    int mNumberOfBins;
public:
    MedianBackground( cv::Mat initial_image, float aging_rate, int values_per_bin );
    cv::Mat GetBackgroundImage();
    void UpdateBackground( cv::Mat current_frame );
    float getAgingRate()
    {
        return mAgingRate;
    }
};

void drawOpticalFlow(cv::Mat& optical_flow, cv::Mat& display, int spacing, cv::Scalar passed_line_colour=-1.0, cv::Scalar passed_point_colour=-1.0);

void LucasKanadeOpticalFlow(cv::Mat& previous_gray_frame, cv::Mat& gray_frame, cv::Mat& display_image);

#endif /* video_hpp */
