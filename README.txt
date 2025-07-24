
0. Delete the build folder if needed

1. Generate build file
	cmake -S . -B out/build or cmake -B out/build -S . -DCMAKE_BUILD_TYPE=Debug

2. Build the project
	cmake --build out/build

3. Run the Application 
	Run OpenGLProject.exe located in out/build/Debug (on Windows)
	Run OpenGLProject.exe located in out/build (on Mac)
