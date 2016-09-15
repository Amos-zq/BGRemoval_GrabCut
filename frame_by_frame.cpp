#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <sys/stat.h>
#include "gcapp.h"

void GCApplication::getMaskCombined (cv::Mat image, cv::Mat mask, 
    cv::Mat mask_c) 
{
    cv::Mat temp, mask_temp;
    temp.create(image.size(), image.type());
    mask_temp.create(mask.size(), mask.type());        

    cv::Mat on;
    cv::Mat off;
    on.create(mask.size(), mask.type());
    off.create(mask.size(), mask.type());
    on.setTo(cv::Scalar(255, 255, 255));
    off.setTo(cv::Scalar(0, 0, 0));
    
    cv::Mat model_db;
    model_db.create(mask.size(), mask.type());
    model_db = on;
    off.copyTo(model_db, mask);

    temp.setTo(BLUE);
    temp.copyTo(mask_c, model_db);

    cv::Mat model_p;
    model_p.create(mask.size(), mask.type());
    model_p = (mask & 2) * 255;
    
    temp.setTo(GREEN);
    temp.copyTo(mask_c, model_p);

    cv::Mat model_f;
    model_f.create(mask.size(), mask.type());
    model_f = mask & 1;
    temp.setTo(PINK);
    temp.copyTo(mask_c, model_f);

    off.copyTo(model_f, model_p);
    temp.setTo(RED);
    temp.copyTo(mask_c, model_f);
}

