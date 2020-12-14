#include "Render.h"




#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"
#include "ctime"
#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;
bool StarFlag, ThrowAwayFlag;
double Speed = 1;
double Tclock = 0;
double delta_time = 0.001, t_max_M = 0, t_max_S = 0, delta_time_E = 0.8, t_max_E = 0, t_max_A = 0;
double r, R, r_z_S = 0, r_z_M = 0, r_z_A = 0, flagF = 0, flagS = 1;
double t_max = 0;

//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;




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
	virtual void SetUpCamera()
	{

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
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//класс недоделан!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


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
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
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
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
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




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
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


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

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

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

//обработчик нажатия кнопок клавиатуры
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
	if (OpenGL::isKeyPressed('A'))
	{
		StarFlag = !StarFlag;
	}
	if (OpenGL::isKeyPressed('D'))
	{
		ThrowAwayFlag = !ThrowAwayFlag;
	}
	if (OpenGL::isKeyPressed('Q'))
	{
		Speed = Speed * 10;
	}
	if (OpenGL::isKeyPressed('E'))
	{
		Speed = Speed/10;
	}
	if (OpenGL::isKeyPressed('S'))
	{
		Speed = 0;
		Tclock = 0;
	}
	if (OpenGL::isKeyPressed('W'))
	{
		Speed = 1;
		Tclock = 0.001;
	}
	if (OpenGL::isKeyPressed('R'))
	{
		delta_time = 0.001;
		t_max_M = 0; 
		t_max_S = 0; 
		delta_time_E = 0.8; 
		t_max_E = 0; 
		t_max_A = 0;
		r; R; 
		r_z_S = 0; 
		r_z_M = 0; 
		r_z_A = 0; 
		flagF = 0; 
		flagS = 1;
		t_max = 0;
		ThrowAwayFlag = false;
	}
	if (key == 'S')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'Q')
		Time = 0;
}

void keyUpEvent(OpenGL *ogl, int key)
{

}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
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
}


ObjFile objModel,monkey;

Texture monkeyTex;

ObjFile Earth, Clouds, Moon, Satellite, Among, AmongDead, Palacas, Pinos, Couro;
Texture EarthTex, CloudsTex, MoonTex, SatelliteTex, AmongTex, AmongDeadTex, PalacasTex, PinosTex, CouroTex;

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
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
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

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем
	

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	

	 //так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	 // создающимися во время компиляции, я переименовал модели в *.obj_m
	loadModel("models\\lpgun6.obj_m", &objModel);


	glActiveTexture(GL_TEXTURE0);
	//loadModel("models\\monkey.obj_m", &monkey);
	//monkeyTex.loadTextureFromFile("textures//tex.bmp");
	//monkeyTex.bindTexture();
	loadModel("models\\Earth.obj_m", &Earth);
	EarthTex.loadTextureFromFile("textures//Earth_daymap.bmp");
	EarthTex.bindTexture();
	//loadModel("models\\Clouds.obj_m", &Clouds);
	//CloudsTex.loadTextureFromFile("textures//Clouds.bmp");
	//CloudsTex.bindTexture();
	loadModel("models\\deathstar.obj_m", &Moon);
	MoonTex.loadTextureFromFile("textures//deathstar.bmp");
	MoonTex.bindTexture();
	loadModel("models\\Satellite.obj_m", &Satellite);
	SatelliteTex.loadTextureFromFile("textures//Satellite.bmp");
	SatelliteTex.bindTexture();
	loadModel("models//among.obj_m", &Among);
	AmongTex.loadTextureFromFile("textures//among.bmp");
	AmongTex.bindTexture();
	loadModel("models//among_dead.obj_m", &AmongDead);
	AmongDeadTex.loadTextureFromFile("textures//among_dead.bmp");
	AmongDeadTex.bindTexture();



	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(500, 200);
	rec.setPosition(10, 20);
	rec.setText("T - вкл/выкл текстур\nL - вкл/выкл освещение\nF - Свет из камеры\nG - двигать свет по горизонтали\nG+ЛКМ двигать свет по вертекали\nQ - увелечение скорости движения\nE - уменьшение скорости движения\nW - Продолжение анимации (Скорость оригинальная)\nS - остановка анимации\nD - выброска астронавта\nA - убиство астронавта\nR - перезапуск анимации",0,0,0);

	
}





Vector3 circleM(double t_max)
{
	Vector3 P;
	P.setCoords(r * cos((2 * 3.141592 * 2000 * t_max) / 180), R * sin((2 * 3.141592  * 2000 * t_max) / 180), r_z_M);
	return P;
}

