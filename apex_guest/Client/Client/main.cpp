#include "main.h"
#include <random>

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
int alt_aim_key = VK_LBUTTON;
bool active = true;
bool ready = false;
int spectators = 1; //write
int allied_spectators = 1; //write
bool aiming = false; //read
uint64_t g_Base = 0; //write
float max_dist = 200.0f * 40.0f; //read
float max_fov = 10.0f;
int bone = 3;

uint64_t add[15];
player players[100];

int aim = 2; //read
bool updateAim_pressed = 0;

int smooth = 80;
bool incSmooth_pressed = 0;
bool decSmooth_pressed = 0;

int safe_level = 0; //read
bool updateSafeLevel_pressed = 0;

bool item_glow = true;
bool toggleItems_pressed = 0;

int player_glow = 1;
bool updatePlayerGlow_pressed = 0;

bool aim_no_recoil = true;
bool toggleNoRecoil_pressed = 0;

bool firing_range = false;
bool toggleFiringRange_pressed = 0;

bool ally_targets = false;
bool toggleAllyTargets_pressed = 0;

bool dynamic_aim = 0;
bool dynamic_aim_pressed = 0;

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
	add[13] = (uintptr_t)&firing_range;
	add[14] = (uintptr_t)&ally_targets;

	printf(XorStr("add_addr: 0x%I64x\n"), (uint64_t)&add[0] - (uint64_t)GetModuleHandle(NULL));
	printf(XorStr("Waiting for host process...\n\n"));

	printf(XorStr(" Smooth\t\t - \t Numpad +/-\n"));
	printf(XorStr(" Aimbot\t\t - \t '0'\n"));
	printf(XorStr(" Safe Level\t - \t '9'\n"));
	printf(XorStr(" Item Glow\t - \t '8'\n"));
	printf(XorStr(" Player Glow\t - \t Numpad *\n"));
	printf(XorStr(" Firing Range\t - \t Numpad /\n"));
	printf(XorStr(" Target Allies\t - \t Numpad 7\n"));
	printf(XorStr(" No-recoil\t - \t Numpad 9\n"));
	printf(XorStr(" Max Distance\t - \t Arrows LEFT/RIGHT\n"));
	printf(XorStr(" Dynamic Aim\t - \t Numpad 8\n"));

	while (spectators == 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	if (active)
	{
		ready = true;
		printf(XorStr("Ready\n"));
	}

	while (active)
	{
		if (IsKeyDown(VK_NUMPAD9) && toggleNoRecoil_pressed == 0) // NUM-9 key
		{
			toggleNoRecoil_pressed = 1;
			aim_no_recoil = !aim_no_recoil;
		}
		else if (!IsKeyDown(VK_NUMPAD9) && toggleNoRecoil_pressed == 1) { toggleNoRecoil_pressed = 0; }

		if (IsKeyDown(VK_MULTIPLY) && updatePlayerGlow_pressed == 0) // NUM-* key
		{
			updatePlayerGlow_pressed = 1;
			switch (player_glow)
			{
			case 0:
				player_glow = 1; // 1 = on - not visible through walls
				break;
			case 1:
				player_glow = 2; // 2 = on - visible through walls 
				break;
			case 2:
				player_glow = 0; // 0 = OFF
				break;
			default:
				break;
			}
		}
		else if (!IsKeyDown(VK_MULTIPLY) && updatePlayerGlow_pressed == 1) { updatePlayerGlow_pressed = 0; }

		if (IsKeyDown(0x38) && toggleItems_pressed == 0) // 8 key
		{
			toggleItems_pressed = 1;
			item_glow = !item_glow;
		}
		else if (!IsKeyDown(0x38) && toggleItems_pressed == 1) { toggleItems_pressed = 0; }

		if (IsKeyDown(VK_NUMPAD7) && toggleAllyTargets_pressed == 0) // NUM-7 key
		{
			toggleAllyTargets_pressed = 1;
			ally_targets = !ally_targets;
		}
		else if (!IsKeyDown(VK_NUMPAD7) && toggleAllyTargets_pressed == 1) { toggleAllyTargets_pressed = 0; }

		if (IsKeyDown(VK_DIVIDE) && toggleFiringRange_pressed == 0) // NUM-/ key
		{
			toggleFiringRange_pressed = 1;
			firing_range = !firing_range;
		}
		else if (!IsKeyDown(VK_DIVIDE) && toggleFiringRange_pressed == 1) { toggleFiringRange_pressed = 0; }

		if (IsKeyDown(VK_ADD) && incSmooth_pressed == 0)
		{
			incSmooth_pressed = 1;
			smooth += 10;
			if (smooth > 80) smooth = 80;
		}
		else if (!IsKeyDown(VK_ADD) && incSmooth_pressed == 1) { incSmooth_pressed = 0; }

		if (IsKeyDown(VK_SUBTRACT) && decSmooth_pressed == 0)
		{
			decSmooth_pressed = 1;
			smooth -= 1;
			if (smooth < 1) smooth = 1;
		}
		else if (!IsKeyDown(VK_SUBTRACT) && decSmooth_pressed == 1) { decSmooth_pressed = 0; }

		if (IsKeyDown(0x30) && updateAim_pressed == 0) // 0 key
		{
			updateAim_pressed = 1;
			switch (aim)
			{
			case 0:
				aim = 1; // 1 = on - no visibility check
				break;
			case 1:
				aim = 2; // 2 = on - use visibility check
				break;
			case 2:
				aim = 0; // 0 = off
				break;
			default:
				break;
			}
		}
		else if (!IsKeyDown(0x30) && updateAim_pressed == 1) { updateAim_pressed = 0; }

		if (IsKeyDown(0x39) && updateSafeLevel_pressed == 0) // 9 key
		{
			updateSafeLevel_pressed = 1;
			switch (safe_level)
			{
			case 0:
				safe_level = 1; // 1 = disable with enemy spectators
				break;
			case 1:
				safe_level = 2; // 2 = disable with enemy OR ally spectators
				break;
			case 2:
				safe_level = 0; // 0 = keep enabled
				break;
			default:
				break;
			}
		}
		else if (!IsKeyDown(0x39) && updateSafeLevel_pressed == 1) { updateSafeLevel_pressed = 0; }

		if (IsKeyDown(VK_LEFT))
		{
			if (max_dist > 100.0f * 40.0f) // can update if over 4,000f
				max_dist -= 50.0f * 40.0f; // move by 2,000f
			std::this_thread::sleep_for(std::chrono::milliseconds(140));
		}

		if (IsKeyDown(VK_RIGHT))
		{
			if (max_dist < 800.0f * 40.0f) // can update if under 32,000f
				max_dist += 50.0f * 40.0f; // move by 2,000f
			std::this_thread::sleep_for(std::chrono::milliseconds(140));
		}

		if (IsKeyDown(aim_key) || IsKeyDown(alt_aim_key))
		{
			aiming = true;
		}
		else
		{
			aiming = false;
		}
		//dynamic bone , numpad 8 key
		if (IsKeyDown(VK_NUMPAD8) && dynamic_aim_pressed == 0)
		{
			dynamic_aim_pressed = 1;
			if (dynamic_aim == 1)
			{
				dynamic_aim = 0;
				printf("dynamic aim 0\t");
			}
			else
			{
				dynamic_aim = 1;
				printf("dynamic aim 1\t");
			}


		}
		else if (!IsKeyDown(VK_NUMPAD8) && dynamic_aim_pressed == 1)
		{
			dynamic_aim_pressed = 0;
			bone = 3;
		}

		if (dynamic_aim == 1)
		{


			bone = rand() % 10;
			//printf("%d\t", rand() % 10);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	ready = false;
	return 0;
}