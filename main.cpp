#include <iostream>
#include <windows.h>

#define WIDTH 480
#define HEIGHT 500
#define SIZE 3*480*500

void BMPtoRAW(int size, char* Filename, char* rawFilename);
void BGRtoRGB(unsigned char* BGR, unsigned char* RGB, unsigned int Size);
void UpDownChange(unsigned char* Image);

void ColorSlicing();
void Smoothing();
void OverwriteImage();
unsigned char** Padding(unsigned char** In, int nFilterSize);

void WhiteBalancing();

using namespace std;

void main()
{
	BMPtoRAW(SIZE, "girl.bmp", "girl_raw.raw");
	ColorSlicing();
	Smoothing();
	OverwriteImage();
	cout << "1. girl.bmp에 Color slicing이 적용 되어 girl_result.raw로 저장되었습니다. " << endl;


	BMPtoRAW(SIZE, "book.bmp", "book_raw.raw");
	WhiteBalancing();
	cout << "2. book.bmp에 White balancing하여 book_result.raw로 저장되었습니다." << endl;
}

void WhiteBalancing()
{
	FILE *in;
	fopen_s(&in, "book_raw.raw", "rb");

	FILE *out;
	fopen_s(&out, "book_result.raw", "wb");

	unsigned char *IpImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);
	unsigned char *OutImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);


	double mr, mg, mb;
	int r, g, b;
	double rr, rg, rb; //비율

	int a0 = 214, a1 = 205, a2 = 200;		//흰색으로 바꾸고 싶은 색

	fread(IpImg, sizeof(unsigned char), SIZE, in);

	for (int i = 0; i<SIZE; i = i + 3)
	{
		r = (double)IpImg[i];
		g = (double)IpImg[i + 1];
		b = (double)IpImg[i + 2];
		
		rr = 255.0 / a0 * r;
		rg = 255.0 / a1 * g;
		rb = 255.0 / a2 * b;

		if (rr > 255) { rr = 255; }
		else if (rr < 0) { rr = 0; }

		if (rg > 255) { rg = 255; }
		else if (rg < 0) { rg = 0; }

		if (rb > 255) { rb = 255; }
		else if (rb < 0) { rb = 0; }

		OutImg[i] = (unsigned char)rr;
		OutImg[i + 1] = (unsigned char)rg;
		OutImg[i + 2] = (unsigned char)rb;

	}

	fwrite(OutImg, sizeof(unsigned char), SIZE, out);
	
	fclose(in);
	fclose(out);
	
	delete[] IpImg;
	delete[] OutImg;
	
}

