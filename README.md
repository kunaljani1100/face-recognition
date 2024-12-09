# Face Recognition

This project consists of making a comparison between two popular methods of face recognition which are the Principal Component Analysis (PCA) and the Discrete Cosine Transform (DCT) algorithm. 

Both the algorithms have been parallelized using the Message Passing Interface (MPI), which is a distributed memory system where jobs are distributed between different nodes resulting in a significant reduction of the execution time to recognize a face when large datasets are distributed across multiple datasets.

## Steps to run the code:

Use the gcc compiler to run the code.

### gaussian_noise.c

Compilation: gcc gaussian_noise.c
Run: ./gaussian_noise.c 5

This command adds gaussian noise to an image with the file named new_image5_greyscaled.ppm.

### salt_and_pepper_noise.c

Compilation: gcc salt_and_pepper_noise.c
Run: ./gsalt_and_pepper_noise.c 5

This command adds gaussian noise to an image with the file named new_image5_greyscaled.ppm.

### greyscaling.c

Compilation: gcc greyscaling.c
Run: ./greyscaling 5

This command converts an image named image5.ppm from an Red, Green, Blue (RGB) format to a greyscale format.

### serial_code_pca.c and serial_code_dct.c

Compilation: 
gcc serial_code_pca.c
gcc serial_code_dct.c

Run:
./serial_code 100
./parallel_code 100 

The command performs face recognition on an image of a dataset of 100 records.

### parallel_code_pca.c and parallel_code_dct.c

Compilation: 
mpicc serial_code_pca.c
mpicc serial_code_dct.c

Run:
./serial_code 100 5
./parallel_code 100 5

The command performs face recognition on an image of a dataset of 100 records and spawns 5 processesors to work in parallel.
