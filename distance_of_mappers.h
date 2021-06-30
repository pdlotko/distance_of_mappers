#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <limits>


using namespace std;



class mapper_graph
{
public:
	mapper_graph( const char* filename );	
	std::vector< size_t > were_are_those_points( const std::vector<size_t>& points ) const;
	
	size_t compute_diameter_of_subgraph_integer( const std::vector<size_t>& vertices ) const;
	double compute_diameter_of_subgraph_double( const std::vector< std::pair< size_t , double > >& vertices ) const;
	
	size_t number_of_verts()const {return this->number_of_vertices;}
	std::vector< size_t > points_covered_by_verts( size_t vert )const{return this->points_covered_by_vertices[vert];}
	
	std::vector< size_t > vertices_covering_points( std::vector<size_t> points )const;
	
	friend ostream& operator<<(ostream& out, const mapper_graph& mg)
	{
		out << "number_of_vertices : " << mg.number_of_vertices << endl;
		out << "number_of_covered_points : " << mg.number_of_covered_points << endl;
		out << "points_covered_by_vertices : \n";
		for ( size_t i = 0 ; i != mg.points_covered_by_vertices.size() ; ++i )
		{
			out << "Vertex : " << i << " and their neigs: ";
			for ( size_t j = 0 ; j != mg.points_covered_by_vertices[i].size() ; ++j )
			{
				out << mg.points_covered_by_vertices[i][j] << " ";
			}
			out << endl;
		}
		out << "Edges : \n";
		for ( size_t i = 0 ; i != mg. edges.size() ; ++i )
		{
			out << mg.edges[i].first << " , : " << mg.edges[i].second << endl;
		}
		out << "point_to_vertex_map : \n";
		for ( size_t i = 0 ; i != mg.point_to_vertex_map.size() ; ++i )
		{
			out << "Point : " << i << " is covered by ";
			for ( size_t j = 0 ; j != mg.point_to_vertex_map[i].size() ; ++j )
			{
				out << mg.point_to_vertex_map[i][j] << " ";
			}
			out << endl;
		}		
		return out;
	}
private:
	size_t number_of_vertices;
	size_t number_of_covered_points;
	std::vector< std::vector< size_t > > points_covered_by_vertices;
	std::vector< std::pair<size_t,size_t> > edges;
	std::vector< std::vector<size_t> > out_edges;
	
	bool is_connected;
	size_t diameter;
	bool check_if_graph_is_connected();
	size_t compute_diameter_of_graph();
	void compute_distance_matrix();
	
	std::vector< std::vector<size_t> > distance_matrix;
	std::vector< std::vector<size_t> > point_to_vertex_map;
	
	void set_point_to_vertex_map();
};


