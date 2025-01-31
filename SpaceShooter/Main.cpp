#include "Player.h"
#include "Enemy.h"
#include "Main.h"
#include "Projectile.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <iterator>
#include <list>
#include <time.h>

#define PLAYER_COORDINATIONS_X 4
#define PLAYER_COORDINATIONS_Y 13

Player* player;
Projectile* projectile;

std::vector<std::vector<char>> main_field;

Vector2 player_coordinates;

Vector2 gun_relative_coordinates = Vector2(3, 1);

std::queue<Projectile> projectile_queue;

std::mutex mtx;

bool isGameOver = false;

std::string statistic_log = "Your statistic is:";
std::string statistic_log_kills = "Kills:";
std::string statistic_log_pass_enemy = "Pass Enemy:";
std::string gameOver_text = "======== Game Over ========";
std::string restart_text = "If you wanna restart a game press 1";

int count_kills, count_pass;
char input_restart_game;

bool Is_Game_Over()
{
	return isGameOver;
}


void Print_Statistics()
{

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	system("cls");

	for (char c : gameOver_text)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		printf("%c", c);
	}

	printf("\n");


	for (char c : statistic_log)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		printf("%c", c);
	}

	printf("\n");

	for (char c : statistic_log_kills)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		printf("%c", c);
	}
	printf("%d\n", count_kills);

	for (char c : statistic_log_pass_enemy)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		printf("%c", c);
	}

	printf("%d\n", count_pass);

	for (char c : restart_text)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		printf("%c", c);
	}

	input_restart_game = _getch();
}

void Draw_Start_Field()
{
	main_field.resize(HEIGHT);

	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			if (i == 0 || i == HEIGHT - 1)
				main_field[i].push_back('-');
			else
				main_field[i].push_back(' ');

		}
	}
}

void Re_Draw_Field()
{
	system("cls");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			printf("%c", main_field[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	printf("Lives: %d\n", g_Life);
}


void Game_Over()
{
	isGameOver = true;

	std::thread print_thread(Print_Statistics);
	print_thread.detach();
	//print_thread.join();
}

void Set_Default_Settings()
{
	count_kills = 0;
	count_pass = 0;
	player = new Player();
	Draw_Start_Field();
}

int main()
{
	std::srand(time(NULL));

	gh:
	Set_Default_Settings();
	std::thread player_thread(&Player::Spawn, player, PLAYER_COORDINATIONS_X, PLAYER_COORDINATIONS_Y);
	player_thread.detach();

	while (Is_Game_Over() == false)
	{
		Enemy* enemy = new Enemy();
		int yEnemyCoord = std::rand() % HEIGHT;
		if (yEnemyCoord == 0)
		{
			yEnemyCoord = 1;
		}
		else
		{
			if (yEnemyCoord == HEIGHT - 1 || yEnemyCoord == HEIGHT - 2 || yEnemyCoord == HEIGHT - 3)//��������� ����� ������ ����������
			{
				yEnemyCoord = HEIGHT - 4;
			}
		}
		enemy->Enemy_Spawn(Vector2(WIDTH - 4, yEnemyCoord));

	}

	input_restart_game = _getch();
	 
	if (input_restart_game == '1')
	{
		g_Life = STARTLIFE;
		isGameOver = false;
		main_field.clear();
		delete player;
		goto gh;//restart game
	}

	exit(0);
	return 0;
}

void Projectile::Projectile_Spawn(Vector2 spawn_coordinates)
{
	main_field[spawn_coordinates.Y][spawn_coordinates.X] = bullet;
	coordinates = spawn_coordinates;
	
	Projectile_Move();
}

bool Projectile::Projectile_Is_End_Of_Field()
{
	return coordinates.X + move_vector.X == WIDTH - 1;
}

void Projectile::Projectile_Move()
{
	if (Is_Game_Over() == false)
		if (Projectile_Is_End_Of_Field() || projectile == nullptr)
		{
			Destroy();
		}
		else
		{
			main_field[coordinates.Y + move_vector.Y][coordinates.X + move_vector.X] = bullet;

			main_field[coordinates.Y][coordinates.X] = ' ';

			coordinates.Y += move_vector.Y;
			coordinates.X += move_vector.X;

			Re_Draw_Field();
			Projectile_Move();
		}

}

void Projectile::Destroy()
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	main_field[coordinates.Y][coordinates.X] = ' ';
	Re_Draw_Field();

	delete this;

	player->Shoot();
}

void Player::Spawn(int x, int y)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			main_field[y + i][x + j] = player_ship[i][j];//��������� �� ���� ������� ������

	player_coordinates = Vector2(x, y);

	Re_Draw_Field();

	std::thread playerThread(&Player::Shoot, player);
	playerThread.detach();

	std::thread player_Thread(&Player::Move, player);
	player_Thread.join();
	
}