Vector3 circleS(double t_max)
{
	Vector3 P;
	P.setCoords(r * cos((2 * 3.141592 * 1000 * t_max) / 180), R * sin((2 * 3.141592 * 1000 * t_max) / 180), r_z_S);
	return P;
}

Vector3 circleA(double t_max)
{
	Vector3 P;
	P.setCoords(r * cos((2 * 3.141592 * 1000 * t_max) / 180), R * sin((2 * 3.141592 * 1000 * t_max) / 180), r_z_A);
	return P;
}

inline double Bez(double p0, double p1, double p2, double p3, double t)
{
	return p0 * (1 - t) * (1 - t) * (1 - t) + 3 * p1 * t * (1 - t) * (1 - t) + 3 * t * t * p2 * (1 - t) + p3 * t * t * t; //посчитаная формула
}


bool flagReverse = false;

double P0[] = { 0, 0, 0 };
double P1[] = { 4, 5, -1 };
double P2[] = { -4, 1, -2 };
double P3[] = { 0, 0, 0 };

Vector3 Bize(double p0[], double p1[], double p2[], double p3[], double t)
{
	Vector3 Vec;
	Vec.setCoords(Bez(p0[0], p1[0], p2[0], p3[0], t), Bez(p0[1], p1[1], p2[1], p3[1], t), Bez(p0[2], p1[2], p2[2], p3[2], t));
	return Vec;
}

