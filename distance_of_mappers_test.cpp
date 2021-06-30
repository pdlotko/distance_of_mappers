#include "distance_of_mappers.h"

int main()
{
	//first test:
	/*
	mapper_graph mg("ex_1");
	std::vector<size_t> pts = {3,4,5,6};
	std::vector<size_t> verts = mg.were_are_those_points( pts );
	std::cerr << "Here are those points : ";
	for ( size_t i = 0 ; i != verts.size() ; ++i )cout << verts[i] << " ";
	cerr << "Now we will compute diameter of the subgraph \n";
	cerr << "Diameter is : " << mg.compute_diameter_of_subgraph_integer( verts ) << endl;
	*/
	
	//second test; with circle and
	mapper_graph circle("circle");
	mapper_graph line("not_closed_circle");
	
	//cerr << "LINE : " << endl << endl << line << endl << endl;
	
	
	cerr << "Distance between those graphs (integer): " << compute_distance_mapper_graphs_integer( line , circle ) << endl;
	cerr << "Distance between those graphs (double) : " << compute_distance_mapper_graphs_double( line , circle ) << endl;
	
	return 0;
}
