In this exercise, you will get to explore the basics of programming on IntelliSat (by praciticng on this FSW_Testing repository instead).

# The Goal
* Learn the general structure of IntelliSat. 
* Learn how we organize new testing code
* Learn how to use some of the simplest resources in IntelliSat

# The General IntelliSat Structure
You'll notice a bunch of functions in the Training directory
* `Drivers/` is for Hardware specific header files. Here live macros to help program things on the lowest layer of IntelliSat.
* `Manuals/` is where certain IntelliSat and Orbital Platform guides are, as well as instructions for CubeIDE
* `Startup/` is where the startup script is located. It is an Assembly script that runs when the computer startups up. It is what called IntelliSat `main()`
* `Src/` is where all the source code for IntelliSat is
    * `system_config/` and `tools` is where pre-existing IntelliSat code is
    * `playground/` is where you'd work. In particular you'll work on branch_main.c

You will also notice some `.ld` files. Those are *Linker Scripts*, important for C programs to function. You'll learn about them more as you learn about C programming.

# What is `branch_main()`
Normally C programs have a `main()` function where all the code goes. But putting all the test code in the main function runs into an issue: 
- Git doesn't like when 2 people change the exact same function. It won't merge them
So if `main()` was edited by 2 separate people in separate branches with their test code, it would never merge. While there are many ways to work around this, our initial approach was:
```
1. main() will call branch_main(). This function won't be defined initially.
2. Whenever a new branch is created, the branch_main() there will be used for all sorts of testing
3. Once the branch is complete, branch_main() will be deleted prior to the merge
```

## Why the infinite loop
Another thing you'll notice is the `branch_main()` has an infinite loop:
```
void branch_main() {
	// space for initializations


	// infinite loop
	while (1) {

	}
}
```
Normally when the `main()` function of C exits, the Operating System takes over. However, in IntelliSat there is no underlying Operating System. IntelliSat is that Operating System. The `main()` function here isn't some random application's main function, it's what keeps IntelliSat running. So, the inifnite loop is there to keep IntelliSat running whatever we intend. Because once the `main()` function exits, there's a much weaker infinite loop that the computer enters (located in the startup file), which is much harder to program and control (and not recommended either).

# The Exercise
In the `branch_main()` function, write code to meet the following requirements:
- Prints a certain string whenever Button 0 is pressed
- Prints a different string whenever Button 1 is pressed
- Toggles LED 1 every 1 second
    - You should look at the `getSysTime()` function in `globals.h` (`Src/inc` folder). Use it for making a counter.

## Resources
You might wonder how you'll program any of this. Assuming you basic comfort with C, this work won't be hard. All you need is learning the resources within IntelliSat. In this case, there are functions and systems within IntelliSat that let you control the hardware (*LEDs*), or get feedback from it (*Buttons*). These are called **Human Interfaces**. Anyway, below is a link to the FSW documentation for IntelliSat, and Human Interfaces in specific.
- Human Interfaces: [IntelliSat Google Drive Documentation](https://drive.google.com/drive/folders/1zpdk_iLhg8gtlJ3CGYCjlBHBFf4T6uPW?usp=sharing)
Read these carefully, especially the Usage Guide.

As an extra detail, *Human Interfaces* are just some of the many systems within IntelliSat. If you're curious about the others, give the IntelliSat Software Architecture a look: [IntelliSat Software Architecture](https://drive.google.com/file/d/1QEXlteOZaiWoB1dDf9I8SrOeJhTq2IYE/view?usp=sharing)
