#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <random>
#include <chrono>
#include <iostream>
#include <cfloat>
#include "Game.h"
#include <thread>
#include <fcntl.h>
#include <sys/stat.h>

Memory apex_mem;
Memory client_mem;

bool DEBUG_PRINT = false;

bool active = true;
uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
uintptr_t lastaimentity = 0;
float max = 999.0f;
float max_dist = 300.0f*40.0f;	// ESP & Glow distance in meters (*40)
int localTeamId = 0;
int tmp_spec = 0, spectators = 0;
int tmp_all_spec = 0, allied_spectators = 0;
float max_fov = 5.0f;
const int toRead = 100;
int aim = 2; 					// 0 = off, 1 = on - no visibility check, 2 = on - use visibility check
int player_glow = 2;			// 0 = off, 1 = on - not visible through walls, 2 = on - visible through walls
float recoil_control = 0.50f;	// recoil reduction by this value, 1 = 100% = no recoil
Vector last_sway = Vector();	// used to determine when to reduce recoil
int last_sway_counter = 0;		// to determine if we might be shooting a semi out rifle so we need to hold to last_sway
bool item_glow = true;
bool firing_range = false;
bool target_allies = false;
int aim_no_recoil = 0;			//  0= normal recoil, 1 = use recoil control, 2 = aiming no recoil // when using recoil control , make sure the aimbot is off
int safe_level = 0;
bool aiming = false;
int smooth = 50;
int bone = 3;
bool walls = true;
bool thirdPerson = false;

bool actions_t = false;
bool aim_t = false;
bool vars_t = false;
bool item_t = false;
bool recoil_t = false;
uint64_t g_Base;
uint64_t c_Base;
bool lock = false;

//recoil control branch related
const char* printPipe = "/tmp/myfifo";	// output pipe
const char* pipeClearCmd = "\033[H\033[2J\033[3J";	// escaped 'clear' command
int shellOut = -1;

float lastvis_aim[toRead];

//////////////////////////////////////////////////////////////////////////////////////////////////

void SetPlayerGlow(Entity& LPlayer, Entity& Target, int index)
{
	if (player_glow >= 1)
	{
		if (LPlayer.getPosition().z < 8000.f && Target.getPosition().z < 8000.f)
		{
			if (!Target.isGlowing() || (int)Target.buffer[OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE] != 1 || (int)Target.buffer[GLOW_FADE] != 872415232) {
				float currentEntityTime = 5000.f;
				if (!isnan(currentEntityTime) && currentEntityTime > 0.f) {
					GColor color;
					if ((Target.getTeamId() == LPlayer.getTeamId()) && !target_allies)
					{
						color = { 0.f, 2.f, 3.f };
					}
					else if (!(firing_range) && (Target.isKnocked() || !Target.isAlive()))
					{
						color = { 3.f, 3.f, 3.f };
					}
					else if (Target.lastVisTime() > lastvis_aim[index] || (Target.lastVisTime() < 0.f && lastvis_aim[index] > 0.f))
					{
						color = { 0.f, 1.f, 0.f };
					}
					else
					{
						int shield = Target.getShield();

						if (shield > 100)
						{ //Heirloom armor - Red
							color = { 3.f, 0.f, 0.f };
						}
						else if (shield > 75)
						{ //Purple armor - Purple
							color = { 1.84f, 0.46f, 2.07f };
						}
						else if (shield > 50)
						{ //Blue armor - Light blue
							color = { 0.39f, 1.77f, 2.85f };
						}
						else if (shield > 0)
						{ //White armor - White
							color = { 2.f, 2.f, 2.f };
						}
						else if (Target.getHealth() > 50)
						{ //Above 50% HP - Orange
							color = { 3.5f, 1.8f, 0.f };
						}
						else
						{ //Below 50% HP - Light Red
							color = { 3.28f, 0.78f, 0.63f };
						}
					}

					Target.enableGlow(color);
				}
			}
		}
		else if((player_glow == 0) && Target.isGlowing())
		{
			Target.disableGlow();
		}
	}
}