void OverwriteImage()
{
	FILE *R;
	fopen_s(&R, "girl_R.raw", "rb");

	FILE *G;
	fopen_s(&G, "girl_G.raw", "rb");

	FILE *B;
	fopen_s(&B, "girl_B.raw", "rb");

	FILE *unface;
	fopen_s(&unface, "girl_unface.raw", "rb");

	FILE *face;
	fopen_s(&face, "girl_face.raw", "rb");

	FILE *result;
	fopen_s(&result, "girl_result.raw", "wb");

	
	unsigned char *IpRImg = (unsigned char*)malloc(sizeof(unsigned char)*WIDTH*HEIGHT);
	unsigned char *IpGImg = (unsigned char*)malloc(sizeof(unsigned char)*WIDTH*HEIGHT);
	unsigned char *IpBImg = (unsigned char*)malloc(sizeof(unsigned char)*WIDTH*HEIGHT);
	unsigned char *unfaceImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);
	unsigned char *faceImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);
	unsigned char *resultImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);

	fread(IpRImg, sizeof(unsigned char), WIDTH*HEIGHT, R);
	fread(IpGImg, sizeof(unsigned char), WIDTH*HEIGHT, G);
	fread(IpBImg, sizeof(unsigned char), WIDTH*HEIGHT, B);
	fread(unfaceImg, sizeof(unsigned char), SIZE, unface);
	fread(faceImg, sizeof(unsigned char), SIZE, face);
	
	int j = 0;
	for (int i = 0; i < SIZE; i = i + 3)
	{

		if(faceImg[i] == (unsigned char)127 && faceImg[i+1] == (unsigned char)127 && faceImg[i+2] == (unsigned char)127)
		{ //얼굴이 아닌부분
			resultImg[i] = unfaceImg[i];
			resultImg[i + 1] = unfaceImg[i + 1];
			resultImg[i + 2] = unfaceImg[i + 2];
		}
		else//얼굴인 부분
		{
			resultImg[i] = IpRImg[j];
			resultImg[i + 1] = IpGImg[j];
			resultImg[i + 2] = IpBImg[j];
		}
		j++;
	}
	
	fwrite(resultImg, sizeof(unsigned char), SIZE, result);
	
	fclose(R);
	fclose(G);
	fclose(B);
	fclose(unface);
	fclose(result);
	fclose(face);


	delete[] IpRImg;
	delete[] IpGImg;
	delete[] IpBImg;
	delete[] unfaceImg;
	delete[] faceImg;
	delete[] resultImg;
	
}
void Smoothing()
{
	FILE *girl;
	fopen_s(&girl, "girl_raw.raw", "rb");

	FILE *R;
	fopen_s(&R, "girl_R.raw", "wb");

	FILE *G;
	fopen_s(&G, "girl_G.raw", "wb");

	FILE *B;
	fopen_s(&B, "girl_B.raw", "wb");

	unsigned char *IpImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);

	unsigned char **lpRImg = (unsigned char**)malloc(sizeof(unsigned char*)*HEIGHT);
	unsigned char **lpGImg = (unsigned char**)malloc(sizeof(unsigned char*)*HEIGHT);
	unsigned char **lpBImg = (unsigned char**)malloc(sizeof(unsigned char*)*HEIGHT);

	unsigned char ** R_Pad;
	unsigned char ** G_Pad;
	unsigned char ** B_Pad;

	unsigned char ** OutRImg = (unsigned char**)malloc(sizeof(unsigned char*)*HEIGHT);
	unsigned char ** OutGImg = (unsigned char**)malloc(sizeof(unsigned char*)*HEIGHT);
	unsigned char ** OutBImg = (unsigned char**)malloc(sizeof(unsigned char*)*HEIGHT);

	int nFilterSize = 7;
	int pad_size = (nFilterSize / 2);
	double **SmoothingF = new double *[nFilterSize];

	fread(IpImg, sizeof(unsigned char), SIZE, girl);

	for (int i = 0; i < HEIGHT; i++)
	{
		lpRImg[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);
		lpGImg[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);
		lpBImg[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);

		OutRImg[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);
		OutGImg[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);
		OutBImg[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);

	}


	for (int h = 0; h < HEIGHT; h++)
	{
		for (int w = 0; w < WIDTH; w++)
		{
			lpRImg[h][w] = IpImg[(WIDTH*h * 3) + (w * 3)];
			lpGImg[h][w] = IpImg[(WIDTH*h * 3) + (w * 3) + 1];
			lpBImg[h][w] = IpImg[(WIDTH*h * 3) + (w * 3) + 2];
		}
	}

	R_Pad = Padding(lpRImg, nFilterSize);
	G_Pad = Padding(lpGImg, nFilterSize);
	B_Pad = Padding(lpBImg, nFilterSize);


	for (int i = 0; i < nFilterSize; i++)
	{
		SmoothingF[i] = new double[nFilterSize];
		for (int j = 0; j < nFilterSize; j++)
		{
			SmoothingF[i][j] = (1.0) / (nFilterSize * nFilterSize);
		}
	}

	for (int i = pad_size; i < HEIGHT + pad_size; i++)
	{
		for (int j = pad_size; j < WIDTH + pad_size; j++)
		{
			double Rtemp = 0;
			double Gtemp = 0;
			double Btemp = 0;
			for (int mask_x = -pad_size; mask_x <= pad_size; mask_x++)
			{
				for (int mask_y = -pad_size; mask_y <= pad_size; mask_y++)
				{
					Rtemp += (R_Pad[i + mask_x][j + mask_y] * SmoothingF[mask_x + pad_size][mask_y + pad_size]);
					Gtemp += (G_Pad[i + mask_x][j + mask_y] * SmoothingF[mask_x + pad_size][mask_y + pad_size]);
					Btemp += (B_Pad[i + mask_x][j + mask_y] * SmoothingF[mask_x + pad_size][mask_y + pad_size]);
				}

				OutRImg[i - pad_size][j - pad_size] = (unsigned char)Rtemp;
				OutGImg[i - pad_size][j - pad_size] = (unsigned char)Gtemp;
				OutBImg[i - pad_size][j - pad_size] = (unsigned char)Btemp;
			}
		}
	}

	for (int h = 0; h < HEIGHT; h++) {
		fwrite(OutRImg[h], sizeof(unsigned char), WIDTH, R);
		fwrite(OutGImg[h], sizeof(unsigned char), WIDTH, G);
		fwrite(OutBImg[h], sizeof(unsigned char), WIDTH, B);

	}

	fclose(girl);
	fclose(R);
	fclose(G);
	fclose(B);

	delete[] IpImg;
	for (int h = 0; h < HEIGHT; h++) 
	{
		delete[] lpRImg[h];
		delete[] lpGImg[h];
		delete[] lpBImg[h];

		delete[] OutRImg[h];
		delete[] OutGImg[h];
		delete[] OutBImg[h];
	}
	
}

