//
//  video.cpp
//  abandoned-objects
//
// The code for video.cpp is largely based on the code from the book
// A practical guide to Computer Vision with OpenCV by Kenneth Dawson-Howe
//
//  Created by David Kelly on 08/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "video.hpp"

using namespace std;
using namespace cv;

void drawOpticalFlow(Mat& optical_flow, Mat& display, int spacing, Scalar passed_line_colour, Scalar passed_point_colour){
    Scalar colour( rand()&0xFF, rand()&0xFF, rand()&0xFF );
    for (int row = spacing/2; row < display.rows; row += spacing)
        for(int column = spacing/2; column < display.cols; column += spacing)
        {
            const Point2f& fxy = optical_flow.at<Point2f>(row,column);
            circle(display, Point(column,row), 1, (passed_point_colour.val[0] == -1.0) ? colour : passed_point_colour, -1);
            line(display, Point(column,row), Point(cvRound(column+fxy.x), cvRound(row+fxy.y)),
                 (passed_line_colour.val[0] == -1.0) ? colour : passed_line_colour);
        }
}

#define MAX_FEATURES 400
void LucasKanadeOpticalFlow(Mat& previous_gray_frame, Mat& gray_frame, Mat& display_image){
    //Size img_sz = previous_gray_frame.size();
    int win_size = 10;
    cvtColor(previous_gray_frame, display_image, CV_GRAY2BGR);
    vector<Point2f> previous_features, current_features;
    const int MAX_CORNERS = 500;
    goodFeaturesToTrack(previous_gray_frame, previous_features, MAX_CORNERS, 0.05, 5, noArray(), 3, false, 0.04);
    cornerSubPix(previous_gray_frame, previous_features, Size(win_size, win_size), Size(-1,-1),
                 TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
    vector<uchar> features_found;
    calcOpticalFlowPyrLK(previous_gray_frame, gray_frame, previous_features, current_features, features_found, noArray(),
                         Size(win_size*4+1,win_size*4+1), 5,
                         TermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ));
    for( int i = 0; i < (int)previous_features.size(); i++ )
    {
        if( !features_found[i] )
            continue;
        circle(display_image, previous_features[i], 1, Scalar(0,0,255));
        line(display_image, previous_features[i], current_features[i], Scalar(0,255,0));
    }
}

MedianBackground::MedianBackground( Mat initial_image, float aging_rate, int values_per_bin ){
    mCurrentAge = 1.0;
    mAgingRate = aging_rate;
    mTotalAges = 0.0;
    mValuesPerBin = values_per_bin;
    mNumberOfBins = 256/mValuesPerBin;
    mMedianBackground = Mat::zeros(initial_image.size(), initial_image.type());
    mLessThanMedian = (float***) new float**[mMedianBackground.rows];
    mHistogram = (float****) new float***[mMedianBackground.rows];
    for (int row=0; (row<mMedianBackground.rows); row++)
    {
        mHistogram[row] = (float***) new float**[mMedianBackground.cols];
        mLessThanMedian[row] = (float**) new float*[mMedianBackground.cols];
        for (int col=0; (col<mMedianBackground.cols); col++)
        {
            mHistogram[row][col] = (float**) new float*[mMedianBackground.channels()];
            mLessThanMedian[row][col] = new float[mMedianBackground.channels()];
            for (int ch=0; (ch<mMedianBackground.channels()); ch++)
            {
                mHistogram[row][col][ch] = new float[mNumberOfBins];
                mLessThanMedian[row][col][ch] = 0.0;
                for (int bin=0; (bin<mNumberOfBins); bin++)
                {
                    mHistogram[row][col][ch][bin] = (float) 0.0;
                }
            }
        }
    }
}

Mat MedianBackground::GetBackgroundImage(){
    return mMedianBackground;
}

void MedianBackground::UpdateBackground( Mat current_frame ){
    mTotalAges += mCurrentAge;
    float total_divided_by_2 = mTotalAges/((float) 2.0);
    for (int row=0; (row<mMedianBackground.rows); row++)
    {
        for (int col=0; (col<mMedianBackground.cols); col++)
        {
            for (int ch=0; (ch<mMedianBackground.channels()); ch++)
            {
                int new_value = (mMedianBackground.channels()==3) ? current_frame.at<Vec3b>(row,col)[ch] : current_frame.at<uchar>(row,col);
                int median = (mMedianBackground.channels()==3) ? mMedianBackground.at<Vec3b>(row,col)[ch] : mMedianBackground.at<uchar>(row,col);
                int bin = new_value/mValuesPerBin;
                mHistogram[row][col][ch][bin] += mCurrentAge;
                if (new_value < median)
                    mLessThanMedian[row][col][ch] += mCurrentAge;
                int median_bin = median/mValuesPerBin;
                while ((mLessThanMedian[row][col][ch] + mHistogram[row][col][ch][median_bin] < total_divided_by_2) && (median_bin < 255))
                {
                    mLessThanMedian[row][col][ch] += mHistogram[row][col][ch][median_bin];
                    median_bin++;
                }
                while ((mLessThanMedian[row][col][ch] > total_divided_by_2) && (median_bin > 0))
                {
                    median_bin--;
                    mLessThanMedian[row][col][ch] -= mHistogram[row][col][ch][median_bin];
                }
                if (mMedianBackground.channels()==3)
                    mMedianBackground.at<Vec3b>(row,col)[ch] = median_bin*mValuesPerBin;
                else mMedianBackground.at<uchar>(row,col) = median_bin*mValuesPerBin;
            }
        }
    }
    mCurrentAge *= mAgingRate;
}

VideoWriter* OpenVideoFile( char* filename, VideoCapture& video_to_emulate, int horizontal_multiple, int vertical_multiple, int spacing ){
    int codec = static_cast<int>(video_to_emulate.get(CV_CAP_PROP_FOURCC));
    Size image_size = Size((int) video_to_emulate.get(CV_CAP_PROP_FRAME_WIDTH),
                           (int) video_to_emulate.get(CV_CAP_PROP_FRAME_HEIGHT));
    double fps = video_to_emulate.get(CV_CAP_PROP_FPS);
    return OpenVideoFile( filename, codec, image_size, fps, horizontal_multiple, vertical_multiple, spacing );
}

VideoWriter* OpenVideoFile( char* filename, int codec, Size image_size, double fps, int horizontal_multiple, int vertical_multiple, int spacing ){
    VideoWriter* output_video = new VideoWriter();
    Size video_size = Size((int) image_size.width*horizontal_multiple + spacing*(horizontal_multiple-1),
                           (int) image_size.height*vertical_multiple + spacing*(vertical_multiple-1));
    output_video->open(filename, codec, fps, video_size, true);
    if (!output_video->isOpened())
    {
        cout  << "Could not open the output video for write: " << filename << endl;
    }
    return output_video;
}

void WriteVideoFrame( VideoWriter* output_video, Mat& video_frame ){
    *output_video << video_frame;
}

void CloseVideoFile( VideoWriter* video ){
    delete video;
}
