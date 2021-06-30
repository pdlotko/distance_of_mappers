#include "distance_of_mappers.h"

using namespace std;

int main( int argc , char** argv )
{
	//This program compute a distance between mappers. Still under developement.
	cout << "Please provide name of two files with mapper graphs as an input.\n";
	if ( argc != 3 )
	{
		cout << "Wrong number of parameters, the program will now terminate. \n";
		return 1;
	}
	
	const char* file1 = argv[1];
	const char* file2 = argv[2];
	
	//create the mapper graphs:
	mapper_graph file1_graph(file1);
	mapper_graph file2_graph(file2);
	
	cerr << "Distance between those graphs (integer): " << compute_distance_mapper_graphs_integer( file1_graph , file2_graph ) << endl;
	cerr << "Distance between those graphs (double) : " << compute_distance_mapper_graphs_double( file1_graph , file2_graph ) << endl;
	
	return 0;
}
