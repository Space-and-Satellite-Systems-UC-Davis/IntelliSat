# Prerequisites & Requirements

A list of software that must be installed before hand:

## [GNU ARM Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain)
The Toolchain can be installed in any folder of your liking. The version we are currently working with is `12.2.MPACBTI-Rel1`

## [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html#get-software)

### Installer
Scroll to the very bottom of the CubeIDE installation page, and click `Get Latest` for whichever Operating System your computer runs. A license agreement form will pop up. Click `Accept`, and you'll be prompted to enter an email. Once that is done, check that mail's inbox. There should be an email by ST. Open it and click on the `Download now` button. You should be taken back to the same installation website, except this time the browser will prompt you to download the installer (or automatically download depending on the browser settings).

### Installation
Follow this guide by ST : [Cube IDE Installation Guide](https://www.st.com/resource/en/user_manual/dm00603964-stm32cubeide-installation-guide-stmicroelectronics.pdf)
#### Starting CubeIDE
When you open CubeIDE, it'll prompty you to slect a workspace. To keep things seperate, it is recommended to make a folder somewhere on your computer just for the REALOP FSW. Then use that as the workspace. An example:

![CubeOpen](../../img/install/ide/cude_ide_open.png)

Upon clicking `Launch`, the IDE will open

### EGit
Before setting up anything else, first get Git integration in the CubeIDE. In the top panel, navigate to `Help -> Eclipse MarketPlace...`. Clicking on it opens a new window.

![Open Marketplace](../../img/install/egit/egit_market.png)
![Marketplace](../../img/install/egit/egit_marketplace.png)

There, search for `EGit`. Install EGit. The default options should be fine. Click on `Confirm`. The installation might take a minute or two.

![Egit](../../img/install/egit/egit_EGit.png)
![Egit Installer](../../img/install/egit/egit_confirm.png)

If the process stops midway requesting confirmation, select the default option and click `Confirm`

![EGit Confirm](../../img/install/egit/egit_options.png)

Once the process is over, the window will close. CubeIDE will now show that it is installing the software

![EGit Installing](../../img/install/egit/egit_installing.png)

When prompted with a window called *Trust*, check the box and click `Trust Selected`.

![EGit Trust](../../img/install/egit/egit_trust.png)

Once the installation is over, you will need be asked to restart the IDE. Click on `Restart`.

When CubeIDE opens back up, navigate to `Windows -> Show View -> Others...`

![Open View Settings](../../img/install/egit/egit_view.png)

In the new window, open the `Git` section and click on `Git Repositories`. Click `open`

![View Window](../../img/install/egit/egit_repoview.png)

The new window opens up in bottom right of the IDE.