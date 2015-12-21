//
//  system_perf.hpp
//  abandoned-objects
//
//  Created by David Kelly on 21/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef system_perf_hpp
#define system_perf_hpp

#include <stdio.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

#define NUMBER_VIDEOS 2
#define VIDEO_ONE_INDEX 0
#define VIDEO_TWO_INDEX 1

// VIDEO ONE
#define VIDEO_ONE "../../abandoned-objects/videos/ObjectAbandonmentAndRemoval1.avi"
#define VIDEO_ONE_FRAME_RATE 25
#define VIDEO_ONE_TOTAL_FRAMES 717
#define VIDEO_ONE_OBJECT_STATIC 183
#define VIDEO_ONE_OBJECT_REMOVED 509
#define VIDEO_ONE_OBJECT_TOP_LEFT_X 356
#define VIDEO_ONE_OBJECT_TOP_LEFT_Y 208
#define VIDEO_ONE_OBJECT_BOTTOM_RIGHT_X 390
#define VIDEO_ONE_OBJECT_BOTTOM_RIGHT_Y 239
// VIDEO TWO
#define VIDEO_TWO "../../abandoned-objects/videos/ObjectAbandonmentAndRemoval2.avi"
#define VIDEO_TWO_FRAME_RATE 25
#define VIDEO_TWO_TOTAL_FRAMES 692
#define VIDEO_TWO_OBJECT_STATIC 215
#define VIDEO_TWO_OBJECT_REMOVED 551
#define VIDEO_TWO_OBJECT_TOP_LEFT_X 287
#define VIDEO_TWO_OBJECT_TOP_LEFT_Y 261
#define VIDEO_TWO_OBJECT_BOTTOM_RIGHT_X 352
#define VIDEO_TWO_OBJECT_BOTTOM_RIGHT_Y 323

enum EVENT_TYPE {EVENT_ABANDONED, EVENT_REMOVED};

class SystemPerfMetric{
public:
    int truePositives = 0;
    int falsePositives = 0;
    int trueNegatives = 0;
    int falseNegatives = 0;
    
    float getPrecision();
    
    float getRecall();
    
    std::string toString();
};

class VideoEvent{
private:
    int frameIndex;
    EVENT_TYPE type;
    cv::Rect roi;
public:
    VideoEvent();
    
    VideoEvent(int frameIndex, EVENT_TYPE type, cv::Rect roi);
    
    int getFrameIndex();
    
    void setFrameIndex(int frameIndex);
    
    EVENT_TYPE getEventType();
    
    void setEventType(EVENT_TYPE type);
    
    cv::Rect getRoi();
    
    void setRoi(cv::Rect roi);
    
    float getOverlap(cv::Rect roi);
};

class SystemPerf{
private:
    std::string videoFile;
    int frameRate;
    int numberFrames;
    std::vector<VideoEvent> events;
public:
    SystemPerfMetric basicMetric;
    
    SystemPerf(std::string videoFile, int frameRate, int numberFrames);
    
    void addEvent(VideoEvent event);
    
    std::vector<VideoEvent> getEvents();
    
    std::string getVideoFile();
};

#endif /* system_perf_hpp */
