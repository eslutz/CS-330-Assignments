/*
 * Module 8 Assignment
 * Eric Slutz
 */

#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:

	float x, y, width;
	float r, g, b;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int health;

	Brick(BRICKTYPE bt, float xx, float yy, float ww)
	{
		brick_type = bt; x = xx; y = yy, width = ww;
		onoff = ON;
		health = 50;
		r = 0.0f;
		g = 0.0f;
		b = 1.0f;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			// Set brick color
			float color = health % 10 == 0 ? 1 : (float)(0.4 + (1 - 0.4) * ((health % 10) - 0) / (9 - 0));
			if (health > 40)
			{
				glColor3d(0, 0, color);
			}
			else if (health > 30)
			{
				glColor3d(0, color, color);
			}
			else if (health > 20)
			{
				glColor3d(0, color, 0);
			}
			else if (health > 10)
			{
				glColor3d(color, color, 0);
			}
			else
			{
				glColor3d(color, 0, 0);
			}

			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				// Check brick health
				// If alive, decrease health when hit
				if (brk->health > 0)
				{
					brk->health--;
					direction = GetRandomDirection();
					x = x + 0.03;
					y = y + 0.04;
				}
				// Else no health left so destroy brick
				else
				{
					brk->onoff = OFF;
				}
			}
		}
	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius)
			{
				y += speed;
			}
			else
		{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius)
			{
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++)
		{
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};

vector<Circle> world;
vector<Brick> bricks;

// Create brick pattern
// --------------------
// 1 for brick
// 0 for open space
vector<vector<int>> gameGrid = {
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 1, 1, 1, 1, 1, 1, 0, 1 },
	{ 1, 0, 1, 1, 0, 0, 1, 1, 0, 1 },
	{ 1, 0, 1, 1, 0, 1, 1, 1, 0, 1 },
	{ 1, 0, 1, 1, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 }
};

int main(void)
{
	srand(time(NULL));

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "Module 8 Assignment - Bricks n Balls", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	for (int i = 0; i < gameGrid.size(); i++)
	{
		for (int j = 0; j < gameGrid[i].size(); j++)
		{
			if (gameGrid[i][j] == 1)
			{
				// Determing position of brick based on position in gameGrid matrix
				const float xPos = (((j + 1) / 10.0) * 2) - 1.1;
				const float yPos = (((i + 1) / 10.0) * -2) + 1.1;
				// 1 in 6 chance the brick will be indestructable
				if (rand() % 6 == 0)
				{
					bricks.push_back(Brick(REFLECTIVE, xPos, yPos, 0.19));
				}
				else
				{
					bricks.push_back(Brick(DESTRUCTABLE, xPos, yPos, 0.19));
				}
			}
		}
	}

	while (!glfwWindowShouldClose(window))
	{
		// Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.412f, 0.412f, 0.412f, 1.0f);

		processInput(window);

		// Movement
		for (int i = 0; i < world.size(); i++)
		{
			for (int x = 0; x < bricks.size(); x++)
			{
				world[i].CheckCollision(&bricks[x]);
			}
			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		// Draw bricks
		for (int x = 0; x < bricks.size(); x++)
		{
			bricks[x].drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 2, 0.05, r, g, b);
		world.push_back(B);
	}
}
