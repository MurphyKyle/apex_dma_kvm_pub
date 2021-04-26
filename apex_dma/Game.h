#include "Math.h"
#include "offsets.h"
#include "../vmread/hlapi/hlapi.h"

#define NUM_ENT_ENTRIES			(1 << 12)
#define ENT_ENTRY_MASK			(NUM_ENT_ENTRIES - 1)

typedef struct Bone
{
	uint8_t pad1[0xCC];
	float x;
	uint8_t pad2[0xC];
	float y;
	uint8_t pad3[0xC];
	float z;
}Bone;

struct GColor 
{
    float r, g, b;
};

struct GlowMode 
{
    int8_t GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
};

struct Fade 
{
    int a, b;
    float c, d, e, f;
};

class Entity
{
public:
	uint64_t ptr;
	uint8_t buffer[0x3FF0];
	Vector getPosition();
	bool isDummy();
	bool isPlayer();
	bool isKnocked();
	bool isAlive();
	float lastVisTime();
	int getTeamId();
	int getHealth();
	int getShield();
	bool isGlowing();
	bool isZooming();
	Vector getAbsVelocity();
	Vector GetSwayAngles();
	Vector GetViewAngles();
	Vector GetCamPos();
	Vector GetRecoil();
	Vector GetViewAnglesV();

	void enableGlow(WinProcess& mem, GColor color);
	void disableGlow(WinProcess& mem);
	void SetViewAngles(WinProcess& mem, SVector angles);
	void SetViewAngles(WinProcess& mem, Vector& angles);
	Vector getBonePosition(WinProcess& mem, int id);
	bool Observing(WinProcess& mem, uint64_t entitylist);
	void get_name(WinProcess& mem, uint64_t g_Base, uint64_t index, char* name);
};

class Item
{
public:
	uint64_t ptr;
	uint8_t buffer[0x3FF0];
	Vector getPosition();
	bool isItem();
	bool isGlowing();
	
	void enableGlow(WinProcess& mem);
	void disableGlow(WinProcess& mem);
};

class WeaponXEntity
{
public:
	void update(WinProcess& mem, uint64_t LocalPlayer);
	float get_projectile_speed();
	float get_projectile_gravity();
	float get_zoom_fov();

private:
	float projectile_scale;
	float projectile_speed;
	float zoom_fov;
};

Entity getEntity(WinProcess& mem, uintptr_t ptr);
Item getItem(WinProcess& mem, uintptr_t ptr);
bool WorldToScreen(Vector from, float* m_vMatrix, int targetWidth, int targetHeight, Vector& to);
float CalculateFov(Entity& from, Entity& target);
Vector CalculateBestBoneAim(WinProcess& mem, Entity& from, Entity& target, float max_fov, int bone, int smooth, bool aim_no_recoil);
//, bool firing_range);