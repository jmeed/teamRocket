You need to create a new project in LPCXpress.
It should be an LPCOpen C++ Project.
During setup, make sure you chose the Chip (selected by default) AND dev (needs added manually) to your project.  After this the defaults should be fine.

Once the project is created, right click on it and choose Properties.
Go to C/C++ Build -> Settings -> Misc and add the -std=C++11 flag to the compiler options.

If everything was set up right then you should be able to build.
