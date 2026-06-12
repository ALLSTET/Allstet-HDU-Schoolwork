#include <iostream>

int function(int n){
  if(n){
    return n*function(n/2);
  }
  else if(n==0){
    return 1;
  }
  else{
    return 0;
  }
}

int main(){
  int n;
  std::cin>>n;
  std::cout<<"f("<<n<<")= "<<function(n)<<std::endl;
  getchar();
  getchar();

  return 0;
}