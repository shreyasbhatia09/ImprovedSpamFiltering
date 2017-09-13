// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#define path1 "C:\\Users\\Shreyas\\Desktop\\Hits\\computational_geometry\\expanded\\nodes"
#define path2 "C:\\Users\\Shreyas\\Desktop\\Hits\\computational_geometry\\expanded\\adj_matrix"
using namespace std;

// Helper function will calculate ceiling value of a / b
int ciel(int a, int b)
{
	if (a%b == 0)
		return a / b;
	else
		return 1 + (a / b);
}
////////////////////////////////// KERNEL //////////////////////////////////////////////




void hubKernel(int Node, int matrix[], float hub[], float authority[], float *norm)
{
	for (int i = 0; i < Node; i++)
	{
		hub[i] = 0;
		for (int j = 0; j < Node; j++)
		{
			if (i == j) continue;
			hub[i] += matrix[i*Node + j] ? authority[j] : 0;
		}
		*norm += (hub[i] * hub[i]);
	}
}

void authorityKernel(int Node, int matrix[], float hub[], float authority[], float *norm)
{

	for (int j = 0; j < Node; j++)
	{
		authority[j] = 0;
		for (int i = 0; i < Node; i++)
		{
			if (i == j) continue;
			authority[j] += matrix[i*Node + j] ? hub[i] : 0;
		}
		*norm += (authority[j] * authority[j]);
	}

}
void Normalize(float div, float arr[], int Node)
{
	for (int i = 0; i < Node; i++)
	{
		arr[i] = arr[i] / div;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

class node
{
public:
	int id;
	string url;
	string title;
};
struct node2
{
	int index;
	float farr;
};
int comparison(struct node2 a, struct node2 b)
{
	if (a.farr > b.farr)
		return 1;
	return 0;
}

class displayValue
{
	int num_node;
	string path;
	node *node_arr;
	void nodeAlloc(string);
public:
	displayValue(string p)
	{
		path = p;
		nodeAlloc(path);
	}
	void Reader();
	void sortFile(float auth[]);
	void write(struct node2 arr[]);

};

void displayValue::sortFile(float auth[])
{
	struct node2 *arr = (struct node2 *) malloc(num_node*sizeof(node2));
	for (int i = 0; i < num_node; i++)
	{
		arr[i].index = i;
		arr[i].farr = auth[i];
	}
	sort(arr, arr + num_node, comparison);
	write(arr);
}
void displayValue::nodeAlloc(string path)
{
	ifstream nodefile;
	nodefile.open(path1);
	nodefile >> num_node;
	node_arr = new node[num_node];
	nodefile.close();
}
void displayValue::Reader()
{
	ifstream nodefile;
	nodefile.open(path1);
	string garbage;
	nodefile >> garbage;

	for (int i = 0; i < num_node; i++)
	{
		garbage = "";
		nodefile >> node_arr[i].id;
		getline(nodefile, garbage);
		string url, title;
		getline(nodefile, url);
		node_arr[i].url = url;
		getline(nodefile, title);
		node_arr[i].title = title;
		getline(nodefile, garbage);

	}
	cout << "Node File Read" << endl;
	nodefile.close();

}
void displayValue::write(struct node2 arr[])
{
	ofstream outfile;
	int index = 0;
	outfile.open("C:\\Users\\Shreyas\\Desktop\\HITS_output_serial.txt");
	for (int i = 0; i < num_node; i++)
	{
		outfile << i + 1 << ".)";
		index = arr[i].index;
		outfile << arr[i].farr << endl;
		outfile << node_arr[index].title << endl;
		outfile << node_arr[index].url << endl;
		outfile << endl;
	}
	outfile.close();
	cout << "Output Written" << endl;
}

// Class for Hits Algorithm
class hitsAlgorithm
{
	//Data Members
	int Node;
	int *matrix;
	//int **matrix;
	float *hub_value;
	float *authority;
	int iterations;
	int converged;
	string test_file;

	//Private Member Functions, to be called only with public member functions
	void executeParallel();
	void initialize();
	void memAllocate(int Node);
	void hubRule(float threshold, int threads, int blocks);
	void AuthorityRule(float threshold, int threads, int blocks);
	int checkConvergence(int converged);
public:
	//Constuctor
	hitsAlgorithm(int Node, string path);

	//Member Functions
	void ReadFile(string path);
	virtual void Compute();
	void Display();

	//For Debugging Purpose Only
	void DisplayGraph();
	void DisplayN();
	void showHub();
	void showAuthority();
};



hitsAlgorithm::hitsAlgorithm(int node, string Path)
{
	test_file = Path;
	Node = node;
	iterations = 100;
	memAllocate(Node);
	ReadFile(Path);
}

void hitsAlgorithm::ReadFile(string path)
{

	ifstream graph;
	graph.open(path2);
	for (int i = 0; i<Node; i++)
	{
		for (int j = 0; j < Node; j++)
		{
			graph >> matrix[i*Node + j];
			//cout << matrix[i*Node + j];
		}
	}
	cout << "File Read successfully" << endl;
}

void hitsAlgorithm::memAllocate(int Node)
{
	hub_value = (float *)malloc(Node*sizeof(float));
	if (hub_value == NULL)
	{
		cout << "Memory Allocation Failed for Hub Array" << endl;
		exit(1);
	}

	authority = (float *)malloc(Node*sizeof(float));
	if (authority == NULL)
	{
		cout << "Memory Allocation Failed for Authority Array" << endl;
		exit(1);
	}
	matrix = (int *)malloc(Node*Node*sizeof(int));
	if (authority == NULL)
	{
		cout << "Memory Allocation Failed for Matrix Array" << endl;
		exit(1);
	}
	cout << "Host Memory Allocated Successfully" << endl;
}


void hitsAlgorithm::executeParallel()
{
	ofstream outfile;
	outfile.open("C:\\Users\\Shreyas\\Desktop\\out_serial.txt");

	cout << "Processing Starts" << endl;

	for (int i = 0; i < iterations; i++)
	{
		//Define the norm vairbales

		float norm = 0;

		//Launch hub Kernel
		hubKernel(Node, matrix, hub_value, authority, &norm);
		// copy back norm
		float n = norm;
		n = sqrt(n);

		Normalize(n, hub_value, Node);
		norm = 0.0;
		authorityKernel(Node, matrix, hub_value, authority, &norm);

		n = norm;
		n = sqrt(n);


		Normalize(n, authority, Node);

	}

	for (int i = 0; i < Node; i++)
	{
		outfile << authority[i] << endl;
	}
	outfile.close();

Error:
	cout << "Processing Ends" << endl;
}

void hitsAlgorithm::Compute()
{
	initialize();
	executeParallel();
}

void hitsAlgorithm::Display()
{
	//displayValue obj("C:\\Users\\Shreyas\\Desktop\\Hits\\net_censorship\\regular\\nodes");

	displayValue obj("C:\\Users\\Shreyas\\Desktop\\Hits\\computational_geometry\\expanded\\nodes");
	obj.Reader();
	obj.sortFile(authority);
}

void hitsAlgorithm::initialize()
{
	//Initialise hub value and authority value
	for (int i = 0; i < Node; i++)
	{
		hub_value[i] = 1;
		authority[i] = 1;
	}
}

void hitsAlgorithm::DisplayGraph()
{

	ofstream outfile;
	outfile.open("C:\\Users\\Shreyas\\Desktop\\graph_serial.txt");
	//For Debugging Purpose Only
	for (int i = 0; i < Node; i++)
	{
		for (int j = 0; j < Node; j++)
		{
			outfile << matrix[i*Node + j] << " ";
			//cout << matrix[i*Node+j] << " ";
		}
		outfile << endl;
		//cout << endl;
	}
}

void hitsAlgorithm::DisplayN()
{
	cout << Node << endl;
}

void hitsAlgorithm::showAuthority()
{
	for (int i = 0; i < Node; i++)
	{
		cout << authority[i] << endl;
	}
}

void hitsAlgorithm::showHub()
{
	for (int i = 0; i < Node; i++)
	{
		cout << hub_value[i] << endl;
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	//string path = "C:\\Users\\Shreyas\\Desktop\\Hits\\net_censorship\\regular\\adj_matrix";
	//hitsAlgorithm run(2583, path);

	string path = "C:\\Users\\Shreyas\\Desktop\\Hits\\computational_geometry\\expanded\\adj_matrix";
	hitsAlgorithm run(1226, path);
	run.Compute();
	run.Display();
	getchar();

}
