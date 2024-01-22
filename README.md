# CPPND: Capstone Snake Game

## 1. Dependencies for Running Locally
* cmake >= 3.7
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* SDL2 >= 2.0
  * All installation instructions can be found [here](https://wiki.libsdl.org/Installation)
  >Note that for Linux, an `apt` or `apt-get` installation is preferred to building from source. 
* gcc/g++ >= 8.0
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## 2. Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./SnakeGame`.

## 3. New Feature Added

In summary, the new feature I've added is to keep track of the top score, display it to the user during each game and ensure that it persists between games.

The expected behaviour of the new feature is as follows:

- During each game, the top score is displayed alongside the current score and FPS.
- When the game is completely quit and restarted, the top score will persist.
- An additional message will be displayed in the console when the game quits telling the user whether they set a new top score.


## 4. Address Rubric Points

### 4.1 Loops, Functions, I/O

**i. The project demonstrates an understanding of C++ functions and control structures. A variety of control structures are added to the project. The project code is clearly organized into functions.**

Most of the new logic I've added is within the `game_data.cpp` and `game_data.h` files. The `GameData` class represents any game data that is persisted between games. During this project I have added the top score to this, but it could easily be extended to include other data points e.g. name of whoever set the current top score.

Within this class there are a number of clearly organised and appropriately named functions and control structures including loops, conditionals and invariants.

**ii. The project reads data from a file and process the data, or the program writes data to a file. The project reads data from an external file or writes data to a file as part of the necessary operation of the program.**

In order to enable the game data to be persisted between games, it uses a txt file (`game_data.txt`) to store and retrieve this data. The file is read in at the start of the game (if it exists) and only writes to the file when there are changes to the data that aren't already reflected in the file. The `GameData::HydrateFromFile` method at `game_data.cpp:59` reads from the file and hydrates class attributes with the values stored in the file and the `GameData::Save` method at `game_data.cpp:40` handles saving the data stored in the class attributes to the file. 

### 4.2 Object Oriented Programming

**i. One or more classes are added to the project with appropriate access specifiers for class members. Classes are organized with attributes to hold data and methods to perform tasks. All class data members are explicitly specified as public, protected, or private. Member data that is subject to an invariant is hidden from the user and accessed via member methods.**

As mentioned above, the `GameData` class has been added to the project to handle persisting game data between games. A number of attributes have been added to hold data e.g. the top score, the path to the file that the data will be saved to as well as several others for allowing the data to be correctly saved from another thread e.g. thread, mutex, condition variable etc.

All data members are explicitly specified as public or private. All attributes have been marked as private and public methods are exposed to interact with the class's data from outside the class including attributes that are subject to an invariant (see the `_topScore` attribute and the `SetTopScore` method at `game_data.cpp:28`).

**ii. Class constructors utilize member initialization lists. All class members that are set to argument values are initialized through member initialization lists.**

The `GameData` constructor at `game_data.cpp:11` shows that all class members that are set to argument values (or hardcoded default values) are initialized using a member initialization list.

**iii. Classes abstract implementation details from their interfaces. All class member functions document their effects, either through function names, comments, or formal documentation. Member functions do not change the program state in undocumented ways.**

The `GameData` class hides all of its implementation details by making almost all of its attributes and methods private. The only methods exposed outside the class are `GetTopScore` at `game_data.cpp:24` and `SetTopScore` at `game_data.cpp:28`. All the implementation details of where the values are stored between games and all the logic of how and when the external file is updated is hidden within the class, and therefore it is not necessary for anyone using the class to understand the inner workings of it.

All attributes and methods of the `GameData` class have been carefully named to describe their purpose and intended function as accurately as possible without being overly verbose. I have also added a leading `_` to all private attributes to clearly indicate that these are not accessible outside the class.

### 4.3 Memory Management

**i. The project uses scope / Resource Acquisition Is Initialization (RAII) where appropriate. The project follows the Resource Acquisition Is Initialization pattern where appropriate, by allocating objects at compile-time, initializing objects when they are declared, and utilizing scope to ensure their automatic destruction.**

The `GameData` class uses a separate thread to handle writing its data to an external file. This thread is initialised within the constructor of the class and the destructor handles joining it back to the main thread to ensure that it is not abandoned when the application is closed or when the object goes out of scope.

**ii. The project uses destructors appropriately. At least one class that uses unmanaged dynamically allocated memory, along with any class that otherwise needs to modify state upon the termination of an object, uses a destructor.**

As described above, because the `GameData` class needs to modify its state (the `_saveThread` attribute) when the class instance is destroyed, the destructor is implemented appropriately to ensure that the thread it initialised is correctly joined back to the main thread before proceeding.

**iii. The project uses smart pointers instead of raw pointers. The project uses at least one smart pointer: unique_ptr, shared_ptr, or weak_ptr.**

The `Game` class constructor has been modified to receive a shared pointer to a `GameData` instance. A shared pointer is used to guarantee that the `Game` instance will continue to receive access to the `GameData` class instance even if it goes out of scope in the context in which it was initialised (as the `GameData` instance is now essential for the proper running of the game).

**iv. The project uses move semantics to move data instead of copying it, where possible. The project relies on the move semantics, instead of copying the object.**

In the `Game` class constructor (see `game.cpp:9`), move semantics are used when initialising the `gameData` attribute.

### 4.4 Concurrency

**i. The project uses multithreading. The project uses multiple threads or async tasks in the execution.**

The changes introduced with the `GameData` class include writes to the file system which are inherently expensive. To prevent these write operations from interfering with the game itself, the `GameData` class initialises another thread to handle them (see `game_data.cpp:13`).

**ii. A mutex or lock is used in the project. A mutex or lock (e.g. std::lock_guard or `std::unique_lock) is used to protect data that is shared across multiple threads in the project code.**

The separate thread used by the `GameData` class for file write operations requires access to some of the attributes used by the main thread, a mutex is used to avoid data races (see `game_data.cpp:29` and `game_data.cpp:49`). This mutex is also used with a lock guard to ensure that a deadlock is avoided if any unexpected or unhandled errors occur at runtime.

## CC Attribution-ShareAlike 4.0 International


Shield: [![CC BY-SA 4.0][cc-by-sa-shield]][cc-by-sa]

This work is licensed under a
[Creative Commons Attribution-ShareAlike 4.0 International License][cc-by-sa].

[![CC BY-SA 4.0][cc-by-sa-image]][cc-by-sa]

[cc-by-sa]: http://creativecommons.org/licenses/by-sa/4.0/
[cc-by-sa-image]: https://licensebuttons.net/l/by-sa/4.0/88x31.png
[cc-by-sa-shield]: https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg
