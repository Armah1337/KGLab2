#include "Render.h"

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"
#include <vector>
#include <string>
#include <array>
#include <cmath>



bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}

void loadTex(LPCSTR fileName, GLuint* id) {
	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP(fileName, &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);


	//генерируем ИД для текстуры
	glGenTextures(1, id);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, *id);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

GLuint id1, id2, id3;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур
	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);

	loadTex("texture.bmp", &id1);
	//loadTex("tex1.bmp", &id2);
	//loadTex("tex2.bmp", &id3);
	loadTex("tex3.bmp", &id2);

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
}

struct threedbl{ // структура для вычисление координат нормалей
	double x,y,z;
};

typedef std::vector < std::array<double, 3>> VtxVec;

void getNormal (VtxVec::const_iterator a, VtxVec::const_iterator b, VtxVec::const_iterator c){

	/*
	    A = (a1,  a2,  a3): a+b -> (b[0] - a[0], b[1] - a[1], b[2] - a[2])
        B = (b1,  b2,  b3): a+c -> (c[0] - a[0], c[1] - a[1], c[2] - a[2])
		A x B = (a2b3  -   a3b2,     a3b1   -   a1b3,     a1b2   -   a2b1) = ( (b[1]-a[1]) * (c[2]-a[2]) - (b[2]-a[2]) * (c[1]-a[1]), (b[2]-a[2]) * (c[0]-a[0]) - (b[0]-a[0]) * (c[2]-a[2]), 
																			   (b[0]-a[0]) * (c[1]-a[1]) - (b[1]-a[1]) * (c[0]-a[0]))
	*/
	threedbl n;
	n.x = ((*b)[1]-(*a)[1]) * ((*c)[2]-(*a)[2]) - ((*b)[2]-(*a)[2]) * ((*c)[1]-(*a)[1]); // x координата нормали данной плоскости
	n.y = -((*b)[2]-(*a)[2]) * ((*c)[0]-(*a)[0]) - ((*b)[0]-(*a)[0]) * ((*c)[2]-(*a)[2]); // y координата нормали данной плоскости
	n.z = ((*b)[0]-(*a)[0]) * ((*c)[1]-(*a)[1]) - ((*b)[1]-(*a)[1]) * ((*c)[0]-(*a)[0]);// z координата нормали данной плоскости
	double len = sqrt(n.x*n.x + n.y*n.y + n.z*n.z); // нормализация нормали
	n.x /= len;
	n.y /= len;
	n.z /= len;
	glNormal3d(n.x,n.y,n.z);
}

VtxVec getCircleSection(double L[3], double M[3], double R[3], int zHeight = 0);

