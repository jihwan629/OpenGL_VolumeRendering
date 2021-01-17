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
int N = 3;
const char *fileName = "..\\data\\bighead.txt";

GVec3 *pNormal = NULL;
double *pOpacity = NULL, *pColor = NULL;

const unsigned char f1 = 100, f2 = 180;
const double alpha = 0.9;
const double interval = 1.0;

// 콜백 함수 선언
void Render();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);

// 함수 선언
void LoadData(const char *fileName);
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
double linterpol(double a, double b, double inter);
// 이중선 보간
double binterpol(double a, double b, double c, double d, double intera, double interb);

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

	// 1. 그레디언트
	Gradient();
	printf("Gradient Completed\n");

	// 2. 분할
	Classfication();
	printf("Classfication Completed\n");

	// 3. 쉐이딩
	Shading();
	printf("Shading Completed\n");

	// 4. 합성
	Composition();
	printf("Composition Completed\n");

	// 이벤트를 처리를 위한 무한 루프로 진입한다.
	glutMainLoop();

	// 배열 할당 해제
	delete[] pVolData, pImage, pScaledImage;
	delete[] pNormal, pOpacity, pColor;

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

int GetIdx(int i, int j)
{
	return (Height - 1 - i) * Width * 3 + j * 3;
}

int GetIdx(int i, int j, int k)
{
	return i * (Width * Height) + j * Width + k;
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

				// 노말 값이 양수가 나와야 하므로 -1 곱함
				pNormal[vidx].Set(-nx, -ny, -nz).Normalize();
			}
		}
	}
}

// 2. 분할
void Classfication()
{
	if (pOpacity == NULL)
		pOpacity = new double[Depth * Height * Width];

	for (int i = 0; i < Depth; ++i)				// z-direction
	{
		for (int j = 0; j < Height; ++j)		// y-direction
		{
			for (int k = 0; k < Width; ++k)		// x-direction
			{
				int vidx = GetIdx(i, j, k);
				unsigned char d = pVolData[vidx];

				if (d > f1 && d < f2)
				{
					double t = (double)(f2 - d) / (double)(f2 - f1);
					if (t <= 0.2)
						pOpacity[vidx] = alpha * t / 0.2;
					else if (t > 0.2 && t < 0.8)
						pOpacity[vidx] = alpha;
					else if (t >= 0.8)
						pOpacity[vidx] = alpha * (5.0 - 5.0 * t);
				}
				else pOpacity[vidx] = 0.0;
			}
		}
	}
}

// 3. 쉐이딩
// 조명 없이 간략하게 퐁 쉐이딩
void Shading()
{
	if (pColor == NULL)
		pColor = new double[Depth * Height * Width];

	// 간략화를 위해 관찰자와 조명의 방향 벡터를 같게 하였다.
	GVec3 V(0, -1, 0), L(0, -1, 0); // 정면
	//GVec3 V(-1, 0, 0), L(-1, 0, 0); // 옆면
	//GVec3 V(0, 0, -1), L(0, 0, -1);  // 윗면

	for (int i = 0; i < Depth; ++i)
	{
		for (int j = 0; j < Height; ++j)
		{
			for (int k = 0; k < Width; ++k)
			{
				int vidx = GetIdx(i, j, k);

				GVec3 N = pNormal[vidx];
				GVec3 H = (V + L).Normalize();

				double diff_color = 0.6;
				double spec_color = 0.8;

				pColor[vidx] = diff_color * MAX(N * L, 0.0) 
							+ spec_color * pow(MAX(N * H, 0.0), 32.0);
			}
		}
	}	
}

// 4. 합성
void Composition()
{
	if (pImage == NULL)
		pImage = new unsigned char[3 * Width * Height * 3];

	int MaxIdx = Width * Height * Depth;

	for (int i = 0; i < Height; ++i)
	{
		for (int j = 0; j < Width; ++j)
		{
			GLine ray(GPos3 (j, 0, i), GVec3(0, 1, 0)); // 정면
			//GLine ray(GPos3(0, j, i), GVec3(1, 0, 0)); // 옆면
			//GLine ray(GPos3(j, i, 0), GVec3(0, 0, 1)); // 윗면

			double t = 0.0;
			double alpha_out = 0.0, color_out = 0.0;

			while (alpha_out <= 1.0)
			{
				GPos3 pos = ray.Eval(t);
				int x = (int)pos[0], y = (int)pos[1], z = (int)pos[2];

				// 0보다 작을 때 제외
				if (x > 255 || y > 255 || z > 224 || x < 0 || y < 0) break;

				// 보간하기 위한 비율 값
				double tx = ABS(x - pos[0]), ty = ABS(y - pos[1]), tz = ABS(z - pos[2]);

				// 보간 과정
				// 두면을 이중 선형 보간 시킨 후 나온 두점을 선형 보간한다
				int vidxa = GetIdx(z, y, x);
				int vidxb = GetIdx(z, y + 1, x);
				int vidxc = GetIdx(z, y + 1, x + 1);
				int vidxd = GetIdx(z, y, x + 1);

				int vidx1a = GetIdx(z + 1, y, x);
				int vidx1b = GetIdx(z + 1, y + 1, x);
				int vidx1c = GetIdx(z + 1, y + 1, x + 1);
				int vidx1d = GetIdx(z + 1, y, x + 1);

				if (vidxa > MaxIdx || vidxb > MaxIdx || vidxc > MaxIdx || vidxd > MaxIdx)
					break;
				if (vidx1a > MaxIdx || vidx1b > MaxIdx || vidx1c > MaxIdx || vidx1d > MaxIdx)
					break;

				double co = binterpol(pColor[vidxa], pColor[vidxb], pColor[vidxc], pColor[vidxd], ty, tx);
				double co1 = binterpol(pColor[vidx1a], pColor[vidx1b], pColor[vidx1c], pColor[vidx1d], ty, tx);
				double color_in = linterpol(co, co1, tz);

				double ao = binterpol(pOpacity[vidxa], pOpacity[vidxb], pOpacity[vidxc], pOpacity[vidxd], ty, tx);
				double ao1 = binterpol(pOpacity[vidx1a], pOpacity[vidx1b], pOpacity[vidx1c], pOpacity[vidx1d], ty, tx);
				double alpha_in = linterpol(ao, ao1, tz);

				// 컬러와 불투명도 값을 누적한다
				color_out = color_out + alpha_in * (1.0 - alpha_out) * color_in;
				alpha_out = alpha_out + alpha_in * (1.0 - alpha_out);

				// interval 값이 작을 수록 부드러워지지만 실행 시간이 길어짐
				t += interval;
			}

			for (int d = 0; d < 3; d++) pImage[GetIdx(i, j) + d] = MIN(color_out, 1.0) * 255;
		}
	}
}

// 선형 보간
double linterpol(double a, double b, double inter)
{
	if (inter > 1)
	{
		printf("선형 보간 오류 alpha 값 : %f\n", inter);
		return b;
	}
	else
		return (a * (1 - inter)) + (b * inter);
}

// 이중 선형 보간
double binterpol(double a, double b, double c, double d, double intera, double interb)
{
	return linterpol(linterpol(a, b, intera), linterpol(c, d, intera), interb);
}
