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

    const CImg<unsigned char> left_rgb("reference/im2.png"); // 450, 375
    const CImg<unsigned char> right_rgb("reference/im6.png");

    // const CImg<unsigned char> left_rgb("frame000869.jpg");
    // const CImg<unsigned char> right_rgb("frame000870.jpg");
    
    const CImg<float> left = left_rgb.get_RGBtoYCbCr().channel(0);
    const CImg<float> right = right_rgb.get_RGBtoYCbCr().channel(0);

    int width = left.width();
    int height = left.height();
    size_t size = left.size(); //width*height*sizeof(unsigned char);

    if(right.width() != width || right.height() != height){
        throw std::runtime_error("Stereo image dimensions do not match"); 
    }

    std::cout << width << "," << height << "," << size << std::endl;

    
    // CImg<float> output(left.width(), left.height(), 1, 3, 0);
    CImg<float> output(left.width(), left.height(), 1, 1, 0);
    
    const int offset = 1;
    const char grad_direction[] = "xy"; // 'x' for Vertical Detection, "xy" for both

    CImgList<float> test = left.get_gradient(grad_direction,2);

    const int w = 5; // Offset from h
    // for(int y=0; y<height-w; y++){
    for(int y=(w/2); y<height-(w/2); y++){
        // std::cout << i << ":";
        // for(int x=0; x<width-w; x++){
        for(int x=(w/2); x<width-(w/2); x++){
            // std::cout << x << "," << y << "\n";

            // std::cout << x << "," << y << ":" << left(x, y, 0) << "\n";
            
            float sum_R = 0;
            // float sum_G = 0;
            // float sum_B = 0;
            // for(int u=0; u<w; u++){
            for(int u=(-w/2); u<((w+1)/2); u++){
                // for(int v=0; v<w; v++){
                for(int v=(-w/2); v<((w+1)/2); v++){
                    // sum_R += left(x+u, y+v, 0) * left(x+u+offset, y+v, 0);
                    // sum_G += left(x+u, y+v, 1) * left(x+u+offset, y+v, 1);
                    // sum_B += left(x+u, y+v, 2) * left(x+u+offset, y+v, 2);

                    // sum_R += std::abs( left(x+u, y+v, 0) - right(x+u+offset, y+v, 0) );
                    // sum_G += std::abs( left(x+u, y+v, 1) - right(x+u+offset, y+v, 1) );
                    // sum_B += std::abs( left(x+u, y+v, 2) - right(x+u+offset, y+v, 2) );
                    
                    float diff = ( left(x+u, y+v, 0) - right(x+u+offset, y+v, 0) );
                    sum_R += (diff * diff);
                }
            }
            
            // output(x, y, 0) = left(x, y, 0);
            // output(x, y, 1) = left(x, y, 1);
            // output(x, y, 2) = left(x, y, 2);

            // output(x, y, 0) = sum_R/255.0f;
            // output(x, y, 1) = sum_G/255.0f;
            // output(x, y, 2) = sum_B/255.0f;

            output(x, y, 0) = sum_R;
            // output(x, y, 1) = sum_G;
            // output(x, y, 2) = sum_B;
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
    output.normalize(0,255);
    output.equalize(256);
    int index = 0;
    for (CImgList<>::iterator it = test.begin(); it<test.end(); ++it){
        std::string name = "test" + std::to_string(index) + ".bmp";
        index++;
        (*it).save(name.c_str());
    }
    // test[1].save("test.bmp");
    output.save("output2.bmp");
    // L2Norm.save("L2.bmp");

}