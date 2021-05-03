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

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

FILE* dfile;

bool active = true;
uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
uintptr_t lastaimentity = 0;
float max = 999.0f;
float max_dist = 200.0f*40.0f;
int localTeamId = 0;
int tmp_spec = 0, spectators = 0;
int tmp_all_spec = 0, allied_spectators = 0;
float max_fov = 1.0f;
int toRead = 100;
int aim = 2;
int player_glow = 1;
bool item_glow = true;
bool firing_range = false;
bool target_allies = false;
bool aim_no_recoil = false;
int safe_level = 0;
bool aiming = false;
int smooth = 80;
int bone = 3;
bool walls = false;

bool actions_t = false;
bool aim_t = false;
bool vars_t = false;
bool item_t = false;
uint64_t g_Base;
uint64_t c_Base;
bool lock = false;

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


struct Matrix
{
	float matrix[16];
};

float lastvis_aim[100];

//////////////////////////////////////////////////////////////////////////////////////////////////

void SetPlayerGlow(WinProcess& mem, Entity& LPlayer, Entity& Target, int index)
{
	if (player_glow >= 1)
	{
		if(LPlayer.getPosition().z < 8000.f && Target.getPosition().z < 8000.f)
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
				
					Target.enableGlow(mem, color);
				}
			}
		}
		else if((player_glow == 0) && Target.isGlowing())
		{
			Target.disableGlow(mem);
		}
	}
}

void ProcessPlayer(WinProcess& mem, Entity& LPlayer, Entity& target, uint64_t entitylist, int index)
{
	int entity_team = target.getTeamId();
	bool obs = target.Observing(mem, entitylist);
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
			target.disableGlow(mem);
		}
		return;
	}

	if (!target.isAlive()) return;

	if (!firing_range && (entity_team < 0 || entity_team>50)) return;
	
	if (!target_allies && (entity_team == localTeamId)) return;

	if(aim==2)
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

	SetPlayerGlow(mem, LPlayer, target, index);

	lastvis_aim[index] = target.lastVisTime();
}

void DoActions(WinProcess& mem)
{
	actions_t = true;
	while (actions_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			uint64_t LocalPlayer = mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT);
			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(mem, LocalPlayer);

			localTeamId = LPlayer.getTeamId();
			if (localTeamId < 0 || localTeamId>50)
			{
				continue;
			}
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = mem.Read<uint64_t>(entitylist);
			if (baseent == 0)
			{
				continue;
			}

			max = 999.0f;
			tmp_spec = 0;
			tmp_all_spec = 0;
			tmp_aimentity = 0;
			if(firing_range)
			{
				int c=0;
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5));
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(mem, centity);
					if (!Target.isDummy() && !target_allies)
					{
						continue;
					}

					ProcessPlayer(mem, LPlayer, Target, entitylist, c);
					c++;
				}
			}
			else
			{
				for (int i = 0; i < toRead; i++)
				{
					uint64_t centity = mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5));
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(mem, centity);
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
								Target.disableGlow(mem);
							}
							continue;
						}
						break;
					case 2:
						if (spectators+allied_spectators > 0)
						{
							if(Target.isGlowing())
							{
								Target.disableGlow(mem);
							}
							continue;
						}
						break;
					default:
						break;
					}

					ProcessPlayer(mem, LPlayer, Target, entitylist, i);
				}
			}
			spectators = tmp_spec;
			allied_spectators = tmp_all_spec;
			if(!lock)
				aimentity = tmp_aimentity;
			else
				aimentity = lastaimentity;
		}
	}
	actions_t = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

player players[100];

static void AimbotLoop(WinProcess& mem)
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
				uint64_t LocalPlayer = mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT);
				if (LocalPlayer == 0) continue;
				Entity LPlayer = getEntity(mem, LocalPlayer);
				Entity target = getEntity(mem, aimentity);

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

				Vector Angles = CalculateBestBoneAim(mem, LPlayer, target, max_fov, bone, smooth, aim_no_recoil);
				if (Angles.x == 0 && Angles.y == 0)
				{
					lock=false;
					lastaimentity=0;
					continue;
				}
				LPlayer.SetViewAngles(mem, Angles);
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
	printf((aim_no_recoil ? "\t  ON\t" : "\t  OFF\t"));

	// distance
	printf("\t%d\n\n", (int)max_dist);
}

