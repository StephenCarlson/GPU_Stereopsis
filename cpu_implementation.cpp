// make
// make clean

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

    const CImg<unsigned char> left = CImg<>("reference/im2.png"); // 450, 375
    const CImg<unsigned char> right = CImg<>("reference/im6.png");

    int width = left.width();
    int height = left.height();
    size_t size = left.size(); //width*height*sizeof(unsigned char);

    if(right.width() != width || right.height() != height){
        throw std::runtime_error("Stereo image dimensions do not match"); 
    }

    std::cout << width << "," << height << "," << size << std::endl;


    CImg<float> output(left.height(), left.width(), 1, 3, 0);

    // Using i and j instead of x and y
    const int w = 3; // Offset from h
    for(int i=0; i<height-w; i++){
        // std::cout << i << ":";
        for(int j=0; j<width-w; j++){
            // std::cout << j << "," << i << "\n";

            // std::cout << j << "," << i << ":" << left(i, j, 0) << "\n";
            
            float sum_R = 0;
            float sum_G = 0;
            float sum_B = 0;
            for(int u=0; u<w; u++){
                for(int v=0; v<w; v++){
                    sum_R += left(i+u, j+v, 0) * 1.0f; // gaFilter3[u][v];
                    sum_G += left(i+u, j+v, 1) * 1.0f; // gaFilter3[u][v];
                    sum_B += left(i+u, j+v, 2) * 1.0f; // gaFilter3[u][v];
                }
            }
            
            output(i, j, 0) = sum_R/9.0f; // 273.0f
            output(i, j, 1) = sum_G/9.0f;
            output(i, j, 2) = sum_B/9.0f;
        }
        // std::cout << std::endl;
    }

    // CImg<float> L2Norm(left.width(), left.height(), 1, 3, 0);
// 
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
// 
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
// 
    // std::cout << "L2 Distance: " << l2Sum << std::endl;
    // // G3-Original: 43762.7
    // // G5-Original: 63600.3
// 
// 
    // // CImg<unsigned char> output(src.data(), src.width(), src.height());
    output.save("output.bmp");
    // L2Norm.save("L2.bmp");

}