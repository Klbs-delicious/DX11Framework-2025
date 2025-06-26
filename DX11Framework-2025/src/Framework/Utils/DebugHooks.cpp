//-----------------------------------------------------------------------------
// DebugHooks.cpp
//-----------------------------------------------------------------------------
#include "Framework/Utils/DebugHooks.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>

#pragma comment(lib, "dbghelp.lib")

/** @namespace DebugHooks
 *  @brief デバッグ用のハンドラーと例外キャッチャー群
 */
namespace DebugHooks
{
#if defined(_DEBUG)

    /** @namespace DebugHooks::Console
     *  @brief Ctrl+C や閉じるイベントのハンドリング
     */
    namespace Console
    {
        /** @brief Ctrlイベントの検出ハンドラ
         *  @param type 発生した制御イベントの種類（CTRL_C_EVENT 等）
         *  @return TRUE: ハンドル済み / FALSE: ハンドルしない
         */
        BOOL WINAPI CtrlHandler(DWORD type)
        {
            if (type == CTRL_CLOSE_EVENT)
            {
                OutputDebugString(L"[Console] CTRL_CLOSE_EVENT 検出\n");
                return TRUE;
            }
            return FALSE;
        }

        /** @brief ハンドラ登録処理
         */
        void Install()
        {
            SetConsoleCtrlHandler(CtrlHandler, TRUE);
            OutputDebugString(L"[DebugHooks] Console CtrlHandler Installed\n");
        }
    }
#endif

#if !defined(_DEBUG)

    /** @namespace DebugHooks::Crash
     *  @brief 未処理例外とミニダンプ生成
     */
    namespace Crash
    {
        /**@brief   未処理例外をキャッチしてミニダンプを出力するハンドラ
         * @param   EXCEPTION_POINTERS* ex  例外情報構造体へのポインタ（クラッシュ状況を保持）
         * @return  LONG WINAPI             常に EXCEPTION_EXECUTE_HANDLER を返して例外処理を継続する
         */
        LONG WINAPI UnhandledException(EXCEPTION_POINTERS* ex)
        {
            OutputDebugString(L"★ 未処理例外キャッチ\n");

            // ダンプ出力用ファイルを作成
            HANDLE hFile = CreateFile(
                L"crash.dmp",
                GENERIC_WRITE,
                0,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            if (hFile != INVALID_HANDLE_VALUE)
            {
                // ダンプ用の例外情報を構築
                MINIDUMP_EXCEPTION_INFORMATION mdei = {};
                mdei.ThreadId = GetCurrentThreadId();        // 現在のスレッドID
                mdei.ExceptionPointers = ex;                 // 発生した例外の情報
                mdei.ClientPointers = FALSE;

                // ミニダンプをファイルへ書き出す
                MiniDumpWriteDump(
                    GetCurrentProcess(),
                    GetCurrentProcessId(),
                    hFile,
                    MiniDumpNormal, // 出力レベル 
                    &mdei,
                    nullptr,
                    nullptr
                );

                CloseHandle(hFile);
                OutputDebugString(L"[Crash] crash.dmp を作成しました\n");
            }

            // 以降の例外処理へ制御を移す
            return EXCEPTION_EXECUTE_HANDLER;
        }

        /** @brief      未処理例外ハンドラの登録関数
         *  @details    SetUnhandledExceptionFilter を使ってアプリ全体にハンドラを適用
         */
        void Install()
        {
            SetUnhandledExceptionFilter(UnhandledException);
            OutputDebugString(L"[DebugHooks] CrashHandler Installed\n");
        }
    }
#endif
    /** @brief      デバッグ用のセットアップ
     *  @details    ビルド構成に応じて使用するハンドラを登録する
     */
    void Install()
    {
#if defined(_DEBUG)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        Console::Install();
#else
        Crash::Install();
#endif
    }
}