void ProcessPlayer(Entity& LPlayer, Entity& target, uint64_t entitylist, int index)
{
	int entity_team = target.getTeamId();
	bool obs = target.Observing(entitylist);
	if (obs)
	{
		/*if(obs == LPlayer.ptr)
		{
			if (entity_team == localTeamId)
			{
				tmp_all_spec++;
			}
			else
			{
				tmp_spec++;
			}
		}*/
		tmp_spec++;
		return;
	}
	Vector EntityPosition = target.getPosition();
	Vector LocalPlayerPosition = LPlayer.getPosition();
	float dist = LocalPlayerPosition.DistTo(EntityPosition);
	if (dist > max_dist)
	{
		if (target.isGlowing())
		{
			target.disableGlow();
		}
		return;
	}

	if (!target.isAlive()) return;

	if (!firing_range && (entity_team < 0 || entity_team>50)) return;
	
	if (!target_allies && (entity_team == localTeamId)) return;

	if(aim == 2)
	{
		if((target.lastVisTime() > lastvis_aim[index]))
		{
			float fov = CalculateFov(LPlayer, target);
			if (fov < max)
			{
				max = fov;
				tmp_aimentity = target.ptr;
			}
		}
		else
		{
			if(aimentity==target.ptr)
			{
				aimentity=tmp_aimentity=lastaimentity=0;
			}
		}
	}
	else
	{
		float fov = CalculateFov(LPlayer, target);
		if (fov < max)
		{
			max = fov;
			tmp_aimentity = target.ptr;
		}
	}

	SetPlayerGlow(LPlayer, target, index);

	lastvis_aim[index] = target.lastVisTime();
}

void DoActions()
{
	actions_t = true;
	while (actions_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		bool tmp_thirdPerson = false;
		while (g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));	
			if (thirdPerson && !tmp_thirdPerson)
			{
				apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, 1);
				tmp_thirdPerson = true;
			}
			else if (!thirdPerson && tmp_thirdPerson)
			{
				apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, -1);
				tmp_thirdPerson = false;
			}

			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(LocalPlayer);

			localTeamId = LPlayer.getTeamId();
			if (localTeamId < 0 || localTeamId > 50)
			{
				continue;
			}
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = 0;
			apex_mem.Read<uint64_t>(entitylist, baseent);
			if (baseent == 0)
			{
				continue;
			}

			max = 999.0f;
			tmp_spec = 0;
			tmp_all_spec = 0;
			tmp_aimentity = 0;
			if (firing_range)
			{
				int c=0;
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(centity);
					if (!Target.isDummy() && !target_allies)
					{
						continue;
					}

					ProcessPlayer(LPlayer, Target, entitylist, c);
					c++;
				}
			}
			else
			{
				for (int i = 0; i < toRead; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(centity);
					if (!Target.isPlayer())
					{
						continue;
					}

					int entity_team = Target.getTeamId();
					if (!target_allies && (entity_team == localTeamId))
					{
						continue;
					}

					switch (safe_level)
					{
					case 1:
						if (spectators > 0)
						{
							if(Target.isGlowing())
							{
								Target.disableGlow();
							}
							continue;
						}
						break;
					case 2:
						if (spectators + allied_spectators > 0)
						{
							if(Target.isGlowing())
							{
								Target.disableGlow();
							}
							continue;
						}
						break;
					default:
						break;
					}

					ProcessPlayer(LPlayer, Target, entitylist, i);
				}
			}
			spectators = tmp_spec;
			allied_spectators = tmp_all_spec;
			if (!lock)
				aimentity = tmp_aimentity;
			else
				aimentity = lastaimentity;
		}
	}
	actions_t = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

