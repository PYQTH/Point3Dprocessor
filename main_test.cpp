#include <iostream>
 
using namespace std;
 
int main(int argc,char *argv[])
{
	for(int i=0;i<argc;i++)
	{
		cout<<"argument["<<i<<"] is: "<<argv[i]<<endl;
	}
	// system("pause");
    cout<<"pause!"<<endl;
    return 0;
}
