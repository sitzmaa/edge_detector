#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

#define LAPLACIAN_THREADS 9    //change the number of threads as you run your concurrency experiment

/* Laplacian filter is 3 by 3 */
#define FILTER_WIDTH 3       
#define FILTER_HEIGHT 3      

#define RGB_COMPONENT_COLOR 255

#define MICRO_SECOND 1000000 // Define number of microseconds in a second

// Mutex Locks for write to result and update total_elapsed_time respectively
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t time_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
      unsigned char r, g, b;
} PPMPixel;

struct parameter {
    PPMPixel *image;         //original image pixel data
    PPMPixel *result;        //filtered image pixel data
    unsigned long int w;     //width of image
    unsigned long int h;     //height of image
    unsigned long int start; //starting point of work
    unsigned long int size;  //equal share of work (almost equal if odd)
};


struct file_name_args {
    char *input_file_name;      //e.g., file1.ppm
    char output_file_name[20];  //will take the form laplaciani.ppm, e.g., laplacian1.ppm
};

// helper function prototypes
void initialize_args(struct file_name_args *args, char* file_name, int i);
int truncate_value(int value, int max);

/*The total_elapsed_time is the total time taken by all threads 
to compute the edge detection of all input images .
*/
double total_elapsed_time = 0; 


/*This is the thread function. It will compute the new values for the region of image specified in params (start to start+size) using convolution.
    For each pixel in the input image, the filter is conceptually placed on top ofthe image with its origin lying on that pixel.
    The  values  of  each  input  image  pixel  under  the  mask  are  multiplied  by the corresponding filter values.
    Truncate values smaller than zero to zero and larger than 255 to 255.
    The results are summed together to yield a single output value that is placed in the output image at the location of the pixel being processed on the input.
 
 */
void *compute_laplacian_threadfn(void *params)
{
    // cast params
    struct parameter *parameters = (struct parameter *)params;
    PPMPixel* result = parameters->result;
    PPMPixel* image = parameters->image;
    int laplacian[FILTER_WIDTH][FILTER_HEIGHT] =
    {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };
    
    // Read params from struct
    unsigned long w = parameters->w;
    unsigned long h = parameters->h;
    unsigned long size = parameters->size;
    unsigned long start = parameters->start;
    int red, green, blue;
    for (int i = 0; i < size; i++) {
        int iteratorImageWidth = (i+start)%w; // Width position of current pixel
        int long iteratorImageHeight = ((i+start)/w); // Height position of current pixel
        for (int j = 0; j < FILTER_WIDTH; j++) {
            for(int k = 0; k < FILTER_HEIGHT; k++) {
                // Get the width and heightof pixel in relation to current pixel and current position in filter
                int x_coordinate = (iteratorImageWidth - (FILTER_WIDTH / 2) + j + w)%w;
                int y_coordinate = (iteratorImageHeight - (FILTER_HEIGHT / 2) + k + h)%h;
                // Apply filter to respective pixel for RGB
                red+= image[(y_coordinate * w) + x_coordinate].r * laplacian[k][j];
                green+= image[(y_coordinate * w) + x_coordinate].g * laplacian[k][j];
                blue+= image[(y_coordinate * w) + x_coordinate].b * laplacian[k][j];

            }
        }
        // truncate values to 0 <= value <= 255
        red = truncate_value(red, RGB_COMPONENT_COLOR);
        green = truncate_value(green, RGB_COMPONENT_COLOR);
        blue = truncate_value(blue, RGB_COMPONENT_COLOR);
        result[iteratorImageHeight * w + iteratorImageWidth].r =red;
        result[iteratorImageHeight * w + iteratorImageWidth].g = green;
        result[iteratorImageHeight * w + iteratorImageWidth].b = blue;
        red = 0;
        green = 0;
        blue = 0;
    }
      
      
        
    return NULL;
}

/* Apply the Laplacian filter to an image using threads.
 Each thread shall do an equal share of the work, i.e. work=height/number of threads. If the size is not even, the last thread shall take the rest of the work.
 Compute the elapsed time and store it in *elapsedTime (Read about gettimeofday).
 Return: result (filtered image)
 */