/**
* Here is a proposed format of a mapper file:
* Integer -- N number of vertices in the mapper graph.
* In the following N lines, the list of points covered by the cover elements -- at the i+1 line we place the ids of points covered by the i-th vertex of the mapper graph.
* The remaining lines contain list of pairs that are edges of mapper graph. 
* The procedure below read a file in such a format.
**/
mapper_graph::mapper_graph( const char* filename )
{
	bool dbg = false;
	
	//add checks if the input file format is OK.
	ifstream in;
	in.open( filename );
	
	if ( dbg )cerr << "Working on a file : " << filename << endl;
	
	in >> this->number_of_vertices;
	this->points_covered_by_vertices = std::vector< std::vector< size_t > >( this->number_of_vertices );
	std::string line;
	std::getline(in, line);
	
	if ( dbg )cerr << "Number of vertices : " << this->number_of_vertices << endl;
	
	size_t point;
	size_t maximal_point_covered = 0;
	for ( size_t i = 0 ; i != this->number_of_vertices ; ++i )
	{		
		std::getline(in, line);
		std::istringstream iss(line);
		std::vector< size_t > neighs_of_points;
		neighs_of_points.reserve( this->number_of_vertices );
		if ( dbg )cerr << "Neighs of a vertex : " << i << " : ";
		while ( iss.good() )
		{
			iss >> point;
			if ( dbg )cerr << point << " "; 
			neighs_of_points.push_back( point );
			if ( point > maximal_point_covered )maximal_point_covered = point;
		}
		if ( dbg )cerr << endl;  
		this->points_covered_by_vertices[i] = neighs_of_points;	
	}
	this->number_of_covered_points = maximal_point_covered;
	
	if ( dbg )cerr << "Here are the edges : " << endl;
	
	this->edges.reserve( this->number_of_vertices );
	size_t first, second;
	while ( in.good() )
	{
		in >> first >> second;	
		this->edges.push_back( std::make_pair( first , second ) );
		if ( dbg )cerr << "Adding an edge : " << first << " , " << second << endl;
	}
	in.close();
	
	//we still need to fill in out_edges:
	this->out_edges = std::vector< std::vector<size_t> >( this->number_of_vertices );
	for ( size_t i = 0 ; i != this->edges.size() ; ++i )
	{
		this->out_edges[ this->edges[i].first ].push_back( this->edges[i].second );
		this->out_edges[ this->edges[i].second ].push_back( this->edges[i].first );
	}
	
	this->compute_distance_matrix();
	this->diameter = this->compute_diameter_of_graph();
	this->is_connected = this->check_if_graph_is_connected();
	this->set_point_to_vertex_map();
}//mapper_graph( const char* filename )


void mapper_graph::compute_distance_matrix()
{
	bool dbg = false;
	//first initialize the structure
	this->distance_matrix = std::vector< std::vector<size_t> >( this->number_of_vertices );
	for ( size_t i = 0 ; i != this->number_of_vertices ; ++i )
	{
		this->distance_matrix[i] = std::vector<size_t>( this->number_of_vertices , std::numeric_limits<size_t>::max() );
		this->distance_matrix[i][i] = 0;
	}
	
	//now we will it in. For every vertex:
	
	for ( size_t ver = 0 ; ver != this->number_of_vertices ; ++ver )
	{
		std::vector< size_t > distance_from_ver_to_other_vertices( this->number_of_vertices , std::numeric_limits<size_t>::max() );
		
		if ( dbg )
		{
			cerr << endl << endl << "Considering shortest paths from a vertex : " << ver << endl;
			getchar();
		}
		std::vector< size_t > neighs_to_consider(1);
		neighs_to_consider[0] = ver;
		size_t distance = 1;
		distance_from_ver_to_other_vertices[ ver ] = 0;
		
		while ( !neighs_to_consider.empty() )
		{			
			if ( dbg )
			{
				cerr << "neighs_to_consider : ";
				for ( size_t hh = 0 ; hh != neighs_to_consider.size() ; ++hh )
				{
					cerr << neighs_to_consider[hh] << " ";
				}
				cerr << endl;
				getchar();
			}
			
			std::vector< size_t > new_neighs_to_consider;
			for ( size_t i = 0 ; i != neighs_to_consider.size() ; ++i )
			{
				if ( dbg )cerr << "Consider a vertex : " <<  neighs_to_consider[i] << " and their neighs : ";
				//check where we can get starting from neighs_to_consider[i]
				for ( size_t aa = 0 ; aa != this->out_edges[ neighs_to_consider[i] ].size() ; ++aa )
				{
					if ( dbg )cerr << this->out_edges[ neighs_to_consider[i] ][aa] << " ";
					size_t considered_neigh = this->out_edges[ neighs_to_consider[i] ][aa];
					if ( distance_from_ver_to_other_vertices[ considered_neigh ] == std::numeric_limits<size_t>::max() )
					{
						//we have not reached this vertex yet, so we can set up its distance from the origin:
						distance_from_ver_to_other_vertices[ considered_neigh ] = distance;
						
						if ( dbg )
						{
							cerr << "The distance between : " << ver << " and " << considered_neigh << " is " << distance << endl;
							getchar();
						}
						
						this->distance_matrix[ver][considered_neigh] = distance;
						this->distance_matrix[considered_neigh][ver] = distance;
						
						
						//now we should add all the neighs of considered_neigh to new_neighs_to_consider
						//for ( size_t bb = 0 ; bb != this->out_edges[considered_neigh].size() ; ++bb )
						//{
						//	new_neighs_to_consider.push_back( this->out_edges[considered_neigh][bb] );
						//}
						new_neighs_to_consider.push_back(considered_neigh);
					}
				}
				if ( dbg )cerr << endl;
			}
			//remove repetitions from new_neighs_to_consider:
			sort( new_neighs_to_consider.begin(), new_neighs_to_consider.end() );
			new_neighs_to_consider.erase( unique( new_neighs_to_consider.begin(), new_neighs_to_consider.end() ), new_neighs_to_consider.end() );
						
			++distance;
			neighs_to_consider = new_neighs_to_consider;
		}
	}
	
	if ( dbg )
	{
		cerr << "Here is the distance matrix :\n";
		for ( size_t i = 0 ; i != this->distance_matrix.size() ; ++i )
		{
			for ( size_t j = 0 ; j != this->distance_matrix[i].size() ; ++j )
			{
				cerr << this->distance_matrix[i][j] << " ";
			}
			cerr << endl;
		}
	}
}//mapper_graph::compute_distance_matrix()