void  GCApplication::FrameByFrame(std::string filename, int start_frame, 
    int iter, int dilation_size, int erosion_size)
{
    int count = 0;

    cv::VideoCapture cap;
    bool update_bg_model = true;

    std::cout << "input video: " << filename.c_str() << std::endl;
    cap.open(filename.c_str());


    if( !cap.isOpened() )
    {
        printf("can not open camera or video file\n");
        return;
    }
    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));

    
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string:: npos != last_slash_idx)
        filename.erase(0, last_slash_idx + 1);
    const size_t first_format_idx = filename.find_first_of(".");
    filename.erase(first_format_idx, 4);

    const std::string winName1 = "input_mask";
    cv::namedWindow( winName1, cv::WINDOW_AUTOSIZE );
    const std::string winName2 = "output_binmask";
    cv::namedWindow(winName2, cv::WINDOW_AUTOSIZE);
    const std::string winName3 = "output_mask";
    cv::namedWindow(winName3, cv::WINDOW_AUTOSIZE);

    //mkdir:
    //output/original_image      -- image of every frame
    //output/input_masks         -- input mask covered on original image
    //output/input_masks_grey    -- input mask 
    //output/output_binmasks     -- output binary mask covered on original image
    //output/output_binmask_grey -- output binary mask 
    //output/output_masks        -- output mask covered on original image 
    //output/output_masks_grey   -- output mask
    std::string toBeBuiltDir;
    toBeBuiltDir = "output";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    toBeBuiltDir = "output/" + filename;
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    toBeBuiltDir = "output/" + filename + "/original_image";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    toBeBuiltDir = "output/" + filename + "/input_masks";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
    toBeBuiltDir = "output/" + filename + "/input_masks_grey";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);   
    toBeBuiltDir = "output/" + filename + "/output_binmasks";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    toBeBuiltDir = "output/" + filename + "/output_binmasks_grey";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    toBeBuiltDir = "output/" + filename + "/output_masks";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);    
    toBeBuiltDir = "output/" + filename + "/output_masks_grey";
    mkdir(toBeBuiltDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    std::string original_image_dir = "output/" + filename + "/original_image/";
    std::string input_mask_dir = "output/" + filename + "/input_masks/";
    std::string input_mask_grey_dir = "output/" + filename + 
        "/input_masks_grey/";
    std::string output_binmask_dir = "output/" +  filename + 
        "/output_binmasks/";
    std::string output_binmask_grey_dir = "output/" + filename + 
        "/output_binmasks_grey/";
    std::string output_mask_dir = "output/" + filename + "/output_masks/";  
    std::string output_mask_grey_dir = "output/" + filename + 
        "/output_masks_grey/";

    cv::Mat img0;
    cv::Mat binMask;
    
    bool initialized = false;   // if the initial input mask has been drawn

    for(;;)
    {
        cap >> img0;
        
        if (count < start_frame) 
        {
            count ++;
            continue;
        }

        if( img0.empty() )
            break;

        cv::Mat mask_c;
        mask_c.create(img0.size(), img0.type());    
        cv::Mat res_c;
        res_c.create(img0.size(), img0.type());

        std::ostringstream ostr;
        ostr << std::setfill('0') << std::setw(5) << count;
        std::string count_str = ostr.str();


        if (!initialized) 
        {
            std::cout << input_mask_grey_dir + filename + "_frame_" + 
                count_str + ".jpg" << std::endl;
            m_mask = cv::imread( input_mask_grey_dir + filename + 
                "_frame_" + count_str + ".jpg", 0);
            if( m_mask.empty() )
            {
                std::cout << "Input mask empty\n";
                return;
            }
            m_mask = m_mask / 60;
            cv::Size S = cv::Size(img0.size().width, img0.size().height);
            grabCutWMask(img0, m_mask, iter);
            initialized = true;
        }


        else {
            deleteHoles(m_mask);
            m_mask = updateMask(m_mask, dilation_size, erosion_size);

            cv::Mat input_c;
            cv::Mat input_mask_c;
            input_c.create(img0.size(), img0.type());
            input_mask_c.create(img0.size(), img0.type());
    
            getMaskCombined(img0, m_mask, input_mask_c);
            input_c = img0 * 0.5 + input_mask_c * 0.5;
            imshow(winName1, input_c);
            imwrite(input_mask_dir + filename + 
                "_frame_" + count_str + ".jpg", input_c);
            imwrite(input_mask_grey_dir + filename + 
                "_frame_" + count_str + ".jpg", m_mask * 60);
            imwrite(original_image_dir + filename + 
                "_frame_" + count_str + ".png", img0);

            grabCutWMask(img0, m_mask, iter);
        }

        getMaskCombined (img0, m_mask, mask_c);
        res_c = img0 * 0.5 + mask_c * 0.5;
        
        imshow(winName3, res_c);
        imwrite(output_mask_dir + filename + 
            "_frame_" + count_str + ".jpg", res_c);
        imwrite(output_mask_grey_dir + filename + 
            "_frame_" + count_str + ".jpg", m_mask * 60);
        
        cv::Mat img;
        getBinMask(m_mask, binMask);
        img0.copyTo(img);
        img = img * 0.5 + PINK * 0.5;
        img0.copyTo(img, binMask);
        m_mask = binMask;

        imshow(winName2, img);
        imwrite(output_binmask_dir + filename + 
            "_frame_" + count_str + ".jpg", img);
        imwrite(output_binmask_grey_dir + filename + 
            "_frame_" + count_str + ".jpg", binMask * 255);
   
        std::cout << "number of frame: " << count_str << std::endl;
        count ++;
        int keycode = cv::waitKey(30);
        if (keycode = 'n')
            continue;
        if ( keycode == 27 )
            break;
    }

    cv::destroyWindow(winName1);
    cv::destroyWindow(winName2);
    cv::destroyWindow(winName3);
    
    // make a video for input and output masks
    // used on linux: need ffmpeg to be installed 
    // (can be commented if using other systems)
    std::string make_video_command = "ffmpeg -y -framerate 25 -i output/" + 
        filename + "/input_masks/" + filename + 
        "_frame_%05d.jpg -c:v libx264 -profile:v high -crf 20 -pix_fmt" + 
        " yuv420p output/" + filename + "/input_mask.avi";
    system(make_video_command.c_str());
    make_video_command = "ffmpeg -y -framerate 25 -i output/" + filename + 
        "/output_masks/" + filename + "_frame_%05d.jpg -c:v libx264 " + 
        "-profile:v high -crf 20 -pix_fmt yuv420p output/" 
        + filename + "/output_mask.avi";
    system(make_video_command.c_str());
    make_video_command = "ffmpeg -y -framerate 25 -i output/" + filename + 
        "/output_binmasks/" + filename + "_frame_%05d.jpg -c:v libx264" + 
        " -profile:v high -crf 20 -pix_fmt yuv420p output/" 
        + filename + "/output_binmask.avi";
    system(make_video_command.c_str());
    
    return;

}

