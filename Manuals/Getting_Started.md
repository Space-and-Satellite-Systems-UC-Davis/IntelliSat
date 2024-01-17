# Getting started with IntelliSat
If you're interested in working on IntelliSat, here's a quick guide on setting up some tools and processes.

## Using ST's CubeIDE
Currently, the FSW team uses CubeIDE for developing, flashing, and debugging the InteliiSat system. To get this setup for yourself:
1. [Install CubeIDE](./CubeIDE_instructions/Installing_CubeIDE.md). \
When selecting a workspace, it is recommended to create a new folder just for IntellISat, to keep things organized.
2. Intstall the [EGit plugin](./CubeIDE_instructions/Git_n_EGit.md) for CubeIDE
3. Clone the IntelliSat repository. Use the following credentials when cloning: \
```
URI      :  https://github.com/Space-and-Satellite-Systems-UC-Davis/IntelliSat.git
User     :  github Email
Password :  github Personal Access Token
```
4. Ensure that the [Build output](./CubeIDE_instructions/Build_Test_Debug.md) is `Binary`.
5. Switch over to the repository branch you intend to work on. Work as you'd like.
6. When done, push your changes to the IntelliSat repo using EGit.

## Developing and Testing
Once the project setup is complete and you beging work, you might want to test out the IntelliSat code onto the Orbital Platform. For this, you can either Flash the program or enter Debug mode. \
Both will require you to connect the Orbital Platform to your PC. Follow the instructions [here](./OrbitalPlatform_Hardware/STLink_Connect.md) \
Once done, follow instructions [here](./CubeIDE_instructions/Build_Test_Debug.md) on how to build and deploy code onto the Orbital Platform.