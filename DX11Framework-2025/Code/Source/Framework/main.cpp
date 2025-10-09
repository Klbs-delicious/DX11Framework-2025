/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Core/WindowSystem.h"
#include"Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Utils/DebugHooks.h"
#include "Include/Framework/Core/FPS.h"

#include "Include/Framework/Scenes/SceneFactory.h"
#include "Include/Framework/Core/Application.h"
#include "Include/Scenes/SceneManager.h"
#include "Include/Scenes/TestScene.h"
#include "Include/Scenes/TitleScene.h"

#include<iostream>
#include <Windows.h> // timeBeginPeriod 用
#pragma comment(lib, "Winmm.lib")
//-----------------------------------------------------------------------------
// EntryPoint
//-----------------------------------------------------------------------------
int main()
{
    Application::AppConfig config = {
        720,
        400,
    };

    Application application(config);
    application.Run();
    return 0;
}