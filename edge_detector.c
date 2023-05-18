#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

#define LAPLACIAN_THREADS 4     //change the number of threads as you run your concurrency experiment

/* Laplacian filter is 3 by 3 */
#define FILTER_WIDTH 3       
#define FILTER_HEIGHT 3      

#define RGB_COMPONENT_COLOR 255

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

// helper function
void initialize_args(struct file_name_args args, char* file_name, int i);

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
    
    int laplacian[FILTER_WIDTH][FILTER_HEIGHT] =
    {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    int red, green, blue;
     
      
      
        
    return NULL;
}

/* Apply the Laplacian filter to an image using threads.
 Each thread shall do an equal share of the work, i.e. work=height/number of threads. If the size is not even, the last thread shall take the rest of the work.
 Compute the elapsed time and store it in *elapsedTime (Read about gettimeofday).
 Return: result (filtered image)
 */
PPMPixel *apply_filters(PPMPixel *image, unsigned long w, unsigned long h, double *elapsedTime) {

    PPMPixel *result;
   


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
    FILE* writer = fopen("laplacian1.ppm", "w+");
    char string[50];
    sprintf(string, "P6\n%lu %lu\n225", width, height);
    fwrite(string, 1, strlen(string), writer);
    
    fwrite(image, width*height, sizeof(PPMPixel), writer);
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
    if ((image = fopen(filename, "r")) == NULL) {
        perror("File could not be opened");
        exit(-1);
    }
    // Determine that file is of proper type P6
    fscanf(image, "%2s\n", buff);
    printf("%s\n", buff);
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
    if (color_max > RGB_COMPONENT_COLOR) {
        perror("Invlaid color maximum");
        exit(-1);
    }
    printf("%lu %lu %d\n", *width, *height, color_max);

    PPMPixel *img;
    // Determine the number of pixels in the bitstring
    long start = ftell(image);
    fseek(image, 0 ,SEEK_END);
    long end = ftell(image);
    fseek(image, start, SEEK_SET);
    long length = (end - start) / sizeof(PPMPixel);
    // Allocate our string of pixels
    img = calloc(length, sizeof(PPMPixel));
    // read pixels into struct
    fread(img, sizeof(PPMPixel), length, image);
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
    struct file_name_args *file_args = (struct file_name_args *)args;
    printf("input: %s\n output: %s\n", file_args->input_file_name, file_args->output_file_name);
    
}
/*The driver of the program. Check for the correct number of arguments. If wrong print the message: "Usage ./a.out filename[s]"
  It shall accept n filenames as arguments, separated by whitespace, e.g., ./a.out file1.ppm file2.ppm    file3.ppm
  It will create a thread for each input file to manage.  
  It will print the total elapsed time in .4 precision seconds(e.g., 0.1234 s). 
 */
int main(int argc, char *argv[])
{
     if (argc<2) {
        perror("No images to read. \nUsage: ./edge_detector filename[s]");
        exit(-1);
    }
    /* test code
    unsigned long int width;
    unsigned long int height;
    PPMPixel* image = read_image(argv[1], &width,&height);
    write_image(image, "", width, height); */

    pthread_t file_threads[argc];
    char iteration_char;
    struct file_name_args file_args[argc];
    for (int i = 0; i < argc-1; i++) {

        initialize_args(file_args[i], argv[i+1], i);
        if(pthread_create(&file_threads[i], NULL, manage_image_file, (void*)&argv[i+1]) != 0)
            perror("unable to create thread");
    }
    for(int i = 0; i < argc-1; i++) {
        pthread_join(file_threads[i], NULL);
    }
    
    return 0;
}

// Helper function
void initialize_args(struct file_name_args args, char* file_name, int i) {
        char output[20];
        char iteration_char = i;
        args.input_file_name = file_name;
        strcpy(output, "laplacian");
        strcat(output, &iteration_char);
        strcat(output, ".txt");
        strcpy(args.output_file_name, output);
}
