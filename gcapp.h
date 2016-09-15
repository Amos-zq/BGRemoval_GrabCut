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
    enum grabCutMode {SINGLE, RGBD, VIDEO};
    static const int m_thickness = 3;
    grabCutMode m_mode;
    //gcapp.cpp
    void reset();
    void setImageAndWinName(const cv::Mat& _image, const std::string& _winName);
    void showImage()  ;
    void mouseClick( int event, int x, int y, int flags, void* param );
    void drawLines(cv::Mat& res, const std::vector<cv::Point> pxls, 
        const cv::Scalar color, const cv::Scalar label);
    cv::Mat m_mask;
    //basic_define.cpp
    static void getBinMask( const cv::Mat& comMask, cv::Mat& binMask );
    cv::Mat grabCutWMask(cv::Mat image, cv::Mat mask, int iter);
    void deleteHoles(cv::Mat & mask);
    cv::Mat updateMask(cv::Mat mask, int dilation_size, int erosion_size);
    //frame_by_frame.cpp
    void getMaskCombined (cv::Mat image, cv::Mat mask, cv::Mat mask_c);
    void FrameByFrame(std::string filename, int start_frame, int iter, 
        int dilation_size, int erosion_size);
    //floodfill.cpp
    void helpFloodfill();
    cv::Mat FloodFill( cv::Mat image_ff0, std::string output_dir );
private:
    void setRectInMask();
    void setLblsInMask( int flags, cv::Point p, bool isPr );

    const std::string* m_winName;
    const cv::Mat* m_image;

    uchar m_rectState, m_lblsState, m_prLblsState;
    bool m_isInitialized;

    cv::Rect m_rect;
    std::vector<cv::Point> m_fgdPxls, m_bgdPxls, m_prFgdPxls, m_prBgdPxls;
};



#endif