static void set_vars(WinProcess& mem, uint64_t add_addr)
{
	printf("Reading client vars...\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	//Get addresses of client vars
	uint64_t spec_addr 			= mem.Read<uint64_t>(add_addr);
	uint64_t all_spec_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t));
	uint64_t aim_addr 			= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*2);
	uint64_t safe_lev_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*3);
	uint64_t aiming_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*4);
	uint64_t g_Base_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*5);
	uint64_t max_dist_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*6);
	uint64_t item_glow_addr 	= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*7);
	uint64_t player_glow_addr 	= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*8);
	uint64_t aim_no_recoil_addr = mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*9);
	uint64_t smooth_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*10);
	uint64_t max_fov_addr 		= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*11);
	uint64_t bone_addr 			= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*12);
	uint64_t firing_range_addr 	= mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*13);
	uint64_t target_allies_addr = mem.Read<uint64_t>(add_addr + sizeof(uint64_t)*14);

	if(mem.Read<int>(spec_addr)!=1)
	{
		printf("Incorrect values read. Restart the client or check if the offset is correct. Quitting.\n");
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
			mem.Write<int>(spec_addr, spectators);
			mem.Write<int>(all_spec_addr, allied_spectators);
			mem.Write<uint64_t>(g_Base_addr, g_Base);

			aim 			= mem.Read<int>(aim_addr);
			safe_level 		= mem.Read<int>(safe_lev_addr);
			aiming 			= mem.Read<bool>(aiming_addr);
			max_dist 		= mem.Read<float>(max_dist_addr);
			item_glow 		= mem.Read<bool>(item_glow_addr);
			player_glow 	= mem.Read<int>(player_glow_addr);
			aim_no_recoil 	= mem.Read<bool>(aim_no_recoil_addr);
			smooth 			= mem.Read<int>(smooth_addr);
			max_fov 		= mem.Read<float>(max_fov_addr);
			bone 			= mem.Read<int>(bone_addr);
			firing_range	= mem.Read<bool>(firing_range_addr);
			target_allies	= mem.Read<bool>(target_allies_addr);

			
			if (nextUpdateTime < std::chrono::system_clock::now()) {
				PrintVarsToConsole();
				nextUpdateTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
			}
		}
	}
	vars_t = false;
}

static void item_glow_t(WinProcess& mem)
{
	item_t = true;
	while(item_t)
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
					uint64_t centity = mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5));
					if (centity == 0) continue;

					Item item = getItem(mem, centity);

					if(item.isItem() && !item.isGlowing())
					{
						item.enableGlow(mem);
					}
				}
				k = 1;
				std::this_thread::sleep_for(std::chrono::milliseconds(600));
				/*std::this_thread::sleep_for(std::chrono::milliseconds(300));
				if (bone == 3)
				{
			
					bone = rand() % 10;
               
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					
					bone = rand() % 10;
                
					printf("%d\t", rand()%10);
				} else
				{
				
					bone = rand() % 10+5;
              
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					bone = rand() % 10;
                 
				}*/
			} else
			{
				if(k==1)
				{
					for (int i = 0; i < 10000; i++)
					{
						uint64_t centity = mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5));
						if (centity == 0) continue;

						Item item = getItem(mem, centity);

						if(item.isItem() && item.isGlowing())
						{
							item.disableGlow(mem);
						}
					}
					k=0;
				}
			}	
		}
	}
	item_t = false;
}

