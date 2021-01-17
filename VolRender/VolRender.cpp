#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gl/glut.h>
#include <direct.h>
#include <cstring>
#include "gmath.h"

// 전역 변수들
int Width, Height, Depth;
unsigned char *pVolData = NULL, *pImage = NULL, *pScaledImage = NULL;
int SliceIdx = 0;
int N = 3;
const char *fileName = "..\\data\\bighead.txt";

GVec3 *pNormal = NULL;
double *pOpacity = NULL;
double **pColor = NULL;

// 콜백 함수 선언
void Render();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);

// 함수 선언
void LoadData(const char *fileName);
void CreateImage();
int GetIdx(int i, int j);
int GetIdx(int i, int j, int k);

// 1. 그레디언트
void Gradient();
// 2. 분할
void Classfication();
// 3. 쉐이딩
void Shading();
// 4. 합성
void Composition();

// 삼선형 보간법
// 선형 보간
double linterpol(float a, float b, double inter);
// 이중선 보간
double binterpol(float a, float b, float c, float d, double intera, double interb);

int main(int argc, char **argv)
{
	// 볼륨 데이터 로딩
	LoadData(fileName);

	// OpenGL 초기화, 윈도우 크기 설정, 디스플레이 모드 설정
	glutInit(&argc, argv);
	glutInitWindowSize(Width * N, Height * N);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// 윈도우 생성 및 콜백 함수 등록
	glutCreateWindow("Volume Viewer");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);

	// 1. 그레디언트
	Gradient();
	// 2. 분할
	Classfication();
	// 3. 쉐이딩
	Shading();
	// 4. 합성
	Composition();

	// 이미지를 생성
	CreateImage();

	// 이벤트를 처리를 위한 무한 루프로 진입한다.
	glutMainLoop();

	// 배열 할당 해제
	delete[] pVolData, pImage, pScaledImage;
	delete[] pNormal, pOpacity;

	for (int i = 0; i < Depth * Height * Width; i++)
		delete[] pColor[i];
	delete[] pColor;

	return 0;
}

void LoadData(const char *fileName)
{
	// 볼륨 헤더(*.txt) 파일을 읽는다.
	FILE *fp;
	char vol_file_name[256];

	fopen_s(&fp, fileName, "r");
	fscanf_s(fp, "%d%d%d", &Width, &Height, &Depth);
	fscanf_s(fp, "%s", vol_file_name, sizeof(vol_file_name));
	fclose(fp);

	// 현재 디렉토리를 헤더 파일이 있는 곳으로 변경한다.
	std::string file_path(fileName);
	int idx = file_path.rfind("\\");
	file_path = file_path.substr(0, idx);
	_chdir(file_path.c_str());

	// 렌더링에 필요한 배열을 할당한다.
	if (pVolData == NULL)
		pVolData = new unsigned char[Depth * Height * Width];

	// 볼륨 데이터를 바이너리 형태로 읽는다.
	fopen_s(&fp, vol_file_name, "rb");
	fread(pVolData, sizeof(unsigned char), Depth * Height * Width, fp);
	fclose(fp);
}

void CreateImage()
{
	if (pImage == NULL)
		pImage = new unsigned char[Depth * Height * Width];

	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			int vidx = GetIdx(SliceIdx, i, j);
			int pidx = GetIdx(i, j);

			for (int d = 0; d < 3; d++) pImage[pidx + d] = pVolData[vidx];
		}
	}
}

int GetIdx(int i, int j)
{
	return (Height - 1 - i) * Width * 3 + j * 3;
}

int GetIdx(int i, int j, int k)
{
	return i * (Width * Height) + j * Width + k;
}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == '1' || key == GLUT_KEY_UP) SliceIdx++;
	if (SliceIdx >= Depth) SliceIdx = Depth - 1;

	if (key == '2' || key == GLUT_KEY_DOWN) SliceIdx--;
	if (SliceIdx <= 0) SliceIdx = 0;

	printf("Slide No. = %d\n", SliceIdx);
	CreateImage();

	glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) SliceIdx++;
	if (SliceIdx >= Depth) SliceIdx = Depth - 1;

	if (key == GLUT_KEY_DOWN) SliceIdx--;
	if (SliceIdx <= 0) SliceIdx = 0;

	printf("Slide No. = %d\n", SliceIdx);
	CreateImage();

	glutPostRedisplay();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Render()
{
	// 칼라 버퍼와 깊이 버퍼 지우기
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (pScaledImage == NULL)
		pScaledImage = new unsigned char[Width * N * Height * N * 3];

	for (int i = 0; i < Height * N; ++i)
	{
		for (int j = 0; j < Width * N; ++j)
		{
			for (int d = 0; d < 3; d++)
				pScaledImage[i * Width * N * 3 + j * 3 + d] = pImage[i / N * Width * 3 + j / N * 3 + d];
		}
	}

	// 칼라 버퍼에 Image 데이터를 직접 그린다.
	glDrawPixels(Width * N, Height * N, GL_RGB, GL_UNSIGNED_BYTE, pScaledImage);

	// 칼라 버퍼 교환한다
	glutSwapBuffers();
}

// 1. 그레디언트
void Gradient()
{
	if (pNormal == NULL)
		pNormal = new GVec3[Depth * Height * Width];

	for (int i = 1; i < Depth - 1; ++i)
	{
		for (int j = 1; j < Height - 1; ++j)
		{
			for (int k = 1; k < Width - 1; ++k)
			{
				int vidx = GetIdx(i, j, k);
				double nx = (pVolData[GetIdx(i, j, k + 1)] - pVolData[GetIdx(i, j, k - 1)]) / 2.0;
				double ny = (pVolData[GetIdx(i, j + 1, k)] - pVolData[GetIdx(i, j - 1, k)]) / 2.0;
				double nz = (pVolData[GetIdx(i + 1, j, k)] - pVolData[GetIdx(i - 1, j, k)]) / 2.0;
				pNormal[vidx].Set(-nx, -ny, -nz);
				pNormal[vidx].Normalize();
			}
		}
	}
}

// 2. 분할
void Classfication()
{
	if (pOpacity == NULL)
		pOpacity = new double[Depth * Height * Width];
}

// 3. 쉐이딩
void Shading()
{
	if (pColor == NULL)
	{
		*pColor = new double[Depth * Height * Width];
		for (int i = 0; i < Depth * Height * Width; i++)
			pColor[i] = new double[3];
	}
		
}

// 4. 합성
void Composition()
{

}

// 선형 보간
double linterpol(float a, float b, double inter)
{
	double result = 0.0f;

	if (inter > 1)
	{
		printf("선형 보간 오류 alpha 값 : %f\n", inter);
		return b;
	}
	else
	{
		result = (a * (1 - inter)) + (b * inter);
		return result;
	}
}

// 이중 선형 보간
double binterpol(float a, float b, float c, float d, double intera, double interb)
{
	double r1 = 0.0f;
	double r2 = 0.0f;
	double result = 0.0f;

	r1 = linterpol(a, b, intera);
	r2 = linterpol(c, d, intera);

	result = linterpol((float)r1, (float)r2, interb);

	return result;
}
