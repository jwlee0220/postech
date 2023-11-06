#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


#include <math.h>
#include <stdio.h>
#include <float.h>


void mirror_transform (unsigned char* in, int const height, int const width, int const channel, unsigned char* out);
void grayScale_transform (unsigned char* in, int const height, int const width, int const channel, unsigned char* out);
void sobelFiltering_transform (unsigned char* in, int const height, int const width, int const channel, unsigned char* out);

int main()
{
 
	int height = 480;
	int width = 640;
	int channel = 3;

	char command;
	
 	printf("Take a picture? (y/n)\n");
 	scanf("%c", &command);

 	if(command == 'n')
 		exit(1);


 	printf("Cheeze !\r\n");
	system("raspistill -w 640 -h 480 -t 10 -o image.bmp");

 	
 	unsigned char* imgIn = stbi_load("image.bmp", &width, &height, &channel, 3);


	unsigned char* imgOut_mirror = (unsigned char*) malloc (sizeof(unsigned char)*3*640*480);
	unsigned char* imgOut_grayScale = (unsigned char*) malloc (sizeof(unsigned char)*3*640*480);
	unsigned char* imgOut_sobelFiltering = (unsigned char*) malloc (sizeof(unsigned char)*3*640*480);
 	
	mirror_transform(imgIn, height, width, channel, imgOut_mirror);
	grayScale_transform(imgIn, height, width, channel, imgOut_grayScale);
	sobelFiltering_transform(imgOut_grayScale, height, width, channel, imgOut_sobelFiltering);


	stbi_write_bmp("image_mirror.bmp", width, height, channel, imgOut_mirror);
	stbi_write_bmp("image_grayScale.bmp", width, height, channel, imgOut_grayScale);
	stbi_write_bmp("image_sobelFiltering.bmp", width, height, channel, imgOut_sobelFiltering);
	
	stbi_image_free(imgIn);
	free(imgOut_mirror);
	free(imgOut_grayScale);
	free(imgOut_sobelFiltering);
 

	return 0;
}

void mirror_transform (unsigned char* in, int const height, int const width, int const channel, unsigned char* out) {
	int i;
	int j;
	int c;
	
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			for (c = 0; c < channel; c++)
			{
				out[i * width * channel + j * channel + c] = in[i * width * channel + (width - j) * channel + c];
			}
		}
	}
}

void grayScale_transform (unsigned char* in, int const height, int const width, int const channel, unsigned char* out) {
	int i;
	int c;
	int sum = 0;
	int avg = 0;

	for (i = 0; i < height * width * channel; i += channel)
	{
		sum = 0;

		for (c = 0; c < channel; c++)
		{
			sum += in[i + c];
		}

		avg = sum / channel;

		for (c = 0; c < channel; c++)
		{
			out[i + c] = avg;
		}

	}
}

void sobelFiltering_transform (unsigned char* in, int const height, int const width, int const channel, unsigned char* out) {
	unsigned char** temp;
	unsigned char** after_filter;
	unsigned char* vector_filter;
	int x_filter[3][3] = { {-1,0,1},{-2,0,2},{-1,0,1} };
	int y_filter[3][3] = { {1,2,1},{0,0,0},{-1,-2,-1} };
	int value = 0;
	int i, j, c;
	
	temp = (unsigned char**)calloc(height, sizeof(unsigned char*));
	after_filter = (unsigned char**)calloc(height, sizeof(unsigned char*));
	vector_filter = (unsigned char*)calloc(width * height, sizeof(unsigned char));

	for (i = 0; i < height; i++)
	{
		temp[i] = (unsigned char*)calloc(width, sizeof(unsigned char));
		after_filter[i] = (unsigned char*)calloc(width, sizeof(unsigned char));
	}

	//in의 데이터를 행렬로 표현
	for (i = 0; i < height * width * channel; i += channel)
	{
		j = i / channel;
		temp[j / width][j % width] = in[i];
	}

	// Filter 적용
	for (i = 1; i < height-1; i++)
	{
		for (j = 1; j < width-1; j++)
		{
			value = 0;

			//x_filter
			value += temp[i - 1][j - 1] * x_filter[0][0];
			value += temp[i - 1][j] * x_filter[0][1];
			value += temp[i - 1][j + 1] * x_filter[0][2];
			value += temp[i][j - 1] * x_filter[1][0];
			value += temp[i][j] * x_filter[1][1];
			value += temp[i][j + 1] * x_filter[1][2];
			value += temp[i + 1][j - 1] * x_filter[2][0];
			value += temp[i + 1][j] * x_filter[2][1];
			value += temp[i + 1][j + 1] * x_filter[2][2];

			if (value < 0)
			{
				value = -value;			//절댓값
			}

			if (value > 255)
			{
				value = 255;
			}

			after_filter[i][j] += value;

			value = 0;

			//y_filter
			value += temp[i - 1][j - 1] * y_filter[0][0];
			value += temp[i - 1][j] * y_filter[0][1];
			value += temp[i - 1][j + 1] * y_filter[0][2];
			value += temp[i][j - 1] * y_filter[1][0];
			value += temp[i][j] * y_filter[1][1];
			value += temp[i][j + 1] * y_filter[1][2];
			value += temp[i + 1][j - 1] * y_filter[2][0];
			value += temp[i + 1][j] * y_filter[2][1];
			value += temp[i + 1][j + 1] * y_filter[2][2];

			if (value < 0)
			{
				value = -value;			//절댓값
			}

			if (value > 255)
			{
				value = 255;
			}

			after_filter[i][j] += value;
		}
	}

	//벡터로 변환
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			vector_filter[width * i + j] = after_filter[i][j];
		}
	}

	//out에 assign
	for (i = 0; i < height * width; i++)
	{
		for (c = 0; c < channel; c++)
		{
			out[channel * i + c] = (unsigned char)vector_filter[i];
		}
	}

	for (i = 0; i < 480; i++)
	{
		free(temp[i]);
		free(after_filter[i]);
	}
	free(vector_filter);
	free(temp);
	free(after_filter);
}