bool mapper_graph::check_if_graph_is_connected()
{
	//check if there is any std::numeric_limits<size_t>::max() in this->distance_matrix:
	for ( size_t i = 0 ; i != this->distance_matrix.size() ; ++i )
	{
		for ( size_t j = 0 ; j != this->distance_matrix[i].size() ; ++j )
		{
			if ( this->distance_matrix[i][j] == std::numeric_limits<size_t>::max() )
			{
				//in this case, the graph is disconnected. 
				return false;
			}
		}
	}
	return true;
}//mapper_graph::check_if_graph_is_connected

size_t mapper_graph::compute_diameter_of_graph()
{
	//this is equivalent to find maximal value of this->distance_matrix that is not equial to std::numeric_limits<size_t>::max()
	size_t diam = 0;
	for ( size_t i = 0 ; i != this->distance_matrix.size() ; ++i )
	{
		for ( size_t j = 0 ; j != this->distance_matrix[i].size() ; ++j )
		{
			if ( this->distance_matrix[i][j] != std::numeric_limits<size_t>::max() )
			{
				if ( diam < this->distance_matrix[i][j] )diam = this->distance_matrix[i][j];
			}
		}
	}
	return diam;
}//mapper_graph::compute_diameter_of_graph



void mapper_graph::set_point_to_vertex_map()
{
	//first initialize the data strucutre:
	this->point_to_vertex_map = std::vector< std::vector<size_t> >( this->number_of_covered_points+1 );
	//we will work on std::vector< std::vector< size_t > > points_covered_by_vertices;
	//for every vertex of our graph:
	for ( size_t ver = 0 ; ver != this->points_covered_by_vertices.size() ; ++ver )
	{
		//for every point covered by this vertex:
		for ( size_t pt = 0 ; pt != this->points_covered_by_vertices[ver].size() ; ++pt )
		{
			this->point_to_vertex_map[ this->points_covered_by_vertices[ver][pt] ].push_back( ver );
		}
	}
}//set_point_to_vertex_map




std::vector< size_t > mapper_graph::were_are_those_points( const std::vector<size_t>& points )const
{
	std::vector< size_t > result;
	for ( size_t i = 0 ; i != points.size() ; ++i )
	{
		for ( size_t j = 0 ; j != this->point_to_vertex_map[ points[i] ].size() ; ++j )
		{
			result.push_back( this->point_to_vertex_map[ points[i] ][j] );
		}
	}
	//now remove the repetitions:
	sort( result.begin(), result.end() );
	result.erase( unique( result.begin(), result.end() ), result.end() );
	return result;
}//were_are_those_points


