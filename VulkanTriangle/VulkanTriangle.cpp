#include "Application.h"

int main()
{
    Application application;

    application.Init();
    application.Run();
    application.Destroy();

    return 0;
}
