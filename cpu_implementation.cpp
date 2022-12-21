// make
// make clean

// References:
// http://programmingcomputervision.com/downloads/ProgrammingComputerVision_CCdraft.pdf
// Section 5.4 - Stereo Images

// TODO Items:
// - Remove the border artifact, need to fill/roll/mirror the edge cases
// - Get the Threshold Mask working for removing poorly-correlated regions from the depth map
// - Try the suggestion in the text to swap left and right pairs and keep best parts, removes occlusions
// - Parameterize the Vertical/Horizontal modes of operation, right now I have to uncomment lines
// - Add command-line arguments/parameters, no more hard-coding everything
// - Get a live / online stream ingestion working
// - Despite having the GPU implementation to do, try doing dynamic programming / memoization
// - Do a 2D search reduction to find the most centered image offsets, might help make better maps
// - Add camera intrinsics/calibration parameters
// - Camera parameters estimation
// - Add CImgList for Depthmaps capture, learn how to do slicing for argmax behavior
// - Move CImg library to a better position
// - Get VSCode Intellisence to imprint correctly, I'm coding blind and having to read docs too much

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <ctime>

#include "CImg.h"
using namespace cimg_library;
// #ifndef cimg_imagepath
// #define cimg_imagepath "img/"
// #endif


// Forward Declarations
void uniform_filter(CImg<float>&, const CImg<float>&, int, int, int);
void plane_sweep_ncc(CImg<int>&, const CImg<float>&, const CImg<float>&, int, int, int, int);


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

void plane_sweep_ncc(CImg<int>& dmap_scores, const CImg<float>& left, const CImg<float>& right, int width, int height, int w, int max_disparity){

    for(int d=0; d<max_disparity; d++){
        for(int y=(w/2); y<height-(w/2); y++){
            for(int x=(w/2); x<width-(w/2); x++){
                
                float sum_N = 0.0f;
                float sum_D1 = 0.0f;
                float sum_D2 = 0.0f;
                for(int u=(-w/2); u<((w+1)/2); u++){
                    for(int v=(-w/2); v<((w+1)/2); v++){
                        float left_term = left(x+u+d, y+v, 0); // Horizontal (Left-vs-Right) Version
                        // float left_term = left(x+u, y+v+d, 0); // Vertical Version
                        float right_term = right(x+u, y+v, 0);

                        sum_N  += ( left_term * right_term ); // / 255.0f;
                        sum_D1 += ( left_term * left_term );
                        sum_D2 += ( right_term * right_term );
                    }
                }
                
                float score = ( ( sum_N/(std::sqrt(sum_D1 * sum_D2)) ) + 1.0f ) * (255.0f / 2.0f);
                // out(x, y, 0) = score;

                if(score > dmap_scores(x, y, 0)){
                    dmap_scores(x, y, 0) = score;
                    dmap_scores(x, y, 1) = d;
                }
            }
        }
    }
}


int main(int argc,char **argv){

    std::cout << "GPU Stereopsis" << std::endl;

    // Simple Test Case
    // const CImg<float> left_rgb("images/test_case_left.bmp");
    // const CImg<float> right_rgb("images/test_case_right.bmp");
    // const char grad_direction[] = "x"; // 'x' for Vertical Detection (left-to-right stereopsis), "xy" for both
    // const int patch_width = 3;
    // const int max_disparity = 20;

    // Middlebury 2003 Set
    // const CImg<float> left_rgb("images/im2.bmp"); // 450, 375
    // const CImg<float> right_rgb("images/im6.bmp");
    // const char grad_direction[] = "x";
    // const int patch_width = 7;
    // const int max_disparity = 50;

    // Golden Eagle Park
    const CImg<float> left_rgb("images/frame000869.bmp");
    const CImg<float> right_rgb("images/frame000871.bmp");
    const char grad_direction[] = "x";
    const int patch_width = 7;
    const int max_disparity = 50;


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
    const CImg<float> left_gradient  = left_gray.get_gradient(grad_direction,2)[0];
    const CImg<float> right_gradient = right_gray.get_gradient(grad_direction,2)[0];

    // Mean
    CImg<float> left_mean(width, height, 1, 1, 0);
    CImg<float> right_mean(width, height, 1, 1, 0);
    uniform_filter(left_mean,  left_gray,  width, height, patch_width);
    uniform_filter(right_mean, right_gray, width, height, patch_width);
    // std::cout << left_gray.print() << std::endl; // 0 to 255

    // Final Assignment
    const CImg<float> left  = left_gray  - left_mean;   // left_gradient;
    const CImg<float> right = right_gray - right_mean; // right_gradient;

    // Final Product/Target
    CImg<int> dmap_scores(width, height, 1, 3, 0);

    // Debugging
    // left_rgb.save("debug1.bmp");
    // right_rgb.save("debug2.bmp");
    // left_gray.save("debug3.bmp");
    // left_gradient.save("debug3.bmp");
    // left_mean.save("debug4.bmp");
    // std::cout << left_mean.print() << std::endl;
    // std::cout << left.print() << std::endl;
    // left.get_normalize(0,255).save("debug3.bmp");
    // right.get_normalize(0,255).save("debug4.bmp");

    //** Inner-Loop Starting Point **
    // When this is converted to a continuous/streamed process, 
    // this is the boundary for where the speed-up for CPU-vs-GPU is best measured.
    clock_t start = clock();

    // CPU Process Instance
    plane_sweep_ncc(dmap_scores, left, right, width, height, patch_width, max_disparity);

    //** Inner-Loop End Boundary **
    clock_t end = clock();
    double runtime = double(end - start) * 1000.0f / CLOCKS_PER_SEC;
    std::cout<< "GPU Inner-Loop Execution Time = " << runtime << "ms" << std::endl;

    // Save Images
    dmap_scores.get_channel(0).save("dmap_scores.bmp");
    dmap_scores.get_channel(1).normalize(0,255).save("dmap_offsets.bmp");

    // CImg<int> mask = dmap_scores.get_channel(0).normalize(0,255).get_threshold(180);
    // mask.save("dmap_mask.bmp");

    // dmap_scores.get_channel(1).print();

}
