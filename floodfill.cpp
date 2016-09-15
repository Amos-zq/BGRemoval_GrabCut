#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include "gcapp.h"

cv::Mat image_ff0, image_ff;
int loDiff = 0, upDiff = 0;

int r, g, b;   // rgb color value

void GCApplication::helpFloodfill() 
{
    std::cout << "Press 'f' to fill foreground" << std::endl
	     << "Press 'b' to fill background" << std::endl
	     << "Press 'p' to fill probable foreground" << std::endl
	     << "Press 's' to save the current image_ff" << std::endl
	     << "Press 'n' to continue to next step" << std::endl;
}


void onMouseFill( int event, int x, int y, int, void* )
{
    if( event == cv::EVENT_LBUTTONDOWN ) {
        cv::Point seed = cv::Point(x,y);
        int lo = loDiff;
        int up = upDiff;
        cv::Rect ccomp;
        cv::Scalar newVal = cv::Scalar(b, g, r);
        int area = cv::floodFill(image_ff, seed, newVal, &ccomp, 
            cv::Scalar(lo, lo, lo), cv::Scalar(up, up, up));
        cv::imshow("mask_ff_input", image_ff);
    
        std::cout << "painted\n";
    }
}


cv::Mat GCApplication::FloodFill( cv::Mat image_ff0, std::string output_dir )
{
    helpFloodfill();
    image_ff0.copyTo(image_ff);
    cv::namedWindow( "mask_ff_input", cv::WINDOW_AUTOSIZE );
    cv::createTrackbar( "lo_diff", "mask_ff_input", &loDiff, 255, 0 );
    cv::createTrackbar( "up_diff", "mask_ff_input", &upDiff, 255, 0 );
    cv::setMouseCallback( "mask_ff_input", onMouseFill, 0 );

    for(;;)
    {
        cv::imshow("mask_ff_input",  image_ff);
        int c = cv::waitKey(0);
        if( (char)c == 'n' )
        {
            std::cout << "Next step ...\n";    
            cv::destroyWindow( "mask_ff_input" );
            break;
        }
        switch( (char)c )
        {
        case 'b':
            std::cout << "background" << std::endl;
            r = 0;
            g = 0;
            b = 0;
            break;
        case 'f':
            std::cout << "foreground" << std::endl;
            r = 60;
            g = 60;
            b = 60;
            break;
        case 'p':
            std::cout << "probable" << std::endl;
            r = 180;
            g = 180;
            b = 180;
            break;
        case 's':
            std::cout << "Save the image_ff to: " << output_dir << std::endl;
            imwrite( output_dir, image_ff);
            break;
        }
    }
    return image_ff / 60;
}
