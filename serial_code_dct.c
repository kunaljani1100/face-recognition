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
double l(int i)
{
	if(i==0)
	{
		return 1/(sqrt(2));
	}
	else
	{
		return 1;
	}
}
void matmul(int size,double a[size][size],double b[size][size],double c[size][size])
{
	int i,j,k;
	for(i=0;i<size;i++)
	{
		for(j=0;j<size;j++)
		{
			c[i][j]=0;
			for(k=0;k<size;k++)
			{
				c[i][j]=c[i][j]+a[i][k]*b[k][j];
			}
		}
	}
}
void dctTransform(int no_of_rows,int no_of_columns,double matrix[no_of_rows][no_of_columns],double dct[no_of_rows][no_of_columns]) 
{ 
    int i, j, k, l;  
    double pi=3.14;
    double ci, cj, dct1, sum; 
    int m=no_of_rows;
    int n=no_of_columns;
    for (i = 0; i < m; i++) { 
        for (j = 0; j < n; j++) { 
  
            // ci and cj depends on frequency as well as 
            // number of row and columns of specified matrix 
            if (i == 0) 
                ci = 1 / sqrt(m); 
            else
                ci = sqrt(2) / sqrt(m); 
            if (j == 0) 
                cj = 1 / sqrt(n); 
            else
                cj = sqrt(2) / sqrt(n); 
  
            // sum will temporarily store the sum of  
            // cosine signals 
            sum = 0; 
            for (k = 0; k < m; k++) { 
                for (l = 0; l < n; l++) { 
                    dct1 = matrix[k][l] *  
                           cos((2 * k + 1) * i * pi / (2 * m)) *  
                           cos((2 * l + 1) * j * pi / (2 * n)); 
                    sum = sum + dct1; 
                } 
            } 
            dct[i][j] = ci * cj * sum; 
        } 
    } 
  /*******
    for (i = 0; i < m; i++) { 
        for (j = 0; j < n; j++) { 
            printf("%f\t", dct[i][j]); 
        } 
        printf("\n"); 
    } 
    *******/
}
void dct_operation_1d(int no_of_rows,int no_of_columns,double image[no_of_rows*no_of_columns],double result[no_of_rows*no_of_columns])
{
	int i,j,u,v;
	for(u=0;u<no_of_rows*no_of_columns;u++)
	{
		result[u]=0;
		for(i=0;i<no_of_rows*no_of_columns;i++)
		{		
			result[u]=result[u]+sqrt(2/(double)(no_of_rows*no_of_columns))*l(i)*cos((3.14*u*(2*i+1))/(2*(double)no_of_rows))*image[i];
		}
	}
}
void dct_operation(int no_of_rows,int no_of_columns,double image[no_of_rows][no_of_columns],double result[no_of_rows][no_of_columns])
{
	int i,j,u,v;
	for(u=0;u<no_of_rows;u++)
	{
		for(v=0;v<no_of_columns;v++)
		{
			result[u][v]=0;
			for(i=0;i<no_of_rows;i++)
			{		
				for(j=0;j<no_of_columns;j++)
				{
					result[u][v]=result[u][v]+(2/(double)no_of_rows)*l(i)*l(j)*
					cos((3.14*u*(2*i+1))/(2*(double)no_of_rows))*cos((3.14*v*(2*j+1))/(2*(double)no_of_columns))*image[i][j];
				}
			}
		}
	}
}
int main(int argc, char* argv[]) {

    struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
    clock_gettime(CLK, &start_e2e);

    /* Check if enough command-line arguments are taken in. */
    if(argc < 3 || argc > 3){
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
//    strcat(filename, argv[1]);
    strcat(filename, "1_greyscaled.ppm");
    PPMImageGS *image;
//    image = (PPMImage *) malloc(sizeof(PPMImage));
	image = readPPMGS(filename);
//	free(image);
    clock_gettime(CLK, &start_alg);                 /* Start the algo timer */
   // PPMImageGS* x;// = change_image_warping(image);
    //----------------------------------------Algorithm Here------------------------------------------
	
	int no_of_images=N;	
	int no_of_rows=image->x;
	int no_of_columns=image->y;
	int i,j,idx,k;
	int cols=no_of_rows*no_of_columns;
	int rows=no_of_images;
	double **dct_components=(double**)malloc(no_of_images*sizeof(double*));
	for(i=0;i<no_of_images;i++)
	{
		dct_components[i]=(double*)malloc(no_of_rows*no_of_columns*sizeof(double*));
	}
	double t[no_of_rows][no_of_columns];
	int size_of_matrix_taken[no_of_images];
	for(i=0;i<no_of_rows;i++)
	{
		for(j=0;j<no_of_columns;j++)
		{
			if(i==0)
			{
				t[i][j]=sqrt(1/(double)no_of_rows);
			}
			else
			{
				t[i][j]=sqrt(2/(double)no_of_rows)*cos(((2*(double)j+1)*(double)i*3.14)/(2*(double)no_of_rows));
			}
		}
	}
	double t_trans[no_of_rows][no_of_columns];
	for(i=0;i<no_of_rows;i++)
	{
		for(j=0;j<no_of_columns;j++)
		{
			t_trans[j][i]=t[i][j];
		}
	}
	/*****
	for(j=0;j<no_of_rows;j++)
	{	
		for(k=0;k<no_of_columns;k++)
		{
			printf("%f ",t[j][k]);
		//	greyscaled_components[j][k]=255;
		}
		printf("\n");
	}
	*****/
	for(i=0;i<no_of_images;i++)
	{
		double greyscaled_components[no_of_rows][no_of_columns];
		double intermediate_matrix[no_of_rows][no_of_columns];
		double result[no_of_rows][no_of_columns];
		char *file=(char*)malloc(1024*sizeof(char));
		file[0]='\0';
		char *ii=(char*)malloc(5*sizeof(char));
		int num=0;
		integer_to_string(ii,i+1);
		strcat(file,ii);
		strcat(file,"_greyscaled.ppm");
		image = readPPMGS(file);
		for(j=0;j<no_of_rows;j++)
		{	
			for(k=0;k<no_of_columns;k++)
			{
				idx=no_of_columns*j+k;
				greyscaled_components[j][k]=(double)image->data[idx].gs;
			//	greyscaled_components[j][k]=255;
			}
		}
		matmul(no_of_rows,t,greyscaled_components,intermediate_matrix);
		matmul(no_of_rows,intermediate_matrix,t_trans,result);
		double total_value=0;
		for(j=0;j<no_of_rows;j++)
		{
			for(k=0;k<no_of_columns;k++)
			{
				total_value=total_value+abs(result[j][k]);
			}
		}
	//	printf("%f\n",total_value);
		double num_value=0;
		int size=0;
		for(j=0;j<no_of_rows;j++)
		{
			for(k=0;k<=j;k++)
			{
				num_value=num_value+abs(result[k][j]);
			}
			for(k=0;k<j;k++)
			{
				num_value=num_value+abs(result[j][k]);
			}
			if(num_value/total_value>0.95)
			{
				size=j;
		//		printf("%d \n",size);
				break;
			}
		}
	//	printf("%f\n",num_value);
		size_of_matrix_taken[i]=size;
		int index=0;
		for(j=0;j<size;j++)
		{
			for(k=0;k<size;k++)
			{
				index=size*j+k;
				dct_components[i][index]=result[j][k]/100000;
		//		printf("%f ",dct_components[i][index]);
			}
		//	printf("\n");
		}
	//	printf("\n");
	}	
	char filename2[30];
	filename2[0]='\0';
	strcat(filename2, argv[2]);
    strcat(filename2, "_warped.ppm");
    PPMImageGS *testimage=readPPMGS(filename2);
    double test_greyscaled_components[no_of_rows][no_of_columns];
    double test_intermediate[no_of_rows][no_of_columns];
    double test_result[no_of_rows][no_of_columns];
    for(j=0;j<no_of_rows;j++)
	{	
		for(k=0;k<no_of_columns;k++)
		{
			idx=no_of_columns*j+k;
			test_greyscaled_components[j][k]=(double)testimage->data[idx].gs;
		//	greyscaled_components[j][k]=255;
		}
	}
	matmul(no_of_rows,t,test_greyscaled_components,test_intermediate);
	matmul(no_of_rows,test_intermediate,t_trans,test_result);
	double test_dct_components[size_of_matrix_taken[atoi(argv[2])-1]*size_of_matrix_taken[atoi(argv[2])-1]];
	int idxx=0;
	for(int j=0;j<size_of_matrix_taken[atoi(argv[2])-1];j++)
	{
		for(k=0;k<size_of_matrix_taken[atoi(argv[2])-1];k++)
		{
			idxx=size_of_matrix_taken[atoi(argv[2])-1]*j+k;
			test_dct_components[idxx]=test_result[j][k]/100000;
		}
	}
	double min=10000000;
	int image_number=0;
	for(i=0;i<no_of_images;i++)
	{
		double distance=0;
		int benchmark;
		if(size_of_matrix_taken[atoi(argv[2])-1]>=size_of_matrix_taken[i])
		{
			benchmark=size_of_matrix_taken[i]*size_of_matrix_taken[i];
		}
		else
		{
			benchmark=size_of_matrix_taken[atoi(argv[2])-1]*size_of_matrix_taken[atoi(argv[2])-1];
		}
		for(j=0;j<benchmark;j++)
		{
			distance=distance+((test_dct_components[j]-dct_components[i][j])*(test_dct_components[j]-dct_components[i][j]));
		}
	//	printf("%f\n",distance);
		if(distance<min)
		{
			min=distance;
			image_number=i;
		}
	}
//	printf("\n");
	if(min<300)
	{
		printf("Image is of image %d.\n",image_number);
	}
	else
	{
		printf("Image does not lie in the database.\n");
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
    //	strcat(outputfilename, "filtered.ppm");
    //	writePPMGS(outputfilename,im2);
    
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
