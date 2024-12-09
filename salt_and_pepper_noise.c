#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define CLK CLOCK_MONOTONIC

void integer_to_string(char ii[],int num)  //Converts integer to a string
{
	int a;
	int i=0;
	char ch;
	if(num==0)
	{
		ii[0]='0';
		ii[1]='\0';
		return;
	}
	while(num!=0)
	{
		a=num%10;
		num=num/10;
		ch=a+48;
//		printf("%c",ch);
		ii[i]=ch;
		i++;
	}
	ii[i]='\0';
	int len=i;
	char jj[len];
	for(i=0;i<len;i++)
	{
		jj[i]=ii[len-1-i];
	}
	for(i=0;i<len;i++)
	{
		ii[i]=jj[i];
	}
	ii[i]='\0';
//	printf("%s\n",ii);
}

struct timespec diff(struct timespec start, struct timespec end){  //Used to calculate the time taken to implement algorithm
    struct timespec temp;
    if((end.tv_nsec-start.tv_nsec)<0){
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else{
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

typedef struct {
    unsigned char red,green,blue;
} PPMPixel;

typedef struct {
    int x, y;
    PPMPixel *data;
} PPMImage;

typedef struct {
    unsigned char gs;
} PPMPixelGS;


typedef struct {
    int x, y;
    PPMPixelGS *data;
} PPMImageGS;

//Two images are used: colour and greyscale images, images must be in .ppm format.

#define RGB_COMPONENT_COLOR 255

static PPMImageGS *readPPMGS(const char *filename) //read the ppm image from the file
{
    char buff[16];
    PPMImageGS *img;
    FILE *fp;
    int c, rgb_comp_color;
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '5') {
        fprintf(stderr, "Invalid image format (must be 'P5')\n");
        exit(1);
    }

    //alloc memory form image
    img = (PPMImageGS *)malloc(sizeof(PPMImageGS));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n') ;
        c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixelGS*)malloc(img->x * img->y * sizeof(PPMPixelGS));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}

void writePPMGS(const char *filename, PPMImageGS *img) //Write a ppm image to a file
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P5\n");



    //image size
    fprintf(fp, "%d %d\n",img->x,img->y);

    // rgb component depth
    fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, img->x, img->y, fp);
    fclose(fp);
}


static PPMImage *readPPM(const char *filename) //read the ppm image from the file
{
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n') ;
        c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}

void writePPM(const char *filename, PPMImage *img) //Write ppm image to file.
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //comments


    //image size
    fprintf(fp, "%d %d\n",img->x,img->y);

    // rgb component depth
    fprintf(fp, "%d\n",255);

    // pixel data
    fwrite(img->data, 3 * img->x, img->y, fp);
    fclose(fp);
}

int main(int argc, char* argv[]) {

    struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
    clock_gettime(CLK, &start_e2e);

    /* Check if enough command-line arguments are taken in. */
    if(argc < 2 || argc > 2){
        printf( "Usage: %s n \n", argv[0] );
        return -1;
    }

    int N = atoi(argv[1]);                  /* size of input array */
//    int p = atoi(argv[2]);                  /* number of processors*/
    char *problem_name = "image_warping";
    char *approach_name = "data_division";

    FILE* outputFile;
    char outputFileName[100];
    sprintf(outputFileName, "output/%s_%s_%s_output.txt", problem_name, approach_name, argv[1]);

//    int number_of_threads = p;
//    omp_set_num_threads(1);
    char filename[1024];
    filename[0] ='\0';
    strcat(filename, argv[1]);
    strcat(filename,"_greyscaled");
    strcat(filename, ".ppm");
    PPMImageGS *image;
//    image = (PPMImage *) malloc(sizeof(PPMImage));
	image = readPPMGS(filename);
//	free(image);
    clock_gettime(CLK, &start_alg);                 /* Start the algo timer */
   // PPMImageGS* x;// = change_image_warping(image);
    //----------------------------------------Algorithm Here------------------------------------------
	
	int no_of_images=1;	
	int no_of_rows=image->x;
	int no_of_columns=image->y;
	int i,j,idx;
	int cols=no_of_rows*no_of_columns;
	int rows=no_of_images;
	for(i=0;i<no_of_images;i++)
	{
		image->x=no_of_rows;
		image->y=no_of_columns;
		//omp_set_num_threads(p);
		//#pragma omp parallel for private(i,j,idx)
		//	unsigned char img_values[no_of_rows*no_of_columns];
		double res;
		srand(time(0));
		for(j=0;j<cols;j++)
		{	
		//	idx=no_of_rows*i+j;
		//	srand(time(0));
		//	srand(time(0));
			double prob=(double)rand()/(RAND_MAX);
			if(prob<0.05)
			{
				res=0;
				image->data[j].gs=res;
			}
			if(prob>0.95)
			{
				res=255;
				image->data[j].gs=res;
			}
		//	printf("%f %f %f",res,yy1,yy2);
		//	printf("%d ",image->data[j].red);
		//	printf("%d ",image->data[j].green);
		//	printf("%d ",image->data[j].blue);
		//	printf("\n");
//		}
		}
	
    	//-----------------------------------------------------------------------------------------
    	 /* End the algo timer */
    	char outputfilename[1024];
    	outputfilename[0] ='\0';
    	int imno=atoi(argv[1]);
    	int ino=i+no_of_images*imno-(no_of_images-1);
    	char number[25];
    	integer_to_string(number,ino);
    	strcat(outputfilename, number);
    	strcat(outputfilename, "_warped33");
    	strcat(outputfilename, ".ppm");
    	writePPMGS(outputfilename,image);
	}
	clock_gettime(CLK, &end_alg);
    clock_gettime(CLK, &end_e2e);
    e2e = diff(start_e2e, end_e2e);
    alg = diff(start_alg, end_alg);
    FILE *fpsssss;
	fpsssss=fopen("combined_logs.csv","a");
	fprintf(fpsssss,"%d,0,%f\n",N,(double)alg.tv_sec+(double)alg.tv_nsec/1000000000);
	fclose(fpsssss);
  
//    outputFile = fopen(outputFileName,"w");
    printf("%s,%s,%ld,%ld,%ld,%ld\n", problem_name, approach_name,e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);
	return 0;
}
