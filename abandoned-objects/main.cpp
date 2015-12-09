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

int main(int argc, const char * argv[]) {
    string filename(VIDEO_ONE);
    VideoCapture video = *new VideoCapture();
    video.open(filename);
    if(!video.isOpened()){
        cout << "Could not open video: " << filename << endl;
        return -1;
    }

    Mat current_frame, medianBgImageOne, medianBgImageTwo, medianDifference;
    video.retrieve(current_frame);
    MedianBackground medianBgOne = MedianBackground(current_frame, 1.01, 4);
    MedianBackground medianBgTwo = MedianBackground(current_frame, 1.005, 4);
    namedWindow("video");
    namedWindow("Median One 1.01");
    namedWindow("Median Two 1.005");
    for(int i = 0; i < (int)video.get(CV_CAP_PROP_FRAME_COUNT); i++){
        video.set(CV_CAP_PROP_POS_FRAMES, i);
        video.retrieve(current_frame);
        medianBgOne.UpdateBackground(current_frame);
        medianBgTwo.UpdateBackground(current_frame);
        medianBgImageOne = medianBgOne.GetBackgroundImage();
        medianBgImageTwo = medianBgTwo.GetBackgroundImage();
        imshow("video", current_frame);
        imshow("Median One 1.01", medianBgImageOne);
        imshow("Median Two 1.005", medianBgImageTwo);
        cvWaitKey(1);
    }
    return 0;
}
