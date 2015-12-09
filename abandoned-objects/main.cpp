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
    namedWindow(VIDEO_ONE);
    for(int i = 0; i < VIDEO_ONE_TOTAL_FRAMES; i++){
        Mat current_frame;
        video.set(CV_CAP_PROP_POS_FRAMES, i);
        video.retrieve(current_frame);
        imshow(VIDEO_ONE, current_frame);
        cvWaitKey(1);
    }
    return 0;
}
