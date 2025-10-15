﻿/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Core/Application.h"

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