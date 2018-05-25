/*
g++ -std=c++0x -fopenmp -O parallelnaivebayesfinal.cpp -o pnb

*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <array>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <map>
#include <omp.h>
#include <time.h>
#include <stdlib.h>



using namespace std;


string classes[]={"no","yes"};


//string classes[]= {"normal.","snmpgetattack.","named.","xlock.","smurf.","ipsweep.","multihop.","xsnoop.","sendmail.","guess_passwd.","saint.","buffer_overflow.","portsweep.","pod.","apache2.","phf.","udpstorm.","warezmaster.","perl.","satan.","xterm.","mscan.","processtable.","ps.","nmap.","rootkit.","neptune.","loadmodule.","imap.","back.","httptunnel.","worm.","mailbomb.","ftp_write.","teardrop.","land.","sqlattack.","snmpguess."};

//string classes[] = {"ad.","nonad."};

const int numThreads=8;
const int num_classes=2;
const int num_attr=5;
const int num_rows=28000;
const int num_test_rows=num_rows;
int count = 0;

/*const int numThreads=4;
const int num_classes=2;
const int num_attr=5;
const int num_rows=28000;
int num_test_rows=num_rows;
*/

map <string,float> table[num_attr];
map <string,float> prob_table[num_attr];
int class_counts[num_classes];
double class_probs[num_classes];
string filename="./datasets/normal/largeData.csv"; // HARDCODED
string classpath = "./datasets/normal/classcounts.txt"; // the py generated file csv
string testfile="./datasets/normal/largeData.csv"; // HARDCODED

/*
string filename="./datasets/networklogs/training.csv";
string classpath = "./datasets/networklogs/classcounts.txt";
string testfile="./datasets/networklogs/testing.csv";
*/
/*string filename="./datasets/addata/addata.csv";
string classpath = "./datasets/addata/classcounts.txt";
string testfile="./datasets/addata/addata.csv";
*/
string data[num_rows][num_attr];
string testData[num_rows][num_attr];

int getIndexOf(string s) {
  for(int i=0 ; i < num_classes;i++ ){
    if(classes[i].compare(s) == 0) {
      return i;
    }
  }
}

void readFileToData()
{
    char temp[1024];
    strcpy(temp,filename.c_str());
    std::ifstream file(temp);

        for(int row = 0; row < num_rows; ++row)
        {
                std::string line;
                std::getline(file, line);
                //std::cout << "line "<< row << ":"<<line << '\n';
                if ( !file.good() )
                    break;
                std::stringstream iss(line);
                for (int col = 0; col < num_attr; ++col)
                {
                    std::string val;
                    std::getline(iss, val, ',');
                    if ( !iss.good() )
                        break;
                    std::stringstream convertor(val);
                    convertor >> data[row][col];
                }
        }

}

void readTestFileToData()
{
    char temp[1024];
    strcpy(temp,testfile.c_str());
    std::ifstream file(temp);
    for(int row = 0; row < num_test_rows; ++row)
    {
        std::string line;
        std::getline(file, line);
        if ( !file.good() )
            break;
        std::stringstream iss(line);
        for (int col = 0; col < num_attr; ++col)
        {
            std::string val;
            std::getline(iss, val, ',');
            if ( !iss.good() )
                break;
            std::stringstream convertor(val);
            convertor >> testData[row][col];
        }
    }

}

void printData()
{
    for(int row=0;row<num_rows;row++)
    {
        cout<<"\n";
        for(int col=0;col<num_attr;col++)
        {
            cout<<data[row][col]<<"\t";
        }
    }
}

void printTestData()
{
    for(int row=0;row<num_test_rows;row++)
    {
        cout<<"\n";
        for(int col=0;col<num_attr;col++)
        {
            cout<<testData[row][col]<<"\t";
        }
    }
}

void printTable()
{
  for(int i=0;i<num_attr;i++)
  {
    cout << "-------\n";
    cout << i << " th table is\n";
    for (std::map<string,float>::iterator it=table[i].begin(); it!=table[i].end(); ++it)
      std::cout << it->first << " => " << it->second << '\n';
    cout <<"-----------\n";
  }
}

void printProbTable()
{
  for(int i=0;i<num_attr;i++)
  {
    cout << "-------\n";
    cout << "\t\t"<< i << " th table is\n";
    for (std::map<string,float>::iterator it=prob_table[i].begin(); it!=prob_table[i].end(); ++it)
      std::cout <<"\t\t"<< it->first << " => " << it->second << '\n';
    cout <<"-----------\n";
  }
}