/**
 * This is a core procedure to compute the simplest type of a distance between mapper graphs. 
 * We start from a collection of points we want to locate in this graph. We use
 * were_are_those_points function above to find vertices that cover those points.
 * And the procedure below compute the diameter of the graph consisting of those vertices.
 **/ 
size_t mapper_graph::compute_diameter_of_subgraph_integer( const std::vector<size_t>& vertices )const
{
	//here we just need to find maximal entry in the distance matrix among those verties
	size_t diam = 0;
	for ( size_t i = 0 ; i != vertices.size() ; ++i )
	{
		for ( size_t j = i+1 ; j != vertices.size() ; ++j )
		{
			if ( diam < this->distance_matrix[ vertices[i] ][ vertices[j] ] )diam = this->distance_matrix[ vertices[i] ][ vertices[j] ];
		}
	}
	return diam;
}//compute_diameter_of_subgraph_integer


/**
 * In this procedure we not only take a list of vertices, but we have another parameter, double, that tells
 * us what is the percentate go the TOTAL mass that we need to move to another vertex. We will select a vertex
 * for which this transport is the cheapest. 
**/ 
double mapper_graph::compute_diameter_of_subgraph_double( const std::vector< std::pair< size_t , double > >& vertices )const
{
	double minimial_transport_plan = std::numeric_limits<double>::max();
	//for every vertex in the graph:
	for ( size_t vert = 0 ; vert != this->number_of_vertices ; ++vert )
	{
		//Assuming, that we want to move everything to the vertex ver, what is the cost of such a transport?
		double cost_of_a_transport_to_vert = 0;
		for ( size_t i = 0 ; i != vertices.size() ; ++i )
		{
			//what is the cost of moving vertices[i].second mass from vertices[i].first to ver? 
			//It will be the distance of vertices[i].first to ver multiplied by vertices[i].second:
			cost_of_a_transport_to_vert += vertices[i].second*( this->distance_matrix[ vert ][ vertices[i].first ] );
		}
		if ( cost_of_a_transport_to_vert < minimial_transport_plan )minimial_transport_plan = cost_of_a_transport_to_vert;
	}
	return minimial_transport_plan;
}//compute_diameter_of_subgraph_double



std::vector< size_t > mapper_graph::vertices_covering_points( std::vector<size_t> points )const
{
	std::vector< size_t > result;	
	for ( size_t i = 0 ; i != points.size() ; ++i )
	{
		//check which vertices cover points[i]:
		for ( size_t j = 0 ; j != this->point_to_vertex_map[ points[i] ].size() ; ++j )
		{
			result.push_back( this->point_to_vertex_map[ points[i] ][j] );
		}
	}	
	//remove repetitions:
	sort( result.begin(), result.end() );
	result.erase( unique( result.begin(), result.end() ), result	.end() );
	return result;
}//vertices_covering_points




size_t compute_distance_mapper_graphs_integer( const mapper_graph& first , const mapper_graph& second )
{
	bool dbg = false;
	//for every vertex of first
	size_t max_diameter_first = 0;
	for ( size_t i = 0 ; i != first.number_of_verts() ; ++i )
	{
		if ( dbg )cerr << "Vertex of the first graph : " << i << " ";
		//get the points covered by this vertex:
		std::vector< size_t > points = first.points_covered_by_verts( i );
		
		if ( dbg )
		{
			cerr << "Points covered by this vetex : \n";
			for ( size_t aa = 0 ; aa != points.size() ; ++aa )cerr << points[aa] << " ";
			cerr << endl;
		} 
		
		std::vector< size_t > where_are_those_points_in_second_graph = second.vertices_covering_points(points);
		
		if ( dbg )
		{
			cerr << "Where are those points in the second graph : ";
			for( size_t aa = 0 ; aa != where_are_those_points_in_second_graph.size() ; ++aa )
			{
				cerr << where_are_those_points_in_second_graph[aa] << " ";
			}
			cerr << endl;
		}
		
		//check where the vertex i is mapped in the second:
		size_t this_diam = second.compute_diameter_of_subgraph_integer( where_are_those_points_in_second_graph );
		if ( max_diameter_first < this_diam )max_diameter_first = this_diam;
		
		if ( dbg )
		{
			cerr <<  " and its diameter in the second : " << this_diam << endl;
			getchar();
		}
	}
	
	
	//for every vertex of second
	size_t max_diameter_second = 0;
	for ( size_t i = 0 ; i != second.number_of_verts() ; ++i )
	{
		//get the points covered by this vertex:
		std::vector< size_t > points = second.points_covered_by_verts( i );	
		std::vector< size_t > where_are_those_points_in_first_graph = first.vertices_covering_points(points);
	
		
		//check where the vertex i is mapped in the second:
		size_t this_diam = first.compute_diameter_of_subgraph_integer( where_are_those_points_in_first_graph );
		if ( max_diameter_second < this_diam )max_diameter_second = this_diam;
	}
	
	return std::max( max_diameter_first , max_diameter_second )-1;
	 
}//compute_distance_mapper_graphs_integer


















