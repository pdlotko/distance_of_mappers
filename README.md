# DISTANCE OF MAPPERS
This is a prototype implementation of distance of mappers, still under developement. At the moment the condition d(x,x) = 0 is not satisfied, but the code should already give a good similarity measure of mapper graphs. 
Please use the function distance_of_mappers.cpp and provide it with two files with mapper graphs. 
The format of each file is as follows:
First line contains an integer -- N number of vertices in the mapper graph.
In the following N lines, the list of points covered by the cover elements -- at the i+1 line we place the ids of points covered by the i-th vertex of the mapper graph.
The remaining lines contain list of pairs that are edges of mapper graph. 
Note that, since we use C++, enumeration starts from zero!

For instance, a file for the following graph:

# 1----0
# |    | 
# 3----2---4

where: 
vertex 0 cover points of indices 0,1,2
vertex 1 cover points of indices 0,1,3
vertex 2 cover points of indices 0,2,3,4
vertex 3 cover points of indices 1,2,3
vertex 4 cover points of indices 2,4

the corresponding file is:
5
0 1 2 
0 1 3
0 2 3 4
1 2 3
2 4
0 1
0 2
1 3
2 3 
2 4

(please note that we only require integer ids of points, we do not care what the points really are).

When you run the distance_of_mappers.cpp code with two files of the format as above, you shall get an otput with values of two similarity measures on the input graphs. 

