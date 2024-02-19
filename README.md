# ESD Group Project

Hi team, welcome to the repository for our ESD group project. This is where we'll keep all of the code for the stopwatch group project, but I also intend to post solutions to the labs in case you want to check against your own.

# Getting code into Code Composer Studio

First of all, download this repository. If you don't have ssh authentication set up, you can just download it the files as a zip like this:

![image](https://github.com/cboswel/stopwatch/assets/71510545/8939e41f-deef-4182-a772-3410369f3d82)

Then you will just need to import the code into CCS. Make a new CCS project:
![image](https://github.com/cboswel/stopwatch/assets/71510545/61a71969-979e-4d9a-845c-0f434762d2f0)

And add the files you just downloaded:
![image](https://github.com/cboswel/stopwatch/assets/71510545/4285ec3c-ac32-464f-9ac0-84ec38ae937c)

Finally, don't forget to turn off optimisation!!
![image](https://github.com/cboswel/stopwatch/assets/71510545/39f77cf7-bf11-432d-988a-59193b09ddb0)

# Code structure

You will notice I've split the code into several files. I think this will make everything more readable when the project grows more complicated. You should find:
stopwatch.c - this is where the main() function lives and the interrupt service routines.
stopwatch.h - this is for #defines, structs, global variable declarations and function prototypes
functions.c - this is where we write all the functions that will be called in the main loop. Maybe we will split this into more files later if it gets huge.
