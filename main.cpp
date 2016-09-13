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
#include <sys/stat.h>
#include "gcapp.h"

static void help_all() {
    std::cout << "There are three modes here:\n\n" 
              << "1. For single image (Mode = 'single')\n" 
              << "  Command line: ./main --mode='single' test.jpg --iter=N\n"
              << "  N -- number of iterations (defalt=3)\n\n"
              << "2. For image with depth mask (Mode = 'rgbd')\n" 
              << "  Command line: ./main --mode='rgbd' test.png " 
              << "depth_mask.png --iter=Ni --dilation=Nd --erosion=Ne\n"
              << "  Ni -- the number of iterations (default=3)\n"
              << "  Nd -- dilation_size (defalut=5)\n"
              << "  Ne -- erosion_size (default=5)\n\n"
              << "3. For video (Mode = 'video')\n"
              << "  Command line: ./main --mode='video' test.mp4 --frame=Nf"
              << " --iter=Ni --dilation=Nd --erosion=Ne\n" 
              << "  Nf -- number of frame to begin (default=0)\n"
              << "    if Nf = 0, then you will segment the first frame \n"
              << "               manually and the program runs from the first\n" 
              << "               frame automatically\n"
              << "    if Nf != 0, then you will segment the Nf-th frame\n"
              << "                manually and the program runs from Nf-th\n" 
              << "                frame automatically, while frames prior to\n" 
              << "                Nf stays the same\n" 
              << "                (It is used to modify Grabcut mistakes)\n"
              << "  Ni -- the number of iterations (default=3)\n"
              << "  Nd -- dilation_size (defalut=5)\n"
              << "  Ne -- erosion_size (default=5)\n";
}

static void help_preprocess() {
    std::cout << "\n" << 
        "Select a rectangular area around the object you want to segment\n" <<
        "\nHot keys: \n"
        "\tleft mouse button - set rectangle\n"
        "\n"
        "\tCTRL+left mouse button - set GC_BGD pixels\n"
        "\tSHIFT+left mouse button - set GC_FGD pixels\n"
        "\n"
        "\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
        "\tSHIFT+right mouse button - set GC_PR_FGD pixels\n" 
        "\n"
        "\tpress 'n' - next step\n" << std::endl;
}

GCApplication gcapp;

static void on_mouse( int event, int x, int y, int flags, void* param )
{
    gcapp.mouseClick( event, x, y, flags, param );
}