static void AimbotLoop()
{
	aim_t = true;
	while (aim_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (aim>0)
			{
				switch (safe_level)
				{
				case 1:
					if (spectators > 0)
					{
						continue;
					}
					break;
				case 2:
					if (spectators+allied_spectators > 0)
					{
						continue;
					}
					break;
				default:
					break;
				}
				
				if (aimentity == 0 || !aiming)
				{
					lock=false;
					lastaimentity=0;
					continue;
				}
				lock=true;
				lastaimentity = aimentity;
				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0) continue;
				Entity LPlayer = getEntity(LocalPlayer);
				Entity target = getEntity(aimentity);

				if (firing_range)
				{
					if (!target.isAlive())
					{
						continue;
					}
				}
				else
				{
					if (!target.isAlive() || target.isKnocked())
					{
						continue;
					}
				}

				Vector Angles = CalculateBestBoneAim(LPlayer, target, max_fov, bone, smooth, aim_no_recoil);
				if (Angles.x == 0 && Angles.y == 0)
				{
					lock=false;
					lastaimentity=0;
					continue;
				}
				LPlayer.SetViewAngles(Angles);
			}
		}
	}
	aim_t = false;
}

static void PrintVarsToConsole() {
	printf("\n Spectators\t\t\t\t\t\t\t     Glow\n");
	printf("Enemy  Ally   Smooth\t   Aimbot\t     If Spectators\t Items  Players\n");

	// spectators
	printf(" %d\t%d\t", spectators, allied_spectators);

	// smooth
	printf("%d\t", smooth);

	// aim definition
	switch (aim)
	{
	case 0:
		printf("OFF\t\t\t");
		break;
	case 1:
		printf("ON - No Vis-check    ");
		break;
	case 2:
		printf("ON - Vis-check       ");
		break;
	default:
		printf("--\t\t\t");
		break;
	}

	// safe level definition
	switch (safe_level)
	{
	case 0:
		printf("Keep ON\t\t");
		break;
	case 1:
		printf("OFF with enemy\t");
		break;
	case 2:
		printf("OFF with any\t");
		break;
	default:
		printf("--\t\t");
		break;
	}
	
	// glow items + key
	printf((item_glow ? "  ON\t" : "  OFF\t"));

	// glow players + key
	switch (player_glow)
	{
	case 0:
		printf("  OFF\t");
		break;
	case 1:
		printf("ON - without walls\t");
		break;
	case 2:
		printf("ON - with walls\t");
		break;
	default:
		printf("  --\t");
		break;
	}

	// new string
	printf("\nFiring Range\tTarget Allies\tNo-recoil    Max Distance\n");

	// firing range + key
	printf((firing_range ? "   ON\t\t" : "   OFF\t\t"));

	// target allies + key
	printf((target_allies ? "   ON\t" : "   OFF\t"));

	// recoil + key
	switch (aim_no_recoil)
	{
	case 0:
		printf("\t  OFF\t");
		break;
	case 1:
		printf("\t  RCS\t");
		break;
	case 2:
		printf("\t  ON\t");
		break;
	default:
		printf("  --\t");
		break;
	}


	// distance
	printf("\t%d\n\n", (int)max_dist);
}

