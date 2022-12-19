// make
// make clean

// References:
// http://programmingcomputervision.com/downloads/ProgrammingComputerVision_CCdraft.pdf
// Section 5.4 - Stereo Images

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "CImg.h"
using namespace cimg_library;
// #ifndef cimg_imagepath
// #define cimg_imagepath "img/"
// #endif


int main(int argc,char **argv){

    std::cout << "GPU Stereopsis" << std::endl;

    // Input Image Options
    const CImg<float> left_rgb("reference/im2.png"); // 450, 375
    const CImg<float> right_rgb("reference/im6.png");

    // const CImg<float> left_rgb("images/frame000869.jpg");
    // const CImg<float> right_rgb("images/frame000870.jpg");

    // const CImg<float> left_rgb("images/test_case_left.bmp");
    // const CImg<float> right_rgb("images/test_case_right.bmp");

    // Grayscale
    const CImg<float> left_gray  = left_rgb.get_RGBtoHSV().channel(2).normalize(0,255);
    const CImg<float> right_gray = right_rgb.get_RGBtoHSV().channel(2).normalize(0,255);

    // Gradient
    const char grad_direction[] = "x"; // 'x' for Vertical Detection (left-to-right stereopsis), "xy" for both
    const CImg<float> left_gradient  = left_gray.get_gradient(grad_direction,2)[0];
    const CImg<float> right_gradient = right_gray.get_gradient(grad_direction,2)[0];

    // Final Assignment
    const CImg<float> left  = left_gradient;
    const CImg<float> right = right_gradient;

    // Debugging
    left_rgb.save("debug1.bmp");
    left_gray.save("debug2.bmp");
    left_gradient.save("debug3.bmp");
    left.save("debug4.bmp");

    // Bounds Check and Print
    int width = left.width();
    int height = left.height();
    size_t size = left.size(); //width*height*sizeof(unsigned char);
    std::cout << width << "," << height << "," << size << std::endl;
    if(right.width() != width || right.height() != height){
        throw std::runtime_error("Stereo image dimensions do not match"); 
    }
    
    const int max_disparity = 50;

    for(int d=0; d<=max_disparity; d++){
        CImg<float> output(left.width(), left.height(), 1, 1, 0);

        const int w = 7; // Window Width
        for(int y=(w/2); y<height-(w/2); y++){
            for(int x=(w/2); x<width-(w/2); x++){
                
                float sum_R = 0.0f;
                float sum_G = 0.0f;
                float sum_B = 0.0f;
                for(int u=(-w/2); u<((w+1)/2); u++){
                    for(int v=(-w/2); v<((w+1)/2); v++){
                        // sum_R += left(x+u, y+v, 0) * left(x+u+offset, y+v, 0);
                        // sum_G += left(x+u, y+v, 1) * left(x+u+offset, y+v, 1);
                        // sum_B += left(x+u, y+v, 2) * left(x+u+offset, y+v, 2);

                        // sum_R += std::abs( left(x+u, y+v, 0) - right(x+u+offset, y+v, 0) );
                        // sum_G += std::abs( left(x+u, y+v, 1) - right(x+u+offset, y+v, 1) );
                        // sum_B += std::abs( left(x+u, y+v, 2) - right(x+u+offset, y+v, 2) );
                        
                        // sum_L += std::abs( right(x+u, y+v, 0) - left(x+u+d, y+v, 0) );

                        // sum_R += left(x+u+d, y+v, 0);
                        // sum_G += left(x+u+d, y+v, 1);
                        // sum_B += left(x+u+d, y+v, 2);

                        sum_R += ( right(x+u, y+v, 0) * left(x+u+d, y+v, 0) ) / 255.0f;
                        // sum_G += ( right(x+u, y+v, 1) * left(x+u+d, y+v, 1) ) / 255.0f;
                        // sum_B += ( right(x+u, y+v, 2) * left(x+u+d, y+v, 2) ) / 255.0f;

                        
                        // std::cout << u << "," << v << std::endl;

                        // sum_L += left(x+u, y+v, 0);
                        // sum_R += right(x+u, y+v, 0);

                        // float diff = ( right(x+u, y+v, 0) - left(x+u+d, y+v, 0) );
                        // sum_R += (diff * diff);
                    }
                }
                sum_R /= (1.0f * w * w);

                // sum_G /= (1.0f * w * w);
                // sum_B /= (1.0f * w * w);
                // float sum_L = std::sqrt( (sum_R * sum_R) + (sum_G * sum_G) + (sum_B * sum_B) ) / std::sqrt(3.0f);
                // output(x, y, 0) = sum_L;

                output(x, y, 0) = sum_R;
            }
            // std::cout << std::endl;
        }

        std::string name = "test_output" + std::to_string(d) + ".bmp";
        // output.equalize(256);
        // output.normalize(0,255);
        output.save(name.c_str());
    }

    // CImg<float> L2Norm(left.width(), left.height(), 1, 3, 0);
    
    // // Map
    // for(int i=0; i<height; i++){
    //     for(int j=0; j<width; j++){
    //         float diff_R = output(i, j, 0) - left(i, j, 0);
    //         float diff_G = output(i, j, 1) - left(i, j, 1);
    //         float diff_B = output(i, j, 2) - left(i, j, 2);
    //         L2Norm(i, j, 0) = (diff_R * diff_R);
    //         L2Norm(i, j, 1) = (diff_G * diff_G);
    //         L2Norm(i, j, 2) = (diff_B * diff_B);
    //     }
    // }
    
    // // Reduce (Euclidian 3D Distance)
    // float l2Sum = 0.0f;
    // for(int i=0; i<height; i++){
    //     for(int j=0; j<width; j++){
    //         l2Sum += L2Norm(i, j, 0);
    //         l2Sum += L2Norm(i, j, 1);
    //         l2Sum += L2Norm(i, j, 2);
    //     }
    // }
    // l2Sum = std::sqrt(l2Sum);
    
    // std::cout << "L2 Distance: " << l2Sum << std::endl;
    // // G3-Original: 43762.7
    // // G5-Original: 63600.3
    

    // int index = 0;
    // for (CImgList<>::iterator it = test.begin(); it<test.end(); ++it){
    //     std::string name = "test" + std::to_string(index) + ".bmp";
    //     index++;
    //     (*it).save(name.c_str());
    // }
    // test[1].save("test.bmp");

    // CImg<unsigned char> output(src.data(), src.width(), src.height());
    // output.normalize(0,255);
    // output.equalize(256);
    // output.save("output2.bmp");
    // L2Norm.save("L2.bmp");

}