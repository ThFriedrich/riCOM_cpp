#!/usr/bin/env python3
"""
Demo script for running riCOM from command line interface through python.
This script demonstrates how to execute the riCOM executable with various arguments.
"""

import subprocess
import os
import matplotlib.pyplot as plt
import numpy as np

# Path to the riCOM executable (adjust as needed)
RICOM_EXECUTABLE = "./build/RICOM"

def demo_timepix_camera():
    """
    Demo: Process data from Timepix camera with dwell time setting.
    Available arguments are:

    Input/Connection:
      -filename <path>        : Read from .mib or .t3p file
    
    Dimensions:
      -nx <number>           : Width of scan image
      -ny <number>           : Height of scan image
      -cam_nx <number>       : Width of camera sensor
      -cam_ny <number>       : Height of camera sensor
    
    Processing:
      -skipr <number>        : Skip rows (skip n frames at the end of each line for flyback correction)
      -skipi <number>        : Skip images (skip n frames at the end of each image for flyback correction)
      -k <number>            : Kernel size for riCOM
      -r <float>             : CBED rotation angle in degrees
      -offset <x> <y>        : CBED center offset (two values)
      -update_offset <0|1>   : Auto-update offset during processing
      -radius <inner> <outer>: Virtual STEM detector radii (enables vSTEM)
      -f <low> <high>        : Kernel filter frequency range
      -threads <number>      : Number of processing threads
      -queue_size <number>   : Size of processing queue
      -rep <number>          : Number of repetitions
    
    Camera-specific:
      -depth <number>        : Bit depth per pixel (Merlin camera)
      -dwell_time <float>    : Dwell time in seconds (Timepix camera)
    
    Output:
      -save_img_path <path>  : Save reconstruction as image file
      -save_data_path <path> : Save reconstruction as numpy array (.npy)
    
    Visualization:
      -redraw_interval <ms>  : Redraw interval in milliseconds (enables SDL display)
      -plot_e_field <0|1>    : Plot electric field magnitude
    """

    args = [
        RICOM_EXECUTABLE,
        # "-filename", "/home/thomas/Documents/Git/4D-STEM-Data/data/cryst_1   mono_120_CL 115  Mag_2.55Mx  defocus_-198.67 CA_21.47 Tue_Jul_16_11_50_09_2024_STEM_300kV_2048x2048_4_us_1_scans.t3p",
        "-filename", "/home/thomas/Documents/Git/4D-STEM-Data/data/cryst_5.5   mono_140_CL 115  Mag_2.55Mx  defocus_0 CA_24.70 Tue_Oct__1_17_48_58_2024_STEM_300kV_2048x2048_4_us_1_scans.t3p",
        
        # Timepix-specific setting
        "-dwell_time", "4000",  # Dwell time in nanoseconds (Timepix camera)
        
        # Scan dimensions
        "-nx", "2049",
        "-ny", "2048",
        
        # Camera dimensions
        "-cam_nx", "256",
        "-cam_ny", "256",
        
        # Kernel
        "-k", "12",   # Kernel half-size for riCOM
        "-r", "260",  # CBED rotation angle in degrees
        "-offset", "135.12", "116.694",  # CBED center (x, y)

        # Processing
        "-threads", "1",
        "-redraw_interval", "200",  # Redraw every 200 ms
        
        # Output
        "-save_data_path", "timepix_output.npy",
    ]
    
    print("Running riCOM with Timepix camera data...")
    print(" ".join(args))
    subprocess.run(args, capture_output=False, text=True)


if __name__ == "__main__":
    
    # Check if executable exists
    if not os.path.exists(RICOM_EXECUTABLE):
        print(f"Warning: Executable not found at {RICOM_EXECUTABLE}")
        print("Please adjust RICOM_EXECUTABLE path in this script.")
        print()

    # Run reconstruction, and get COM_AVG from the output, which can be used for correction in the second run if needed
    demo_timepix_camera()

    # When given a data path, the script will save the reconstruction as a numpy array. We can load and visualize it here.
    img_data = np.load("timepix_output.npy")
    fig, ax = plt.subplots(1, 1, figsize=(8, 8))
    im = ax.imshow(img_data, cmap='viridis')

    ax.axis('off')
    plt.colorbar(im, ax=ax, shrink=0.8)
    plt.tight_layout()
    plt.show()
    
