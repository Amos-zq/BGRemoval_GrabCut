#ifndef GCAPP_H
#define GCAPP_H

const int BGD_KEY = cv::EVENT_FLAG_CTRLKEY;
const int FGD_KEY = cv::EVENT_FLAG_SHIFTKEY;

const cv::Scalar RED = cv::Scalar(0,0,255);
const cv::Scalar PINK = cv::Scalar(230,130,255);
const cv::Scalar BLUE = cv::Scalar(255,0,0);
const cv::Scalar LIGHTBLUE = cv::Scalar(255,255,160);
const cv::Scalar GREEN = cv::Scalar(0,255,0);

class GCApplication
{
public:
    enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
    static const int radius = 2;
    static const int thickness = 3;

    void reset();
    void setImageAndWinName( const cv::Mat& _image, const std::string& _winName );
    void showImage()  ;
    void mouseClick( int event, int x, int y, int flags, void* param );
    int nextIter();
    int getIterCount() const { return iterCount; }
    cv::Mat mask;
    //basic_define.h
    static void getBinMask( const cv::Mat& comMask, cv::Mat& binMask );
    cv::Mat grab_cut(cv::Mat image, cv::Mat mask, int iter);
    void delete_holes(cv::Mat & mask);
    cv::Mat mask_update(cv::Mat mask, int dilation_size, int erosion_size);
    //frame_by_frame.h
    void getMaskCombined (cv::Mat image, cv::Mat mask, cv::Mat mask_c);
    void FrameByFrame(std::string filename, int start_frame, int iter, int dilation_size, int erosion_size);
    //floodfill.h
    void help_floodfill();
    cv::Mat FloodFill( cv::Mat image_ff0, std::string output_dir );
private:
    void setRectInMask();
    void setLblsInMask( int flags, cv::Point p, bool isPr );

    const std::string* winName;
    const cv::Mat* image;
    
    cv::Mat bgdModel, fgdModel;

    uchar rectState, lblsState, prLblsState;
    bool isInitialized;

    cv::Rect rect;
    std::vector<cv::Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
    int iterCount;
};



#endif




