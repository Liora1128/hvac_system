#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::VideoCapture cap(0); // Open the default camera

    if(!cap.isOpened()) // Check if we succeeded
    {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    std::cout << "Camera opened successfully." << std::endl;

    cv::Mat frame;
    while(true)
    {
        cap >> frame;

        if(frame.empty())
        {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }

        cv::imshow("Building HVAC Camera", frame);
        int key = cv::waitKey(1);

        if(key == 27) // Exit on ESC key
        {
            std::cout << "Exiting..." << std::endl;
            break;
        }        
    }

    cap.release(); // Release the camera resource
        cv::destroyAllWindows(); // Close all OpenCV windows

        std::cout << "Camera released and windows closed." << std::endl;

        return 0;    
}