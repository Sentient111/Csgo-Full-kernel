#pragma once
#include "Modules.h"
#include "Drawing.h"
#include "Mouse.h"
#include "Offsets.h"
#include "Keymap.h"


void ExitThread()
{
	Print("exiting systhem thread");
	if (isWin32Thread)
		UnspoofWin32Thread();

	PsTerminateSystemThread(STATUS_SUCCESS);
}

bool SaveWhileLoop()
{
	if (!UpdateKeyMap())
		ExitThread();

	if (!KeyDown(VK_NUMPAD5))
	{
		return true;
	}

	ExitThread();
}

Vector2 WorldToScreen(Vector3& pos, MAT4X4& g_viewMatrix)
{
	float _x = g_viewMatrix.c[0][0] * pos.x + g_viewMatrix.c[0][1] * pos.y + g_viewMatrix.c[0][2] * pos.z + g_viewMatrix.c[0][3];
	float _y = g_viewMatrix.c[1][0] * pos.x + g_viewMatrix.c[1][1] * pos.y + g_viewMatrix.c[1][2] * pos.z + g_viewMatrix.c[1][3];

	float w = g_viewMatrix.c[3][0] * pos.x + g_viewMatrix.c[3][1] * pos.y + g_viewMatrix.c[3][2] * pos.z + g_viewMatrix.c[3][3];

	if (w < 0.01f)
		return { 0,0 };


	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float x = targetWindowWidth * .5f;
	float y = targetWindowHeight * .5f;

	x += 0.5f * _x * targetWindowWidth + 0.5f;
	y -= 0.5f * _y * targetWindowHeight + 0.5f;

	return { x,y };
}

void CsgoMain()
{
	Print("csgo main");

	while (SaveWhileLoop())
	{

		if (!SpoofWin32Thread())
			continue;

		//check if cs focused
		if (!IsWindowFocused("csgo.exe"))
		{
			Print("cs not focused\n");
			Sleep(100);
			UnspoofWin32Thread();
			continue;
		}

		hdc = NtUserGetDC(0);
		if (!hdc)
		{
			Print("failed to get userdc");
			UnspoofWin32Thread();
			continue;
		}

		brush = NtGdiCreateSolidBrush(RGB(255, 0, 0), NULL);
		if (!brush)
		{
			Print("failed create brush");
			NtUserReleaseDC(hdc);
			UnspoofWin32Thread();
			continue;
		}


		DWORD localplayer = ReadMemory<DWORD>(clientBase + hazedumper::signatures::dwLocalPlayer);
		DWORD localTeam = ReadMemory<DWORD>(localplayer + hazedumper::netvars::m_iTeamNum);//fixed


		Vector3 punchAngle = ReadMemory<Vector3>(localplayer + hazedumper::netvars::m_aimPunchAngle);


		punchAngle.x = punchAngle.x * 12; punchAngle.y = punchAngle.y * 12;
		float x = targetWindowWidth / 2 - punchAngle.y;
		float y = targetWindowHeight / 2 + punchAngle.x;

		
		RECT rect = { x - 2, y - 2, x + 2, y + 2 };
		FrameRect(hdc, &rect, brush, 2);


		MAT4X4 viewMatrix = ReadMemory<MAT4X4>(clientBase + hazedumper::signatures::dwViewMatrix);

		for (size_t i = 0; i < 32; i++)
		{
			DWORD currEnt = ReadMemory<DWORD>(clientBase + hazedumper::signatures::dwEntityList + (i * 0x10));
			if (!currEnt)
				continue;

			int entHealth = ReadMemory<int>(currEnt + hazedumper::netvars::m_iHealth);
			if (0 >= entHealth)
				continue;

			DWORD dormant = ReadMemory<DWORD>(currEnt + hazedumper::signatures::m_bDormant);
			if (dormant)
				continue;

			DWORD teamNum = ReadMemory<DWORD>(currEnt + hazedumper::netvars::m_iTeamNum);
			if (teamNum == localTeam)
				continue;

			Vector3 feetPos = ReadMemory<Vector3>(currEnt + hazedumper::netvars::m_vecOrigin);
			Vector2 feetPosScreen = WorldToScreen(feetPos, viewMatrix);

			DWORD bonePtr = ReadMemory<DWORD>(currEnt + hazedumper::netvars::m_dwBoneMatrix);
			MAT3X4 boneMatrix = ReadMemory<MAT3X4>(bonePtr + 0x30 * 8); //head
			Vector3 headPos = { boneMatrix.c[0][3], boneMatrix.c[1][3], boneMatrix.c[2][3] };
			headPos.z += 8.75;
			Vector2 headScreen = WorldToScreen(headPos, viewMatrix);

			int height = headScreen.y - feetPosScreen.y;
			int width = height / 4;

			float Entity_x = feetPosScreen.x - width;
			float Entity_y = feetPosScreen.y;
			float Entity_w = height / 2;

			RECT boxEsp = { Entity_x + Entity_w, Entity_y + height, Entity_x, Entity_y };
			FrameRect(hdc, &boxEsp, brush, 1);
		}


		NtGdiDeleteObjectApp(brush);
		NtUserReleaseDC(hdc);


		UnspoofWin32Thread();
		YieldProcessor();
	}
}