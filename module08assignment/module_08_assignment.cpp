/*
 * Module 8 Assignment
 * Eric Slutz
 */

#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tuple>
#include <vector>
#include <windows.h>

#include "linmath.h"

#include <GLFW/glfw3.h>
// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

const float DEG2RAD = 3.14159 / 180;
const int window_width = 480;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

enum BRICKTYPE { LAUNCHER, REFLECTIVE, DESTRUCTABLE }; // Represents a destructable or non-destructable brick
enum ONOFF { ON, OFF }; // Represents the on or off state of a brick or circle
enum DIRECTION { UP, RIGHT, DOWN, LEFT }; // Represents the four possible collision directions

// Defines a Collision typedef that represents collision data
typedef std::tuple<bool, DIRECTION, glm::vec2> Collision;

class Brick
{
public:
	glm::vec2 position;
	float width;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int health;

	Brick(BRICKTYPE bt, glm::vec2 pos, float ww)
	{
		brick_type = bt;
		position = pos;
		width = ww;
		onoff = ON;
		health = 50;
	};

	void DrawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			if (brick_type == LAUNCHER)
			{
				glColor3d(0.16f, 0.161f, 0.157f);
			}
			else if (brick_type == REFLECTIVE)
			{
				glColor3d(1.0f, 0.0f, 1.0f);
			}
			else
			{
				// Set brick color
				float color = health % 10 == 0 ? 1 : (float)(0.4 + (1 - 0.4) * ((health % 10) / 9.0));
				if (health > 40)
				{
					glColor3d(0.0f, 0.0f, color);
				}
				else if (health > 30)
				{
					glColor3d(0.0f, color, color);
				}
				else if (health > 20)
				{
					glColor3d(0.0f, color, 0.0f);
				}
				else if (health > 10)
				{
					glColor3d(color, color, 0.0f);
				}
				else if (health > 0)
				{
					glColor3d(color, 0.0f, 0.0f);
				}
				else
				{
					onoff = OFF;
				}
			}

			glBegin(GL_POLYGON);

			glVertex2d(position.x + halfside, position.y + halfside);
			glVertex2d(position.x + halfside, position.y - halfside);
			glVertex2d(position.x - halfside, position.y - halfside);
			glVertex2d(position.x - halfside, position.y + halfside);

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
	ONOFF onoff;
	int health;
	int strength;

	Circle(glm::vec2 pos, float rad, glm::vec2 vel)
	{
		position = pos;
		radius = rad;
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		velocity = vel;
		onoff = ON;
		health = 100;
		strength = 1;
	}

