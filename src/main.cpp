#include "Application.h"

int main(int argc, char* argv[]) {
    
    // arguments to set the basepath and state file to avoid writing on SD very ofter
    // state file in /tmp
    if(argc == 3)
    {
        char szBasePath[255], szStateFile[255];
        strcpy(szBasePath, argv[1]);
        strcpy(szStateFile, argv[2]);
        printf("Using BasePath %s\r\n",szBasePath);
        printf("Using StateFile %s\r\n",szStateFile);
        Application app(szBasePath, szStateFile);
        app.run();
        return 0;
    }
    Application app;
    app.run();
    return 0;
}