PPMPixel *apply_filters(PPMPixel *image, unsigned long w, unsigned long h, double *elapsedTime) {

    struct timeval time;
    if (gettimeofday(&time, NULL) != 0) {
        perror("get time error\n");
        exit(-1);
    }
    double micro = time.tv_usec;
    double start_time = (time.tv_sec+(micro/MICRO_SECOND));

    PPMPixel *result;
    // Allocate space for result
    result = calloc((h*w), sizeof(PPMPixel));
    if (result == NULL) {
        perror("calloc error");
        exit(-1);
    }
    // Copy image over to result
    memcpy(result, image, (h*w)*sizeof(PPMPixel));
    
    // Set up parameters
    struct parameter params[LAPLACIAN_THREADS];
    unsigned long int size = (w*h)/LAPLACIAN_THREADS;
    pthread_t filter_threads[LAPLACIAN_THREADS];
    // Populate params and initialize threads
    for (int i = 0; i < LAPLACIAN_THREADS; i++) {
        params[i].image = image;
        params[i].result = result;
        params[i].h = h;
        params[i].w = w;
        params[i].size = size;
        params[i].start = size*i;
        if(pthread_create(&filter_threads[i], NULL, compute_laplacian_threadfn, (void*)&params[i]) != 0){
            perror("unable to create thread"); 
            exit(-1);
        }
    }
    // Join threads
    for(int i = 0; i < LAPLACIAN_THREADS; i++) {
        pthread_join(filter_threads[i], NULL);
    }
    if (gettimeofday(&time, NULL) != 0) {
        perror("get time error\n");
        exit(-1);
    }
    micro = time.tv_usec;
    double end_time = (time.tv_sec+(micro/MICRO_SECOND));
    *elapsedTime = end_time-start_time;
    return result;
}

/*Create a new P6 file to save the filtered image in. Write the header block
 e.g. P6
      Width Height
      Max color value
 then write the image data.
 The name of the new file shall be "filename" (the second argument).
 */
void write_image(PPMPixel *image, char *filename, unsigned long int width, unsigned long int height)
{
    // create output file with appropriate naem
    FILE* writer;
    if((writer = fopen(filename, "w+")) == NULL) {
        perror("could not open file\n");
        exit(-1);
    }
    char string[50];
    // create file header
    sprintf(string, "P6\n%lu %lu\n%d\n", width, height, RGB_COMPONENT_COLOR);
    if(fwrite(string, 1, strlen(string), writer) < strlen(string)) {
        perror("Write Error");
        exit(-1);
    } 
    // write image
    if(fwrite(image, width*height, sizeof(PPMPixel), writer) < sizeof(PPMPixel)){
        perror("Write Error");
        exit(-1);
    }
    free(image);
    fclose(writer);
    
}



/* Open the filename image for reading, and parse it.
    Example of a ppm header:    //http://netpbm.sourceforge.net/doc/ppm.html
    P6                  -- image format
    # comment           -- comment lines begin with
    ## another comment  -- any number of comment lines
    200 300             -- image width & height 
    255                 -- max color value
 
 Check if the image format is P6. If not, print invalid format error message.
 If there are comments in the file, skip them. You may assume that comments exist only in the header block.
 Read the image size information and store them in width and height.
 Check the rgb component, if not 255, display error message.
 Return: pointer to PPMPixel that has the pixel data of the input image (filename).The pixel data is stored in scanline order from left to right (up to bottom) in 3-byte chunks (r g b values for each pixel) encoded as binary numbers.
 */
