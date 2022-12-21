# GPU_Stereopsis
A project for using CUDA/HIP to produce disparity maps from pairs of stereo images, for CS691

# Description and Scope

The purpose of this project is to learn how to implement fast stereo pair matching, toward implementing a similar pipeline on a small flying robot using something like the Khadas VIM3 or a similar Single-Board Computer. While various commercial RGBD cameras exist, such as the Intel RealSense D4xx Series, there is a significant case for trying to roll our own custom depth-mapping system. The reasons for this are as follows:

- Our UAV is operating in an outdoors environment with harsh physical issues including ambient water and dust. Many commercial sensors are only made for indoor use, and may typically be inappropiate without adding extra protection.
- We need a very light-weight system, hence repurposing common USB RGB web cameras is common, and many commerical sensors are already very heavy without the extra protective measures as mentioned above.
-  The concept of operations for this stereo-matching pipeline is that the aircraft motion can be used to create a synthetic stereo baseline, i.e. Structure-from-Motion, to map the terrain. Given our VTOL can fly at 20 m/s at altitudes of 100 meters or more, the baseline between sequential camera frames is more significant than what a pair of wing-mounted stereo cameras could produce. Thus, while two cameras could be used, a single down-looking camera may be sufficient.
- The vision system being designed here is for allowing the aircraft to detect valid and safe landing spots in unstructured terrain. Thus, the system needs to update a depth map at or above 20 Hz if possible, such that the vehicle control loop can reliabily use this data during the landing process.

With the nature of this problem shown above, the following is the sequence of data processing tasks that need to be crafted, from Camera to Final Outputs:

1. Use V4L2 (Video4Linux) to grab frames from the downfacing RGB camera.
2. Perform preliminary image processing on the current frame, such as equalization/normalization.
3. From prior camera calibration, rectify the image.
4. Register the metadata for the frame: GPS Position, Aircraft Pose (quaternion in world-frame), timestamp, etc.
5. Buffer the frame into the stereo-matching pipeline.
6. The pipeline generates or updates a data structure which consists of a disparity (depth) map and a correlation score per pixel in the input frame.
7. The disparity map is masked to only allow regions which a score above a given threshold; any regions that are too poorly-correlated are too ambiguous to permit consideration in the rest of the algorithm
8. The depth map is scaled and corrected for the camera intrinsic parameters, such that pixel values are linearly mapped to actual distances. Given the aircraft pose and position estimates, this could be transformed to a world-frame reference.
9. The product at this stage could be a candidate for submittion to the ROS RTAB-MAP package, which updates a 3D point cloud map and its associated 2D occupancy grid.
10. The topology of the map can be searched for valid landing sites, which would typically be smooth and level for our VTOL.
11. The list of possible landing site candidates is scored, and a best candidate is selected and updated or switched as the flight progresses. The candidate position is localized in the world frame, so landing should be rudimentray.


Note that while it is possible that the above Strucuture-from-Motion scheme might already exist within the ROS ecosystem, the primary purpose of this project is to learn the principals behind building a GPU-accelerated matching pipeline. Therefore, the main focus of this project will be on items #6 and #7.

# Project Checklist:

Core Set:
- [X] [Skipped] Install Ubuntu 20.04 (Skipping as this is a project in itself)
- [X] [Skipped] Install ROCm / HIP Tool Chain Locally (Dependent on Ubuntu 20)
- [X] [DONE] Prototype the Stereo Matching Pipeline in Python (Previous project, used as reference)
- [X] [DONE] Pull the Python pipeline over to a C++ implementation, offline
- [X] [Skipped] Online live video stream ingestion (Moved to Extended Set)
- [X] [DONE] Implement the stereo matching kernel in GPU code - HIP or CUDA

Extended Set (Items from the larger pipeline in anticipation of making this a real-time program):
- [ ] Utility Functions: Implement Uniform Filter, Greyscale, and Gradient in GPU Code
- [ ] Online live video stream ingestion via V4L2
- [ ] Live Display using OpenCV and/or V4L2
- [ ] Use a Docker Image to instantiate a portable runtime environment
- [ ] Replace HIP with CUDA for the GPU Implementation