void ColorSlicing()
{
	FILE *in;
	fopen_s(&in, "girl_raw.raw", "rb");

	FILE *face_out;
	fopen_s(&face_out, "girl_face.raw", "wb");

	FILE *un_face_out;
	fopen_s(&un_face_out, "girl_unface.raw", "wb");

	unsigned char a0 = 213;
	unsigned char a1 = 152;
	unsigned char a2 = 123;

	int R = 50;

	int r0, r1, r2;

	unsigned char *IpImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);
	unsigned char *FaceOutImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);
	unsigned char *UnFaceOutImg = (unsigned char*)malloc(sizeof(unsigned char)*SIZE);
	
	fread(IpImg, sizeof(unsigned char), SIZE, in);

	for (int i = 0; i<SIZE; i = i + 3)
	{
		r0 = (int)IpImg[i];
		r1 = (int)IpImg[i + 1];
		r2 = (int)IpImg[i + 2];

		unsigned int result = (a0-r0)*(a0-r0) + (a1-r1)*(a1-r1) + (a2-r2)*(a2-r2);

		if (result > R*R)	//살색구를 벗어난다면 -> 얼굴이 부분이 아님
		{
			UnFaceOutImg[i] = (unsigned char)r0;
			UnFaceOutImg[i + 1] = (unsigned char)r1;
			UnFaceOutImg[i + 2] = (unsigned char)r2;

			FaceOutImg[i] = (unsigned char)127;
			FaceOutImg[i + 1] = (unsigned char)127;
			FaceOutImg[i + 2] = (unsigned char)127;
		}
		else //살색구 안에 있다면 ->얼굴부분
		{
			UnFaceOutImg[i] = (unsigned char)127;
			UnFaceOutImg[i + 1] = (unsigned char)127;
			UnFaceOutImg[i + 2] = (unsigned char)127;

			FaceOutImg[i] = (unsigned char)r0;
			FaceOutImg[i + 1] = (unsigned char)r1;
			FaceOutImg[i + 2] = (unsigned char)r2;
		}
	}

	fwrite(FaceOutImg, sizeof(unsigned char), SIZE, face_out);
	fwrite(UnFaceOutImg, sizeof(unsigned char), SIZE, un_face_out);

	fclose(in);
	fclose(face_out);
	fclose(un_face_out);

	delete[] IpImg;
	delete[] FaceOutImg;
	delete[] UnFaceOutImg;

}