	// Changes movement of circle if there is a collsion
	void CheckCollision(Brick* brk)
	{
		Collision collision = CollisionHappened(brk);
		// If collision is true
		if (std::get<0>(collision))
		{
			if (brk->brick_type == LAUNCHER ||
				brk->brick_type == DESTRUCTABLE)
			{
				brk->health -= strength;
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
			std::get<0>(collision) = true;
		}
		else if (position.x >= 1)
		{
			velocity.x = -velocity.x;
			position.x = 0.99f;
			std::get<0>(collision) = true;
		}
		else if (position.y <= -1.0f)
		{
			velocity.y = -velocity.y;
			position.y = -0.99f;
			std::get<0>(collision) = true;
		}
		else if (position.y >= 1.0f)
		{
			velocity.y = -velocity.y;
			position.y = 0.99f;
			std::get<0>(collision) = true;
		}

		// If brick or wall collision occurs, decrease circle health
		if (std::get<0>(collision))
		{
			if (health > 0)
			{
				health--;
			}
			else
			{
				onoff = OFF;
			}
		}
	}

	// Checks for circle collision with a brick
	Collision CollisionHappened(Brick* brk) // AABB - Circle collision
	{
		// Get center point circle first 
		glm::vec2 center(glm::vec2(position.x, position.y) + radius);
		// Calculate AABB info (center, half-extents)
		glm::vec2 aabb_half_extents(brk->width / 2.0f, brk->width / 2.0f);
		glm::vec2 aabb_center(
			brk->position.x + aabb_half_extents.x,
			brk->position.y + aabb_half_extents.y
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

	// Get new direction after collision
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

	// Check for circle collision with another circle
	bool CheckCollision(Circle* cir)
	{
		// Collision x-axis?
		bool collisionX = position.x + radius >= cir->position.x &&
			cir->position.x + cir->radius >= position.x;
		// Collision y-axis?
		bool collisionY = position.y + radius >= cir->position.y &&
			cir->position.y + cir->radius >= position.y;

		// Collision only if on both axes
		return collisionX && collisionY;
	}

	// Upgrades circle stats when it consumes another circle
	void LevelUp(Circle* cir)
	{
		// Consuming circle gains remaining health of circle consumed
		health += cir->health;
		// Consuming circle gains strength of circle consumed
		for (int i = 0; i < cir->strength; i++)
		{
			strength++;
			// Every 5 strength level increases will increase the size of the circle
			// Size maxes out at strength level 20
			if ((strength <= 20) && (strength % 5 == 0))
			{
				radius += 0.015f;
			}
		}
	}

	void DrawCircle()
	{
		if (onoff == ON)
		{
			// Set circle color
			float color = health % 250 == 0 ? 1 : (float)(0.4 + (1 - 0.4) * ((health % 250) / 249.0));
			if (health > 1500)
			{
				glColor3d(1.0f, 0.0f, 1.0f);
			}
			else if (health > 1250)
			{
				glColor3d(color, 0.0f, color);
			}
			else if (health > 1000)
			{
				glColor3d(0.0f, 0.0f, color);
			}
			else if (health > 750)
			{
				glColor3d(0.0f, color, color);
			}
			else if (health > 500)
			{
				glColor3d(0.0f, color, 0.0f);
			}
			else if (health > 250)
			{
				glColor3d(color, color, 0.0f);
			}
			else if (health > 0)
			{
				glColor3d(color, 0.0f, 0.0f);
			}

			glBegin(GL_POLYGON);
			for (int i = 0; i < 360; i++)
			{
				float degInRad = i * DEG2RAD;
				glVertex2f((cos(degInRad) * radius) + position.x, (sin(degInRad) * radius) + position.y);
			}
			glEnd();
		}
	}
};

vector<Circle> circles;
vector<Brick> bricks;
Brick launcher(LAUNCHER, glm::vec2(0, -1), 0.2f);
float launchAngle = 0.0f;

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

	glfwSetKeyCallback(window, key_callback);

	// Create bricks in positions specified in gameGrid
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
					bricks.push_back(Brick(REFLECTIVE, glm::vec2(xPos, yPos), 0.19));
				}
				else
				{
					bricks.push_back(Brick(DESTRUCTABLE, glm::vec2(xPos, yPos), 0.19));
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

		// Move and draw circles
		for (int i = 0; i < circles.size(); i++)
		{
			if (circles[i].onoff == OFF)
			{
				circles.erase(circles.begin() + i);
			}
			else
			{
				circles[i].CheckCollision(&launcher);
				for (int x = 0; x < bricks.size(); x++)
				{
					circles[i].CheckCollision(&bricks[x]);
				}
				for (int y = 0; y < circles.size(); y++)
				{
					if (i != y &&
						circles[i].CheckCollision(&circles[y]) &&
						circles[i].strength >= circles[y].strength)
					{
						circles[i].LevelUp(&circles[y]);
						circles[y].onoff = OFF;
					}
				}
				circles[i].DrawCircle();
			}
		}

		// Draw ball launcher
		launcher.DrawBrick();

		// Draw bricks
		for (int x = 0; x < bricks.size(); x++)
		{
			if (bricks[x].onoff == OFF)
			{
				bricks.erase(bricks.begin() + x);
			}
			else
			{
				bricks[x].DrawBrick();
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		launcher.position.x -= 0.03f;
		if (launcher.position.x < (- 1.0f + (launcher.width / 2)))
		{
			launcher.position.x = -1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		launcher.position.x += 0.03f;
		if (bricks[0].position.x > (1.0f - (launcher.width / 2)))
		{
			launcher.position.x = 1.0f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		launchAngle += 0.0001f;
		if (launchAngle >= 0.0009f)
		{
			launchAngle = 0.0009f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		launchAngle -= 0.0001f;
		if (launchAngle <= -0.0009f)
		{
			launchAngle = -0.0009f;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		// velocity.x: neg goes left, pos goes right, 1 goes vertical, 9 goes horizontal
		Circle ball(glm::vec2(launcher.position.x, -1.0f), 0.05, glm::vec2(launchAngle, 0.0005f));
		circles.push_back(ball);
	}
}