VtxVec drawHorizontal(int zOffset = 0) {
	double vtx[9][3] = { { 0,0,0 + zOffset },{ 5,-4,0 + zOffset },{ 5,3,0 + zOffset },{ 9,0,0 + zOffset },{ 13,4,0 + zOffset },{ 7,6,0 + zOffset },{ 3,6,0 + zOffset },{ 3,2,0 + zOffset },{ 12,1,0 + zOffset } };
	VtxVec vec;
	VtxVec concavePoints(getCircleSection(vtx[0], vtx[7], vtx[6], zOffset));
	VtxVec convexPoints(getCircleSection(vtx[3], vtx[8], vtx[4], zOffset));
	double xTexOffset = 0, yTexOffset = 0.5;
	if (zOffset) { 
		glColor4d(1, 0, 0, 0.5); // если это верхняя грань, делаем ее полупрозрачной при включенном альфаналожении
	}
	else { 
		glColor4d(1, 0, 0, 1);
		xTexOffset = 0.5;
	}
	glBegin(GL_TRIANGLES);
	bool reachedMiddle = false;
	for (VtxVec::iterator it = concavePoints.begin(); it != concavePoints.end() - 1; ++it) {
		glTexCoord2d(((*it)[0] / 13)*0.5 + xTexOffset, (((*it)[1] + 4) / 10)*0.5 + yTexOffset);
		glVertex3d((*it)[0], (*it)[1], (*it)[2]);
		if (concavePoints.end() - 1 - it > (concavePoints.size() / 2)) {
			glTexCoord2d((vtx[1][0] / 13)*0.5 + xTexOffset, ((vtx[1][1] + 4) / 10)*0.5 + yTexOffset);
			glVertex3dv(vtx[1]);
		}
		else if (!reachedMiddle) {
			reachedMiddle = true;
			glTexCoord2d((vtx[1][0] / 13)*0.5 + xTexOffset, ((vtx[1][1] + 4) / 10)*0.5 + yTexOffset);
			glVertex3dv(vtx[1]);
			glTexCoord2d((vtx[2][0] / 13)*0.5 + xTexOffset, ((vtx[2][1] + 4) / 10)*0.5 + yTexOffset);
			glVertex3dv(vtx[2]);
			--it;
			continue;
		}
		else { 
			glTexCoord2d((vtx[2][0] / 13)*0.5 + xTexOffset, ((vtx[2][1] + 4) / 10)*0.5 + yTexOffset);
			glVertex3dv(vtx[2]);
		}
		glTexCoord2d(((*(it+1))[0] / 13)*0.5 + xTexOffset, (((*(it+1))[1] + 4) / 10)*0.5 + yTexOffset);
		glVertex3d((*(it + 1))[0], (*(it + 1))[1], (*(it + 1))[2]);
	}
	glEnd();
	glBegin(GL_POLYGON);
	glTexCoord2d((vtx[6][0] / 13)*0.5 + xTexOffset, ((vtx[6][1] + 4) / 10)*0.5 + yTexOffset);
	glVertex3dv(vtx[6]);
	glTexCoord2d((vtx[2][0] / 13)*0.5 + xTexOffset, ((vtx[2][1] + 4) / 10)*0.5 + yTexOffset);
	glVertex3dv(vtx[2]);
	for (std::array<double, 3> vtx : convexPoints) {
		glTexCoord2d((vtx[0] / 13)*0.5 + xTexOffset, ((vtx[1] + 4) / 10)*0.5 + yTexOffset);
		glVertex3d(vtx[0], vtx[1], vtx[2]);
	}
	glTexCoord2d((vtx[4][0] / 13)*0.5 + xTexOffset, ((vtx[4][1] + 4) / 10)*0.5 + yTexOffset);
	glVertex3dv(vtx[4]);
	glTexCoord2d((vtx[5][0] / 13)*0.5 + xTexOffset, ((vtx[5][1] + 4) / 10)*0.5 + yTexOffset);
	glVertex3dv(vtx[5]);
	glEnd();
	vec.insert(vec.cbegin(), concavePoints.begin(), concavePoints.end());
	vec.push_back({ vtx[5][0], vtx[5][1], vtx[5][2] });
	vec.insert(vec.cend(), convexPoints.rbegin(), convexPoints.rend());
	vec.push_back({ vtx[2][0], vtx[2][1], vtx[2][2] });
	vec.push_back({ vtx[1][0], vtx[1][1], vtx[1][2] });
	return vec;
}

