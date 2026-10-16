#pragma once

#include "Dependencies.h"

class Application;

class Drawer
{
    
public:
    Drawer(Application*);
    
    void CreateSyncObjects();
    
    void DrawFrame();
    
private:
    vk::raii::Semaphore present_complete_semaphore = nullptr;
    vk::raii::Semaphore render_finished_semaphore = nullptr;
    vk::raii::Fence draw_fence = nullptr;
    
    // Refs
    Application* application;
};