PPMPixel *read_image(const char *filename, unsigned long int *width, unsigned long int *height)
{   
    // open file and set buffer
    FILE* image;
    int color_max;
    char* buff = malloc(sizeof(char)*4);
    if (buff == NULL) {
        perror("Malloc Error\n");
        exit(-1);
    }
    if ((image = fopen(filename, "r")) == NULL) {
        perror("File could not be opened");
        exit(-1);
    }
    // Determine that file is of proper type P6
    fscanf(image, "%2s\n", buff);
    if (strcmp(buff, "P6") != 0) {
        perror("Incorrect File Format");
        exit(-1);
    }
    // Skip comments
    while (fgetc(image) == '#') {
        while (fgetc(image) != '\n');
    }
    fseek(image, ftell(image)-1, SEEK_SET);
    // Scan for image data
    fscanf(image, "%lu %lu\n%d", width, height, &color_max);
    // Compare file color maximum to out color maximum
    if (color_max != RGB_COMPONENT_COLOR) {
        perror("Invlaid color maximum");
        exit(-1);
    }

    PPMPixel *img;
    // Determine the number of pixels in the bitstring
    long start = ftell(image);
    fseek(image, start+1, SEEK_SET);
    long length = (*height * *width);
    // Allocate our string of pixels
    img = calloc(length, sizeof(PPMPixel));
    if (img == NULL) {
        perror("Calloc Error");
        exit(-1);
    }
    // read pixels into struct
    if (fread(img, sizeof(PPMPixel), length, image) < length) {
        perror("Read Error");
        exit(-1);
    }
    free(buff);
    fclose(image);
    return img;
}

/* The thread function that manages an image file. 
 Read an image file that is passed as an argument at runtime. 
 Apply the Laplacian filter. 
 Update the value of total_elapsed_time.
 Save the result image in a file called laplaciani.ppm, where i is the image file order in the passed arguments.
 Example: the result image of the file passed third during the input shall be called "laplacian3.ppm".

*/
void *manage_image_file(void *args){
    // cast args
    struct file_name_args *file_args = (struct file_name_args *)args;
    char* output = file_args->output_file_name;
    // initialize widht and height
    unsigned long width;
    unsigned long height;
    double elapsed_time = 0;
    // read image
    PPMPixel* image = read_image(file_args->input_file_name, &width, &height);
    // filter image
    PPMPixel* result = apply_filters(image, width, height, &elapsed_time);
    // write image
    write_image(result, file_args->output_file_name, width, height);
    free(image);
    pthread_mutex_lock(&time_lock);
    // Update total time
    total_elapsed_time += elapsed_time;
    pthread_mutex_unlock(&time_lock);
    return NULL;
}
/*The driver of the program. Check for the correct number of arguments. If wrong print the message: "Usage ./a.out filename[s]"
  It shall accept n filenames as arguments, separated by whitespace, e.g., ./a.out file1.ppm file2.ppm    file3.ppm
  It will create a thread for each input file to manage.  
  It will print the total elapsed time in .4 precision seconds(e.g., 0.1234 s). 
 */
int main(int argc, char *argv[])
{
     if (argc<2) {
        printf("No images to read. \nUsage: ./edge_detector filename[s]\n");
        exit(-1);
    }

    // Initialize threads for each image
    pthread_t file_threads[argc];
    struct file_name_args file_args[argc];
    for (int i = 0; i < argc-1; i++) {
        initialize_args(&file_args[i], argv[i+1], i+1);
        if(pthread_create(&file_threads[i], NULL, manage_image_file, (void*)&file_args[i]) != 0) {
            perror("unable to create thread");
            exit(-1);
        }
    }
    for(int i = 0; i < argc-1; i++) {
        pthread_join(file_threads[i], NULL);
    }
    printf("Total time: %f\n", total_elapsed_time);
    return 0;
}

// Helper functions
// Create appropriate output filename and populate params for threads
void initialize_args(struct file_name_args *args, char* file_name, int i) {
        args->input_file_name = file_name;
        char output[20] = "";
        sprintf(output, "laplacian%d.ppm", i);
        strcpy(args->output_file_name, output);
}

// Truncate values to between 0 and max color value
// If outside of bounds, return the bound exceded
int truncate_value(int value, int max) {
    if (value > max) {
        return max;
    } else if (value < 0) {
        return 0;
    }
    return value;
}
