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
#include "world_object.hpp"

#define MB_LEARN_RATE_ONE 1.001
#define MB_LEARN_RATE_TWO 1.01
#define MB_NUMBER_BINS 4
#define MEDIAN_DIFFERENCE_THRESHOLD 50

using namespace std;
using namespace cv;

int find_objects(VideoCapture video, WorldObjectManager woManager, float learn_rate_one, float learn_rate_two, int bins);

int main(int argc, const char * argv[]) {
    string filename(VIDEO_TWO);
    VideoCapture video = *new VideoCapture(filename);
    WorldObjectManager worldObjectManager = *new WorldObjectManager();
    int res = find_objects(video, worldObjectManager, MB_LEARN_RATE_ONE, MB_LEARN_RATE_TWO, MB_NUMBER_BINS);
    if(res == -1)
        return -1;
    return 0;
}

int find_objects(VideoCapture video, WorldObjectManager woManager, float learn_rate_one, float learn_rate_two, int bins){
    if(!video.isOpened()){
        cout << "Error: Video not opened." << endl;
        return -1;
    }
    Mat current_frame, medianBgImageOne, medianBgImageTwo, medianDifference;
    video.retrieve(current_frame);
    woManager.setOriginalBackgroundImage(current_frame.clone());
    MedianBackground medianBgOne = MedianBackground(current_frame, learn_rate_one, bins);
    MedianBackground medianBgTwo = MedianBackground(current_frame, learn_rate_two, bins);
    namedWindow("video");
    namedWindow("difference");
    for(int i = 0; i < (int)video.get(CV_CAP_PROP_FRAME_COUNT); i++){
        
        // updates the current frame and retrieves it
        video.set(CV_CAP_PROP_POS_FRAMES, i);
        video.retrieve(current_frame);
        
        // update the median background images
        medianBgOne.UpdateBackground(current_frame);
        medianBgTwo.UpdateBackground(current_frame);
        medianBgImageOne = medianBgOne.GetBackgroundImage();
        medianBgImageTwo = medianBgTwo.GetBackgroundImage();
        
        // get the binary difference image
        absdiff(medianBgImageOne, medianBgImageTwo, medianDifference);
        cvtColor(medianDifference, medianDifference, CV_BGR2GRAY);
        threshold(medianDifference, medianDifference, MEDIAN_DIFFERENCE_THRESHOLD, 255, THRESH_BINARY);
        
        // clean the binary difference image
        binary_closing_operation(&medianDifference);
        binary_opening_operation(&medianDifference);
        
        // get object region contours
        Mat medianDifferenceTemp = medianDifference.clone();
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(medianDifferenceTemp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
        medianDifferenceTemp.release();
        
        // update world object manager if there are detected object regions
        woManager.update(contours, current_frame.clone(), i);
        woManager.drawCurrentObjectRegions(current_frame);
//        cout << woManager.currentObjectsToString();
        
        // Update Window
        writeText(current_frame, "Frame: " + to_string(i), 15, 10, Scalar(0, 255, 0));
        writeText(current_frame, "Current Objects: " + to_string((int)woManager.getCurrentObjects().size()), 35, 10, Scalar(0, 255, 0));
        writeText(current_frame, "Processed Objects: " + to_string((int)woManager.getProcessedObjects().size()), 55, 10, Scalar(0, 255, 0));
        moveWindow("video", 0, 20);
        imshow("video", current_frame);
        moveWindow("difference", 0, current_frame.rows + 40);
        imshow("difference", medianDifference);
        cvWaitKey(1);
    }
    // updates the world object manager if the video ends while there were still contours.
    if((int)woManager.getCurrentObjects().size() > 0){
        vector<vector<Point>> empty;
        woManager.update(empty, current_frame.clone(), (int)video.get(CV_CAP_PROP_FRAME_COUNT));
    }
    cout << woManager.processedObjectsToString();
    return 0;
}
