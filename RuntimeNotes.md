# How to Compile and Run

1. Install the CImg library from the [GitHub Cimg Repo](https://github.com/GreycLab/CImg).
2. Clone the project with `git clone https://github.com/StephenCarlson/GPU_Stereopsis.git`.
3. Edit the `Makefile` such that the `CFLAGS` match the location(s) of your CImg library.
4. The CPU Implementation should compile with `make` or `make cpu_impl`.
5. For the GPU Implementation, first source the ROCm environment with `source /opt/rocm/activate.sh`
6. Then, compile with `make gpu_impl`. Note that you may need to call `make clean` first.
7. Both versions are executed as `./cpu_impl` and `./gpu_impl` respectively.



## Typical Runtime Console Outputs

### CPU Implementation
```plaintext
[stevecarlson@gpuh-node1 GPU_Stereopsis]$ ./cpu_impl 
GPU Stereopsis
1080,1920,6220800
GPU Inner-Loop Execution Time = 67319.6ms
```

### GPU Implementation

`#define BLOCK_SIZE 15 `:
```plaintext
[stevecarlson@gpuh-node1 GPU_Stereopsis]$ ./gpu_impl 
GPU Stereopsis
1080,1920,6220800
15x15=225 threads per block, 72x128 blocks
GPU Inner-Loop Execution Time = 77.0384ms
```

`#define BLOCK_SIZE 30 `:
```plaintext
...
30x30=900 threads per block, 36x64 blocks
GPU Inner-Loop Execution Time = 67.01ms
```

`#define BLOCK_SIZE 5 `:
```plaintext
...
5x5=25 threads per block, 216x384 blocks
GPU Inner-Loop Execution Time = 123.157ms
```


## Output File Descriptions

Both the CPU and GPU versions of the project emit the same pair of images: `dmap_offsets.bmp` and `dmap_scores.bmp`. 
- The `_scores` file shows the correlation coefficient for each pixel, where a value of 0 (black) is no correlation, and 255 (white) is a perfect correlation match for the patch around that pixel. 
- The `_offset` file is the disparity (or depth) map. A lighter-valued pixel is "closer", and a darker value is "further" or infinite distance. Note that this is a non-linear relationship and much be corrected for the camera's intrinsic optical characteristics, so this a "raw" disparity, not distance.

With both the disparity estimation and the associated quality for each disparity pixel, we can proceed to mask the depthmap for only those regions that exceed a given correlation quality score.



# Description of the UNR CC Runtime Environment:

## [stevecarlson@gpuh-node1 ~]$ `lspci | grep VGA`
```plaintext
03:00.0 VGA compatible controller: NVIDIA Corporation GP104 [GeForce GTX 1080] (rev a1)
04:00.0 VGA compatible controller: NVIDIA Corporation GP104 [GeForce GTX 1080] (rev a1)
08:00.0 VGA compatible controller: ASPEED Technology, Inc. ASPEED Graphics Family (rev 30)
82:00.0 VGA compatible controller: NVIDIA Corporation GP104 [GeForce GTX 1080] (rev a1)
83:00.0 VGA compatible controller: NVIDIA Corporation GP104 [GeForce GTX 1080] (rev a1)
```

## [stevecarlson@gpuh-node1 ~]$ `lscpu`
```plaintext
Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              40
On-line CPU(s) list: 0-39
Thread(s) per core:  2
Core(s) per socket:  10
Socket(s):           2
NUMA node(s):        2
Vendor ID:           GenuineIntel
CPU family:          6
Model:               79
Model name:          Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
Stepping:            1
CPU MHz:             3100.000
CPU max MHz:         3100.0000
CPU min MHz:         1200.0000
BogoMIPS:            4399.63
Virtualization:      VT-x
L1d cache:           32K
L1i cache:           32K
L2 cache:            256K
L3 cache:            25600K
NUMA node0 CPU(s):   0-9,20-29
NUMA node1 CPU(s):   10-19,30-39
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb cat_l3 cdp_l3 invpcid_single pti intel_ppin ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm cqm rdt_a rdseed adx smap intel_pt xsaveopt cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local dtherm ida arat pln pts md_clear flush_l1d
```
