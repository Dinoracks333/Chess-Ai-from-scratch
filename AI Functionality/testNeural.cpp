//
//  testNeural.cpp
//  Just runs the NN on given inputs and can compare to desired output for testing, not training, purposes
//
//  Created by Alejandro Casillas on 4/11/23.
//

#include "neural_network_bylayer.cpp"

void numToArr(int num, int arr[], int size){
    for(int i=0;i<size;i++){
        arr[size-i-1]=num%10;
        num/=10;
    }
}

bool isSquare(int x){
    int t=sqrt(x);
    return x==t*t;
}

int main() {
  srand(time(0)); //set seed for random numbers which is used to randomly generate the intial weights
    
  makeLayer(784); //make-your-own version
  for(int i=0;i<3;i++){
      makeLayer(100,"relu");
  }
  Node** nodes=makeLayer(10,"softmax","binary crossentropy");
     
  //Node** nodes=brainRead((char*)"testNeural1.txt"); //read-in version
  char fname[]="mnist_brain.txt";
  //brainRead(fname,nodes);
  int digit;
  double des[10];
  double* res;
  FILE* fin=fopen("/Users/alejandrocasillas/Desktop/Chess Files/mnist_train.txt","r");
  FILE* flabel=fopen("/Users/alejandrocasillas/Desktop/Chess Files/mnist_train_labels.txt","r");
  assert(fin!=NULL);
  assert(flabel!=NULL);
  double scoring=0;
  int perc_error=0;
  double high;
  int spot;
  double rate=.006;
  for(int i=0;i<10;i++){
    des[i]=0;
  }
  for(int w=0;w<5;w++){
      std::cout<<"repeating data set number "<<w<<"\n";
        for(int i=0;i<60000;i++){
            for(int j=0;j<28;j++){
                for(int x=0;x<28;x++){
                    fscanf(fin,"%lf",&nodes[0][j*28+x+1].val);
                    nodes[0][j*28+x+1].val/=255;
                }
            }
            res=activate(nodes);
            /*
            for(int i=0;i<10;i++)
                std::cout<<res[i]<<" ";
            std::cout<<"\n";
             */
            fscanf(flabel,"%d",&digit);
            des[digit]=1;
            double a=getCost(nodes,des);
            //std::cout<<a<<"\n\n";
            if(a<10) //not including crazy stupid error in test but do include it in percent error. We do adjust based on it though
                scoring+=a;
            high=0;
            spot=0;
            for(int i=0;i<10;i++){
                if(res[i]>high){
                    high=res[i];
                    spot=i;
                }
            }
            if(spot!=digit){
                perc_error++;
            }
            adjust(nodes,des,rate);
             //sample output compared to desired
            /*
            if(i%100==0){
                std::cout<<"\nres: ";
                for(int t=0;t<10;t++){
                    std::cout<<*(res+t)<<" ";
                }
                std::cout<<"\ndes: ";
                for(int t=0;t<10;t++){
                    std::cout<<*(des+t)<<" ";
                }
                std::cout<<"   "<<spot;
            }
             */
            if(i && (i%1000==0 || i==59999)){ //(i%1000==0 || i==59999)
                std::cout<<"we are on iteration "<<i<<"\n";
                std::cout<<"\nERRORS: "<<perc_error/10.0<<" "<<scoring/1000.0<<"\n";
                perc_error=0;
                scoring=0;
            }
            des[digit]=0;
        }
        fseek(fin, 0, SEEK_SET);
        fseek(flabel, 0, SEEK_SET);
      delete[] res;
  }
  brainWrite((char*)"testNeural1.txt",nodes);
  return 0;
}
