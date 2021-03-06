#include"MapleStory.h"

AirMemory AM;

DWORD Memory = 0, Memory_Start = 0, Memory_End = 0;
DWORD MSCRC1, MSCRC1_Ret;

void _declspec(naked) MSCRC1_Hook(){
	_asm{
		xor eax,eax
		add eax,edx
		mov edx,[ebp+0x18]
		sub eax,0x08
		mov eax,[edx]
		shr eax,0x08
		xor ecx,ecx
		mov ecx,eax
		shl ecx,0x08
		mov ecx,[ebp+0x08]
		add ecx,[ebp-0x38]
		xor edx,edx
		mov ebx,[ebp+0x08]
		cmp ecx,[Memory_Start]
		jb Ending
		cmp ecx,[Memory_End]
		ja Ending
		sub ecx,[Memory_Start]
		add ecx,[Memory]
Ending:
		jmp dword ptr [MSCRC1_Ret]
	}
}

DWORD HS_Init_Ret;
void _declspec(naked) HS_ManualInit(){
	_asm{
		mov dword ptr [esp+0x14],0x024A61AE
		push ebp
		mov ebp,esp
		sub esp,0x2B0
		jmp dword ptr [HS_Init_Ret]
	}
}

char MapleID[128];
void _stdcall SetMapleID(char s[]){
	int i;
	ZeroMemory(MapleID, 128);
	for(i=0; s[i]; i++){
		MapleID[i] = s[i];
	}
}

DWORD Login_Ret;
DWORD Login_Push;
void _declspec(naked) Login_Hook(){
	_asm{
		mov eax,[esp+0x08]//MapleID
		pushad
		push eax
		call SetMapleID
		popad
		push -1
		push [Login_Push]
		jmp dword ptr [Login_Ret]
	}
}

void _stdcall HWID_Randomizer(BYTE HWID[]){
	int i, MapleID_Size;
	DWORD seed = 0, seed2 = 0;

	for(i=0; MapleID[i]; i++){
		seed += MapleID[i];
		seed2 ^= MapleID[i];
	}
	MapleID_Size = i;


	for(i=0; i<0x10; i++){
		HWID[i] ^= MapleID_Size;
		HWID[i] ^= seed;
	}
	for(i=0; i<0x10; i++){
		HWID[i] ^= MapleID[i%MapleID_Size];
	}
	*(DWORD *)&HWID[0x0C] ^= seed2;
	*(DWORD *)&HWID[0x08] ^= *(DWORD *)&HWID[0x0C];
	*(DWORD *)&HWID[0x04] ^= *(DWORD *)&HWID[0x08];
	*(DWORD *)&HWID[0x00] ^= *(DWORD *)&HWID[0x04];
}

DWORD HWID_Ret;
void _declspec(naked) HWID_Hook(){
	_asm{
		push ecx
		call Create_HWID
		pop ecx
		add ecx,0x14//HIWD PLZ
		push ecx
		call HWID_Randomizer
		ret
Create_HWID:
		sub esp,0x00000628
		jmp dword ptr [HWID_Ret]
	}
}
DWORD DR_Check, ProcessListPacket, ASPLunchr, mkd25tray, autoup;
DWORD WindowsHook, HSUpdate;
DWORD HS_Init;
DWORD HWID, Login;

void Bypass(){
	AM.Init();
	AM.CreateMemoryDump();
	AM.GetDumpInfo(&Memory_Start, &Memory_End, &Memory);

	MSCRC1 = AM.AobScan("3B 8D 70 FF FF FF 0F 85 ?? ?? ?? ?? 6A 00 E9") + 6;
	MSCRC1_Ret = AM.AobScan("8A 11 80 C2 01 8B 4D 18");

	DR_Check = AM.AobScan("55 8B EC 81 EC F0 02 00 00 A1 ?? ?? ?? ?? 33 C5 89 45 FC 53 56 57 6A 00 E9");
	ProcessListPacket = AM.AobScan("6A FF 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 81 EC ?? ?? ?? ?? A1 ?? ?? ?? ?? 33 C4 89 84 24 ?? ?? ?? ?? 53 55 56 57 A1 ?? ?? ?? ?? 33 C4 50 8D 84 24 ?? ?? ?? ?? 64 A3 ?? ?? ?? ?? 8B AC 24 ?? ?? ?? ?? 33 DB 53 8B F9 E9");
	
	ASPLunchr = AM.AobScan("83 EC 0C 56 57 6A 00 8B F1 E9");
	mkd25tray = AM.AobScan("83 EC 0C 56 57 6A 00 8B F1 E9", 2);
	autoup = AM.AobScan("56 8D 71 2C 8B CE E8");

	AM.WriteHook(MSCRC1, JNE, MSCRC1_Hook);

	AM.MemoryWriter(DR_Check, "31 C0 C3");
	AM.MemoryWriter(ProcessListPacket, "31 C0 C2 04 00");

	AM.MemoryWriter(ASPLunchr, "31 C0 C3");
	AM.MemoryWriter(mkd25tray, "31 C0 C3");
	AM.MemoryWriter(autoup , "31 C0 C3");

	WindowsHook = AM.AobScan("56 8B 74 24 08 57 8B F9 8B 47 14 85 F6 74 06 85 C0 74 78 EB 04");
	HSUpdate = AM.AobScan("55 8B EC 6A FF 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 C4 ?? ?? ?? ?? 53 56 57 89 65 E8 C7 45 BC 00 00 00 00 C7 45 E4 00 00 00 00 C7 85 A8 FE FF FF 00 00 00 00 C7 45 D4 00 00 00 00 33 C0 89 45 D8 89 45 DC 89 45 E0 C7 85 18 FE FF FF 00 00 00 00 B9 10 00 00 00 33 C0 8D BD 1C FE FF FF F3 AB");

	AM.MemoryWriter(WindowsHook, "31 C0 C2 04 00");
	AM.MemoryWriter(HSUpdate, "31 C0 C2 10 00");

	HS_Init = AM.AobScan("55 8B EC 81 EC B0 02 00 00 57 C7 ?? ?? 00 00 00 00 C7 ?? ?? 00 00 00 00 C6 ?? ?? ?? ?? ?? 00");
	
	if(HS_Init){
		AM.WriteHook(HS_Init, JMP, HS_ManualInit, &HS_Init_Ret, 4);
	}

	HWID = AM.AobScan("81 EC 28 06 00 00 33 C0 56 89 44 24 ?? 8B F1 89 44 24  ?? 8D ?? ?? ?? 89 44 24 ?? 51");
	if(HWID){
		AM.WriteHook(HWID, JMP, HWID_Hook, &HWID_Ret, 1);
	}

	Login = AM.AobScan("6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC ?? A1 ?? ?? ?? ?? 33 C4 89 44 24 ?? 56 A1 ?? ?? ?? ?? 33 C4 50 8D 44 24 ?? 64 A3 00 00 00 00 8B F1 6A 00 C7 44 24 ?? 01 00 00 00 E9");
	if(Login){
		Login_Push = *(DWORD *)(Login + 3);
		AM.WriteHook(Login, JMP, Login_Hook, &Login_Ret, 2);
	}

	AM.MemoryWriter(AM.AobScan("75 ?? 6A ?? 8D 8D ?? ?? ?? ?? E8 ?? ?? ?? ?? C6 45 ?? 08 8D 4D ?? E8 ?? ?? ?? ?? 50 8D 8D ?? ?? ?? ?? E8"), "EB");
}