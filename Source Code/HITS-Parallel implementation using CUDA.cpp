#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
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




__global__ void hubKernel(int Node, int matrix[], float hub[], float authority[],float *norm)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < Node)
	{
		hub[i] = 0;
		for (int j = 0; j < Node; j++)
		{
			if (i == j) continue;
		    hub[i] += matrix[i*Node + j] ? authority[j] : 0;	
		}
		atomicAdd(&norm[0],( hub[i]*hub[i] ));
	}
}

__global__ void authorityKernel(int Node, int matrix[], float hub[], float authority[],float *norm)
{
	int j = blockIdx.x * blockDim.x + threadIdx.x;

	if (j < Node)
	{
		authority[j] = 0;
		for (int i = 0; i < Node; i++)
		{
			if (i == j) continue;
			authority[j] += matrix[i*Node + j] ? hub[i] : 0;
		}
		atomicAdd(&norm[0], authority[j] * authority[j] );
	}

}
__global__ void Normalize(float div, float arr[],int Node)
{
	int i = blockIdx.x*blockDim.x + threadIdx.x;
	if (i < Node)
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
	nodefile.open(path);
	nodefile >> num_node;
	node_arr = new node[num_node];
	nodefile.close();
}
void displayValue::Reader()
{
	ifstream nodefile;
	nodefile.open(path);
	string garbage;
	nodefile >> garbage;

	for (int i = 0; i < num_node; i++)
	{
		garbage = "";
		nodefile >> node_arr[i].id; getline(nodefile, garbage);
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
	outfile.open("C:\\Users\\Shreyas\\Desktop\\HITS_output.txt");
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
	
	//For GPU memory
	int *matrixGPU;
	float *hub_value_GPU;
	float *authority_GPU;

	//Private Member Functions, to be called only with public member functions
	void executeParallel();
	void initialize();
	void memAllocate(int Node);
	void hubRule(float threshold,int threads, int blocks);
	void allocateGpuBuffer_Hub();
	void allocateGpuBuffer_Authority();
	void allocateGpuBuffer_matrix();
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
	graph.open(path);
	for (int i = 0; i<Node; i++)
	{
		for (int j = 0; j < Node; j++)
		{
			graph >> matrix[i*Node+j];
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
	/*matrix = (int**)malloc(Node*sizeof(int *));
	if (matrix == NULL)
	{
		cout << "Memory Allocation Failed for Adjacency Matrix" << endl;
		exit(1);
	}

	for (int i = 0; i < Node; i++)
	{
		matrix[i] = (int *)malloc(Node*sizeof(int));
		if (matrix[i] == NULL)
		{
			cout << "Memory Allocation Failed for Adjacency Matrix" << endl;
			exit(1);
		}
	}*/
	cout << "Host Memory Allocated Successfully"<<endl;
}

void hitsAlgorithm::executeParallel()
{
	 ofstream outfile;
	 outfile.open("C:\\Users\\Shreyas\\Desktop\\out_serial.txt");
	 int threads = 512;
	 int blocks = ciel(Node, threadsy);

	cudaError_t cudaStatus;
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		cout << "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?" << endl;
		goto Error;
	}

	allocateGpuBuffer_matrix();
	allocateGpuBuffer_Hub();
	allocateGpuBuffer_Authority();
	
	cout << "Processing Starts" << endl;

	for (int i = 0; i < iterations; i++)
	{
		//Define the norm vairbales
		
		float dummy = 0.0;
		float *norm=&dummy;
		float *norm_GPU;


		// Allocate memory for Norm GPU
		cudaStatus = cudaMalloc((void **)&norm_GPU, sizeof(float));
		if (cudaStatus != cudaSuccess)
		{
			cout << "cudaMalloc failed for norm variable\n";
			exit(1);
		}
		cudaStatus = cudaMemcpy(norm_GPU, norm, sizeof(float), cudaMemcpyHostToDevice);
		if (cudaStatus != cudaSuccess)
		{
			cout << "cudaMemcpy failed for norm " << cudaStatus;
			exit(1);
		}
		
		
		//Launch hub Kernel
		hubKernel<<<blocks,threads>>>(Node, matrixGPU, hub_value_GPU, authority_GPU, norm_GPU);
		cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) {
			cout<<"Kernel launch failed:\n"<< cudaGetErrorString(cudaStatus);
			goto Error;
		}
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			cout << "cudaDeviceSynchronize returned error code after launching hubKernel" << cudaStatus << endl;
			goto Error;
		}

		// copy back norm
		cudaStatus = cudaMemcpy(norm, norm_GPU, sizeof(float), cudaMemcpyDeviceToHost);
		if (cudaStatus != cudaSuccess)
		{
			cout << "cudaMemcpy failed for norm " << cudaStatus;
			exit(1);
		}
		float n = *norm;
		n = sqrt(n);
		

		Normalize << <blocks, threads >> >(n, hub_value_GPU, Node );
		cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) {
			cout << "Kernel launch failed:\n" << cudaGetErrorString(cudaStatus);
			goto Error;
		}
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			cout << "cudaDeviceSynchronize returned error code after launching authorityKernel" << cudaStatus << endl;;
			goto Error;
		}


		
		*norm = 0.0;
		cudaStatus = cudaMemcpy(norm, norm_GPU, sizeof(float), cudaMemcpyDeviceToHost);
		if (cudaStatus != cudaSuccess)
		{
			cout << "cudaMemcpy failed for norm " << cudaStatus;
			exit(1);
		}
		authorityKernel <<<blocks, threads >>>(Node, matrixGPU, hub_value_GPU, authority_GPU, norm_GPU);
		cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) {
			cout << "Kernel launch failed:\n" << cudaGetErrorString(cudaStatus);
			getchar();
			goto Error;
		}
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			cout << "cudaDeviceSynchronize returned error code after launching authorityKernel" << cudaStatus << endl;;
			getchar();
			goto Error;
		}
		cudaStatus = cudaMemcpy(norm, norm_GPU, sizeof(float), cudaMemcpyDeviceToHost);
		if (cudaStatus != cudaSuccess)
		{
			cout << "cudaMemcpy failed for norm " << cudaStatus;
			getchar();
			exit(1);
		}

		n = *norm;
		n = sqrt(n);

		
		Normalize <<<blocks, threads >>>(n, authority_GPU, Node);
		cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) {
			cout << "Normal Kernel launch failed:\n" << cudaGetErrorString(cudaStatus);
			getchar();
			goto Error;
		}
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			cout << "cudaDeviceSynchronize returned error code after launching Normalize" << cudaStatus << endl;;
			getchar();
			goto Error;
		}
	}

	cudaMemcpy(hub_value, hub_value_GPU, sizeof(float)*Node, cudaMemcpyDeviceToHost);
	cudaMemcpy(authority, authority_GPU, sizeof(float)*Node, cudaMemcpyDeviceToHost);

	for (int i = 0; i < Node; i++)
	{
		outfile << authority[i] << endl;
	}
	outfile.close();
	
Error:
	cudaFree(authority_GPU);
	cudaFree(hub_value_GPU);
	cudaFree(matrixGPU);
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
	outfile.open("C:\\Users\\Shreyas\\Desktop\\graph.txt");
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

int main()
{
	//string path = "C:\\Users\\Shreyas\\Desktop\\Hits\\net_censorship\\regular\\adj_matrix";
	//hitsAlgorithm run(2583, path);

	string path = "C:\\Users\\Shreyas\\Desktop\\Hits\\computational_geometry\\expanded\\adj_matrix";
	hitsAlgorithm run(1226,path);
	run.DisplayGraph();
	run.Compute();
	run.Display();
	getchar();

}