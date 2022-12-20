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

void uniform_filter(CImg<float>& out, const CImg<float>& img, int width, int height, int size){
    const int w = size; // Window Width

    for(int y=(w/2); y<height-(w/2); y++){
        for(int x=(w/2); x<width-(w/2); x++){
            float sum_R = 0.0f;
            float sum_G = 0.0f;
            float sum_B = 0.0f;

            for(int u=(-w/2); u<((w+1)/2); u++){
                for(int v=(-w/2); v<((w+1)/2); v++){
                    sum_R += img(x+u, y+v, 0);
                    sum_G += img(x+u, y+v, 0);
                    sum_B += img(x+u, y+v, 0);
                }
            }
            sum_R /= (1.0f * w * w);
            sum_G /= (1.0f * w * w);
            sum_B /= (1.0f * w * w);

            float sum_L = std::sqrt( (sum_R * sum_R) + (sum_G * sum_G) + (sum_B * sum_B) ) / std::sqrt(3.0f);
            out(x, y, 0) = sum_L;
        }
    }
}

// void plane_sweep_ncc(Cimg<float>& out, const CImg<float>& left, const CImg<float> right, int width, int height, int size){
// 
// }


int main(int argc,char **argv){

    std::cout << "GPU Stereopsis" << std::endl;

    const int max_disparity = 50;
    const int w = 7; // Window Width

    // Input Image Options
    const CImg<float> left_rgb("reference/im2.png"); // 450, 375
    const CImg<float> right_rgb("reference/im6.png");
    // const CImg<float> left_rgb("images/frame000869.jpg");
    // const CImg<float> right_rgb("images/frame000870.jpg");
    // const CImg<float> left_rgb("images/test_case_left.bmp");
    // const CImg<float> right_rgb("images/test_case_right.bmp");

    // Bounds Check and Registration
    int width = left_rgb.width();
    int height = left_rgb.height();
    size_t size = left_rgb.size(); //width*height*sizeof(unsigned char);
    std::cout << width << "," << height << "," << size << std::endl;
    if(right_rgb.width() != width || right_rgb.height() != height){
        throw std::runtime_error("Stereo image dimensions do not match"); 
    }

    // Grayscale
    const CImg<float> left_gray  = left_rgb.get_RGBtoHSV().channel(2).normalize(0,255);
    const CImg<float> right_gray = right_rgb.get_RGBtoHSV().channel(2).normalize(0,255);

    // Gradient
    const char grad_direction[] = "x"; // 'x' for Vertical Detection (left-to-right stereopsis), "xy" for both
    const CImg<float> left_gradient  = left_gray.get_gradient(grad_direction,2)[0];
    const CImg<float> right_gradient = right_gray.get_gradient(grad_direction,2)[0];

    // Mean
    CImg<float> left_mean(width, height, 1, 1, 0);
    CImg<float> right_mean(width, height, 1, 1, 0);
    uniform_filter(left_mean,  left_gray,  width, height, w);
    uniform_filter(right_mean, right_gray, width, height, w);
    // std::cout << left_gray.print() << std::endl; // 0 to 255

    // Final Assignment
    const CImg<float> left  = left_gray  - left_mean;   // left_gradient;
    const CImg<float> right = right_gray - right_mean; // right_gradient;

    // Debugging
    left_rgb.save("debug1.bmp");
    left_gray.save("debug2.bmp");
    left_gradient.save("debug3.bmp");
    left_mean.save("debug4.bmp");
    // std::cout << left_mean.print() << std::endl;
    // std::cout << left.print() << std::endl;
    left.get_normalize(0,255).save("debug5.bmp");



    for(int d=0; d<=max_disparity; d++){
        CImg<float> output(width, height, 1, 1, 0);

        for(int y=(w/2); y<height-(w/2); y++){
            for(int x=(w/2); x<width-(w/2); x++){
                
                float sum_N = 0.0f;
                float sum_D1 = 0.0f;
                float sum_D2 = 0.0f;
                for(int u=(-w/2); u<((w+1)/2); u++){
                    for(int v=(-w/2); v<((w+1)/2); v++){
                        float left_term = left(x+u+d, y+v, 0);
                        float right_term = right(x+u, y+v, 0);

                        sum_N  += ( left_term * right_term ); // / 255.0f;
                        sum_D1 += ( left_term * left_term );
                        sum_D2 += ( right_term * right_term );
                    }
                }
                // sum_N  /= (1.0f * w * w);
                // sum_D1 /= (1.0f * w * w);
                // sum_D2 /= (1.0f * w * w);

                // output(x, y, 0) = sum_D1 / (255.0f * w * w); // 0 to 255
                // output(x, y, 0) = sum_N/std::sqrt(sum_D1 * sum_D2); // -1.0 to 1.0
                output(x, y, 0) = ( ( sum_N/(std::sqrt(sum_D1 * sum_D2)) )+ 1.0f ) * (255.0f / 2.0f);
            }
            // std::cout << std::endl;
        }

        std::string name = "test_output" + std::to_string(d) + ".bmp";
        // output.equalize(256);
        std::cout << output.print() << std::endl;
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