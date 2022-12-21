// source /opt/rocm/activate.sh
// make gpu_impl
// make clean

// References:
// http://programmingcomputervision.com/downloads/ProgrammingComputerVision_CCdraft.pdf
// Section 5.4 - Stereo Images

// TODO Items:
// - [X] Fix image size not divisible by BLOCK_SIZE, resolved for now by just selecting divisable BLOCK_SIZE (GCF)
// - [X] Parameterize the Vertical/Horizontal modes of operation, [skip for now, all image pairs left-right]
// - [X] Add performance timing to both versions
// - [ ] Implement the Uniform, Gradient and Grayscale Filters in HIP
// - [ ] Add command-line arguments/parameters, no more hard-coding everything
// - [ ] Remove the border artifact, need to fill/roll/mirror the edge cases
// - [ ] Get the Threshold Mask working for removing poorly-correlated regions from the depth map
// - [ ] Try the suggestion in the text to swap left and right pairs and keep best parts, removes occlusions
// - [ ] Get a live / online stream ingestion working
// - [ ] Despite having the GPU implementation, try adding dynamic programming / memoization
// - [ ] Do a 2D search reduction to find the most centered image offsets, might help make better maps
// - [ ] Add camera intrinsics/calibration parameters
// - [ ] Camera parameters estimation
// - [ ] Add CImgList for Depthmaps capture, learn how to do slicing for argmax behavior
// - [ ] Move CImg library to a better position
// - [ ] Get VSCode Intellisence to imprint correctly, I'm coding blind and having to read docs too much

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "CImg.h"
using namespace cimg_library;

#include <hip/hip_runtime.h>

// For 450x375 (Middlebury Cones): {5, 15, 25, 75} GCD=75
// For 1280x1080 (Webcam Frames):  {5, ..., 15, 20, 24, 30, 40, 60, 120} GCD=120
// The largest common shared factor is 15
#define BLOCK_SIZE 15 


// Forward Declarations
void uniform_filter(CImg<float>&, const CImg<float>&, int, int, int);

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


__global__ void PlaneSweep_NCC(float *dmapData, const float *leftData, const float *rightData, int width, int height, const int patchWidth, const int maxDisparity){
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // float3 sum;
    // sum.x = leftData[ (y) * width + x];
    // sum.y = rightData[(y) * width + x];
    // sum.z = (float) x;
    
    for(int d=0; d<maxDisparity; d++){

        int w = (patchWidth/2);
        if(x>(w) && x<(width-w) && y>(w) && y<(height-w)){
            float sum_N = 0.0f;
            float sum_D1 = 0.0f;
            float sum_D2 = 0.0f;

            for(int u= -w; u <= w; u++){
                for(int v= -w; v <= w; v++){
                    float left_term  = leftData[ (y+v) * width + x+u + d];
                    float right_term = rightData[(y+v) * width + x+u    ];

                    sum_N  += ( left_term * right_term );
                    sum_D1 += ( left_term * left_term );
                    sum_D2 += ( right_term * right_term );

                }
            }

            float score = ( ( sum_N/(std::sqrt(sum_D1 * sum_D2)) ) + 1.0f ) * (255.0f / 2.0f);

            if(score > dmapData[(y) * width + x]){
                dmapData[(y           ) * width + x] = score;
                dmapData[(y + height  ) * width + x] = (float)d;
                dmapData[(y + height*2) * width + x] = (float)x; // For debugging, might as well put something here
            }

        } else {
            dmapData[(y           ) * width + x] = 0.0f;
            dmapData[(y + height  ) * width + x] = 0.0f;
            dmapData[(y + height*2) * width + x] = 0.0f;
        }
    }
    
    // dmapData[(y           ) * width + x] = sum.x;
    // dmapData[(y + height  ) * width + x] = sum.y;
    // dmapData[(y + height*2) * width + x] = sum.z;
    __syncthreads();
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
    CImg<float> dmap_scores(width, height, 1, 3, 0);

    // Debugging
    left_rgb.save("debug1.bmp");
    right_rgb.save("debug2.bmp");
    // left_gray.save("debug3.bmp");
    // left_gradient.save("debug3.bmp");
    // left_mean.save("debug4.bmp");
    // std::cout << left_mean.print() << std::endl;
    // std::cout << left.print() << std::endl;
    left.get_normalize(0,255).save("debug3.bmp");
    right.get_normalize(0,255).save("debug4.bmp");

    // Performance Measurement Items
    hipEvent_t start, stop;
    float elapsed_mSecs;
    hipEventCreate(&start);
    hipEventCreate(&stop);

    //** Inner-Loop Starting Point **
    // When this is converted to a continuous/streamed process, 
    // this is the boundary for where the speed-up for CPU-vs-GPU is best measured.
    hipEventRecord(start, 0);

    // Input Image Handles
    size_t imageSize = width * height * sizeof(float); // 1-Channel Image
    float *leftData;
    hipMalloc(&leftData, imageSize);
    hipMemcpy(leftData, left.data(), imageSize, hipMemcpyHostToDevice);

    float *rightData;
    hipMalloc(&rightData, imageSize);
    hipMemcpy(rightData, right.data(), imageSize, hipMemcpyHostToDevice);

    // Output Image Handles
    imageSize = width * height * sizeof(float) * 3; // 3-Channel Image
    float *dmapData;
    hipMalloc(&dmapData, imageSize);

    // Kernel Configuration
    dim3 dimBlock (BLOCK_SIZE, BLOCK_SIZE, 1);
    dim3 dimGrid ((width + (BLOCK_SIZE-1))/BLOCK_SIZE, (height + (BLOCK_SIZE-1))/BLOCK_SIZE, 1);
    std::cout << BLOCK_SIZE << " threads per block, " << dimGrid.x << "x" << dimGrid.y << " blocks" << std::endl;

    // GPU Process Instances
    PlaneSweep_NCC<<<dimGrid, dimBlock>>>(dmapData, leftData, rightData, width, height, patch_width, max_disparity);
    
    // Copy Data back to host
    hipMemcpy(dmap_scores.data(), dmapData, imageSize, hipMemcpyDeviceToHost);

    //** Inner-Loop End Boundary **
    hipEventRecord(stop, 0);
    hipEventSynchronize(stop);
    hipEventElapsedTime(&elapsed_mSecs, start, stop);
    std::cout << "GPU Inner-Loop Execution Time = " << elapsed_mSecs << "ms" << std::endl;

    // Save Images
    dmap_scores.get_channel(0).get_normalize(0,255).save("dmap_scores.bmp");
    dmap_scores.get_channel(1).get_normalize(0,255).save("dmap_offsets.bmp"); // .normalize(0,255).save("dmap_offsets.png");
    dmap_scores.get_channel(2).save("debug6.bmp");

    // CImg<int> mask = dmap_scores.get_channel(0).normalize(0,255).get_threshold(180);
    // mask.save("dmap_mask.bmp");

    // dmap_scores.get_channel(1).print();

    // Cleanup
    hipFree(leftData);
    hipFree(rightData);
    hipFree(dmapData);

}