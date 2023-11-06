#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <math.h>
#include <stdio.h>
#include <float.h>


void downsampling (unsigned char* in, int const height, int const width, int const channel, unsigned char* out); 
void upsampling (unsigned char* in, int const height, int const width, int const channel, unsigned char* out); 
void bilinear (unsigned char* in, int const height, int const width, int const channel, unsigned char* out); 
double imagePSNR(unsigned char* frame1, unsigned char* frame2, unsigned int size);

int main()
{
 
	int height;
	int width;
	int channel;

    float PSNR_up,PSNR_bilinear;

	char command;

 	printf("Take a picture? (y/n)\n");
 	scanf("%c", &command);

 	if(command == 'n')
 		exit(1);


 	printf("Cheeze !\r\n");
	system("libcamera-still --width 960 --height 720 -o src.bmp");

 	unsigned char* imgIn = stbi_load("src.bmp", &width, &height, &channel, 3);
	unsigned char* imgOut_down0 = (unsigned char*) malloc (sizeof(unsigned char)*3*320*240);
	unsigned char* imgOut_up0 = (unsigned char*) malloc (sizeof(unsigned char)*3*960*720);
	unsigned char* imgOut_up1 = (unsigned char*) malloc (sizeof(unsigned char)*3*960*720);

	downsampling(imgIn, height, width, channel, imgOut_down0);
	upsampling(imgOut_down0, height, width, channel, imgOut_up0);
	bilinear(imgOut_down0, height, width, channel, imgOut_up1);
	PSNR_up = imagePSNR(imgIn,imgOut_up0, 3*960*720);
	PSNR_bilinear = imagePSNR(imgIn,imgOut_up1, 3*960*720);
    	printf("======PSNR RESULT======\n");
	printf("%f, %f\n",PSNR_up,PSNR_bilinear);
	printf("=======================\n");
				
	stbi_write_bmp("image_down.bmp", width/3, height/3, channel, imgOut_down0);
	stbi_write_bmp("image_up.bmp", width, height, channel, imgOut_up0);
	stbi_write_bmp("image_bi.bmp", width, height, channel, imgOut_up1);
	
	stbi_image_free(imgIn);
	free(imgOut_down0);
	free(imgOut_up0);
	free(imgOut_up1);
 

	return 0;
}

void downsampling (unsigned char* in, int const height, int const width, int const channel, unsigned char* out) {  
	unsigned char down[720][960][3] = { 0 };

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			for (int k = 0; k < channel; k++) {
				down[i][j][k] = in[i * channel * width + j * channel + k];
			}

	for (int i = 0; i < height / 3; i++)
		for (int j = 0; j < width / 3; j++)
			for (int k = 0; k < channel; k++) {
				out[i * channel * width / 3 + j * channel + k] = down[3 * i][3 * j][k];
			}
}

void upsampling (unsigned char* in, int const height, int const width, int const channel, unsigned char* out) { 
	unsigned char up[720][960][3] = { 0 };

	for (int i = 0; i < height / 3; i++)
		for (int j = 0; j < width / 3; j++)
			for (int k = 0; k < channel; k++) {
				up[3 * i][3 * j][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i][3 * j + 1][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i][3 * j + 2][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i + 1][3 * j][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i + 1][3 * j + 1][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i + 1][3 * j + 2][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i + 2][3 * j][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i + 2][3 * j + 1][k] = in[i * channel * width / 3 + j * channel + k];
				up[3 * i + 2][3 * j + 2][k] = in[i * channel * width / 3 + j * channel + k];
			}

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			for (int k = 0; k < channel; k++) {
				out[i * channel * width + j * channel + k] = up[i][j][k];
			}
}

void bilinear (unsigned char* in, int const height, int const width, int const channel, unsigned char* out) {  
	unsigned char bilinear[720][960][3] = { 0 };

	for (int i = 0; i < height / 3; i++)
		for (int j = 0; j < width / 3; j++)
			for (int k = 0; k < channel; k++) {
				bilinear[3 * i][3 * j][k] = in[i * channel * width / 3 + j * channel + k];
				bilinear[3 * i][3 * j + 1][k] = (in[i * channel * width / 3 + j * channel + k] * 2 + in[i * channel * width / 3 + (j + 1) * channel + k] * 1) / 3;
				bilinear[3 * i][3 * j + 2][k] = (in[i * channel * width / 3 + j * channel + k] * 1 + in[i * channel * width / 3 + (j + 1) * channel + k] * 2) / 3;
				bilinear[3 * i + 1][3 * j][k] = (in[i * channel * width / 3 + j * channel + k] * 2 + in[(i + 1) * channel * width / 3 + j * channel + k] * 1) / 3;
				bilinear[3 * i + 1][3 * j + 1][k] = (in[i * channel * width / 3 + j * channel + k] * 2 * 2 + in[i * channel * width / 3 + (j + 1) * channel + k] * 2 * 1 +
					in[(i + 1) * channel * width / 3 + j * channel + k] * 1 * 2 + in[(i + 1) * channel * width / 3 + (j + 1) * channel + k] * 1 * 1) / 9;
				bilinear[3 * i + 1][3 * j + 2][k] = (in[i * channel * width / 3 + j * channel + k] * 2 * 1 + in[i * channel * width / 3 + (j + 1) * channel + k] * 2 * 2 +
					in[(i + 1) * channel * width / 3 + j * channel + k] * 1 * 1 + in[(i + 1) * channel * width / 3 + (j + 1) * channel + k] * 1 * 2) / 9;
				bilinear[3 * i + 2][3 * j][k] = (in[i * channel * width / 3 + j * channel + k] * 1 + in[(i + 2) * channel * width / 3 + j * channel + k] * 2) / 3;
				bilinear[3 * i + 2][3 * j + 1][k] = (in[i * channel * width / 3 + j * channel + k] * 1 * 2 + in[i * channel * width / 3 + (j + 1) * channel + k] * 1 * 1 +
					in[(i + 1) * channel * width / 3 + j * channel + k] * 2 * 2 + in[(i + 1) * channel * width / 3 + (j + 1) * channel + k] * 2 * 1) / 9;
				bilinear[3 * i + 2][3 * j + 2][k] = (in[i * channel * width / 3 + j * channel + k] * 1 * 1 + in[i * channel * width / 3 + (j + 1) * channel + k] * 1 * 2 +
					in[(i + 1) * channel * width / 3 + j * channel + k] * 2 * 1 + in[(i + 1) * channel * width / 3 + (j + 1) * channel + k] * 2 * 2) / 9;
			}

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			for (int k = 0; k < channel; k++) {
				out[i * channel * width + j * channel + k] = bilinear[i][j][k];
			}
}

//Calculates image PSNR value
double imagePSNR(unsigned char* frame1, unsigned char* frame2, unsigned int size){  
	float MSE = 0;
	double PSNR = 0;

	for (int i = 0; i < 720; i++)
		for (int j = 0; j < 960; j++)
			for (int k = 0; k < 3; k++) {
				MSE += pow(abs(frame1[i * 3 * 960 + j * 3 + k] - frame2[i * 3 * 960 + j * 3 + k]), 2);
			}

	MSE = MSE / size;

	PSNR = 10 * log10(pow(255, 2) / MSE);

	return PSNR;
}