void printClass_counts(){
  std::cout << "Class counts" << '\n';
  for (int i = 0; i < num_classes; i++) {
    std::cout << class_counts[i] << '\n';
  }
  std::cout << "end of class counts" << '\n';
}

void printClass_probs(){
  std::cout << "Class Probs" << '\n';
  for (int i = 0; i < num_classes; i++) {
    std::cout << class_probs[i] << '\n';
  }
  std::cout << "end of class probs" << '\n';
}


void generateTablesForEachAttb()
{
  #pragma omp parallel shared(data)
  #pragma omp for schedule(static)
  for(int i = 0; i < num_attr; ++i)
  {
      for(int j = 0; j < num_rows; ++j)
      {
          if(table[i].count(data[j][i] + "," + data[j][num_attr - 1]) <= 0) // first time encountering key
          {
              table[i][data[j][i] + "," + data[j][num_attr - 1]] = 1.0;
          }
          else
          {
              table[i][data[j][i] + "," + data[j][num_attr - 1]] += 1.0;
          }
          prob_table[i][data[j][i] + "," + data[j][num_attr - 1]] = 1.0;
      }
  }
}

void calculateProbabilitiesOfEachAttbTable()
{
  #pragma omp parallel shared(data)
  #pragma omp for schedule(static)
  for(int i = 0; i < num_attr; ++i)
  {
      for(int j = 0; j < num_rows; ++j)
      {
        prob_table[i][data[j][i] + "," + data[j][num_attr - 1]] = table[i][data[j][i] + "," + data[j][num_attr - 1]]/(float)class_counts[getIndexOf(data[j][num_attr-1])];
      }
  }
}

void getClassCountsandProbs()
{
  char temp[1024];
  strcpy(temp,classpath.c_str());
  std::ifstream file(temp);
  for(int row = 0; row < num_classes; ++row)
  {
          std::string line;
          std::getline(file, line);
          if ( !file.good() )
              break;
          std::stringstream iss(line);
          string classCount;
          string classProb;
          for (int col = 0; col < 3; ++col)
          {
            // after two iterations classCount has the 2nd value which is the classcount
              std::string val;
              std::getline(iss, val, ',');
              if ( !iss.good() )
                  break;
              if (col == 1) { class_counts[row] = stoi(val); }
              if (col == 2) { class_probs[row] = stod(val); }
          }
  }
}

void testing()
{
  omp_lock_t lock;
  omp_init_lock(&lock);
  double prob[num_classes];
  #pragma omp parallel private(prob) shared(prob_table)
  #pragma omp for schedule(static)
  for(int row = 0; row < num_test_rows;row++)
  {

    for(int class1 = 0; class1 < num_classes; class1++)
    {
      prob[class1] = class_probs[class1];
    }

    for(int class2 = 0; class2 < num_classes;class2++)
    {
        for(int col = 0; col < num_attr-1; col++)
        {
            std::cout << '\n';
            //omp_set_lock(&lock);
            prob[class2] *= prob_table[col][testData[row][col] + "," + classes[class2]];
            //omp_unset_lock(&lock);
        }
    }

    int maxclass = 0;
    for( int class3 = 1; class3 < num_classes; class3++)
    {
        if(prob[class3] > prob[maxclass])
          maxclass = class3;
    }

    if(classes[maxclass] == testData[row][num_attr - 1])
    {
      count++;
    }
  }
  omp_destroy_lock(&lock);
}

int main()
{

    double runtime;
    getClassCountsandProbs();
    omp_set_num_threads(numThreads);
    readFileToData();
    runtime = omp_get_wtime();
    generateTablesForEachAttb();
    calculateProbabilitiesOfEachAttbTable();
    runtime = omp_get_wtime() - runtime;
    cout<< "Learning runs in "  << runtime << " seconds\n";
    readTestFileToData();
    runtime = omp_get_wtime();
    testing();
    runtime = omp_get_wtime() - runtime;
    cout<< "Testing runs in "  << runtime << " seconds\n";
    std::cout << "Correctly classified : " << count << " items "<<'\n';
    std::cout << "Percentage accuracy :" << (count/(float)num_test_rows)*100 << '\n';

    return 0;
}
