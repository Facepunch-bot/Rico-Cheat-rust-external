#include "include.hpp"
#include "math.hpp"
#include "object_loop.hpp"
#include "esp.hpp"
#include "aimbot.hpp"
#include "misc.hpp"
#include <DbgHelp.h>
#include <ShlObj_core.h>

#include "timer.hpp"

#pragma comment( lib, "ws2_32.lib" )

LONG WINAPI ExpHandler ( PEXCEPTION_POINTERS ExceptionInfo )
{
	MINIDUMP_EXCEPTION_INFORMATION M;

	M.ThreadId = GetCurrentThreadId ( );
	M.ExceptionPointers = ExceptionInfo;
	M.ClientPointers = 0;

	mist::other.release ( );
	mist::aim.release ( );
	mist::esp.release ( );
	mist::misc.release ( );
	mist::obj.release ( );
	mist::wep.release ( );

	MessageBoxA ( 0, _ ( "Crash" ), 0, 0 );

	HANDLE hFile = CreateFileA ( _("C:\\CATJPG\\crash.dmp"), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	MiniDumpWriteDump ( GetCurrentProcess ( ), GetCurrentProcessId ( ), hFile, MiniDumpNormal, &M, NULL, NULL );
	CloseHandle ( hFile );

	return EXCEPTION_EXECUTE_HANDLER;
}

void OnDetach ( )
{
	mist::other.release ( );
	mist::aim.release ( );
	mist::esp.release ( );
	mist::misc.release ( );
	mist::obj.release ( );
	mist::wep.release ( );
}

#ifdef _WINDLL
DWORD WINAPI MainThread ( PVOID )
#else
int main ( )
#endif
{
#ifdef _DEBUG
	AllocConsole ( );
	logging::print ( TYPE_LOG, "Loading..." );
#else
	LI_FN ( FreeConsole ) ( );
#endif

	logging::print ( TYPE_DEBUG, "Setting exception handlers.." );
	SetUnhandledExceptionFilter ( ExpHandler );
	std::atexit ( OnDetach );
	logging::print ( TYPE_DEBUG, "Creating memory views.." );
	mist::other = std::make_unique< CSharedMemComms > ( _ ( "Global\\7165353" ), true );
	mist::aim = std::make_unique< CSharedMemComms > ( _ ( "Global\\71696969" ), false );
	mist::esp = std::make_unique< CSharedMemComms > ( _ ( "Global\\742257" ), false );
	mist::misc = std::make_unique< CSharedMemComms > ( _ ( "Global\\1574441" ), true );
	mist::obj = std::make_unique < CSharedMemComms > ( _ ( "Global\\7545711111" ), true );
	mist::wep = std::make_unique < CSharedMemComms > ( _ ( "Global\\15646577877" ), true );

	logging::print ( TYPE_DEBUG, "Synchronizing with loader.." );

	logging::print ( TYPE_DEBUG, "Initializing communications.." );
	mist::other->Initialize ( );
	mist::aim->Initialize ( );
	mist::esp->Initialize ( );
	mist::misc->Initialize ( );
	mist::obj->Initialize ( );
	mist::wep->Initialize ( );
	Beep ( 512, 256 );
	logging::print ( TYPE_DEBUG, "Cheat loaded! Waiting for game.." );

	std::thread overlay ( esp::main );
	overlay.detach ( );

	while ( !LI_FN ( FindWindowW )( _ ( L"UnityWndClass" ), nullptr ) )
	{
		if ( GetAsyncKeyState ( VK_END ) )
		{
			mist::other.release ( );
			mist::aim.release ( );
			mist::esp.release ( );
			mist::misc.release ( );
			mist::obj.release ( );
			mist::wep.release ( );
			ExitProcess ( 0 );
		}
		sleep_for ( milliseconds ( 200 ) );
	}
	Beep ( 512, 256 );
	mist::other->attach ( h ( "RustClient.exe" ) );
	logging::print ( TYPE_DEBUG, "Game found!" );

	object::setup ( );

	std::thread keys ( misc::keybindh );
	keys.detach ( );

	std::thread objectloop ( object::main );
	objectloop.detach ( );

	logging::print ( TYPE_DEBUG, "Waiting for player to enter game.." );
	while ( game::m_rLocalInfo.m_pPlayer == 0 )
		sleep_for ( milliseconds ( 200 ) );

	logging::print ( TYPE_DEBUG, "Player entered game!" );
	std::thread aimbot ( aimbot::thread );
	aimbot.detach ( );

	std::thread misc ( misc::thread );
	misc.detach ( );

	Beep ( 500, 1250 );
	logging::print ( TYPE_DEBUG, "Have fun!" );
	
	/*
	PPEB pPeb = (PPEB)__readgsqword ( 0x60 );
	PLDR_DATA_TABLE_ENTRY tableEntry = (PLDR_DATA_TABLE_ENTRY)( pPeb->Ldr->InMemoryOrderModuleList.Flink );
	tableEntry->DllBase = (PVOID)( (INT_PTR)tableEntry->DllBase + 0x100000 );
	*/

	while ( true )
	{
		sleep_for ( 400ms );
	}
}

#ifdef _WINDLL
BOOL WINAPI DllMain ( HMODULE hDll, DWORD dwReason, PVOID )
{
	if ( dwReason == DLL_PROCESS_ATTACH )
		CreateThread ( 0, NULL, MainThread, 0, NULL, NULL );
	return true;
}
#endif