void drawSides(VtxVec upperSurface, VtxVec lowerSurface) {
	glColor3d(0, 1, 0);

	double sideLen = 0;
	for (VtxVec::const_iterator it = upperSurface.begin(); it != upperSurface.end() - 1; ++it) {
		if (it == upperSurface.begin()) {
			VtxVec::const_iterator lastUpperIt = upperSurface.end() - 1;
			sideLen += sqrt(pow((*it)[0] - (*lastUpperIt)[0], 2) + pow((*it)[1] - (*lastUpperIt)[1], 2));
		}
		sideLen += sqrt(pow((*it)[0] - (*(it+1))[0],2) + pow((*it)[1] - (*(it + 1))[1], 2));
	}
	
	double curW = 0, deltaX = 0;
	glBegin(GL_QUADS);
	for (VtxVec::const_iterator upperIt = upperSurface.begin(), lowerIt = lowerSurface.begin(); upperIt != upperSurface.end() - 1, lowerIt != lowerSurface.end() - 1; ++upperIt, ++lowerIt) {
		/*if (upperIt == upperSurface.begin()) {
			glEnd();
			glBindTexture(GL_TEXTURE_2D, id2);
			glBegin(GL_QUADS);
			VtxVec::const_iterator lastUpperIt = upperSurface.end() - 1, lastLowerIt = lowerSurface.end() - 1;
			getNormal(lastUpperIt, upperIt, lowerIt);
			glTexCoord2d(0, 1);
			glVertex3d((*upperIt)[0], (*upperIt)[1], (*upperIt)[2]);
			glTexCoord2d(0, 0);
			glVertex3d((*lowerIt)[0], (*lowerIt)[1], (*lowerIt)[2]);
			glTexCoord2d(1, 0);
			glVertex3d((*(lastLowerIt))[0], (*(lastLowerIt))[1], (*(lastLowerIt))[2]);
			glTexCoord2d(1, 1);
			glVertex3d((*(lastUpperIt))[0], (*(lastUpperIt))[1], (*(lastUpperIt))[2]);
		}
		else if (upperIt + 1 == upperSurface.end() - 1) {
			glEnd();
			glBindTexture(GL_TEXTURE_2D, id3);
			glBegin(GL_QUADS);
			getNormal(upperIt, upperIt + 1, lowerIt + 1);
			glTexCoord2d(1, 1);
			glVertex3d((*upperIt)[0], (*upperIt)[1], (*upperIt)[2]);
			glTexCoord2d(1, 0);
			glVertex3d((*lowerIt)[0], (*lowerIt)[1], (*lowerIt)[2]);
			glTexCoord2d(0, 0);
			glVertex3d((*(lowerIt + 1))[0], (*(lowerIt + 1))[1], (*(lowerIt + 1))[2]);
			glTexCoord2d(0, 1);
			glVertex3d((*(upperIt + 1))[0], (*(upperIt + 1))[1], (*(upperIt + 1))[2]);
		}*/
		if (upperIt == upperSurface.begin()) {
			VtxVec::const_iterator lastUpperIt = upperSurface.end() - 1, lastLowerIt = lowerSurface.end() - 1;
			deltaX = sqrt(pow((*upperIt)[0] - (*lastUpperIt)[0], 2) + pow((*upperIt)[1] - (*lastUpperIt)[1], 2));
			getNormal(lastUpperIt, upperIt, lowerIt);
			glTexCoord2d((curW + deltaX) / sideLen, 0.5);
			glVertex3d((*upperIt)[0], (*upperIt)[1], (*upperIt)[2]);
			glTexCoord2d((curW + deltaX) / sideLen, 0);
			glVertex3d((*lowerIt)[0], (*lowerIt)[1], (*lowerIt)[2]);
			glTexCoord2d(curW / sideLen, 0);
			glVertex3d((*(lastLowerIt))[0], (*(lastLowerIt))[1], (*(lastLowerIt))[2]);
			glTexCoord2d(curW / sideLen, 0.5);
			glVertex3d((*(lastUpperIt))[0], (*(lastUpperIt))[1], (*(lastUpperIt))[2]);
			curW += deltaX;
		}
		deltaX = sqrt(pow((*upperIt)[0] - (*(upperIt + 1))[0], 2) + pow((*upperIt)[1] - (*(upperIt + 1))[1], 2));
		getNormal(upperIt, upperIt+1, lowerIt + 1);
		//getNormal(upperIt, upperIt + 1, lowerIt);
		//getNormal(upperIt, lowerIt + 1, lowerIt);
		glTexCoord2d(curW/sideLen, 0.5);
		glVertex3d((*upperIt)[0], (*upperIt)[1], (*upperIt)[2]);
		glTexCoord2d(curW / sideLen, 0);
		glVertex3d((*lowerIt)[0], (*lowerIt)[1], (*lowerIt)[2]);
		glTexCoord2d((curW + deltaX) / sideLen, 0);
		glVertex3d((*(lowerIt + 1))[0], (*(lowerIt + 1))[1], (*(lowerIt + 1))[2]);
		glTexCoord2d((curW + deltaX) / sideLen, 0.5);
		glVertex3d((*(upperIt + 1))[0], (*(upperIt + 1))[1], (*(upperIt + 1))[2]);
		curW += deltaX;
	}
	glEnd();
}

