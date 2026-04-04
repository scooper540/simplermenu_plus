#include <cstdlib>
#include "Application.h"

int main(int argc, char* argv[]) 
{
    std::string basePath;
    std::string stateFile;

    if (argc == 3) {
        basePath  = argv[1];
        stateFile = argv[2];
    } else {
        const char* envBase  = std::getenv("SMP_BASEPATH");
        const char* envState = std::getenv("SMP_STATEFILE");
        if (envBase)  basePath  = envBase;
        if (envState) stateFile = envState;
    }

    if (!basePath.empty() && !stateFile.empty()) {
        printf("BasePath  : %s\r\n", basePath.c_str());
        printf("StateFile : %s\r\n", stateFile.c_str());
        Application app(basePath, stateFile);
        app.run();
    } else {
        Application app;
        app.run();
    }

    return 0;
}