void Render(OpenGL *ogl)
{   
	double renclock = clock();
	
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat dif[] = { 0.6, 0.6, 0.6, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//Прогать тут  


	//





	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
	glUniform3fARB(location, light.pos.X(), light.pos.Y(),light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());

	//первый пистолет
	//objModel.DrawObj();


	Shader::DontUseShaders();
	//
	////второй, без шейдеров
	//glPushMatrix();
	//	glTranslated(-5,15,0);
	//	//glScaled(-1.0,1.0,1.0);
	//	objModel.DrawObj();
	//glPopMatrix();



	//обезьяна

	/*s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program,"tex"); 
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(-90, 0, 0, 1);
	monkeyTex.bindTexture();
	monkey.DrawObj();
	glPopMatrix();*/
	
	t_max_E += (Tclock*50) * Speed;;
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glRotated(t_max_E, 0, 1, 0);
	glScaled(7.0, 7.0, 7.0);
	EarthTex.bindTexture();
	Earth.DrawObj();
	glPopMatrix();

	//glPushMatrix();
	//glRotated(90, 1, 0, 0);
	//glRotated(t_max_E, 0, 1, 0);
	//glScaled(7.0, 7.0, 7.0);
	//CloudsTex.bindTexture();
	//Clouds.DrawObj();
	//glPopMatrix();

	t_max_M -= (Tclock / 30) * Speed; //t_max становится = 1 за 5 секунд
	if (t_max_M > 1)
	{
		t_max_M = 0; //после обнуляется
	}
	r = 5.6;
	R = 8.4;
	Vector3 M = circleM(t_max_M);
	glPushMatrix();
	glTranslated(M.X(), M.Y(), M.Z());
	glRotated(90, 1, 0, 0);
	glRotated(180, 0, 1, 0);
	//glScaled(1.0, 1.0, 1.0);
	MoonTex.bindTexture();
	Moon.DrawObj();
	glPopMatrix();

	t_max_S += (Tclock / 15)*Speed;
	r = 3.2;
	R = 4.8;
	Vector3 F = circleS(t_max_S);
	glPushMatrix();
	glTranslated(F.X(), F.Y(), F.Z());
	//glTranslated(3, 0, 0);
	glRotated(90, 1, 0, 0);
	glRotated(180, 0, 1, 0);
	glRotated(t_max_E/3.09 * 8.2, 0, 1, 0);
	glRotated(t_max_E/3.09 * 8.2, 1, 0, 0);
	//glRotated(l_por, 0, 0, 1);
	//glRotated(t_max_E / 12, 0, 0, 1);
	glScaled(0.08, 0.08, 0.08);
	SatelliteTex.bindTexture();
	Satellite.DrawObj();
	glPopMatrix();
	if (Speed == 0)
	{
		r_z_M = 0;
		r_z_S = 0;
	}
	else
	{
		if (((r_z_S <= 1.3) || (r_z_S >= 1.3)) && flagF == 1)
		{
			r_z_S -= Tclock / 2;
			//l_por += 0.05;
		}
		else if (((r_z_S <= -1.3) || (r_z_S >= -1.3)) && flagF == 0)
		{
			r_z_S += Tclock / 2;
			//l_por -= 0.01;
		}
		if (r_z_S <= -1.3)
		{
			flagF = 0;
		}
		else if (r_z_S >= 1.3)
		{
			flagF = 1;
		}

		if (((r_z_M <= 5) || (r_z_M >= 5)) && flagS == 1)
		{
			r_z_M -= Tclock / 2;
		}
		else if (((r_z_M <= -5) || (r_z_M >= -5)) && flagS == 0)
		{
			r_z_M += Tclock / 2;
		}
		if (r_z_M <= -5)
		{
			flagS = 0;
		}
		else if (r_z_M >= 5)
		{
			flagS = 1;
		}
	}
	

	if (ThrowAwayFlag == true)
	{
		//glBegin(GL_LINE_STRIP);
		//for (double t = 0; t <= 1.0001; t += 0.01)
		//{
		//	double P[3];
		//	P[0] = Bez(P0[0], P1[0], P2[0], P3[0], t);
		//	P[1] = Bez(P0[1], P1[1], P2[1], P3[1], t);
		//	P[2] = Bez(P0[2], P1[2], P2[2], P3[2], t);
		//	glVertex3dv(P); //Рисуем точку P
		//}
		//glEnd();
		t_max += (delta_time / 5) * Speed; //t_max становится = 1 за 5 секунд
		if (t_max > 1)
		{
			t_max = 1; //после обнуляется
		}
	
		Vector3 P_old = Bize(P0, P1, P2, P3, t_max + delta_time);
		Vector3 P = Bize(P0, P1, P2, P3, t_max);
		Vector3 VecP_P_old = (P - P_old).normolize();

		Vector3 rotateX(VecP_P_old.X(), VecP_P_old.Y(), 0);
		rotateX = rotateX.normolize();

		Vector3 VecPrX = Vector3(1, 0, 0).vectProisvedenie(rotateX);
		double CosX = Vector3(1, 0, 0).ScalarProizv(rotateX);
		double SinAngleZ = VecPrX.Z() / abs(VecPrX.Z());
		double AngleOZ = acos(CosX) * 180 / 3.14 * SinAngleZ;

		double AngleOY = acos(VecP_P_old.Z()) * 180 / 3.14 - 90;
		//r_z_A = 0;
		//Vector3 A = circleA(t_max_M);
		if (StarFlag == true)
		{
			glPushMatrix();
			glTranslated(P.X(), P.Y(), P.Z());
			glRotated(90, 1, 0, 0);
			glRotated(90, 0, 1, 0);
			glScaled(0.08, 0.08, 0.08);
			AmongDeadTex.bindTexture();
			AmongDead.DrawObj();
			glPopMatrix();
		}
		else
		{
			glPushMatrix();
			glTranslated(P.X(), P.Y(), P.Z());
			glRotated(90, 1, 0, 0);
			glRotated(90, 0, 1, 0);
			glScaled(0.08, 0.08, 0.08);
			AmongTex.bindTexture();
			Among.DrawObj();
			glPopMatrix();
		}
	}
	else
	{

		if (StarFlag == true)
		{
			glPushMatrix();
			glTranslated(M.X(), M.Y(), M.Z());
			glRotated(90, 1, 0, 0);
			glRotated(90, 0, 1, 0);
			glScaled(0.08, 0.08, 0.08);
			AmongDeadTex.bindTexture();
			glPopMatrix();
			P0[0] = M.X();
			P0[1] = M.Y();
			P0[2] = M.Z();
			t_max = 0;
		}
		else
		{
			glPushMatrix();
			glTranslated(M.X(), M.Y(), M.Z());
			glRotated(90, 1, 0, 0);
			glRotated(90, 0, 1, 0);
			glScaled(0.08, 0.08, 0.08);
			AmongTex.bindTexture();
			glPopMatrix();
			P0[0] = M.X();
			P0[1] = M.Y();
			P0[2] = M.Z();
			t_max = 0;
		}
	}

	
	

	//////Рисование фрактала

	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		frac.UseShader();

		int location = glGetUniformLocationARB(frac.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());

		location = glGetUniformLocationARB(frac.program, "uOffset");
		glUniform2fARB(location, offsetX, offsetY);

		location = glGetUniformLocationARB(frac.program, "uZoom");
		glUniform1fARB(location, zoom);

		location = glGetUniformLocationARB(frac.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();

	}
	*/
	
	
	//////Овал Кассини
	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cassini.UseShader();

		int location = glGetUniformLocationARB(cassini.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());


		location = glGetUniformLocationARB(cassini.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();
	}

	*/

	
	
	

	
	Shader::DontUseShaders();

	
	Tclock = (clock() - renclock)/CLOCKS_PER_SEC;
	if (Tclock == 0)
	{
		Tclock = 0.001;
	}
}   //конец тела функции



bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	glActiveTexture(GL_TEXTURE0);
	rec.Draw();


		
	Shader::DontUseShaders(); 



	
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