size_t compute_distance_mapper_graphs_double( const mapper_graph& first , const mapper_graph& second )
{
	bool dbg = false;
	//for every vertex of first
	double max_diameter_first = 0;
	for ( size_t i = 0 ; i != first.number_of_verts() ; ++i )
	{
		if ( dbg )cerr << "Vertex of the first graph : " << i << " ";
		//get the points covered by this vertex:
		std::vector< size_t > points = first.points_covered_by_verts( i );
		std::vector< size_t > where_are_those_points_in_second_graph = second.vertices_covering_points(points);
		
		//now we know where do those points go, but we do not know how many of them (what is their percentage). 
		//We will fix that later, at the moment, we assume equal weights;
		
		std::vector< std::pair< size_t , double > > where_are_those_points_in_second_graph_weighted( where_are_those_points_in_second_graph.size() );
		for ( size_t i = 0 ; i != where_are_those_points_in_second_graph.size() ; ++i )
		{
			//TODO!!!!
			where_are_those_points_in_second_graph_weighted[i] = std::make_pair( where_are_those_points_in_second_graph[i] , 1 );
		}
		
		//check where the vertex i is mapped in the second:
		size_t this_diam = second.compute_diameter_of_subgraph_double( where_are_those_points_in_second_graph_weighted );
		if ( max_diameter_first < this_diam )max_diameter_first = this_diam;
		
		if ( dbg )
		{	
			cerr <<  " and its diameter in the second : " << this_diam << endl;
			getchar();
		}
	}
	
	
	
	double max_diameter_second = 0;
	for ( size_t i = 0 ; i != second.number_of_verts() ; ++i )
	{
		if ( dbg )cerr << "Vertex of the second graph : " << i << " ";
		//get the points covered by this vertex:
		std::vector< size_t > points = second.points_covered_by_verts( i );
		std::vector< size_t > where_are_those_points_in_first_graph = first.vertices_covering_points(points);
		
		//now we know where do those points go, but we do not know how many of them (what is their percentage). 
		//We will fix that later, at the moment, we assume equal weights;
		
		std::vector< std::pair< size_t , double > > where_are_those_points_in_first_graph_weighted( where_are_those_points_in_first_graph.size() );
		for ( size_t i = 0 ; i != where_are_those_points_in_first_graph.size() ; ++i )
		{
			//TODO!!!!
			where_are_those_points_in_first_graph_weighted[i] = std::make_pair( where_are_those_points_in_first_graph[i] , 1 );
		}
		
		//check where the vertex i is mapped in the first:
		size_t this_diam = first.compute_diameter_of_subgraph_double( where_are_those_points_in_first_graph_weighted );
		if ( max_diameter_second < this_diam )max_diameter_second = this_diam;
			
		if ( dbg )
		{	
			cerr <<  " and its diameter in the second : " << this_diam << endl;
			getchar();
		}
	}
	
	return std::max( max_diameter_first , max_diameter_second )-1;
}//compute_distance_mapper_graphs_integer
