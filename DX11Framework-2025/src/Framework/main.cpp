/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/RenderSystem.h"
#include "Framework/Utils/DebugHooks.h"
#include "Framework/Core/FPS.h"

#include "Framework/Scenes/SceneFactory.h"
#include "Framework/Core/Application.h"
#include "Scenes/SceneManager.h"
#include "Scenes/TestScene.h"
#include "Scenes/TitleScene.h"


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