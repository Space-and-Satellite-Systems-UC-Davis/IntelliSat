# Setting up the Intellisat project in CubeIDE

## Cloning the Intellisat repository

Click on `Clone a Git Repository`. A new window will open

![Cloning Repo](../../img/cloning/egit_cloning.png)

Fill it out with the following info:
```
URI      :  https://github.com/Space-and-Satellite-Systems-UC-Davis/IntelliSat.git
User     :  github Email
Password :  github Personal Access Token
```
The rest of the necessary fields will fill up by themselved. Leave the others as is. Click `Next`. The window will display a list of branches to clone. Select whichever ones you want, and then click `Next`. You'll be asked where to clone the repo. Click on `Browse` and navigate to the location of the workplace, and select that folder.

![Where to Clone](../../img/cloning/egit_clonedest.png)

Click `Finish`. After some time, the Git Repo window will show the Intellisat repo.

![Intellisat in Repos](../../img/cloning/egit_repoIntellisat.png)

The code can't be built and tested yet. To allow for that, go the the `Project Explorer` and click on `Import Projects...`. In the *Import* window, click on `Existing Projects into Workspace` under the `General` section.

![Import window](../../img/cloning/egit_import.png)

In the next window, select the `Select root directory` option. Make sure the directory is the workspace directory, where the repo from earlier was cloned. *Intellisat* must be visible as a project. Make sure it's selected, then click `Finish`. When done, the Intellisat project will be visible in the Projects window.

![Final Import Step](../../img/cloning/egit_importFinal.png)
![Intellisat in Projects](../../img/cloning/project_intellisat.png)

## IntelliSat Project Properties
**NOTE**: The following settings should already be configured through the *.cprojects* file, but still check them to be on the safer side.

With the Intellisat project selected in the *Projects* window, press `Alt + Enter`. A properties window will open. Navigate to `C/C++ Build -> Settings`. Make the following changes:

![Build Outputs](../../img/project_settings/projects_output.png)
![Build Includes](../../img/project_settings/projects_include.png)

Click `Apply and Close`. Now the project can be worked on.