void Player::Shoot()
{
	Projectile *new_projectile = new Projectile();
	projectile = new_projectile;

	std::thread sb(&Projectile::Projectile_Spawn, new_projectile, player_coordinates + gun_relative_coordinates);
	sb.join();
}

void Player::Move()
{
	if (Is_Game_Over() == false)
	{
		char input = _getch();
		char input_array[4] = { 'w','a','s','d' };
		Vector2 move_array[4] = { Vector2(0,-1), Vector2(-1,0), Vector2(0,1), Vector2(1,0) };
		for (int i = 0; i < 4; i++)
		{
			if (input == input_array[i])
			{
				Vector2 Next = player_coordinates + move_array[i];
				if (Next.X >= 0 && Next.X < WIDTH - 2 && main_field[Next.Y][Next.X] != '-' && main_field[Next.Y + 2][Next.X] != '-')//��������� �� ����� �� ������� ����
				{
					for (int g = 0; g < 3; g++)
						for (int k = 0; k < 3; k++)//������� ���� �� ������
							main_field[player_coordinates.Y + g][player_coordinates.X + k] = ' ';

					player_coordinates.X += move_array[i].X;
					player_coordinates.Y += move_array[i].Y;

					for (int g = 0; g < 3; g++)
						for (int k = 0; k < 3; k++)//��������� �� ���� ������
							main_field[Next.Y + g][Next.X + k] = player_ship[g][k];
				}
			}
		}


		Move();
	}

}

void Enemy::Enemy_Move()
{
	if (Is_Game_Over() == false)
		if (Enemy_Is_Collide() || Enemy_Is_End_Field())
		{
			for (int j = 0; j < 3; j++)
				for (int k = 0; k < 3; k++)
					main_field[enemy_coordinates.Y + j][enemy_coordinates.X + k] = ' ';//������� ����� � ����

			delete this;
		}
		else
		{
			for (int j = 0; j < 3; j++)
				for (int k = 0; k < 3; k++)
					main_field[enemy_coordinates.Y + j][enemy_coordinates.X + k] = ' ';

			enemy_coordinates.Y += move_vector.Y;
			enemy_coordinates.X += move_vector.X;

			for (int j = 0; j < 3; j++)
				for (int k = 0; k < 3; k++)
					main_field[enemy_coordinates.Y + j][enemy_coordinates.X + k] = enemy_ship[j][k];

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			Enemy_Move();

		}
}


bool Enemy::Enemy_Is_End_Field()
{
	if (enemy_coordinates.X - 1 == 0)
	{
		count_pass++;
		return true;
	}
	else
	{
		return false;
	}
	
}

bool Enemy::Enemy_Is_Collide()
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3 && enemy_coordinates.X - k >= 0; k++)
				if (main_field[enemy_coordinates.Y + j][enemy_coordinates.X - k] == '*')//��������� ����� �� ������ ��������� � �������
				{
					projectile = nullptr;
					count_kills++;
					return true;
				}
				else
				{
					if (main_field[enemy_coordinates.Y + j][enemy_coordinates.X - k] == '@')
					{
						if (g_Life > 0)
							g_Life--;
						else
						{
							Game_Over();
						}

						count_kills++;

						return true;
					}
				}
		}
	return false;
}

void Enemy::Enemy_Spawn(Vector2 spawn_coordinates)
{
	enemy_coordinates = spawn_coordinates;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			main_field[spawn_coordinates.Y + i][spawn_coordinates.X + j] = enemy_ship[i][j];



	std::thread enemyThread(&Enemy::Enemy_Move, this);
	enemyThread.detach();

	std::this_thread::sleep_for(std::chrono::milliseconds(10000));//�������� ������ ����������
	//Enemy_Move();
}