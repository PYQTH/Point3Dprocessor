// Copyright 2021 Visionary Machines. All rights reserved.
// Confidential information. Do not redistribute.

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <math.h>
#include <vector>
using namespace std;

// define the structure of the reference, which contains locations of all known
// 3d points and all related distances in the world reference
struct reference
{
    double location[3];
    double dist;
};

// declare functions
double *solve(reference refer_1, reference refer_2, reference refer_3);
int** comb(const int N, const int K);

int main(int argc, char *argv[]){

  // 1.read .txt file which contains 3D locations and paired distances.
  ifstream inPoint_Distance_data;
  string filename = "GGCalibration_2021-08-08/RawPointMeasurements.txt";

  inPoint_Distance_data.open(filename);
  if (inPoint_Distance_data.fail()) {
      cout<<"The RawPointMeasurements.txt dose not exist!!!";
  };

  // 2.data preparation
  string line, tmp;
  vector<char> baseP = {'A', 'B', 'C', 'D'};
  vector<char> pairedP = {'E', 'F', 'G', 'H', 'I', 'J'};
  double basePoints[baseP.size()][3], distance[pairedP.size()][baseP.size()];

  while (getline(inPoint_Distance_data, line))
  {
    for(int32_t i = 0; i < baseP.size(); i++ ){
      if(line[0] == baseP[i] && line[2] == '['){
        // remove the first three chars, which aren't digits
        line = line.substr(3, line.length() - 3 );
        line.erase(remove(line.begin(), line.end(), '*'), line.end()); // remove *
        line.erase(remove(line.begin(), line.end(), ']'), line.end()); // remove [
        line.erase(remove(line.begin(), line.end(), ' '), line.end()); // remove space
        stringstream string_stream(line);
        int32_t j = 0;
        while (string_stream.good())
        {
            getline(string_stream, tmp, ',');
            basePoints[i][j] = stod(tmp);
            j++;
        }
      }else{
        for(int32_t j = 0; j < pairedP.size(); j++){
          if((line[0] == baseP[i] && line[2] == pairedP[j]) || (line[2] == baseP[i] && line[0] == pairedP[j])){
            // remove the first three chars, which aren't digits
            line = line.substr(3, line.length() - 3 );
            line.erase(remove(line.begin(), line.end(), ' '), line.end()); // remove space
            distance[j][i] = stod(line);
          }
        }
      }
    }
  }

  // initilize data
  reference refer[pairedP.size()][baseP.size()];
  for (int32_t i = 0; i < pairedP.size(); i++)
  {
      for (int32_t j = 0; j < baseP.size(); j++)
      {
          refer[i][j].dist = distance[i][j];
          for (int32_t k = 0; k < 3; k++){
          if (j < baseP.size()){
                refer[i][j].location[k] = basePoints[j][k];
            }else{
                refer[i][j].location[k] = 0;
              }
          }
      }
  }

  // four combinations
  const int32_t N = baseP.size();
  const int32_t K = 3;
  int** combo = comb(N,K);

  // define a 3D array for four combinations. 6*4
  // each result contains x, y z, hence *3
  double point_location[pairedP.size()][baseP.size()][3];
  for (int32_t i = 0; i < pairedP.size(); i++){
      for (int32_t j = 0; j < baseP.size(); j++){
        for(int32_t k = 0; k < 3 ; k++){
          point_location[i][j][k] = solve(refer[i][combo[j][0]], refer[i][combo[j][1]], refer[i][combo[j][2]])[k];
        }
      }
  }

  // get the average locations for 6 points
  double average_location[pairedP.size()][3];
  for (int32_t i = 0; i < pairedP.size(); i++){
    for(int32_t j = 0; j < 3 ; j++){
      average_location[i][j] = (point_location[i][0][j] + point_location[i][1][j] + point_location[i][2][j] + point_location[i][3][j]) / baseP.size();
    }
  }

  // get all points
  double points[baseP.size()+pairedP.size()][3];
  for (int32_t i = 0; i < baseP.size(); i++) {
    for (int32_t j = 0; j < 3; j++)
    points[i][j] = basePoints[i][j];
  }

  for (int32_t i = 0; i < pairedP.size(); i++) {
    for (int32_t j = 0; j < 3; j++) {
      points[i+baseP.size()][j] = average_location[i][j];
    }
  }

  vector<char> P;
  set_union(baseP.begin(), baseP.end(), pairedP.begin(), pairedP.end(), back_inserter(P));

  for (int32_t i = 0; i < P.size(); i++){
    cout << P[i] << ": [" << points[i][0] << ", "<< points[i][1] << ", "<< points[i][2] <<"]"<< endl;
  }
  return 0;
}

int** comb(const int N, const int K)
{
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    int **combo = 0;
    combo = new int*[N];
    int tmp_x = 0;
    // print integers and permute bitmask
    do {
      combo[tmp_x] = new int[K];
      int tmp_y = 0;
      for (int i = 0; i < N; ++i){ // [0..N-1] integers
          if (bitmask[i]) {
            combo[tmp_x][tmp_y] = i;
            tmp_y++;
          }
      }
      tmp_x++;
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    return combo;
}

double* solve(reference refer_1, reference refer_2, reference refer_3)
{
    double A[3];
    double B[3];
    double C[3];
    for (int i = 0; i < 3; i++)
    {
        A[i] = refer_1.location[i];
        B[i] = refer_2.location[i];
        C[i] = refer_3.location[i];
    }

    double EA = refer_1.dist;
    double EB = refer_2.dist;
    double EC = refer_3.dist;
    // cout<<EA<<endl;

    double x = ((2 * (A[1] - C[1])) * (EB*EB - EA*EA + A[0]*A[0] + A[1]*A[1] - B[0]*B[0] - B[1]*B[1])
     - (EC*EC - EA*EA + A[0]*A[0] + A[1]*A[1] - C[0]*C[0] - C[1]*C[1]) * 2 * (A[1] - B[1]))
     / (4 * (A[0] - B[0]) * (A[1] - C[1]) - 4 * (A[0] - C[0]) * (A[1] - B[1]));

    double y = (EC*EC - EA*EA + A[0]*A[0] + A[1]*A[1] - C[0]*C[0] - C[1]*C[1] - 2 * x * (A[0] - C[0]))
     / (2 * (A[1] - C[1]));

    double z = pow((EA*EA - x*x + 2 * A[0] * x - A[0]*A[0] - y*y + 2 * A[1] * y - A[1]*A[1]), 0.5);

    double *point_location = new double[3];
    point_location[0] = x;
    point_location[1] = y;
    point_location[2] = z;

    return point_location;
}
