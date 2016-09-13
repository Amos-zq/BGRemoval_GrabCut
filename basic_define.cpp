#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <iomanip>
#include "gcapp.h"

void GCApplication::getBinMask( const cv::Mat& comMask, cv::Mat& binMask )
{
    if( comMask.empty() || comMask.type()!=CV_8UC1 )
        std::cout << "comMask is empty or has incorrect type (not CV_8UC1)\n";
    if( binMask.empty() || binMask.rows!=comMask.rows || 
        binMask.cols!=comMask.cols )
        binMask.create( comMask.size(), CV_8UC1 );
    binMask = comMask & 1;

}


cv::Mat GCApplication::grab_cut(cv::Mat image, cv::Mat mask, int iter) 
{
    cv::Mat bgdModel, fgdModel;
    cv::Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = mask.cols;
    rect.height = mask.rows;
    cv::grabCut(image, mask, rect, bgdModel, fgdModel, 
        1, cv::GC_INIT_WITH_MASK);
    for (int i = 0; i < iter; i ++) 
        cv::grabCut(image, mask, rect, bgdModel, fgdModel, 1);
    return mask;
}

void GCApplication::delete_holes(cv::Mat & mask) 
{
    cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, 
        cv::Size( 3, 3 ), cv::Point( 1, 1 ) );
    erode( mask, mask, element );
    element = cv::getStructuringElement( cv::MORPH_RECT, 
        cv::Size( 5, 5 ), cv::Point( 2, 2 ) );
    dilate( mask, mask, element );
    element = cv::getStructuringElement( cv::MORPH_RECT, 
        cv::Size( 3, 3 ), cv::Point( 1, 1 ) );
    erode( mask, mask, element );
}

cv::Mat GCApplication::mask_update(cv::Mat mask, int dilation_size, 
    int erosion_size) 
{
    cv::Mat temp, mask_d, mask_e;
    mask_d.create(mask.size(), CV_8UC1);
    mask_e.create(mask.size(), CV_8UC1);

    cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT, 
        cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ), 
        cv::Point( erosion_size, erosion_size ) );
    erode( mask, mask_e, element );
    element = cv::getStructuringElement( cv::MORPH_RECT, 
        cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ), 
        cv::Point( dilation_size, dilation_size ) );
    dilate( mask, mask_d, element );

    temp.create(mask.size(), CV_8UC1);
    mask.setTo(cv::Scalar(0, 0, 0));
    temp.setTo(cv::Scalar(3, 3, 3));
    temp.copyTo(mask, mask_d);
    temp.setTo(cv::Scalar(1, 1, 1));
    temp.copyTo(mask, mask_e);
    
    return mask;
}