//#define PI 3.14159265358979323846
VtxVec getCircleSection(double L[3], double M[3], double R[3], int zHeight) {
	double m1 = M[0] - L[0], m2 = R[0] - M[0], n1 = M[1] - L[1], n2 = R[1] - M[1];
	double x2 = (M[0] + L[0]) / 2, y2 = (M[1] + L[1]) / 2, x3 = (R[0] + M[0]) / 2, y3 = (R[1] + M[1]) / 2;
	double x1, y1;
	x1 = (-m2 * x3 - n2 * y3 + n2 * m1 * x2 / n1 + n2 * y2) / (((n2 * m1) - (m2 *n1)) / n1);
	y1 = (m1 * (x2 - x1) + n1 * y2) / n1;
	double centerVtx[] = { x1, y1, 0 + zHeight };
	VtxVec vtxVec;
	double r_len = sqrt(pow(L[0] - centerVtx[0], 2) + pow(L[1] - centerVtx[1], 2));
	double left_angle = acos((L[0] - centerVtx[0]) / r_len);
	vtxVec.push_back({ L[0], L[1], 0.0 + zHeight });
	for (double i = left_angle - 0.01; ; i -= 0.01) {
		double x = r_len * cos(i) + centerVtx[0], y = -1 * r_len * sin(i) + centerVtx[1];
		if (sqrt(pow(R[0] - x, 2) + pow(R[1] - y, 2)) <= 0.1) break;
		vtxVec.push_back({ x, y, 0.0 + zHeight });
	}
	vtxVec.push_back({ R[0], R[1], 0.0 + zHeight });
	/*glColor3d(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	for (std::array<double, 3> vtx : vtxVec) {
		glVertex3d(vtx[0], vtx[1], vtx[2]);
	}
	glEnd();
	glColor3d(0, 255, 0); // ERASE LATER
	glBegin(GL_LINES);
	glVertex3dv(L);
	glVertex3dv(M);
	glVertex3dv(R);
	glVertex3dv(M);
	glVertex3dv(centerVtx);
	glVertex3d(x2, y2, 0 + zHeight);
	glVertex3dv(centerVtx);
	glVertex3d(x3, y3, 0 + zHeight);
	glEnd(); // MAYBE ERASE LATER */
	return vtxVec;
}



void Render(OpenGL *ogl)
{       	
		
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);  // для работы альфаналожение требуется выключить режим текстур и освещения
	/*
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);
	*/


	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);\
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

    //чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	/*//Начало рисования квадратика станкина
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };
	glBindTexture(GL_TEXTURE_2D, id1);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();

	//конец рисования квадратика станкина*/
	glBindTexture(GL_TEXTURE_2D, id1);
	double centerVtx[] = { 6,3,-10 }, r = 4;
	glNormal3d(0, 0, 1);
	glBegin(GL_TRIANGLES);
	for (double angle = 0; angle < 2*PI; angle += PI/100.0) { // цикл текстурирования круга
		double x1 = r * cos(angle) + centerVtx[0], y1 = r * sin(angle) + centerVtx[1], x2 = r * cos(angle+PI/100.0) + centerVtx[0], y2 = r * sin(angle+PI/100.0) + centerVtx[1];
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(centerVtx);
		glTexCoord2d((256 * cos(angle) + 256)/512.0, (256 * sin(angle) + 256) / 512.0);
		glVertex3d(x1, y1, centerVtx[2]);
		glTexCoord2d((256 * cos(angle+PI/100.0) + 256) / 512.0, (256 * sin(angle+PI/100.0) + 256) / 512.0);
		glVertex3d(x2, y2, centerVtx[2]);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, id2); // биндинг текстуры для призмы
	glNormal3d(0,0,-1);
	VtxVec lower = drawHorizontal();
	glNormal3d(0,0,1);
	VtxVec upper = drawHorizontal(5);
	drawSides(upper, lower);
	
    
	
	//текст сообщения вверху слева, если надоест - закоментировать, или заменить =)
	char c[250];  //максимальная длина сообщения
	sprintf_s(c, "(T)Текстуры - %d\n(L)Свет - %d\n\nУправление светом:\n"
		"G - перемещение в горизонтальной плоскости,\nG+ЛКМ+перемещение по вертикальной линии\n"
		"R - установить камеру и свет в начальное положение\n"
		"F - переместить свет в точку камеры", textureMode, lightMode);
	ogl->message = std::string(c);




}   //конец тела функции

