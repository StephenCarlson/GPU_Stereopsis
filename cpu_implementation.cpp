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

    std::cout << "Homework 3" << std::endl;

    // CImg<unsigned char> src("ana_noise.bmp");
    const CImg<unsigned char> src = CImg<>("ana_noise.bmp");
    const CImg<unsigned char> original = CImg<>("ana_sbk.bmp");

    int width = src.width();
    int height = src.height();
    size_t size = src.size(); //width*height*sizeof(unsigned char);

    std::cout << size << std::endl;

    std::vector<std::vector<float>> gaFilter3
    {
        { 1.0,  2.0,  1.0},
        { 2.0,  4.0,  2.0},
        { 1.0,  2.0,  1.0}
    }; // 16

    std::vector<std::vector<float>> gaFilter5
    {
        {1.0,  4.0,  7.0,  4.0,  1.0},
        {4.0, 16.0, 26.0, 16.0,  4.0},
        {7.0, 26.0, 41.0, 26.0,  7.0},
        {4.0, 16.0, 26.0, 16.0,  4.0},
        {1.0,  4.0,  7.0,  4.0,  1.0}
    }; // 273.0

    CImg<float> output(src.width(), src.height(), 1, 3, 0);

    // Using i and j instead of x and y
    const int w = 3; // Offset from h
    for(int i=0; i<height-w; i++){
        // std::cout << i << ":";
        for(int j=0; j<width-w; j++){
            // std::cout << j << ";";
            // output(i, j, 0) = src(i, j, 0);
            // output(i, j, 1) = src(i, j, 1);
            // output(i, j, 2) = src(i, j, 2);

            float sum_R = 0;
            float sum_G = 0;
            float sum_B = 0;
            for(int u=0; u<w; u++){
                for(int v=0; v<w; v++){
                    sum_R += src(i+u, j+v, 0) * gaFilter3[u][v];
                    sum_G += src(i+u, j+v, 1) * gaFilter3[u][v];
                    sum_B += src(i+u, j+v, 2) * gaFilter3[u][v];
                }
            }
            
            output(i, j, 0) = sum_R/16.0f; // 273.0f
            output(i, j, 1) = sum_G/16.0f;
            output(i, j, 2) = sum_B/16.0f;
        }
        // std::cout << std::endl;
    }

    CImg<float> L2Norm(src.width(), src.height(), 1, 3, 0);

    // Map
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            float diff_R = output(i, j, 0) - original(i, j, 0);
            float diff_G = output(i, j, 1) - original(i, j, 1);
            float diff_B = output(i, j, 2) - original(i, j, 2);
            L2Norm(i, j, 0) = (diff_R * diff_R);
            L2Norm(i, j, 1) = (diff_G * diff_G);
            L2Norm(i, j, 2) = (diff_B * diff_B);
        }
    }

    // Reduce (Euclidian 3D Distance)
    float l2Sum = 0.0f;
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            l2Sum += L2Norm(i, j, 0);
            l2Sum += L2Norm(i, j, 1);
            l2Sum += L2Norm(i, j, 2);
        }
    }
    l2Sum = std::sqrt(l2Sum);

    std::cout << "L2 Distance: " << l2Sum << std::endl;
    // G3-Original: 43762.7
    // G5-Original: 63600.3


    // CImg<unsigned char> output(src.data(), src.width(), src.height());
    output.save("test.bmp");
    L2Norm.save("L2.bmp");

}