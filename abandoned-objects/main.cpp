//
//  main.cpp
//  abandoned-objects
//
//  Created by David Kelly on 08/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "utilities.hpp"
#include "video.hpp"

using namespace std;
using namespace cv;

int find_objects(VideoCapture video, float learn_rate_one, float learn_rate_two, int bins);

int main(int argc, const char * argv[]) {
    string filename(VIDEO_ONE);
    VideoCapture video = *new VideoCapture(filename);
    int res = find_objects(video, 1.008, 1.005, 4);
    if(res == -1)
        return -1;
    return 0;
}

int find_objects(VideoCapture video, float learn_rate_one, float learn_rate_two, int bins){
    if(!video.isOpened()){
        cout << "Error: Video not opened." << endl;
        return -1;
    }
    
    Mat current_frame, medianBgImageOne, medianBgImageTwo, medianDifference;
    video.retrieve(current_frame);
    MedianBackground medianBgOne = MedianBackground(current_frame, learn_rate_one, bins);
    MedianBackground medianBgTwo = MedianBackground(current_frame, learn_rate_two, bins);
    namedWindow("video");
    namedWindow("difference");
    for(int i = 0; i < (int)video.get(CV_CAP_PROP_FRAME_COUNT); i++){
        // updates the current frame and retrieve it
        video.set(CV_CAP_PROP_POS_FRAMES, i);
        video.retrieve(current_frame);
        medianBgOne.UpdateBackground(current_frame);
        medianBgTwo.UpdateBackground(current_frame);
        medianBgImageOne = medianBgOne.GetBackgroundImage();
        medianBgImageTwo = medianBgTwo.GetBackgroundImage();
        absdiff(medianBgImageOne, medianBgImageTwo, medianDifference);
        cvtColor(medianDifference, medianDifference, CV_BGR2GRAY);
        threshold(medianDifference, medianDifference, 50, 255, THRESH_BINARY);
        binary_closing_operation(&medianDifference);
        Mat medianDifferenceTemp = medianDifference.clone();
        vector<Rect> rois = get_object_roi(medianDifferenceTemp);
        for(int j = 0; j < (int)rois.size(); j++){
            rectangle(current_frame, rois[j], Scalar(0, 0, 255));
        }
        writeText(current_frame, "Frame: " + to_string(i), 15, 10, Scalar(0, 255, 0));
        writeText(current_frame, "Current Objects: " + to_string((int)rois.size()), 35, 10, Scalar(0, 255, 0));
        moveWindow("video", 0, 20);
        imshow("video", current_frame);
        moveWindow("difference", current_frame.cols, 20);
        imshow("difference", medianDifference);
        cvWaitKey(1);
    }
    return 0;
}
