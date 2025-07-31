// comp3016.cpp: Define the entry point for the application.
//

#include "comp3016.h"

using namespace std;

int main()
{
    try {
        // Create application instance
        Application app(1200, 800, "COMP3016 - OpenGL 3D Scene with Signature");
        
        // Initialize application
        if (!app.Initialize()) {
            cerr << "Failed to initialize application!" << endl;
            return -1;
        }
        
        cout << "=== COMP3016 OpenGL Application ===" << endl;
        cout << "Controls:" << endl;
        cout << "WASD - Move camera" << endl;
        cout << "Mouse - Look around" << endl;
        cout << "Mouse Scroll - Zoom" << endl;
        cout << "ESC - Exit" << endl;
        cout << "===================================" << endl;
        
        // Run application main loop
        app.Run();
        
        cout << "Application terminated successfully." << endl;
        return 0;
    }
    catch (const exception& e) {
        cerr << "Application error: " << e.what() << endl;
        return -1;
    }
}