void BMPtoRAW(int size, char* Filename, char* rawFilename)
{
	FILE *in;
	fopen_s(&in, Filename, "rb");

	FILE *out;
	fopen_s(&out, rawFilename, "wb");


	BITMAPFILEHEADER bitmapfileheader;	//비트맵 파일 헤더
	fread(&bitmapfileheader, sizeof(BITMAPFILEHEADER), 1, in);

	BITMAPINFOHEADER bitmapinfoheader;	//비트맵 info 헤더
	fread(&bitmapinfoheader, sizeof(BITMAPINFOHEADER), 1, in);

	//bmp이미지의 info헤더에 있는 사이즈만큼 input이미지, output이미지 1차원 배열 할당
	unsigned char *IpImg = (unsigned char*)malloc(sizeof(unsigned char)*size);
	unsigned char *OutImg = (unsigned char*)malloc(sizeof(unsigned char)*size);

	//input이미지 배열에 읽어오기
	fread(IpImg, sizeof(unsigned char), size, in);

	BGRtoRGB(IpImg, OutImg, size);

	UpDownChange(OutImg);

	fwrite(OutImg, sizeof(unsigned char), size, out);

	fclose(in);
	fclose(out);

	delete[] IpImg;
	delete[] OutImg;
}

void BGRtoRGB(unsigned char* BGR, unsigned char* RGB, unsigned int Size) // RGB로 변환 함수
{
	for (unsigned int i = 0; i<Size; i = i + 3)
	{
		RGB[i] = BGR[i + 2];
		RGB[i + 1] = BGR[i + 1];
		RGB[i + 2] = BGR[i];
	}
}

void UpDownChange(unsigned char* Image)    // 상하 반전 함수
{
	unsigned int i, j, ch;
	for (i = 0; i<HEIGHT / 2; i++)

		for (j = 0; j<WIDTH *3; j++)
		{
			ch = Image[i * WIDTH * 3 + j];
			Image[i * WIDTH * 3 + j] = Image[(HEIGHT - i - 1) * WIDTH * 3 + j];
			Image[(HEIGHT - i - 1) * WIDTH * 3 + j] = ch;
		}

}


unsigned char** Padding(unsigned char** In, int nFilterSize)
{
	int nPadSize = (int)(nFilterSize / 2);
	unsigned char ** Pad;

	Pad = (unsigned char**)malloc(sizeof(unsigned char*) * (HEIGHT + 2 * nPadSize));	//읽어올 파일 메모리 할당

	for (int i = 0; i < (HEIGHT + 2 * nPadSize); i++) {
		Pad[i] = (unsigned char*)malloc(sizeof(unsigned char) * (WIDTH + 1 * nPadSize));
	}


	for (int h = 0; h < HEIGHT; h++)
	{
		for (int w = 0; w < WIDTH; w++) {
			Pad[h + nPadSize][w + nPadSize] = In[h][w];
		}
	}

	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < WIDTH; w++) {
			Pad[h][w + nPadSize] = In[0][w];
			Pad[h + (HEIGHT - 1)][w + nPadSize] = In[HEIGHT - 1][w];
		}
	}

	for (int h = 0; h < HEIGHT; h++)
	{
		for (int w = 0; w < nPadSize; w++) {
			Pad[h + nPadSize][w] = In[h][0];
			Pad[h + nPadSize][w + (WIDTH - 1)] = In[h][WIDTH - 1];
		}
	}

	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h][w] = In[0][0];
			Pad[h + (HEIGHT - 1)][w] = In[HEIGHT - 1][0];
			Pad[h][w + (WIDTH - 1)] = In[0][WIDTH - 1];
			Pad[h + (HEIGHT - 1)][w + (WIDTH - 1)] = In[HEIGHT - 1][WIDTH - 1];
		}
	}
	return Pad;
}