//
//  system_perf.cpp
//  abandoned-objects
//
//  Created by David Kelly on 21/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "system_perf.hpp"

using namespace std;
using namespace cv;

/*** SystemPerfMetric ***/
float SystemPerfMetric::getPrecision(){
    int denom = truePositives + falsePositives;
    if(denom != 0){
        return (float)truePositives / (float)denom;
    }else{
        return -1;
    }
}

float SystemPerfMetric::getRecall(){
    int denom = truePositives + falseNegatives;
    if(denom != 0){
        return (float)truePositives / (float)denom;
    }else{
        return -1;
    }
}

string SystemPerfMetric::toString(){
    stringstream str;
    str << "TP:" << truePositives << " FP:" << falsePositives << " FN:" << falseNegatives;
    str << " Precision:" << this->getPrecision() << " Recall:" << this->getRecall() << endl;
    return str.str();
}

/*** VideoEvent ***/

VideoEvent::VideoEvent(){
    this->frameIndex = -1;
}

VideoEvent::VideoEvent(int frameIndex, EVENT_TYPE type, Rect roi){
    this->frameIndex = frameIndex;
    this->type = type;
    this->roi = roi;
}

int VideoEvent::getFrameIndex(){
    return this->frameIndex;
}

void VideoEvent::setFrameIndex(int frameIndex){
    this->frameIndex = frameIndex;
}

EVENT_TYPE VideoEvent::getEventType(){
    return this->type;
}

void VideoEvent::setEventType(EVENT_TYPE type){
    this->type = type;
}

Rect VideoEvent::getRoi(){
    return this->roi;
}

void VideoEvent::setRoi(Rect roi){
    this->roi = roi;
}

float VideoEvent::setOverlap(Rect roi){
    if(this->roi.area() != 0){
        int overlapArea = (this->roi & roi).area();
        this->overlap = (float)overlapArea / (float)this->roi.area();
        return this->overlap;
    }else{
        return 0.0;
    }
}

/*** SystemPerf ***/

SystemPerf::SystemPerf(string videoFile, int frameRate, int numberFrames){
    this->videoFile = videoFile;
    this->frameRate = frameRate;
    this->numberFrames = numberFrames;
}

void SystemPerf::addEvent(VideoEvent event){
    this->events.push_back(event);
}

vector<VideoEvent> SystemPerf::getEvents(){
    return this->events;
}

string SystemPerf::getVideoFile(){
    return this->videoFile;
}