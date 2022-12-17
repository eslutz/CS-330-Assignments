/*
 * Module 8 Assignment
 * Eric Slutz
 */

#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<tuple>
#include <vector>
#include <windows.h>

#include "linmath.h"

#include <GLFW\glfw3.h>
// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

const float DEG2RAD = 3.14159 / 180;
const int window_width = 480;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };
enum DIRECTION { UP, RIGHT, DOWN, LEFT };

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
		brick_type = bt;
		x = xx;
		y = yy,
		width = ww;
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

			if (brick_type == REFLECTIVE)
			{
				glColor3d(1, 0, 1);
			}
			else
			{
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
				else if (health > 0)
				{
					glColor3d(color, 0, 0);
				}
				else
				{
					onoff = OFF;
				}
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
	glm::vec2 position;
	float radius;
	glm::vec2 velocity;
	glm::vec3 color;

	Circle(glm::vec2 pos, float rad, glm::vec2 vel, glm::vec3 col)
	{
		position = pos;
		radius = rad;
		velocity = vel;
		color = col;
	}

	void CheckCollision(Brick* brk)
	{
		tuple<boolean, DIRECTION, glm::vec2> collision = Collision(brk);
		// If collision is true
		if (std::get<0>(collision))
		{
			if (brk->brick_type == DESTRUCTABLE)
			{
				brk->health--;
			}
			// Collision resolution
			if (brk->onoff == ON)
			{
				DIRECTION dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if (dir == LEFT || dir == RIGHT) // Horizontal collision
				{
					velocity.x = -velocity.x; // Reverse horizontal velocity
					// Relocate
					float penetration = radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						position.x += penetration; // Move ball to right
					else
						position.x -= penetration; // Move ball to left;
				}
				else // Vertical collision
				{
					velocity.y = -velocity.y; // Reverse vertical velocity
					// Relocate
					float penetration = radius - std::abs(diff_vector.y);
					if (dir == UP)
						position.y -= penetration; // Move ball back up
					else
						position.y += penetration; // Move ball back down
				}
			}
		}

		// Move the ball
		position += velocity;
		// Then check if outside window bounds
		// If so, reverse velocity and restore at correct position
		if (position.x <= -1.0f)
		{
			velocity.x = -velocity.x;
			position.x = -0.99f;
		}
		else if (position.x >= 1)
		{
			velocity.x = -velocity.x;
			position.x = 0.99f;
		}
		else if (position.y <= -1.0f)
		{
			velocity.y = -velocity.y;
			position.y = -0.99f;
		}
		else if (position.y >= 1.0f)
		{
			velocity.y = -velocity.y;
			position.y = 0.99f;
		}
	}

	tuple<boolean, DIRECTION, glm::vec2> Collision(Brick* brk) // AABB - Circle collision
	{
		// Get center point circle first 
		glm::vec2 center(glm::vec2(position.x, position.y) + radius);
		// Calculate AABB info (center, half-extents)
		glm::vec2 aabb_half_extents(brk->width / 2.0f, brk->width / 2.0f);
		glm::vec2 aabb_center(
			brk->x + aabb_half_extents.x,
			brk->y + aabb_half_extents.y
		);
		// Get difference vector between both centers
		glm::vec2 difference = center - aabb_center;
		glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
		// Now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
		glm::vec2 closest = aabb_center + clamped;
		// Now retrieve vector between center circle and closest point AABB and check if length < radius
		difference = closest - center;
		// Set collision
		if (glm::length(difference) < radius) // Not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
			return make_tuple(true, VectorDirection(difference), difference);
		else
			return make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
	}

	DIRECTION VectorDirection(glm::vec2 target)
	{
		// Get new direction
		glm::vec2 compass[] = {
				glm::vec2(0.0f, 1.0f),	// up
				glm::vec2(1.0f, 0.0f),	// right
				glm::vec2(0.0f, -1.0f),	// down
				glm::vec2(-1.0f, 0.0f)	// left
		};
		float max = 0.0f;
		unsigned int best_match = -1;
		for (unsigned int i = 0; i < 4; i++)
		{
			float dot_product = glm::dot(glm::normalize(target), compass[i]);
			if (dot_product > max)
			{
				max = dot_product;
				best_match = i;
			}
		}
		return (DIRECTION)best_match;
	}

	void DrawCircle()
	{
		glColor3f(color.r, color.g, color.b);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++)
		{
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + position.x, (sin(degInRad) * radius) + position.y);
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
	GLFWwindow* window = glfwCreateWindow(window_width, window_width, "Module 8 Assignment - Bricks n Balls", NULL, NULL);
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
			world[i].DrawCircle();
		}

		// Draw bricks
		for (int x = 0; x < bricks.size(); x++)
		{
			if (bricks[x].onoff == OFF)
			{
				bricks.erase(bricks.begin() + x);
			}
			else
			{
				bricks[x].drawBrick();
			}
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
		Circle B(glm::vec2(0, -1), 0.05, glm::vec2(0.0005f, 0.0005f), glm::vec3(r, g, b));
		world.push_back(B);
	}
}
