#include "main.h"

typedef struct player
{
	float dist = 0;
	int entity_team = 0;
	float boxMiddle = 0;
	float h_y = 0;
	float width = 0;
	float height = 0;
	float b_x = 0;
	float b_y = 0;
	bool knocked = false;
	bool visible = false;
	int health = 0;
	int shield = 0;
	char name[33] = { 0 };
}player;

int aim_key = VK_RBUTTON;
bool active = true;
bool ready = false;
int spectators = 1; //write
int allied_spectators = 1; //write
int aim = 0; //read
int safe_level = 0; //read
bool item_glow = false;
bool player_glow = false;
bool aim_no_recoil = true;
bool aiming = false; //read
uint64_t g_Base = 0; //write
float max_dist = 200.0f*40.0f; //read
float smooth = 12.0f;
float max_fov = 15.0f;
int bone = 2;

uint64_t add[13];
player players[100];

bool k_f5 = 0;
bool k_f6 = 0;
bool k_f7 = 0;
bool k_f8 = 0;

bool IsKeyDown(int vk)
{
	return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

int main(int argc, char** argv)
{
	add[0] = (uintptr_t)&spectators;
	add[1] = (uintptr_t)&allied_spectators;
	add[2] = (uintptr_t)&aim;
	add[3] = (uintptr_t)&safe_level;
	add[4] = (uintptr_t)&aiming;
	add[5] = (uintptr_t)&g_Base;
	add[6] = (uintptr_t)&max_dist;
	add[7] = (uintptr_t)&item_glow;
	add[8] = (uintptr_t)&player_glow;
	add[9] = (uintptr_t)&aim_no_recoil;
	add[10] = (uintptr_t)&smooth;
	add[11] = (uintptr_t)&max_fov;
	add[12] = (uintptr_t)&bone;
	printf(XorStr("add_addr: 0x%I64x\n"), (uint64_t)&add[0] - (uint64_t)GetModuleHandle(NULL));
	printf(XorStr("Waiting for host process...\n"));
	while (spectators == 1)
	{
		if (IsKeyDown(VK_F4))
		{
			active = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	if (active)
	{
		ready = true;
		printf(XorStr("Ready\n"));
	}
		
	while (active)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (IsKeyDown(VK_F4))
		{
			active = false;
		}

		if (IsKeyDown(VK_F5) && k_f5 == 0)
		{
			k_f5 = 1;
			esp = !esp;
		}
		else if (!IsKeyDown(VK_F5) && k_f5 == 1)
		{
			k_f5 = 0;
		}

		if (IsKeyDown(VK_F6) && k_f6 == 0)
		{
			k_f6 = 1;
			switch (aim)
			{
			case 0:
				aim = 1;
				break;
			case 1:
				aim = 2;
				break;
			case 2:
				aim = 0;
				break;
			default:
				break;
			}
		}
		else if (!IsKeyDown(VK_F6) && k_f6 == 1)
		{
			k_f6 = 0;
		}

		if (IsKeyDown(VK_F7) && k_f7 == 0)
		{
			k_f7 = 1;
			switch (safe_level)
			{
			case 0:
				safe_level = 1;
				break;
			case 1:
				safe_level = 2;
				break;
			case 2:
				safe_level = 0;
				break;
			default:
				break;
			}
		}
		else if (!IsKeyDown(VK_F7) && k_f7 == 1)
		{
			k_f7 = 0;
		}

		if (IsKeyDown(VK_F8) && k_f8 == 0)
		{
			k_f8 = 1;
			item_glow = !item_glow;
		}
		else if (!IsKeyDown(VK_F8) && k_f8 == 1)
		{
			k_f8 = 0;
		}

		if (IsKeyDown(VK_LEFT))
		{
			if (max_dist > 100.0f * 40.0f)
				max_dist -= 50.0f * 40.0f;
			std::this_thread::sleep_for(std::chrono::milliseconds(130));
		}

		if (IsKeyDown(VK_RIGHT))
		{
			if (max_dist < 800.0f * 40.0f)
				max_dist += 50.0f * 40.0f;
			std::this_thread::sleep_for(std::chrono::milliseconds(130));
		}

		if (IsKeyDown(aim_key))
			aiming = true;
		else
			aiming = false;
	}
	ready = false;
	return 0;
}