static void set_vars(uint64_t add_addr)
{
	printf("Reading client vars...\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	//Get addresses of client vars
	uint64_t spec_addr = 0;
	client_mem.Read<uint64_t>(add_addr, spec_addr);
	uint64_t all_spec_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t), all_spec_addr);
	uint64_t aim_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 2, aim_addr);
	uint64_t safe_lev_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 3, safe_lev_addr);
	uint64_t aiming_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 4, aiming_addr);
	uint64_t g_Base_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 5, g_Base_addr);
	uint64_t max_dist_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 6, max_dist_addr);
	uint64_t item_glow_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 7, item_glow_addr);
	uint64_t player_glow_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 8, player_glow_addr);
	uint64_t aim_no_recoil_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 9, aim_no_recoil_addr);
	uint64_t smooth_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 10, smooth_addr);
	uint64_t max_fov_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 11, max_fov_addr);
	uint64_t bone_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 12, bone_addr);
	uint64_t firing_range_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 13, firing_range_addr);
	uint64_t target_allies_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 14, target_allies_addr);
	uint64_t thirdPerson_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 15, thirdPerson_addr);

	int tmp = 0;
	client_mem.Read<int>(spec_addr, tmp);
	
	if(tmp != 1)
	{
		printf("Incorrect values read. Check if the add_off is correct. Quitting.\n");
		active = false;
		return;
	}
	vars_t = true;
	auto nextUpdateTime = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while(vars_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if(c_Base!=0 && g_Base!=0)
			printf("\nReady\n");

		while(c_Base!=0 && g_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));	
			client_mem.Write<int>(all_spec_addr, allied_spectators);
			client_mem.Write<int>(spec_addr, spectators);
			client_mem.Write<uint64_t>(g_Base_addr, g_Base);

			client_mem.Read<int>(aim_addr, aim);
			client_mem.Read<int>(safe_lev_addr, safe_level);
			client_mem.Read<bool>(aiming_addr, aiming);
			client_mem.Read<float>(max_dist_addr, max_dist);
			client_mem.Read<bool>(item_glow_addr, item_glow);
			client_mem.Read<int>(player_glow_addr, player_glow);
			client_mem.Read<int>(aim_no_recoil_addr, aim_no_recoil);
			client_mem.Read<int>(smooth_addr, smooth);
			client_mem.Read<float>(max_fov_addr, max_fov);
			client_mem.Read<int>(bone_addr, bone);
			client_mem.Read<bool>(firing_range_addr, firing_range);
			client_mem.Read<bool>(target_allies_addr, target_allies);
			client_mem.Read<bool>(thirdPerson_addr, thirdPerson);
						
			if (nextUpdateTime < std::chrono::system_clock::now()) {
				PrintVarsToConsole();
				nextUpdateTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
			}
		}
	}
	vars_t = false;
}

static void item_glow_t()
{
	item_t = true;
	while (item_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		int k = 0;
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			if (item_glow)
			{
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					Item item = getItem(centity);

					if(item.isItem() && !item.isGlowing())
					{
						item.enableGlow();
					}
				}
				k=1;
				std::this_thread::sleep_for(std::chrono::milliseconds(600));
			}
			else
			{		
				if(k==1)
				{
					for (int i = 0; i < 10000; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0) continue;

						Item item = getItem(centity);

						if(item.isItem() && item.isGlowing())
						{
							item.disableGlow();
						}
					}
					k = 0;
				}
			}
		}
	}
	item_t = false;
}

static void RecoilLoop()
{
	recoil_t = true;
	while (recoil_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(12));
			if (aim_no_recoil == 1)
			{
				uint64_t LocalPlayer = apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0) continue;
				last_sway_counter++;
				if (last_sway_counter > 10000)
					last_sway_counter = 86;
				int attackState = apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);
				if (attackState != 5) {
					if (last_sway.x != 0 && last_sway_counter > 85) {	// ~ about 1 second between shot is considered semi-auto so we keep last_sway
						last_sway.x = 0;
						last_sway.y = 0;
						last_sway_counter = 0;
					}
					continue; // is not firing
				}

				Entity LPlayer = getEntity(LocalPlayer);
				Vector ViewAngles = LPlayer.GetViewAngles();
				Vector SwayAngles = LPlayer.GetSwayAngles();

				// calculate recoil angles
				Vector recoilAngles = SwayAngles - ViewAngles;
				if (recoilAngles.x == 0 || recoilAngles.y == 0 || (recoilAngles.x - last_sway.x) == 0 || (recoilAngles.y - last_sway.y) == 0)
					continue;

				// reduce recoil angles by last recoil as sway is continous
				ViewAngles.x -= ((recoilAngles.x - last_sway.x) * recoil_control);
				ViewAngles.y -= ((recoilAngles.y - last_sway.y) * recoil_control);
				LPlayer.SetViewAngles(ViewAngles);
				last_sway = recoilAngles;
				last_sway_counter = 0;
			}
		}
	}
	recoil_t = false;
}

// Requires an open pipe
static void printToPipe(std::string msg, bool clearShell = false)
{
	char buf[80];
	if (clearShell) {
		strcpy(buf, pipeClearCmd);
		write(shellOut, buf, strlen(buf)+1);
	}
	strcpy(buf, msg.c_str());
	write(shellOut, buf, strlen(buf)+1);
}

