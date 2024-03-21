// Author: APD team, except where source was noted

#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<math.h>

#include<pthread.h>

#define CONTOUR_CONFIG_COUNT    16
#define FILENAME_MAX_SIZE       50
#define STEP                    8
#define SIGMA                   200
#define RESCALE_X               2048
#define RESCALE_Y               2048

#define CLAMP(v, min, max) if(v < min) { v = min; } else if(v > max) { v = max; }

// Creates a map between the binary configuration (e.g. 0110_2) and the corresponding pixels
// that need to be set on the output image. An array is used for this map since the keys are
// binary numbers in 0-15. Contour images are located in the './contours' directory.
ppm_image **init_contour_map() {
    ppm_image **map = (ppm_image **)malloc(CONTOUR_CONFIG_COUNT * sizeof(ppm_image *));
    if (!map) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    for (int i = 0; i < CONTOUR_CONFIG_COUNT; i++) {
        char filename[FILENAME_MAX_SIZE];
        sprintf(filename, "./contours/%d.ppm", i);
        map[i] = read_ppm(filename);
    }

    return map;
}

// Updates a particular section of an image with the corresponding contour pixels.
// Used to create the complete contour image.
void update_image(ppm_image *image, ppm_image *contour, int x, int y) {
    for (int i = 0; i < contour->x; i++) {
        for (int j = 0; j < contour->y; j++) {
            int contour_pixel_index = contour->x * i + j;
            int image_pixel_index = (x + i) * image->y + y + j;

            image->data[image_pixel_index].red = contour->data[contour_pixel_index].red;
            image->data[image_pixel_index].green = contour->data[contour_pixel_index].green;
            image->data[image_pixel_index].blue = contour->data[contour_pixel_index].blue;
        }
    }
}

typedef struct {
    unsigned char **grid; //the grid
    int start_x;  // starting row for this thread
    int end_x;  // ending row for this thread  
    int threadid;
    ppm_image *image;
    ppm_image **contour_map;
    int step_x;
    int step_y;
    unsigned char sigma;
    pthread_barrier_t *barrier;
    int total_threads; // number of total threads available
    ppm_image *dest_image;  // new_image where to scale the initial image
} thread_data;


void thread_sample(unsigned char **grid, int startRow, int endRow, ppm_image *image, int step_x, int step_y, unsigned char sigma, int thread_id, int totalthreads) {
    int p = image->x / step_x;
    int q = image->y / step_y;

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < q ; j++) { 
           ppm_pixel curr_pixel = image->data[i * step_x * image->y + j * step_y];
            unsigned char curr_color = (curr_pixel.red + curr_pixel.green + curr_pixel.blue) / 3;

            if (curr_color > sigma) {
                grid[i][j] = 0;
            } else {
                grid[i][j] = 1;
            }
        }
    }

     grid[p][q] = 0;

    // last sample points have no neighbors below / to the right, so we use pixels on the
    // last row / column of the input image for them
    for (int i = startRow; i < endRow; i++) {
        ppm_pixel curr_pixel = image->data[i * step_x * image->y + image->x - 1];

        unsigned char curr_color = (curr_pixel.red + curr_pixel.green + curr_pixel.blue) / 3;

        if (curr_color > sigma) {
            grid[i][q] = 0;
        } else {
            grid[i][q] = 1;
        }
    }

        double rowsPerThread = (double) q / totalthreads;

    // Calculate the start and end rows for this thread
    int newstartrow = thread_id * rowsPerThread;

    int newendrow = (int) (fmin((thread_id + 1) * rowsPerThread, q));


    for (int j = newstartrow; j < newendrow; j++) {
        ppm_pixel curr_pixel = image->data[(image->x - 1) * image->y + j * step_y];

        unsigned char curr_color = (curr_pixel.red + curr_pixel.green + curr_pixel.blue) / 3;

        if (curr_color > sigma) {
            grid[p][j] = 0;
        } else {
            grid[p][j] = 1;
        }
    }
}

void thread_march(unsigned char **grid, int startRow, int endRow, ppm_image *image, ppm_image **countour_map, int step_x, int step_y) {

    int q = image->y / step_y;

    for (int i = startRow; i < endRow  ; i++) {
        for (int j = 0; j < q   ; j++) {
            unsigned char k = 8 * grid[i][j] + 4 * grid[i][j + 1] + 2 * grid[i + 1][j + 1] + 1 * grid[i + 1][j];
            update_image(image, countour_map[k], i * step_x, j * step_y);
        }
    }
}


// Modified rescale_image function to handle a section of the image using bicubic interpolation
void thread_rescale_image_section(ppm_image *image, ppm_image *dest_image, int thread_id, int totalthreads) {

     //we only rescale downwards
    if (image->x <= RESCALE_X && image->y <= RESCALE_Y) {
        return;
    }

    double rowsPerThread = (double) RESCALE_X / totalthreads;

    // Calculate the start and end rows for this thread
    int startrow = thread_id * rowsPerThread;
    int endrow = (int) (fmin((thread_id + 1) * rowsPerThread, RESCALE_X));

    int dest_x = dest_image->x;
    int dest_y = dest_image->y;

    uint8_t sample[3]; // For bicubic interpolation

    for (int i = startrow; i < endrow; i++) {
        for (int j = 0; j < dest_y; j++) {
            float u = (float)i / (float)(dest_x - 1);
            float v = (float)j / (float)(dest_y - 1);

            sample_bicubic(image, u, v, sample);

            int index_dest = i * dest_y + j;
            dest_image->data[index_dest].red = sample[0];
            dest_image->data[index_dest].green = sample[1];
            dest_image->data[index_dest].blue = sample[2];
        }
    }
}



