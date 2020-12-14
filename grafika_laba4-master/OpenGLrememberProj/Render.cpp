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

//��������� ������ ��� ��������� ����
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //��������� ��� ������ ��������
Shader frac;
Shader cassini;




//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
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



}  camera;   //������� ������ ������


//����� ���������!
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


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
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
			//����� �� ��������� ����� �� ����������
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
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

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����



//������ ���������� ����
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//���������� �������� ����
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
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


	
	//������� ���� �� ���������, � ����� ��� ����
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

//���������� �������� ������  ����
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

//���������� ������� ������ ����������
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

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{

	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	
	


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   �������� �������� �� �����
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	frac.FshaderFileName = "shaders\\frac.frag"; //��� ����� ������������ �������
	frac.LoadShaderFromFile(); //��������� ������� �� �����
	frac.Compile(); //�����������

	cassini.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	cassini.FshaderFileName = "shaders\\cassini.frag"; //��� ����� ������������ �������
	cassini.LoadShaderFromFile(); //��������� ������� �� �����
	cassini.Compile(); //�����������
	

	s[0].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[0].FshaderFileName = "shaders\\light.frag"; //��� ����� ������������ �������
	s[0].LoadShaderFromFile(); //��������� ������� �� �����
	s[0].Compile(); //�����������

	s[1].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //��� ����� ������������ �������
	s[1].LoadShaderFromFile(); //��������� ������� �� �����
	s[1].Compile(); //�����������

	

	 //��� ��� ��� ������� ������ *.obj �����, ��� ��� ��� ��������� �� ���������� � ���������� �������, 
	 // ������������ �� ����� ����������, � ������������ ������ � *.obj_m
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
	rec.setText("T - ���/���� �������\nL - ���/���� ���������\nF - ���� �� ������\nG - ������� ���� �� �����������\nG+��� ������� ���� �� ���������\nQ - ���������� �������� ��������\nE - ���������� �������� ��������\nW - ����������� �������� (�������� ������������)\nS - ��������� ��������\nD - �������� ����������\nA - ������� ����������\nR - ���������� ��������",0,0,0);

	
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
	return p0 * (1 - t) * (1 - t) * (1 - t) + 3 * p1 * t * (1 - t) * (1 - t) + 3 * t * t * p2 * (1 - t) + p3 * t * t * t; //���������� �������
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

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��������� ���������
	GLfloat amb[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat dif[] = { 0.6, 0.6, 0.6, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//������� ���  


	//





	s[0].UseShader();

	//�������� ���������� � ������.  ��� ���� - ���� ����� uniform ���������� �� �� �����. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//��� 2 - �������� �� ��������
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

	//������ ��������
	//objModel.DrawObj();


	Shader::DontUseShaders();
	//
	////������, ��� ��������
	//glPushMatrix();
	//	glTranslated(-5,15,0);
	//	//glScaled(-1.0,1.0,1.0);
	//	objModel.DrawObj();
	//glPopMatrix();



	//��������

	/*s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program,"tex"); 
	glUniform1iARB(l, 0);     //��� ��� ����� �� ��������� �������� ������� �� GL_TEXTURE0
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

	t_max_M -= (Tclock / 30) * Speed; //t_max ���������� = 1 �� 5 ������
	if (t_max_M > 1)
	{
		t_max_M = 0; //����� ����������
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
		//	glVertex3dv(P); //������ ����� P
		//}
		//glEnd();
		t_max += (delta_time / 5) * Speed; //t_max ���������� = 1 �� 5 ������
		if (t_max > 1)
		{
			t_max = 1; //����� ����������
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

	
	

	//////��������� ��������

	
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
	
	
	//////���� �������
	
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
}   //����� ���� �������



bool gui_init = false;

//������ ���������, ��������� ����� �������� �������
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

