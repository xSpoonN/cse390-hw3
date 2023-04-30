# CSE 390 - HW3 - Undergraduate
> Contributors:\
> James Leonardi - <james.leonardi@stonybrook.edu>\
> Kevin Tao - <kevin.tao@stonybrook.edu>

## Build Instructions
1. Navigate to `~/A3/`.
2. Run `cmake -S . -B build/` to generate Makefiles.
3. Navigate to the newly created `~/A3/build/`.
4. Run `make` to build the binaries.
5. The Simulation binary is located at `~/A3/build/Simulation/Simulation`, and the Algorithm libraries are located at `~/A3/build/Algo_.../libAlgo_X.so`

## Running the Program
To run the simulation: `Simulation [arg1] [arg2] ...`

The program supports the following arguments:
- `-house_path=<filepath>`: Specify a directory to search for .house files. Default: CWD
- `-algo_path=<filepath>`: Specify a directory to search for .so files. Default: CWD
- `-num_threads=<int>`: Specify the max number of concurrent threads to execute. Default: 10
- `-verbose`: Show a printout when a thread is started, containing the thread ID, algorithm name, and house name.

The args can be given in any order. In case of repeat args, the rightmost value is used.

## Solution Approach

### Main Algorithm
The algorithm dynamically generates a map of the house as it navigates through it, storing it as a kind of graph.
Data structures:
- unordered_set **mapped**: stores the coordinates of all the mapped tiles
- unordered_set **visited**: stores the coordinates of all the mapped tiles that the robot has already visited.
- *unordered_set **unfinished**: stores the coordinates of a visited tiled that has not been completely cleaned.*
- unordered_map **returnPath**: stores the most efficient path to return to the starting point.
- class **Node**: models a position in the house. Stores its coords, its neighbours, and its parent. Note that the coords for the node is not necessarily unique.
- struct **Position**: simply a struct of the x and y coordinates of a position.
- class **PositionHasher**: a custom hash function for the Position struct. This is used to store the Position struct in an unordered_set/unordered_map.

The start is set to a position of (0,0). Since we do not know the starting position of the robot, we put it at (0,0). This limitation means we cannot keep a vector\<vector\> of the house, as negative indexing will become a major issue. Instead, the house is modeled as a graph using **Node**s and a set of coordinates.

There are some checks that happen before the bulk of the algorithm, pertaining to returning to the charger and staying to clean dirt, these will be outlined later.

The algorithm is as follows:
Create a **choice** vector that keeps track of the valid moves. For every direction (NESW), we'll check if there's a wall there. If there is, we just disregard that direction. Otherwise, we'll add it to the **choice** vector, add it to the neighbours of the current node, and insert it into the **mapped** set. Next, we need to do some stuff to properly manage the shortest path home. 
- We get the shortest path of the position in that direction, stored in **returnPath**.
- If that path is empty or if that path is longer than the current path, we update that path to the current path + the direction to get here.
- Otherwise, if the current path is empty or the path is longer than the direction's path, we update this path to the directions path + the direction to get there.

This procedure acts as a way to roughly minimize the shortest path home. It's not perfect, but it does alleviate the issue greatly, particularly for snaking and winding paths.

Next, we need to generate a list of directions that get priority. This priority is given to positions that have not yet been visited. This is why the **visited** set is necessary. 

If there are no possible directions we can move, and the path is empty, we simply stay still. This will only happen if the charger is blocked with walls on all sides. If there are no priority directions, as in all directions are visited, we start backtracking through the **path** stack.

Otherwise, if there *is* a priority, we go in that direction. We mark that position as visited, and "move" to that path, by returning the direction and setting the current node to the node in that direction. We also append this move to the path stack, for later backtracking.

### Return Logic

The first thing we do in the algorithm at every step is check for an out of battery condition. This is done by fetching the returnPath for the current position, and checking that we wouldn't run out of battery by moving away.

*When starting to return, if the current position no longer has dirt on it, it is erased from the unfinished set. This unfinished set is used for when the robot resumes. It's to ensure the robot doesn't falsely believe it can't go resume when it actually can, just to an unfinished tile. Another addition is that at the end, when the robot realizes that it is nearing the max number of steps, it forgoes fully charging in order to squeeze out some more cleaning. Additionally, when it realizes it can't do anything more by following the resume path, it will disregard that branch and start working on other ones.*

Here, **returnQ** is used to keep a queue of directions to get back to the charger. We only set this once right when we start to return. As the robot makes its way back to the charger, we pop off the directions from the queue and return them. During this, we append the opposite direction to the **resumePath**, which keeps track of how to get back to where the robot was before it started returning. 

Once the robot reaches the charger, it will charge until it is full. After this is done, it will begin consuming the resumePath to get back to where it was previously.

Throughout this procedure we need the variable **curPos** which keeps track of the current position of the robot. This is usually an unnecessary redundancy, except for when the robot is returning to the charger. Because of the way the Node's neighbours and parents are populated, it makes it rather difficult to keep track of the current position when moving in a way that doesn't conform to the main algorithm. You might think that we can just rely on the returnPath always being correct, so that we can just keep track of the size of the returnPath to know when we are the charger. However, this is problematic due to how the returnQ is initially set. Maybe there is a workaround to this, but I found using the curPos variable to be the easiest solution.

In order to further optimize returning to the charger, we compare the sizes of **mapped** and **visited**. If these two are ever the same, it means that we have already visited all the visitable nodes, and so we can return to charger using the more optimized **returnPath** rather than the **path** stack.

There are no uses of unique_ptr in this solution. This is because of the way we are keeping track of the current node. We are using **c** to traverse the generated graph, so it is incompatible with unique_ptr.
