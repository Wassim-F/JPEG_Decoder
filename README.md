# JPEG Decoder in C

A custom JPEG decoder implemented from scratch in C as part of a one-month project.  
The program converts `.jpg` images into `.ppm` format for visualization.  

## Features  

- Step-by-step development:  
  - Grayscale decoding (8x8 blocks)  
  - Support for larger and non-standard dimensions
  - Color image decoding
  - Support for sub-sampling 

- Output images are generated in `.ppm` format and stored in the `images/` folder  

## Tools & Technologies  

- C (core implementation)  
- Git (version control)  
- GDB (debugging)
- Valgrind  

## Build & Run  

- make
- ./jpeg2ppm images/name.jpg
- open images/name.ppm   # to visualize the decoded image
