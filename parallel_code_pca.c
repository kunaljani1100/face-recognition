#include <stdio.h>
#include <math.h>
#include <mpi.h>
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

void calculate_mean(int sum,int rows,int cols,double *means,double **a,int id) //Calculate the mean image vector
{
//	int data_ops=0;
//	int compute_ops=0;
	int i,j;
	for(i=0;i<cols;i++)
	{
		sum=0;
		for(j=0;j<rows;j++)
		{
			sum=sum+a[j][i];
			/**************
			if(id==0)
			{
				data_ops=data_ops+3;
				compute_ops=compute_ops+1;
			}
			*****************/
		}
		means[i]=sum/rows;
		/***********
		if(id==0)
		{
			data_ops=data_ops+3;
			compute_ops=compute_ops+1;
		}
		***********/
	//	printf("%f\n",means[i]);
	}
	/******
	if(id==0)
	{
		printf("Data operations in calculate_mean():%d \n",data_ops);
		printf("Compute operations in calculate_mean():%d \n",compute_ops);
	}
	********/
}
void normalize_matrix(int rows,int cols, double **a,double **b,double *means,int id)//Subtract the mean vector from each image vector
{
//	int data_ops=0;
//	int compute_ops=0;
	int i,j;
	for(i=0;i<rows;i++)
	{
		for(j=0;j<cols;j++)
		{
			b[i][j]=a[i][j]-means[j];
			/**********
			if(id==0)
			{
				data_ops=data_ops+3;
				compute_ops=compute_ops+1;
			}
			***********/
		//	printf("%f ",b[i][j]);
		}
	//	printf("\n");
	//	printf("\n");
	}
	/*******
	if(id==0)
	{
		printf("Data operations in normalize_matrix():%d \n",data_ops);
		printf("Compute operations in normalize_matrix:%d \n",compute_ops);
	}
	*******/
}
void calculate_transpose(int rows,int cols,double **b,double **c,int id) //Calculating the transpost of a matrix.
{
//	int data_ops=0;
//	int compute_ops=0;
	int i,j;
	for(i=0;i<rows;i++)
	{
		for(j=0;j<cols;j++)
		{
			c[j][i]=b[i][j];
			/*************
			if(id==0)
			{
				data_ops=data_ops+2;
			}
			****************/
		}
	}
	/********
	if(id==0)
	{
		printf("Data operations in calculate_transpose():%d \n",data_ops);
	}
	*********/
}
void initialize_eigenvectors(int cols,double **aaa,int id,int ncomponents)//Set all eigenvectors in the higher dimensional space to zero.
{
	int i,j;
//	int data_ops=0;
	for(i=0;i<ncomponents;i++)
	{
		for(j=0;j<cols;j++)
		{
			aaa[i][j]=0;
			/***********
			if(id==0)
			{
				data_ops=data_ops+1;
			}
			**********/
		}
	}
	/********
	if(id==0)
	{
		printf("Data operations in initialize_eigenvectors():%d \n",data_ops);
	}
	*********/
}
void calculate_new_eigenvectors(int rows,int cols,double **aaa,double **eigenvectors,double **b,int id,int ncomponents)
{
	//Map the eigenvectors from a lower dimensional space to a higher dimensional space.
//	int data_ops=0;
//	int compute_ops=0;
	int i,j,k;
	for(i=0;i<ncomponents;i++)
	{
		for(j=0;j<cols;j++)
		{
			for(k=0;k<rows;k++)
			{
				aaa[i][j]=aaa[i][j]+eigenvectors[i][k]*b[k][j];
				/*****************
				if(id==0)
				{
					data_ops=data_ops+4;
					compute_ops=compute_ops+2;
				}
				********************/
			}
		//	printf("%f ",aaa[i][j]);
		}
	//	printf("\n");
	//	printf("\n");
	}
	/*********
	if(id==0)
	{
		printf("Data operations in calculate_new_eigenvectors():%d \n",data_ops);
		printf("Compute operations in calculate_new_eigenvecors():%d \n",compute_ops);
	}
	*********/
}
void initialize_resultant(int rows,double **resultant,int id,int ncomponents) //Intitialize the resultant where results of training phase are stored.
{
	int i,j;
//	int data_ops=0;
	for(i=0;i<ncomponents;i++)
	{
		for(j=0;j<rows;j++)
		{
			resultant[i][j]=0;
			/****************
			if(id==0)
			{
				data_ops=data_ops+1;
			}
			****************/
		}
	}
	/**********
	if(id==0)
	{
		printf("Data operations in initialize_resultant():%d \n");
	}
	**********/
}
void calculate_resultant(int rows,int cols,double **resultant,double **aaa,double **c,int id,int ncomponents)//Calculate results of training.
{
	int i,j,k;
//	int data_ops=0;
//	int compute_ops=0;
	for(i=0;i<ncomponents;i++)
	{
		for(j=0;j<rows;j++)
		{
			for(k=0;k<cols;k++)
			{
				resultant[i][j]=resultant[i][j]+aaa[i][k]*c[k][j];
				/******************
				if(id==0)
				{
					data_ops=data_ops+4;
					compute_ops=compute_ops+2;
				}
				*******************/
			}
		}
	}
	/***********
	if(id==0)
	{
		printf("Data operations in calculate_resultant():%d \n");
		printf("Compute operations in calculate_resultant():%d \n");
	}
	***************/
}
void scale_resultant(int rows,double **resultant,int id,int ncomponents) //Training phase scaled by diving all values by 100000.
{
	int i,j;
//	int data_ops=0;
//	int compute_ops=0;
	for(i=0;i<ncomponents;i++)
	{
		for(j=0;j<rows;j++)
		{
			resultant[i][j]=resultant[i][j]/100000;
			/****************
			if(id==0)
			{
				data_ops=data_ops+2;
				compute_ops=compute_ops+1;
			}
			*****************/
	//		printf("%f ",resultant[i][j]);
		}
	//	printf("\n");
	}
	/***********
	if(id==0)
	{
		printf("Data operations in scale_resultant():%d \n",data_ops);
		printf("Compute operations in scale_resultant():%d \n",compute_ops);
	}
	************/
//	printf("\n");
}
void initialize_new_resultant(double *resultant2,int id,int ncomponents) //Array to store test value initialized.
{
	int i;
	int data_ops=0;
	for(i=0;i<ncomponents;i++)
	{
		resultant2[i]=0;
		if(id==0)
		{
			data_ops=data_ops+1;
		}
	}
	/**************
	if(id==0)
	{
		printf("Data operations in initialize_new_resultant():%d \n",data_ops);
	}
	****************/
}
void calculate_new_resultant(int rrs,int cols,double **aaa,int aa[rrs][cols],double *resultant2,int id,int ncomponents) //Calculate resultant vector
{
	int i,j,k;
//	int data_ops=0;
//	int compute_ops=0;
	for(i=0;i<ncomponents;i++)
	{
		for(j=0;j<1;j++)
		{
			for(k=0;k<cols;k++)
			{
				resultant2[i]=resultant2[i]+aaa[i][k]*aa[j][k];
				/*****************
				if(id==0)
				{
					data_ops=data_ops+4;
					compute_ops=compute_ops+2;
				}
				*******************/
			}
		}
	}
	/***********
	if(id==0)
	{
		printf("Data operations in calculate_new_resultant():%d \n",data_ops);
		printf("Compute operations in calculate_new_resultant():%d \n",compute_ops);
	}
	*************/
}
void find_suitable_matching_image(int no_of_images,double *resultant2,double **resultant,double *min_square_of_distances,int *image_no,int ncomponents) //Compare with all images from the training set anf find which image
//the test image matches the best.
{
	int i,j;
	int image_number=0;
	double min=1000000000000000000;
	for(i=0;i<no_of_images;i++)
	{
		double distance=0;
		for(j=0;j<ncomponents;j++)
		{
			distance=distance+((resultant2[j]-resultant[j][i])*(resultant2[j]-resultant[j][i]));
		//	printf("%f ",distance);
		}
		if(distance<min)
		{
			min=distance;
			image_number=i;
		}
	}
	*min_square_of_distances=min;
	*image_no=image_number;
//	printf("%d\n",*image_no);
}
int main(int argc, char* argv[]) {

    struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
    clock_gettime(CLK, &start_e2e);

    /* Check if enough command-line arguments are taken in. */
    /*******
	if(argc < 2){
        printf( "Usage: %s n p \n", argv[0] );
        return -1;
    }
*********/
    int N = atoi(argv[1]);                  /* size of input array */
//    int p = atoi(argv[2]);                  /* number of processors*/
    char *problem_name = "image_warping";
    char *approach_name = "data_division";

    FILE* outputFile;
    char outputFileName[100];
    sprintf(outputFileName, "output/%s_%s_%s_output.txt", problem_name, approach_name, argv[1]);

//    int number_of_threads = p;
//    omp_set_num_threads(p);
    char filename[1024];
    filename[0] ='\0';
    strcat(filename, argv[1]);
    strcat(filename, ".ppm");
    PPMImageGS *image;
//    image = (PPMImage *) malloc(sizeof(PPMImage));
	image = readPPMGS("1_greyscaled.ppm");
//	free(image);
                    /* Start the algo timer */
   // PPMImageGS* x;// = change_image_warping(image);
    clock_gettime(CLK, &start_alg);
    //----------------------------------------Algorithm Here------------------------------------------
	
//	int compute_ops=0;
//	int data_ops=0;
	double min_square_of_distances[1];
	double absolute_minimum=100000000;
	time_t now,later;
	int image_no[1];
	int abs_image_no;
	int p,id;
//	data_ops=data_ops+1;
//	struct timespec begin_process,comp_process,diff_process;
//	clock_gettime(CLK,&begin_process);
	int ierr=MPI_Init(&argc,&argv);
//	clock_gettime(CLK,&comp_process);
//	diff_process=diff(begin_process,comp_process);
//	printf("The process creation takes %ld secs and %ld nsecs.\n",diff_process.tv_sec,diff_process.tv_nsec);
	ierr=MPI_Comm_rank(MPI_COMM_WORLD,&id);
	ierr=MPI_Comm_size(MPI_COMM_WORLD,&p);	

//		struct timespec begin_read,end_read,read_time;
//		clock_gettime(CLK,&begin_read);
		int i,j,idx;
		int total_images=N;
		int no_of_images=total_images/p;
		int no_of_rows=image->x;
		int no_of_columns=image->y;
		int cols=no_of_rows*no_of_columns;
		int rows=no_of_images;
		/*************
		if(id==0)
		{
			data_ops=data_ops+14;
			compute_ops=compute_ops+2;
		}
		***************/
		double **a=(double**)malloc(rows*sizeof(double*));
		for(i=0;i<rows;i++)
		{
			a[i]=(double*)malloc(cols*sizeof(double*));
		}
		double **b=(double**)malloc(rows*sizeof(double*));
		for(i=0;i<rows;i++)
		{
			b[i]=(double*)malloc(cols*sizeof(double*));
		}
		double *means=(double*)malloc(cols*sizeof(double));
		PPMImageGS *im2=(PPMImageGS *) malloc(sizeof(PPMImageGS));
		double sum=0;
		int no_of_images_read=0;
		/****************
		if(id==0)
		{
			data_ops=data_ops+2;
		}
		*****************/
		for(i=id*no_of_images;i<no_of_images*(id+1);i++)
		{	
			char *file=(char*)malloc(1024*sizeof(char));
			file[0]='\0';
		//	strcat(file,"image");
			char *ii=(char*)malloc(5*sizeof(char));
			int num=0;
			integer_to_string(ii,i+1);
			strcat(file,ii);
			strcat(file,"_greyscaled.ppm");
			image = readPPMGS(file);
			int no_of_rows=image->x;
			int no_of_columns=image->y;
			im2->x=no_of_rows;
			im2->y=no_of_columns;
			/*****************
			if(id==0)
			{	
				data_ops=data_ops+8;
			}
			*******************/
			im2->data = (PPMPixelGS *) malloc(no_of_rows*no_of_columns*sizeof(PPMPixelGS));
			unsigned char img_values[no_of_rows*no_of_columns];
			for(j=0;j<cols;j++)
			{	
				im2->data[j].gs=image->data[j].gs;
				img_values[j]=im2->data[j].gs;
				a[no_of_images_read][j]=(double)image->data[j].gs;
				/*************
				if(id==0)
				{
					data_ops=data_ops+8;
					compute_ops=compute_ops+3;
				}
				**************/
			//	a[no_of_images_read][j]=(double)i+j;
		//		printf("%d ",(int)a[no_of_images_read][j]);
			}
		//	printf("\n");
			no_of_images_read++;
		}
//		clock_gettime(CLK,&end_read);
//		read_time=diff(begin_read,end_read);
//		if(id==0)
//		{
//			printf("Time taken to read in process %d: %ld sec and %ld nsec.\n",id,read_time.tv_sec,read_time.tv_nsec);
//		}
//		struct timespec begin_mean,end_mean,mean_diff;
//		clock_gettime(CLK,&begin_mean);
		calculate_mean(sum,rows,cols,means,a,id);
//		clock_gettime(CLK,&end_mean);
//		mean_diff=diff(begin_mean,end_mean);
//		if(id==0)
//		{
//			printf("Time taken to calculate in process %d: %ld sec and %ld nsec.\n",id,mean_diff.tv_sec,mean_diff.tv_nsec);
//		}
//		struct timespec begin_norm,end_norm,norm_diff;
//		clock_gettime(CLK,&begin_norm);
		normalize_matrix(rows,cols,a,b,means,id);
//		clock_gettime(CLK,&end_norm);
//		norm_diff=diff(begin_norm,end_norm);
//		if(id==0)
//		{
//			printf("Time taken to normalize in process %d: %ld sec and %ld nsec.\n",id,norm_diff.tv_sec,norm_diff.tv_nsec);
//		}
		double **c=(double**)malloc(cols*sizeof(double*));
		for(i=0;i<cols;i++)
		{
			c[i]=(double*)malloc(rows*sizeof(double*));
		}
//		struct timespec begin_tran,end_tran,tran_diff;
//		clock_gettime(CLK,&begin_tran);
		calculate_transpose(rows,cols,b,c,id);
//		clock_gettime(CLK,&end_tran);
//		tran_diff=diff(begin_tran,end_tran);
//		if(id==0)
//		{
//			printf("Time taken to calculate transpose in process %d: %ld and %ld nsec.\n",id,tran_diff.tv_sec,tran_diff.tv_nsec);
//		}
//		struct timespec begin_eigs,end_eigs,eigs_diff;
//		clock_gettime(CLK,&begin_eigs);
		double **d=(double**)malloc(rows*sizeof(double*));
		for(i=0;i<rows;i++)
		{
			d[i]=(double*)malloc(rows*sizeof(double*));
		}
		double **eigenvectors=(double**)malloc(rows*sizeof(double*));
		for(i=0;i<rows;i++)
		{
			eigenvectors[i]=(double*)malloc(rows*sizeof(double*));
		}
		double *eigenvalues=(double*)malloc(rows*sizeof(double));
		int k;
		FILE *fp;
		char thread_id[5];
		integer_to_string(thread_id,id);
		char fn[150];
		fn[0]='\0';
		strcat(fn,"matrix");
		strcat(fn,thread_id);
		strcat(fn,".csv");
		fp=fopen(fn,"w+");
		for(i=0;i<rows;i++)
		{
			fprintf(fp,"Element,");
		}
		fprintf(fp,"\n");
		for(i=0;i<rows;i++)
		{
			for(j=0;j<rows;j++)
			{
				d[i][j]=0;
			//	data_ops=data_ops+1;
				for(k=0;k<cols;k++)
				{
					d[i][j]=d[i][j]+b[i][k]*c[k][j];
					/***************
					if(id==0)
					{
						data_ops=data_ops+4;
						compute_ops=compute_ops+2;
					}
					****************/
				}
				d[i][j]=d[i][j]/(cols-1);
				/*******************
				if(id==0)
				{
					data_ops=data_ops+3;
					compute_ops=compute_ops+2;
				}
				*********************/
				if(j!=cols-1)
				{
					fprintf(fp,"%f,",d[i][j]);
				}
				else
				{
					fprintf(fp,"%f",d[i][j]);
				}
			}
			fprintf(fp,"\n");
		}
		fclose(fp);
		fn[0]='\0';
		strcat(fn,"/apps/codes/python/bin/python eigs");
	//	strcat(fn,"python eigs");
		strcat(fn,thread_id);
		strcat(fn,".py");
		system(fn);
	//	system(strcat("python eigs",strcat(thread_id,".py"));
		char *number;
	//	char number[60];
		size_t len;
		FILE *f1,*f2;
		fn[0]='\0';
		strcat(fn,"eigenvectors");
		strcat(fn,thread_id);
		strcat(fn,".txt");
		f1=fopen(fn,"r");
		fn[0]='\0';
		strcat(fn,"eigenvalues");
		strcat(fn,thread_id);
		strcat(fn,".txt");
	//	printf("%s ",fn);
		f2=fopen(fn,"r");
		for(i=0;i<rows;i++)
		{
			for(j=0;j<rows;j++)
			{
				/******
				int cc=0;
				while(1)
				{
					number[cc]=fgetc(f1);
					if(number[cc]==EOF || number[cc]=='\n')
					{
						number[cc]='\0';
						break;
					}
					else
					{
						cc++;
					}
				}
				******/
				getline(&number,&len,f1);
			//	printf("%s ",number);
				eigenvectors[i][j]=(double)atof(number);
				/***********************
				if(id==0)
				{
					data_ops=data_ops+2;
				}
				*******************/
			//	printf("%f ",eigenvectors[i][j]);
			}
		//	printf("\n");
		}
	//	printf("\n");
		for(i=0;i<rows;i++)
		{
			/******
			int cc=0;
			while(1)
			{
				number[cc]=fgetc(f1);
				if(number[cc]==EOF || number[cc]=='\n')
				{
					number[cc]='\0';
					break;
				}
				else
				{
					cc++;
				}
			}
			******/
			getline(&number,&len,f1);
	//		printf("%s \n",number);
			eigenvalues[i]=(double)atof(number);
			/**************
			if(id==0)
			{
				data_ops=data_ops+2;
			}
			****************/
	//		printf("%f ",eigenvalues[i]);
	//		printf("\n");
		}
		fclose(f1);
		fclose(f2);
	//	}
		for(i=0;i<rows;i++)
		{
			for(j=0;j<rows-1;j++)
			{
				/***************
				if(id==0)
				{
					data_ops=data_ops+2;
					compute_ops=compute_ops+1;
				}
				******************/
				if(eigenvalues[j]<eigenvalues[j+1])
				{
					double tmp=eigenvalues[j];
					eigenvalues[j]=eigenvalues[j+1];
					eigenvalues[j+1]=tmp;
					/********************
					if(id==0)
					{
						data_ops=data_ops+6;
					}
					**********************/
					double tmp2[rows];
					for(k=0;k<rows;k++)
					{
						tmp2[k]=eigenvectors[j][k];
						eigenvectors[j][k]=eigenvectors[j+1][k];
						eigenvectors[j+1][k]=tmp2[k];
						/****************
						if(id==0)
						{
							data_ops=data_ops+6;
						}
						******************/
					}
				}
			}
		}
		double total_variance=0;
		double present_variance=0;
		for(i=0;i<rows;i++)
		{
			total_variance=total_variance+eigenvalues[i];
		}
		int ncomponents;
		for(i=0;i<rows;i++)
		{
			present_variance=present_variance+eigenvalues[i];
			if(present_variance/total_variance>0.95)
			{
				ncomponents=i;
				break;
			}
		}
	//	printf("%d \n",ncomponents);
		double **aaa=(double**)malloc(ncomponents*sizeof(double*));
		for(i=0;i<ncomponents;i++)
		{
			aaa[i]=(double*)malloc(cols*sizeof(double*));
		}
		initialize_eigenvectors(cols,aaa,id,ncomponents);
		calculate_new_eigenvectors(rows,cols,aaa,eigenvectors,b,id,ncomponents);
//		clock_gettime(CLK,&end_eigs);
//		eigs_diff=diff(begin_eigs,end_eigs);
//		if(id==0)
//		{
//			printf("Time taken to compute eigs in process %d: %ld sec and %ld nsec\n",id,eigs_diff.tv_sec,eigs_diff.tv_nsec);
//		}
		double **resultant=(double**)malloc(ncomponents*sizeof(double*));
		for(i=0;i<ncomponents;i++)
		{
			resultant[i]=(double*)malloc(rows*sizeof(double*));
		}
//		struct timespec res_begin,res_end,res_diff;
//		clock_gettime(CLK,&res_begin);
		initialize_resultant(rows,resultant,id,ncomponents);
		calculate_resultant(rows,cols,resultant,aaa,c,id,ncomponents);
		scale_resultant(rows,resultant,id,ncomponents);
//		clock_gettime(CLK,&res_end);
//		res_diff=diff(res_begin,res_end);
//		if(id==0)
//		{
//			printf("Time taken to calculate resultant in process %d: %ld sec and %ld nsec\n",id,res_diff.tv_sec,res_diff.tv_nsec);
//		}
		//Read the image to be tested.

//		struct timespec testread_begin,testread_end,testread_diff;
//		clock_gettime(CLK,&testread_begin);
		int aaax;
		time_t tt;
		srand((unsigned) time(&tt));
		for(aaax=0;aaax<1;aaax++)
		{
		int nn=rand()%50+1;
		char nns[5];
		integer_to_string(nns,nn);
		PPMImageGS *image2;
		char image_name[1024];
		image_name[0]='\0';
	//	strcat(image_name,nns);
	//	strcat(image_name,"_warped.ppm");
		image2 = readPPMGS("2_warped.ppm");
		int rrs=1;
		int aa[rrs][cols];
		unsigned char img_values2[no_of_rows*no_of_columns];
		PPMImageGS *im4=(PPMImageGS *) malloc(sizeof(PPMImageGS));
		im4->x=no_of_rows;
		im4->y=no_of_columns;
		im4->data = (PPMPixelGS *) malloc(no_of_rows*no_of_columns*sizeof(PPMPixelGS));
		/****************
		if(id==0)
		{
			data_ops=data_ops+4;
		}
		******************/
		for(i=0;i<cols;i++)
		{	
		//	idx=no_of_rows*i+j;
			im4->data[i].gs=image2->data[i].gs;
			img_values2[i]=im4->data[i].gs;
			aa[0][i]=(double)image2->data[i].gs;
			aa[0][i]=aa[0][i]-means[i];
			/*****************
			if(id==0)
			{
				data_ops=data_ops+11;
				compute_ops=compute_ops+4;
			}
			*******************/
	//		printf("%d ",(int)aa[0][i]);
		}
		free(im4);
//		clock_gettime(CLK,&testread_end);
//		testread_diff=diff(testread_begin,testread_end);
//		if(id==0)
//		{
//			printf("Time taken to read test image in process %d: %ld sec and %ld nsec\n",id,testread_diff.tv_sec,testread_diff.tv_nsec);
//		}
		double *resultant2=(double*)malloc(ncomponents*sizeof(double));
		//Compare with image and fine the suitable image, if no image matches, then print that no image was found.

//		struct timespec match_begin,match_end,match_diff;
//		clock_gettime(CLK,&match_begin);
		initialize_new_resultant(resultant2,id,ncomponents);
		calculate_new_resultant(rrs,cols,aaa,aa,resultant2,id,ncomponents);
		for(i=0;i<ncomponents;i++)
		{
			resultant2[i]=resultant2[i]/100000;
			/*************
			if(id==0)
			{
				data_ops=data_ops+2;
				compute_ops=compute_ops+1;
			}
			*************/
	//		printf("%f ",resultant2[i]);
		}
	//	printf("\n");
		find_suitable_matching_image(no_of_images,resultant2,resultant,&min_square_of_distances[0],&image_no[0],ncomponents);
//		clock_gettime(CLK,&match_end);
//		match_diff=diff(match_begin,match_end);
//		if(id==0)
//		{
//			printf("Time taken to match image in process %d: %ld sec and %ld nsec\n",id,match_diff.tv_sec,match_diff.tv_nsec);
//		}
		/********
		if(id==0)
		{
			printf("%d ",image_no[0]);
		}
		*******/
		double msqd[p];
		int ino[p];
		struct timespec begin_comm2,end_comm2,diff2;
		clock_gettime(CLK,&begin_comm2);
		if(id==0)
		{
		//	int data_ops_in_communication=0;
			msqd[id]=min_square_of_distances[0];
		//	data_ops_in_communication=data_ops_in_communication+2;
		//	struct timespec bc,ec,cdiff;
		//	clock_gettime(CLK,&bc);
			for(i=1;i<p;i++)
			{
				ierr=MPI_Recv(&msqd[i],1,MPI_DOUBLE,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//		data_ops_in_communication=data_ops_in_communication+1;
			}
		//	clock_gettime(CLK,&ec);
		//	cdiff=diff(bc,ec);
		//	printf("%ld seconds and %ld nanoseconds.\n",cdiff.tv_sec,cdiff.tv_nsec);
			/******
			for(i=0;i<p;i++)
			{
				printf("%f ",msqd[i]);
			}
		******/
		}		
		else
		{
			MPI_Send(min_square_of_distances,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
		}
	//	clock_gettime(CLK,&end_comm2);
	//	diff2=diff(begin_comm2,end_comm2);
	//	printf("%ld seconds and %ld namoseconds.\n",diff2.tv_sec,diff2.tv_nsec);
	//	struct timespec begin_comm1,end_comm1,diff1;
	//	clock_gettime(CLK,&begin_comm1);
		if(id==0)
		{
			ino[id]=image_no[0];
		//	int data_ops_in_communication=0;
		//	data_ops_in_communication=data_ops_in_communication+2;
		//	struct timespec bc,ec,cdiff;
		//	clock_gettime(CLK,&bc);
			for(i=1;i<p;i++)
			{
				ierr=MPI_Recv(&ino[i],1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//		data_ops_in_communication=data_ops_in_communication+1;
			}
		//	clock_gettime(CLK,&end_comm2);
		//	cdiff=diff(begin_comm2,end_comm2);
		//	printf("%ld seconds and %ld nanoseconds\n",cdiff.tv_sec,cdiff.tv_nsec);
			/*******
			for(i=0;i<p;i++)
			{
				printf("%d ",ino[i]);
			}
			********/
		//	printf("Data operations in first communication part:%d \n.",data_ops_in_communication);
		}
		else
		{
			MPI_Send(image_no,1,MPI_INT,0,0,MPI_COMM_WORLD);
		
		}
	//	clock_gettime(CLK,&end_comm1);
	//	diff1=diff(begin_comm1,end_comm1);
	//	printf("2 %ld seconds and %ld nanosecs\n",diff1.tv_sec,diff1.tv_nsec);
		if(id==0)
		{
		//	int data_ops_in_communication=0;
		//	int compute_ops_in_communication=0;
			for(i=0;i<p;i++)
			{
		//		data_ops_in_communication=data_ops_in_communication+2;
		//		compute_ops_in_communication=compute_ops_in_communication+1;
				if(absolute_minimum>msqd[i])
				{
					absolute_minimum=msqd[i];
				abs_image_no=(rows*i)+ino[i];
		//			data_ops_in_communication=data_ops_in_communication+6;
		//			compute_ops_in_communication=compute_ops_in_communication+2;
				}
			}
			clock_gettime(CLK,&end_comm2);
			diff2=diff(begin_comm2,end_comm2);
		//	printf("%ld secs and %ld nsecs\n",diff2.tv_sec,diff2.tv_nsec);
		//	printf("Data operations in second communication part:%d \n",data_ops_in_communication);
		//	printf("Compute operations in second communication part:%d \n",compute_ops_in_communication);
			FILE *fppppp2;
			fppppp2=fopen("parallel_accuracy.csv","a");
			if(aaax==0)
				fprintf(fppppp2,"Value\n");
			if(absolute_minimum<10000000000000000)
			{
				if(abs_image_no+1==nn)
				{
					fprintf(fppppp2,"%d\n",1);
				}
				else
				{
					fprintf(fppppp2,"%d\n",0);
				}
					printf("Image is of image %d.\n",abs_image_no);
			}
			else
			{
				printf("Image does not lie in the database.\n");
				fprintf(fppppp2,"%d\n",0);
			}
			fclose(fppppp2);
			clock_gettime(CLK, &end_alg);
			alg = diff(start_alg, end_alg);
			FILE *fpsssss;
			fpsssss=fopen("combined_logs.csv","a");
			fprintf(fpsssss,"%d,%d,%f\n",N,p,(double)alg.tv_sec+(double)alg.tv_nsec/1000000000);
			fclose(fpsssss);
		    printf("%s,%s,%d,%d,%ld,%ld\n", problem_name, approach_name, N, 0, alg.tv_sec, alg.tv_nsec);
		}
	}
	/*********
	if(id==0)
	{
		printf("Data operations in main():%d \n",data_ops);
		printf("Compute operations in main():%d \n",compute_ops);
	}
	*********/
//	fclose(fppppp2);
	ierr=MPI_Finalize();
	
    //-----------------------------------------------------------------------------------------
     /* End the algo timer */
    char outputfilename[1024];
    outputfilename[0] ='\0';
    strcat(outputfilename, argv[1]);
    strcat(outputfilename, "_warped");
    strcat(outputfilename, ".ppm");
//    writePPMGS(outputfilename,im2);

    clock_gettime(CLK, &end_e2e);
    e2e = diff(start_e2e, end_e2e);
    
    
//    outputFile = fopen(outputFileName,"w");
	return 0;
}
