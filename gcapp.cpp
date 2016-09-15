// This script is based on OpenCV sample: samples/cpp/grabcut.cpp

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include "gcapp.h"

void GCApplication::drawLines(cv::Mat& res, const std::vector<cv::Point>& pxls, 
const cv::Scalar color, const cv::Scalar label) 
{
    for ( int i = 0; !pxls.empty() && i < pxls.size() - 1; i ++ ) 
    {
        if (pxls[i] != cv::Point(0, 0) && pxls[i+1] != cv::Point(0, 0))
        {
            line(res, pxls[i], pxls[i+1], color, m_thickness);
            line(m_mask, pxls[i], pxls[i+1], label, m_thickness);
        }
    }
}

void GCApplication::reset()
{
    if( !m_mask.empty() )
        m_mask.setTo(cv::Scalar::all(cv::GC_BGD));
    m_bgdPxls.clear(); m_fgdPxls.clear();
    m_prBgdPxls.clear();  m_prFgdPxls.clear();

    m_isInitialized = false;
    m_rectState = NOT_SET;
    m_lblsState = NOT_SET;
    m_prLblsState = NOT_SET;
}

void GCApplication::setImageAndWinName( const cv::Mat& _image, 
    const std::string& _winName  )
{
    if( _image.empty() || _winName.empty() )
        return;
    m_image = &_image;
    m_winName = &_winName;
    m_mask.create( m_image->size(), CV_8UC1);
    reset();
}

void GCApplication::showImage() 
{
    if( m_image->empty() ||  m_winName->empty() )
        return;

    cv::Mat res;
    cv::Mat binMask;
    if( !m_isInitialized )
        m_image->copyTo( res );
    else
    {
        getBinMask( m_mask, binMask );  // Mask - 0 1
        m_image->copyTo(res);
        float alpha = 0.5;
        res = res * alpha + PINK * (1 - alpha);
        m_image->copyTo(res, binMask);
        
    }
    
    drawLines(res, m_bgdPxls, BLUE, cv::GC_BGD);
    drawLines(res, m_fgdPxls, RED, cv::GC_FGD);
    drawLines(res, m_prBgdPxls, LIGHTBLUE, cv::GC_PR_BGD);
    drawLines(res, m_prFgdPxls, PINK, cv::GC_PR_FGD);

    if( m_rectState == IN_PROCESS || m_rectState == SET )
        rectangle( res, cv::Point( m_rect.x, m_rect.y ), 
        cv::Point(m_rect.x + m_rect.width, m_rect.y + m_rect.height ), GREEN, 2);

    imshow( *m_winName, res );
}

void GCApplication::setRectInMask()
{
    CV_Assert( !m_mask.empty() );
    m_mask.setTo( cv::GC_BGD );
    m_rect.x = std::max(0, m_rect.x);
    m_rect.y = std::max(0, m_rect.y);
    m_rect.width = std::min(m_rect.width, m_image->cols-m_rect.x);
    m_rect.height = std::min(m_rect.height, m_image->rows-m_rect.y);
    m_mask(m_rect).setTo( cv::Scalar(cv::GC_PR_FGD) );
}

void GCApplication::setLblsInMask( int flags, cv::Point p, bool isPr )
{
    std::vector<cv::Point> *bpxls, *fpxls;
    uchar bvalue, fvalue;
    if( !isPr )
    {
        bpxls = &m_bgdPxls;
        fpxls = &m_fgdPxls;
        bvalue = cv::GC_BGD;
        fvalue = cv::GC_FGD;
    }
    else
    {
        bpxls = &m_prBgdPxls;
        fpxls = &m_prFgdPxls;
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
    case cv::EVENT_LBUTTONDOWN: // set m_rect or cv::GC_BGD(cv::GC_FGD) labels
        {
            bool isb = (flags & BGD_KEY) != 0,
                 isf = (flags & FGD_KEY) != 0;
            if( m_rectState == NOT_SET && !isb && !isf )
            {
                m_rectState = IN_PROCESS;
                m_rect = cv::Rect( x, y, 1, 1 );
            }
            if ( (isb || isf) && m_rectState == SET )
                m_lblsState = IN_PROCESS;
        }
        break;
    case cv::EVENT_RBUTTONDOWN: // set cv::GC_PR_BGD(cv::GC_PR_FGD) labels
        {
            bool isb = (flags & BGD_KEY) != 0,
                 isf = (flags & FGD_KEY) != 0;
            if ( (isb || isf) && m_rectState == SET )
                m_prLblsState = IN_PROCESS;
        }
        break;
    case cv::EVENT_LBUTTONUP:
        if( m_rectState == IN_PROCESS )
        {
            m_rect = cv::Rect( cv::Point(m_rect.x, m_rect.y), cv::Point(x,y) );
            m_rectState = SET;
            setRectInMask();
            CV_Assert( m_bgdPxls.empty() && m_fgdPxls.empty() && m_prBgdPxls.empty() 
                && m_prFgdPxls.empty() );
            showImage();
        }
        if( m_lblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), false);
            m_lblsState = SET;
            m_bgdPxls.push_back(cv::Point(0, 0));
            m_fgdPxls.push_back(cv::Point(0, 0));
            showImage();
        }
        break;
    case cv::EVENT_RBUTTONUP:
        if( m_prLblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), true);
            m_prLblsState = SET;
            m_prBgdPxls.push_back(cv::Point(0, 0));
            m_prFgdPxls.push_back(cv::Point(0, 0));
            showImage();
        }
        break;
    case cv::EVENT_MOUSEMOVE:
        if( m_rectState == IN_PROCESS )
        {
            m_rect = cv::Rect( cv::Point(m_rect.x, m_rect.y), cv::Point(x,y) );
            CV_Assert( m_bgdPxls.empty() && m_fgdPxls.empty() && m_prBgdPxls.empty() 
                && m_prFgdPxls.empty() );
            showImage();
        }
        else if( m_lblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), false);
            showImage();
        }
        else if( m_prLblsState == IN_PROCESS )
        {
            setLblsInMask(flags, cv::Point(x,y), true);
            showImage();
        }
        break;
    }
}