static void DebugLoop()
{
	while (DEBUG_PRINT)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			uint64_t LocalPlayer = apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(LocalPlayer);

			int attackState = apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);
			Vector LocalCamera = LPlayer.GetCamPos();
			Vector ViewAngles = LPlayer.GetViewAngles();
			Vector SwayAngles = LPlayer.GetSwayAngles();

			uint64_t wephandle = apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_WEAPON, wephandle);
			wephandle &= 0xffff;
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			uint64_t wep_entity = apex_mem.Read<uint64_t>(entitylist + (wephandle << 5), wep_entity);
			int ammoInClip = apex_mem.Read<int>(wep_entity + OFFSET_AMMO_IN_CLIP, ammoInClip);

			printToPipe("Attack State:\t" + std::to_string(attackState) + "\n", true);
			printToPipe("Local Camera:\t" + std::to_string(LocalCamera.x) + "." + std::to_string(LocalCamera.y) + "." + std::to_string(LocalCamera.z) + "\n");
			printToPipe("View Angles: \t" + std::to_string(ViewAngles.x) + "." + std::to_string(ViewAngles.y) + "." + std::to_string(ViewAngles.z) + "\n");
			printToPipe("Sway Angles: \t" + std::to_string(SwayAngles.x) + "." + std::to_string(SwayAngles.y) + "." + std::to_string(SwayAngles.z) + "\n");
			printToPipe("Ammo Count:  \t" + std::to_string(ammoInClip)  + "\n");
		}
	}
}

int main()
{
	const char* cl_proc = "client_ap.exe";
	const char* ap_proc = "R5Apex.exe";
	//const char* ap_proc = "EasyAntiCheat_launcher.exe";

	//Client "add" offset
	uint64_t add_off = 0xABCDE;
	
	// start external terminal and open pipe to print to it
	if (DEBUG_PRINT) {
		system("gnome-terminal -- cat /tmp/myfifo");
		mkfifo(printPipe, 0666);
		shellOut = open(printPipe, O_WRONLY);
	}


	std::thread aimbot_thr;
	std::thread actions_thr;
	std::thread itemglow_thr;
	std::thread vars_thr;
	std::thread recoil_thr;
	std::thread debug_thr;
	
	while(active)
	{
		if(apex_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(aim_t)
			{
				aim_t = false;
				actions_t = false;
				item_t = false;
				recoil_t = false;
				g_Base = 0;

				aimbot_thr.~thread();
				actions_thr.~thread();
				itemglow_thr.~thread();
				recoil_thr.~thread();
				debug_thr.~thread();
				
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for apex process...\n");

			apex_mem.open_proc(ap_proc);

			if(apex_mem.get_proc_status() == process_status::FOUND_READY)
			{
				g_Base = apex_mem.get_proc_baseaddr();
				printf("\nApex process found\n");
				printf("Base: %lx\n", g_Base);

				aimbot_thr = std::thread(AimbotLoop);
				actions_thr = std::thread(DoActions);
				itemglow_thr = std::thread(item_glow_t);
				recoil_thr = std::thread(RecoilLoop);

				if (DEBUG_PRINT)
				{
					debug_thr = std::thread(DebugLoop);
					debug_thr.detach();
				}

				aimbot_thr.detach();
				actions_thr.detach();
				itemglow_thr.detach();
				recoil_thr.detach();
			}
		}
		else
		{
			apex_mem.check_proc();
		}

		if(client_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(vars_t)
			{
				vars_t = false;
				c_Base = 0;

				vars_thr.~thread();
			}
			
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for client process...\n");

			client_mem.open_proc(cl_proc);

			if(client_mem.get_proc_status() == process_status::FOUND_READY)
			{
				c_Base = client_mem.get_proc_baseaddr();
				printf("\nClient process found\n");
				printf("Base: %lx\n", c_Base);

				vars_thr = std::thread(set_vars, c_Base + add_off);
				vars_thr.detach();
			}
		}
		else
		{
			client_mem.check_proc();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}