void* thread_combined_work(void* arg) {
    thread_data *data = (thread_data *)arg;

    int numRows = data->dest_image->x / data->step_x;
    double rowsPerThread = (double) numRows / data->total_threads;

    // Calculate the start and end rows for this thread
    data->start_x = data->threadid * rowsPerThread;
    data->end_x = (int) (fmin((data->threadid + 1) * rowsPerThread, numRows));

    // call rescale function
    thread_rescale_image_section(data->image, data->dest_image, data->threadid, data->total_threads);

    // wait for all threads
    pthread_barrier_wait(data->barrier);


    // call sample grid function
    thread_sample(data->grid, data->start_x, data->end_x, data->dest_image, data->step_x, data->step_y, data->sigma, data->threadid, data->total_threads);

    //wait for all threads
    pthread_barrier_wait(data->barrier);

    // call march function
    thread_march(data->grid, data->start_x, data->end_x, data->dest_image, data->contour_map, data->step_x, data->step_y);

    // wait for all threads
    pthread_barrier_wait(data->barrier);

    pthread_exit(NULL);
}

// Calls `free` method on the utilized resources.
void free_resources(ppm_image *image, ppm_image **contour_map, unsigned char **grid, int step_x) {
    for (int i = 0; i < CONTOUR_CONFIG_COUNT; i++) {
        free(contour_map[i]->data);
        free(contour_map[i]);
    }
    free(contour_map);

    for (int i = 0; i <= image->x / step_x; i++) {
        free(grid[i]);
    }
    free(grid);

    free(image->data);
    free(image);
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: ./tema1 <in_file> <out_file> <P>\n");
        return 1;
    }

    // See number of threads passed
    int NUM_THREADS = atoi(argv[3]);  // Convert string to integer
    if (NUM_THREADS <= 0) {
        printf("Error: Invalid number of threads.\n");
        exit(1);
    }

    pthread_t* threads = (pthread_t*) malloc(NUM_THREADS * sizeof(pthread_t));
    thread_data* data = (thread_data*) malloc(NUM_THREADS * sizeof(thread_data));

    pthread_barrier_t barrier;


    // initiate barrier
    if (pthread_barrier_init(&barrier, NULL, NUM_THREADS)) {
    // Handle error, for example:
    printf("Could not create the barrier\n");
    return -1;
    }

    // read initial image
    ppm_image *image = read_ppm(argv[1]);
    int step_x = STEP;
    int step_y = STEP;


    // 0. Initialize contour map
    ppm_image **contour_map = init_contour_map();

    // 1. Rescale the image

    //ppm_image *scaled_image = rescale_image(image);

    ppm_image *new_image = (ppm_image *)malloc(sizeof(ppm_image));

   // we only rescale downwards
    if (image->x <= RESCALE_X && image->y <= RESCALE_Y) {
        new_image = image;
    } else {
         // allocate memory for image
    if (!new_image) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }
    new_image->x = RESCALE_X;
    new_image->y = RESCALE_Y;

    new_image->data = (ppm_pixel*)malloc(new_image->x * new_image->y * sizeof(ppm_pixel));
    
    if (!new_image) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }
    }
    

        // allocate memory for grid
    unsigned char **grid = (unsigned char **)malloc((new_image->x / step_x + 1) * sizeof(unsigned char*));    //aveam new_image 3 locuri
    if (!grid) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    for (int i = 0; i <= new_image->x / step_x; i++) {
        grid[i] = (unsigned char *)malloc((new_image->y / step_y + 1) * sizeof(unsigned char));
        if (!grid[i]) {
            fprintf(stderr, "Unable to allocate memory\n");
            exit(1);
        }
    }

        // Spawn the threads for the combined work
    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].threadid = i;
        data[i].image = image;
        data[i].grid = grid;
        data[i].contour_map = contour_map;
        data[i].step_x = step_x;
        data[i].step_y = step_y;
        data[i].sigma = SIGMA;
        data[i].barrier = &barrier;
        data[i].total_threads = NUM_THREADS;
        data[i].dest_image = new_image;
        pthread_create(&threads[i], NULL, thread_combined_work, &data[i]);
    }

    // Join the threads after all work is done
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 2. Sample the grid
    //unsigned char **grid = sample_grid(scaled_image, step_x, step_y, SIGMA);

    // 3. March the squares
    //march(scaled_image, grid, contour_map, step_x, step_y);

    // 4. Write output
    write_ppm(new_image, argv[2]);

    free_resources(new_image, contour_map, grid, step_x);

    // destroy barrier
    pthread_barrier_destroy(&barrier);
    
    free(threads);
    free(data);

    return 0;
}