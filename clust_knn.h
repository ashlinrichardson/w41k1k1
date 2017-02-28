#pragma once // later: restore and min-heap
#include "SA.h"
#include "sas.cpp"
#include <pthread.h>
#include <queue> 
#include "./glut.h"
#include "pick.h"
#include "global.h"

using namespace std;
using namespace _SA;

namespace myglut{
	class GLUTWindow;
	class GLUT3d;
	class GLUT2d;
	class clust_knn;
	
	
	class clust_knn{
	public:
		int N;
		int K;
		int KMax;
		int Rand_Iter_Max;
		int nj;   //number of points used

    // floating point data
		vector < SA<float> * > * float_buffers; 
		
		SA<int> origIndex;
		SA<float> dat;    //the sampled data set (a subset of the full data set). 
		SA<int> nnIndex;  //listing the indices of the nearest neighbors of each of the data points.
		SA<float> nnD;    //the listing of distances between each point and it's K-nearest neighbors.
		vector< vector<unsigned int> > classmembers;
		

		SA<float> dE;
		SA<int> knn_indices;
		SA<int> badData;
		vector<int> knn_J_indices;
		int nkci;   // next knn class index;
		int n_knn_centres;	
		int NRow;
		int NCol;
				
		clust_knn(int _NRow, int _NCol);	
		
		void init(GLUT3d * _my3d, 
				      GLUT2d * _my2d,
				      vector < SA<float> * > * _float_buffers,
				      int nskip,  //calculate nj from N and nskip.
				      int _KMax);	
		
		int getK(){
			return K;
		}
		
		void reinit(int nskip, int _KMax);
		float densityEstimate(int j, int K);	
		
		int classf(int j,  
				       SA<int> * highestdensityneighborindex, 
				       SA<float> * highestdensityneighbordensity);	
		
		void knn_clustering();
		int get_n_knn_centres();
		int get_n_knn_elements(int centre_index);
		float get_centre_coord(int centre_index, int elem_ind, int coord);
		float get_centre_coord( int centre_index, int coord);
		float distance(int i, int j);
		float euclidean_distance(int i, int j);
		float T3_distance(int i, int j);
		float normalized_euclidean_distance(int i, int j);
		
		void set_Rand_Iter_Max(int rim){
			Rand_Iter_Max = rim;
		}

		int get_Rand_Iter_Max(){
			return Rand_Iter_Max;
		}

	};
}