int main( int argc, char** argv )
{
    cv::CommandLineParser parser(argc, argv, 
        "{mode||}{@input||}{@depth||}{frame||}{iter||}{dilation||}{erosion||}{help h||}");
    
    if (parser.has("help")) {
        help_all();
        return 0;
    }
        
    std::string mode_string = parser.get<std::string>("mode");
    int mode;
    // 0-single image   1-rgbd image   2-video
    if (mode_string == "single") mode = 0;
    else if (mode_string == "rgbd") mode = 1;
    else if (mode_string == "video") mode = 2;
    else {
        std::cout << "invalid mode -- Please choose from" <<
            "'single', 'rgbd' and 'video'." << std::endl;
        return 1;
    }
    
    cv::Mat image, mask;

    std::string input_filename = parser.get<std::string>("@input");
    std::string filename = input_filename;
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string:: npos != last_slash_idx)
        filename.erase(0, last_slash_idx + 1);
    const size_t first_format_idx = filename.find_first_of(".");
    filename.erase(first_format_idx, 4);    
    
    int iter;
    if (parser.has("iter")) iter = parser.get<int>("iter");
    else iter = 3;
    
    int dilation_size;
    if (parser.has("dilation")) dilation_size = parser.get<int>("dilation");
    else dilation_size = 5;
    
    int erosion_size;
    if (parser.has("erosion")) erosion_size = parser.get<int>("erosion");
    else erosion_size = 5;
    
    //mode 0-single image
    if (mode == 0) {
        std::cout << "You are in Mode 0----single image." << std::endl;
        image = cv::imread(input_filename);
        std::cout << "Read image from " << input_filename << std::endl;
    }

    // mode 1-rgbd image with depth mask provided
    if (mode == 1) {
        std::cout << "You are in Mode 1----rgbd image with " <<
            "depth mask provided." << std::endl;
        image = cv::imread(input_filename);
        std::cout << "Read image from " << input_filename << std::endl;
        std::string depth_filename = parser.get<std::string>("@depth");
        mask = cv::imread(depth_filename, CV_8UC1);
        std::cout << "depth filename: " << depth_filename << std::endl;
    }
        
    //mode 2-video
    int start_frame;
    std::string start_frame_str;
    std::string output_dir;
    cv::VideoCapture cap;

    // preprocess: get input mask
    if (mode == 2) {
        std::cout << "You are in Mode 2---video with frame number provided." 
            << std::endl;
        if (parser.has("frame")) start_frame = parser.get<int>("frame");
        else start_frame = 0;

        std::ostringstream ostr;
        ostr << std::setfill('0') << std::setw(5) << start_frame;
        start_frame_str = ostr.str();

        if (start_frame == 0) {

            std::cout << "input video: " << input_filename << std::endl;
            cap.open(input_filename.c_str());

            if( !cap.isOpened() )
            {
                std::cout << "can not open camera or video file\n" << std::endl;
                return -1;
            }
            cap >> image;        
        }

        else {
            std::string changed_frame_input_mask = "output/" + filename + 
                "/original_image/" + filename + "_frame_" + 
                start_frame_str + ".png";
            image = cv::imread(changed_frame_input_mask, 1);
            if (image.empty()) {
                std::cout << "cannot open input mask: " 
                          << changed_frame_input_mask << std::endl;
                return 1; 
            }
        }
    }

    if (mode != 1) {
        help_preprocess();
        const std::string winName = "image";
        cv::namedWindow( winName, cv::WINDOW_AUTOSIZE );
        cv::setMouseCallback( winName, on_mouse, 0 );

        gcapp.setImageAndWinName( image, winName );
        gcapp.showImage();
    
    
        bool preprocess_done = false;
        while(!preprocess_done)
        {
            int c = cv::waitKey(0);
            switch ((char) c) {
            case 'n':  
                std::string to_be_build_dir = "output";
                mkdir(to_be_build_dir.c_str(), 
                    S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                if (mode == 2) {
                    to_be_build_dir = "output/" + filename;
                    mkdir(to_be_build_dir.c_str(), 
                        S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    to_be_build_dir = to_be_build_dir + "/input_masks_grey";
                    mkdir(to_be_build_dir.c_str(), 
                        S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    output_dir = "output/" + filename + "/input_masks_grey/" + 
                        filename + "_frame_" + start_frame_str + ".jpg";
                }
                if (mode == 0) output_dir = filename + "_input_mask.png";

                cv::Mat mask1 = gcapp.mask * 60;

                cv::destroyWindow( winName );
                gcapp.mask = gcapp.FloodFill(mask1, output_dir);
                preprocess_done = true;
                break;
            }
        }
        std::cout << "preprocess_done." << std::endl;
    }
    
    // grabcut: get output mask
    if (mode == 0 || mode == 1) {
        if (mode == 1) {
            gcapp.mask = gcapp.mask_update(mask, dilation_size, erosion_size);
        }
        gcapp.mask = gcapp.grab_cut(image, gcapp.mask, iter);
        cv::Mat binMask;
        binMask.create(mask.size(), CV_8UC1);
        gcapp.getBinMask(gcapp.mask, binMask);
        std::string to_be_build_dir = "output";
        mkdir(to_be_build_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cv::imwrite("output/" + filename + "_output.png", binMask * 255);
        std::cout << "Write output mask in: " << "output/" + filename + 
            "_output.png" << std::endl;
    }
    if (mode == 2) gcapp.FrameByFrame(input_filename, start_frame, 
        iter, dilation_size, erosion_size);
   
    return 0;
}
