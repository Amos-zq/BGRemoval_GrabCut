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

void GCApplication::reset()
{
    if( !mask.empty() )
        mask.setTo(cv::Scalar::all(cv::GC_BGD));
    bgdPxls.clear(); fgdPxls.clear();
    prBgdPxls.clear();  prFgdPxls.clear();

    isInitialized = false;
    rectState = NOT_SET;
    lblsState = NOT_SET;
    prLblsState = NOT_SET;
    iterCount = 0;
}

void GCApplication::setImageAndWinName( const cv::Mat& _image, 
    const std::string& _winName  )
{
    if( _image.empty() || _winName.empty() )
        return;
    image = &_image;
    winName = &_winName;
    mask.create( image->size(), CV_8UC1);
    reset();
}

void GCApplication::showImage() 
{
    if( image->empty() || winName->empty() )
        return;

    cv::Mat res;
    cv::Mat binMask;
    if( !isInitialized )
        image->copyTo( res );
    else
    {
        getBinMask( mask, binMask );  // Mask - 0 1
        image->copyTo(res);
        res = res * 0.5 + PINK * 0.5;
        image->copyTo(res, binMask);
        
    }

    for ( int i = 0; !bgdPxls.empty() && i < bgdPxls.size() - 1; i ++ ) 
    {
        if (bgdPxls[i] != cv::Point(0, 0) && bgdPxls[i+1] != cv::Point(0, 0))
        {
            line(res, bgdPxls[i], bgdPxls[i+1], BLUE, thickness);
            line(mask, bgdPxls[i], bgdPxls[i+1], cv::GC_BGD, thickness);
        }
    }
    for ( int i = 0; !fgdPxls.empty() && i < fgdPxls.size() - 1; i ++ ) 
    {
        if (fgdPxls[i] != cv::Point(0, 0) && fgdPxls[i+1] != cv::Point(0, 0))
        {
            line(res, fgdPxls[i], fgdPxls[i+1], RED, thickness);
            line(mask, fgdPxls[i], fgdPxls[i+1], cv::GC_FGD, thickness);
        }
    }
    for ( int i = 0; !prBgdPxls.empty() && i < prBgdPxls.size() - 1; i ++ ) 
    {
        if (prBgdPxls[i] != cv::Point(0, 0) 
            && prBgdPxls[i+1] != cv::Point(0, 0))
        {
            line(res, prBgdPxls[i], prBgdPxls[i+1], LIGHTBLUE, thickness);
            line(mask, prBgdPxls[i], prBgdPxls[i+1], cv::GC_PR_BGD, thickness);
        }
    }
    for ( int i = 0; !prFgdPxls.empty() && i < prFgdPxls.size() - 1; i ++ ) 
    {
        if (prFgdPxls[i] != cv::Point(0, 0) && 
            prFgdPxls[i+1] != cv::Point(0, 0))
        {
            line(res, prFgdPxls[i], prFgdPxls[i+1], PINK, thickness);
            line(mask, prFgdPxls[i], prFgdPxls[i+1], cv::GC_PR_FGD, thickness);
        }
    }


    if( rectState == IN_PROCESS || rectState == SET )
        rectangle( res, cv::Point( rect.x, rect.y ), 
        cv::Point(rect.x + rect.width, rect.y + rect.height ), GREEN, 2);

    imshow( *winName, res );
}

void GCApplication::setRectInMask()
{
    CV_Assert( !mask.empty() );
    mask.setTo( cv::GC_BGD );
    rect.x = std::max(0, rect.x);
    rect.y = std::max(0, rect.y);
    rect.width = std::min(rect.width, image->cols-rect.x);
    rect.height = std::min(rect.height, image->rows-rect.y);
    (mask(rect)).setTo( cv::Scalar(cv::GC_PR_FGD) );
}

void GCApplication::setLblsInMask( int flags, cv::Point p, bool isPr )
{
    std::vector<cv::Point> *bpxls, *fpxls;
    uchar bvalue, fvalue;
    if( !isPr )
    {
        bpxls = &bgdPxls;
        fpxls = &fgdPxls;
        bvalue = cv::GC_BGD;
        fvalue = cv::GC_FGD;
    }
    else
    {
        bpxls = &prBgdPxls;
        fpxls = &prFgdPxls;
        bvalue = cv::GC_PR_BGD;
        fvalue = cv::GC_PR_FGD;
    }
    if( flags & BGD_KEY )
    {
        bpxls->push_back(p);
    }
    if( flags & FGD_KEY )
    {
        fpxls->push_back(p);
    }
}

void GCApplication::mouseClick( int event, int x, int y, int flags, void* )
{
    switch( event )
    {
    case cv::EVENT_LBUTTONDOWN: // set rect or cv::GC_BGD(cv::GC_FGD) labels
        {
            bool isb = (flags & BGD_KEY) != 0,
                 isf = (flags & FGD_KEY) != 0;
            if( rectState == NOT_SET && !isb && !isf )
            {
                rectState = IN_PROCESS;
                rect = cv::Rect( x, y, 1, 1 );
            }
            if ( (isb || isf) && rectState == SET )
                lblsState = IN_PROCESS;
        }
        break;
    case cv::EVENT_RBUTTONDOWN: // set cv::GC_PR_BGD(cv::GC_PR_FGD) labels
        {
            bool isb = (flags & BGD_KEY) != 0,
                 isf = (flags & FGD_KEY) != 0;
            if ( (isb || isf) && rectState == SET )
                prLblsState = IN_PROCESS;
        }
        break;
    case cv::EVENT_LBUTTONUP:
        if( rectState == IN_PROCESS )
        {
            rect = cv::Rect( cv::Point(rect.x, rect.y), cv::Point(x,y) );
            rectState = SET;
            setRectInMask();
            CV_Assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() 
                && prFgdPxls.empty() );
            showImage();
        }
        if( lblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), false);
            lblsState = SET;
            bgdPxls.push_back(cv::Point(0, 0));
            fgdPxls.push_back(cv::Point(0, 0));
            showImage();
        }
        break;
    case cv::EVENT_RBUTTONUP:
        if( prLblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), true);
            prLblsState = SET;
            prBgdPxls.push_back(cv::Point(0, 0));
            prFgdPxls.push_back(cv::Point(0, 0));
            showImage();
        }
        break;
    case cv::EVENT_MOUSEMOVE:
        if( rectState == IN_PROCESS )
        {
            rect = cv::Rect( cv::Point(rect.x, rect.y), cv::Point(x,y) );
            CV_Assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() 
                && prFgdPxls.empty() );
            showImage();
        }
        else if( lblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), false);
            showImage();
        }
        else if( prLblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), true);
            showImage();
        }
        break;
    }
}

int GCApplication::nextIter()
{
    if( isInitialized )
        grabCut( *image, mask, rect, bgdModel, fgdModel, 1 );
    else
    {
        if( rectState != SET )
            return iterCount;

        if( lblsState == SET || prLblsState == SET )
            grabCut( *image, mask, rect, bgdModel, fgdModel, 1, 
                cv::GC_INIT_WITH_MASK );
        else
            grabCut( *image, mask, rect, bgdModel, fgdModel, 1, 
                cv::GC_INIT_WITH_RECT );

        isInitialized = true;
    }
    iterCount++;

    bgdPxls.clear(); fgdPxls.clear();
    prBgdPxls.clear(); prFgdPxls.clear();

    return iterCount;
}