__attribute__((constructor))
static void init()
{
	FILE* out = stdout;
	const char* cl_proc = "boxsecclient.exe";
	const char* ap_proc = "r5apex.exe";
	int lostClientCount = 10;

	pid_t pid;
	#if (LMODE() == MODE_EXTERNAL())
	FILE* pipe = popen("pidof qemu-system-x86_64", "r");
	fscanf(pipe, "%d", &pid);
	pclose(pipe);
	#else
	out = fopen("/tmp/testr.txt", "w");
	pid = getpid();
	#endif
	fprintf(out, "Using Mode: %s\n", TOSTRING(LMODE));

	dfile = out;

	try
	{
		printf("\nStarting client context...\n");
		WinContext ctx_client(pid);
		printf("\nStarting apex context...\n");
		WinContext ctx_apex(pid);
		printf("\nStarting refresh process list context...\n");
		WinContext ctx_refresh(pid);
		printf("\n");
		bool apex_found = false;
		bool client_found = false;
		//Client "add" offset
		uint64_t add_off = 0xABCDE;
		
		while(active)
		{
			if(!apex_found)
			{
				aim_t = false;
				actions_t = false;
				item_t = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				printf("Searching apex process...\n");
				ctx_apex.processList.Refresh();
				for (auto& i : ctx_apex.processList)
				{
					if (!strcasecmp(ap_proc, i.proc.name))
					{					
						PEB peb = i.GetPeb();
						short magic = i.Read<short>(peb.ImageBaseAddress);
						g_Base = peb.ImageBaseAddress;
						if(g_Base!=0)
						{
							apex_found = true;
							fprintf(out, "\nApex found %lx:\t%s\n", i.proc.pid, i.proc.name);
							fprintf(out, "\tBase:\t%lx\tMagic:\t%hx (valid: %hhx)\n", peb.ImageBaseAddress, magic, (char)(magic == IMAGE_DOS_SIGNATURE));
							std::thread aimbot(AimbotLoop, std::ref(i));
							std::thread actions(DoActions, std::ref(i));
							std::thread itemglow(item_glow_t, std::ref(i));
							aimbot.detach();
							actions.detach();
							itemglow.detach();
						}
					}
				}
			}

			if(!client_found)
			{
				vars_t = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				printf("Searching client process...\n");
				lostClientCount--;
				ctx_client.processList.Refresh();
				for (auto& i : ctx_client.processList)
				{
					if (!strcasecmp(cl_proc, i.proc.name))
					{	
						PEB peb = i.GetPeb();
						short magic = i.Read<short>(peb.ImageBaseAddress);
						c_Base = peb.ImageBaseAddress;
						if(c_Base!=0)
						{
							client_found = true;
							fprintf(out, "\nClient found %lx:\t%s\n", i.proc.pid, i.proc.name);
							fprintf(out, "\tBase:\t%lx\tMagic:\t%hx (valid: %hhx)\n", peb.ImageBaseAddress, magic, (char)(magic == IMAGE_DOS_SIGNATURE));
							std::thread vars(set_vars, std::ref(i), c_Base + add_off);
							vars.detach();
						}
					}
				}
			}

			if(apex_found || client_found)
			{
				apex_found = false;
				//client_found = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				ctx_refresh.processList.Refresh();
				for (auto& i : ctx_refresh.processList)
				{
					if (!strcasecmp(cl_proc, i.proc.name))
					{
						PEB peb = i.GetPeb();
						if(peb.ImageBaseAddress != 0)
						{
							if(vars_t)
								client_found = true;
						}
					}

					if (!strcasecmp(ap_proc, i.proc.name))
					{
						PEB peb = i.GetPeb();
						if(peb.ImageBaseAddress != 0)
						{
							if(actions_t)
								apex_found = true;
						}
					}
				}

				if(!apex_found && !client_found)
				{
					g_Base = 0;
					c_Base = 0;
					active = false;
				}
				else
				{
					if(!apex_found)
					{
						g_Base = 0;
					}

					if(!client_found)
					{
						c_Base = 0;
					}
				}
			}

			if (lostClientCount == 0) {
				printf("Lost the client application!");
				active = false;
			}
		}
	} catch (VMException& e)
	{
		fprintf(out, "Initialization error: %d\n", e.value);
	}
	fclose(out);
}

int main()
{
	return 0;
}
