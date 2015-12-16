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


enum OBJECT_TYPE {OBJ_ABANDONED, OBJ_REMOVED, OBJ_UNKNOWN};
enum OBJECT_STATUS {OBJ_GROWING, OBJ_SHRINKING, OBJ_SAME_SIZE, OBJ_GONE};

class Object{
public:
    Rect roi;
    OBJECT_TYPE type = OBJ_UNKNOWN;
    OBJECT_STATUS status;
    int frame_found;
    
    Object(Rect roi, OBJECT_TYPE type, OBJECT_STATUS status, int frame){
        this->roi = roi;
        this->type = type;
        this->status = status;
        this->frame_found = frame;
    }
    
    void update_region(Rect roi, int frame){
        this->roi = roi;
        this->frame_found = frame;
    }
    
    bool check_overlap(Rect overlap){
        Rect intersection = this->roi & overlap;
        if(intersection.area() != 0){
            return true;
        }
        else{
            return false;
        }
    }
};

vector<Object> objects;

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
        // update the median background images
        medianBgOne.UpdateBackground(current_frame);
        medianBgTwo.UpdateBackground(current_frame);
        medianBgImageOne = medianBgOne.GetBackgroundImage();
        medianBgImageTwo = medianBgTwo.GetBackgroundImage();
        // get the cleaned up binary difference image
        absdiff(medianBgImageOne, medianBgImageTwo, medianDifference);
        cvtColor(medianDifference, medianDifference, CV_BGR2GRAY);
        threshold(medianDifference, medianDifference, 50, 255, THRESH_BINARY);
        // clean the binary difference image
        binary_closing_operation(&medianDifference);
        binary_opening_operation(&medianDifference);
        
        // if object regions are found then update the object array
        Mat medianDifferenceTemp = medianDifference.clone();
        vector<Rect> rois = get_object_roi(medianDifferenceTemp);
        medianDifferenceTemp.release();
        
        bool found_any_match = false;
        for(int j = 0; j < (int)objects.size(); j++){
            bool found_match = false;
            int roi_match = -1;
            for(int k = 0; k < (int)rois.size(); k++){
                if(objects[j].status != OBJ_GONE && objects[j].check_overlap(rois[k])){
                    found_any_match = true;
                    found_match = true;
                    roi_match = k;
                }
                // draw region
                rectangle(current_frame, rois[j], Scalar(0, 0, 255));
            }
            if(found_match == false){
                // make object gone
                objects[j].status = OBJ_GONE;
            }
            else if(objects[j].roi.area() < rois[roi_match].area()){
                // if area increasing
                objects[j].update_region(rois[roi_match], i);
                objects[j].status = OBJ_GROWING;
            }
            else if(objects[j].roi.area() > rois[roi_match].area()){
                // if area decreasing
                objects[j].status = OBJ_SHRINKING;
            }
            else{
                objects[j].status = OBJ_SAME_SIZE;
            }
        }
        if(found_any_match == false && (int) rois.size() != 0){
            // if no matching objects then create new ones
            for(int j = 0; j < (int) rois.size(); j++){
                // create new object
                Object obj = *new Object(rois[j], OBJ_UNKNOWN, OBJ_SAME_SIZE, i);
                objects.push_back(obj);
            }
        }
        
        writeText(current_frame, "Frame: " + to_string(i), 15, 10, Scalar(0, 255, 0));
        writeText(current_frame, "Current Objects: " + to_string((int)rois.size()), 35, 10, Scalar(0, 255, 0));
        moveWindow("video", 0, 20);
        imshow("video", current_frame);
        moveWindow("difference", current_frame.cols, 20);
        imshow("difference", medianDifference);
        if((int)rois.size() != 0){
            cvWaitKey(0);
        }else{
            cvWaitKey(1);
        }
        
        for(int j = 0; j < (int) objects.size(); j++){
            cout << "Object #" << j << " Location: " << objects[j].roi.tl() << " Area: " << objects[j].roi.area() << " Frame Found: " << objects[j].frame_found << " STATUS:" << objects[j].status << endl;
        }
    }
    return 0;
}
