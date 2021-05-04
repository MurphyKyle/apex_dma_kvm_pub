#define ORIGIN 1
#define STEAM 2

#define VERSION STEAM

#if VERSION == STEAM

    #define OFFSET_ENTITYLIST			0x18EDA78
    #define OFFSET_LOCAL_ENT			0x1C9D198 //LocalPlayer
    #define OFFSET_NAME_LIST            0x81342f0

    #define OFFSET_TEAM					0x448 //m_iTeamNum
    #define OFFSET_HEALTH				0x438 //m_iHealth
    #define OFFSET_SHIELD				0x170 //m_shieldHealth
    #define OFFSET_NAME					0x589 //m_iName
    #define OFFSET_SIGN_NAME			0x580 //m_iSignifierName
    #define OFFSET_ABS_VELOCITY         0x140 //m_vecAbsVelocity
    #define OFFSET_VISIBLE_TIME         0x1ACC
    #define OFFSET_ZOOMING      		0x1BE1 //m_bZooming

    #define OFFSET_LIFE_STATE			0x798  //m_lifeState, >0 = dead
    #define OFFSET_BLEED_OUT_STATE		0x2688 //m_bleedoutState, >0 = knocked

    #define OFFSET_ORIGIN				0x14c //m_vecAbsOrigin
    #define OFFSET_BONES				0xF38 //m_bConstrainBetweenEndpoints
    #define OFFSET_AIMPUNCH				0x2430 //m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
    #define OFFSET_CAMERAPOS			0x1ED0
    #define OFFSET_VIEWANGLES			0x2518 //m_ammoPoolCapacity - 0x14
    #define OFFSET_BREATH_ANGLES		OFFSET_VIEWANGLES - 0x10
    #define OFFSET_OBSERVER_MODE		0x334C //m_iObserverMode
    #define OFFSET_OBSERVING_TARGET		0x3350 //m_hObserverTarget

    #define OFFSET_MATRIX				0x1b3bd0
    #define OFFSET_RENDER				0x40BF2E8

    #define OFFSET_WEAPON				0x1A6C //m_latestPrimaryWeapons
    #define OFFSET_BULLET_SPEED         0x1ED0
    #define OFFSET_BULLET_SCALE         0x1ED8
    #define OFFSET_ZOOM_FOV             0x16F8 + 0xb8 //m_playerData + m_curZoomFOV

    #define OFFSET_ITEM_GLOW            0x2c0 //m_highlightFunctionBits

    #define OFFSET_GLOW_T1              0x262 //16256 = enabled, 0 = disabled 
    #define OFFSET_GLOW_T2              0x2dc //1193322764 = enabled, 0 = disabled 
    #define OFFSET_GLOW_ENABLE          0x3c8 //7 = enabled, 2 = disabled
    #define OFFSET_GLOW_THROUGH_WALLS   0x3d0 //2 = enabled, 5 = disabled

    #define OFFSET_GLOW_ENABLE_GLOW_CONTEXT                 OFFSET_GLOW_ENABLE // Script_Highlight_SetCurrentContext
    #define OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE     OFFSET_GLOW_THROUGH_WALLS // Script_Highlight_SetVisibilityType 5th mov
    #define GLOW_LIFE_TIME              0x3A4 // Script_Highlight_SetLifeTime + 4
    #define GLOW_DISTANCE               0x3B4 // Script_Highlight_SetFarFadeDist
    #define GLOW_TYPE                   0x2C4 // Script_Highlight_GetState + 4
    #define GLOW_COLOR                  0x1D0 // Script_CopyHighlightState 15th mov
    #define GLOW_FADE                   0x388 // Script_Highlight_GetCurrentInsideOpacity 3rd result of 3 offsets consecutive or first + 8    

#elif VERSION == ORIGIN

    #define OFFSET_ENTITYLIST			0x18db438
    #define OFFSET_LOCAL_ENT			0x1c8aa98 //LocalPlayer
    #define OFFSET_NAME_LIST            0x81342f0

    #define OFFSET_TEAM					0x450 //m_iTeamNum
    #define OFFSET_HEALTH				0x440 //m_iHealth
    #define OFFSET_SHIELD				0x170 //m_shieldHealth
    #define OFFSET_NAME					0x589 //m_iName
    #define OFFSET_SIGN_NAME			0x580 //m_iSignifierName
    #define OFFSET_ABS_VELOCITY         0x140 //m_vecAbsVelocity
    #define OFFSET_VISIBLE_TIME         0x1A4C
    #define OFFSET_ZOOMING      		0x1b41 //m_bZooming

    #define OFFSET_LIFE_STATE			0x798  //m_lifeState, >0 = dead
    #define OFFSET_BLEED_OUT_STATE		0x25e8 //m_bleedoutState, >0 = knocked

    #define OFFSET_ORIGIN				0x14c //m_vecAbsOrigin
    #define OFFSET_BONES				0xF38 //m_bConstrainBetweenEndpoints
    #define OFFSET_AIMPUNCH				0x2390 //m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
    #define OFFSET_CAMERAPOS			0x1e30
    #define OFFSET_VIEWANGLES			0x248c - 0x14 //m_ammoPoolCapacity - 0x14
    #define OFFSET_BREATH_ANGLES		OFFSET_VIEWANGLES - 0x10
    #define OFFSET_OBSERVER_MODE		0x32a4 //m_iObserverMode
    #define OFFSET_OBSERVING_TARGET		0x32a8 //m_hObserverTarget

    #define OFFSET_MATRIX				0x1b3bd0
    #define OFFSET_RENDER				0x408c968

    #define OFFSET_WEAPON				0x19ec //m_latestPrimaryWeapons
    #define OFFSET_BULLET_SPEED         0x1e50
    #define OFFSET_BULLET_SCALE         0x1e58
    #define OFFSET_ZOOM_FOV             0x1678 + 0xb8 //m_playerData + m_curZoomFOV

    #define OFFSET_ITEM_GLOW            0x2c0 //m_highlightFunctionBits

    #define OFFSET_GLOW_T1              0x262 //16256 = enabled, 0 = disabled 
    #define OFFSET_GLOW_T2              0x2dc //1193322764 = enabled, 0 = disabled 
    #define OFFSET_GLOW_ENABLE          0x3c8 //7 = enabled, 2 = disabled
    #define OFFSET_GLOW_THROUGH_WALLS   0x3d0 //2 = enabled, 5 = disabled

#endif