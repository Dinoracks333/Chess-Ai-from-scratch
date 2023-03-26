#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <string.h>
#include <cstdio>
#include <ctype.h>
#include <assert.h>


//globals for size for ease of use
const int in_num=1;
const int hidden_num=20;
const int out_num=1;
const int hlayer_num=3;
const double sig_fact=.01;
const double initpsize=.05;
const int msize=fmax(fmax(in_num,out_num),hidden_num);

/*Activation Functions*/
double sigmoid(double x){ //deriv: s*(1-s)*sig_fact
  return 1/(1+pow(M_E,-x*sig_fact));
}
double logit(double x){ //deriv: 1/(x*(1-x))
  return log(x/(1-x));
}
double relu(double x){ //deriv 1 for x>0 else 0
  if(x>0)
    return x;
  return 0;
}
//(e^x-e^-x)/(e^x+e^-x)  deriv: 1-tanh^2=sech^2
//double tanh(double x); //just use the tanh in c++ library cause it's probably optimized. Anyways, the relevant info is still above
double* softmax(double* vals){ //can either integrate this into the actual activation part or use this function
  //deriv is a jacobian, and anyways youll use it with certain cost functions which I will note later but diags are s[i]*(1-s[i]) otherwise -s[i]*s[j]
  double s=0;
  for(int i=0;i<out_num;i++){
    s+=pow(M_E,*(vals+i));
  }
  static double out[out_num]; //using static because I'm afraid that the pointers aren't returning properly otherwise. also mem save?
  for(int i=0;i<out_num;i++){
    out[i]=pow(M_E,*(vals+i))/s;
  }
  return out;
}
//Derivatives
double dsigmoid(double x){
  double t=sigmoid(x);
  return t*(1-t)*sig_fact;
}
double dlogit(double x){
  return 1/(x*(1-x));
}
double drelu(double x){
  return x>0 ? x:0;
}
double** dsoftmax(double* vals){
  double** out=0;
  out=new double*[out_num];
  double s=0;
  for(int i=0;i<out_num;i++){
    s+=pow(M_E,*(vals+i));
  }
  double softs[out_num];
  for(int i=0;i<out_num;i++){
    softs[i]=pow(M_E,*(vals+i));
  }
  for(int i=0;i<out_num;i++){
    out[i]=new double[out_num];
    for(int j=0;j<out_num;j++){
      out[i][j]=softs[i]*((i==j ? 1:0)-softs[j]);
    }
  }
  return out;
}

/*Cost Functions*/
double diffSquared(double* acts,double* des){ //deriv: 2*(acts[i]-des[i])
  double tot=0;
  for(int i=0;i<out_num;i++){
    tot+=(*(acts+i)-*(des+i))*(*(acts+i)-*(des+i));
  }
  return tot;
}
double sparseCCE(double* acts,double* des){
  double tot=0;
  for(int i=0;i<out_num;i++){
    tot-=*(des+i)*log(*(acts+i));
  }
  return tot;
}
double binaryCCE(double* acts,double* des){
  double tot=0;
  for(int i=0;i<out_num;i++){
    tot-=*(des+i)*log(*(acts+i))+(1-*(des+i))*log(1-*(acts+i));
  }
  return tot;
}
//Derivatives
double ddiffquared(double des,double x){
  return 2*(x-des);
}
double dsparseCCE(double des,double x){
  return -des/x;
}
double dbinaryCCE(double des,double x){
  return -des/x+(1-des)/(1-x);
}
double* dSoftsparse(double* des,double* acts){
  static double out[out_num];
  for(int i=0;i<out_num;i++){
    out[i]=*(acts+i)-*(des+i);
  }
  return out;
}
double* dSoftbinary(double des[],double acts[]){
  static double out[out_num];
  double temp[out_num];
  double s=0;
  for(int i=0;i<out_num;i++){
    if(!des[i]){
      temp[i]=(des[i]-1)*acts[i]/(1-acts[i]);
    }
    else{
      temp[i]=0;
    }
    s+=temp[i];
  }
  for(int i=0;i<out_num;i++){
    out[i]=acts[i]-des[i]+(1-des[i])*acts[i]+acts[i]*(s-temp[i]);
  }
  return out;
}

class Node{
  public:
    int layer;
    int row;
    double bias;
    double val;
    double activation;
    double delta_a;
    double weights[784]; //i have to manually enter it in for some reason??? won't let me use const and memory allocation errors otherwise... goddamn man (new keyword might fix...)
    Node(int layer_num, int node_num){
        for(int i=0;i<msize;i++) weights[i]=((double)(rand())/RAND_MAX)*initpsize;
      layer=layer_num;
      row=node_num;
        bias=((double)(rand())/RAND_MAX)*initpsize;
      if(layer_num==0) bias=0; //no bias for input
      val=0;
      activation=0;
      delta_a=0;
    }
    void backProp(double learn_rate, Node** nodes, int size);
};

void Node::backProp(double learn_rate, Node** nodes, int size){
  double delta_bias;
  double s=sigmoid(val);
  delta_bias=delta_a*s*(1-s)*sig_fact;
  bias-=delta_bias*learn_rate;
  for(int i=0;i<size;i++){
    (*(*(nodes+layer-1)+i)).delta_a+=(*(*(nodes+layer-1)+i)).weights[row]*delta_bias; //im excluding delta_a from learn_rate weight cause i think unecessarily propogating that might be bad and just mitigate the change... idk
    (*(*(nodes+layer-1)+i)).weights[row]-=(*(*(nodes+layer-1)+i)).activation*delta_bias*learn_rate;
  }
  //reset registers
  delta_a=0;
  val=0;
  activation=0;
}

void adjust(Node** nodes, double* desired, double learn_rate){ //can return if you want to dynamically update the learning rate using gradient data
  for(int i=0;i<out_num;i++){
    (*(*(nodes+hlayer_num+1)+i)).delta_a=2*((*(*(nodes+hlayer_num+1)+i)).activation-*(desired+i));
    (*(*(nodes+hlayer_num+1)+i)).backProp(learn_rate,nodes,hidden_num);
  }
  for(int i=hlayer_num;i>1;i--){ //do not do backProp on inputs
    for(int j=0;j<hidden_num;j++){
      (*(*(nodes+i)+j)).backProp(learn_rate,nodes,hidden_num);
    }
  }
  for(int i=0;i<hidden_num;i++){
    (*(*(nodes+1)+i)).backProp(learn_rate,nodes,in_num);
  }
}

double* activate(Node** nodes, int layer){
  if(layer==hlayer_num+1){
    static double out[out_num];
    double vals[out_num];
    for(int i=0;i<out_num;i++){
      (*(*(nodes+hlayer_num+1)+i)).val+=(*(*(nodes+hlayer_num+1)+i)).bias;
      (*(*(nodes+hlayer_num+1)+i)).activation=sigmoid((*(*(nodes+hlayer_num+1)+i)).val);
      out[i]=(*(*(nodes+hlayer_num+1)+i)).activation;
    }
    return out;
  }
  if(layer==0){
    for(int i=0;i<in_num;i++){
      (*(*nodes+i)).activation=(*(*nodes+i)).val; //no activation or bias for input
      for(int j=0;j<hidden_num;j++){
        (*(*(nodes+1)+j)).val+=(*(*nodes+i)).activation*(*(*nodes+i)).weights[j];
      }
    }
  }
  else{
    for(int i=0;i<hidden_num;i++){
      (*(*(nodes+layer)+i)).val+=(*(*(nodes+layer)+i)).bias;
      (*(*(nodes+layer)+i)).activation=sigmoid((*(*(nodes+layer)+i)).val);

      //last hidden layer only has out_num weights
      if(layer==hlayer_num){
        for(int j=0;j<out_num;j++){
          (*(*(nodes+layer+1)+j)).val+=(*(*(nodes+layer)+i)).activation*(*(*(nodes+layer)+i)).weights[j];
        }
      }
      //multiply activation by weights and add to next layer
      else{
        for(int j=0;j<hidden_num;j++){
          (*(*(nodes+layer+1)+j)).val+=(*(*(nodes+layer)+i)).activation*(*(*(nodes+layer)+i)).weights[j];
        }
      }
    }
  }
  return activate(nodes,layer+1);
}

void brainRead(char fname[],Node** nodes){
    //technically can read first line for size, but the assumption is that the size matches your nodes anyways. I could have it just read in a file and give you the node array with the correct size automatically but eh. You can just look at the header for the text file you are reading in... Well maybe I will change it at some point but then you gotta set the constants too. Im just gonna keep it like this for now
    FILE* file=fopen(fname,"r");
    int dummy;
    fscanf(file,"%d %d %d %d",&dummy,&dummy,&dummy,&dummy);
    for(int i=0;i<in_num;i++){
        //no bias for inputs
      for(int j=0;j<hidden_num;j++){
        fscanf(file,"%lf",&nodes[0][i].weights[j]);
      }
    }
    for(int x=1;x<hlayer_num;x++){
      for(int i=0;i<hidden_num;i++){
        fscanf(file,"%lf",&nodes[x][i].bias);
        for(int j=0;j<hidden_num;j++){
          fscanf(file,"%lf",&nodes[x][i].weights[j]);
        }
      }
    }
    for(int i=0;i<hidden_num;i++){
      fscanf(file,"%lf",&nodes[hlayer_num][i].bias);
      for(int j=0;j<out_num;j++){
        fscanf(file,"%lf",&nodes[hlayer_num][i].weights[j]);
      }
    }
    for(int i=0;i<out_num;i++){
        fscanf(file,"%lf",&nodes[hlayer_num+1][i].bias); //no weights for outputs
    }
    fclose(file);
}

void brainWrite(char fname[],Node** nodes){
    FILE* file=fopen(fname,"w");
    fprintf(file,"%d %d %d %d\n",in_num,hidden_num,hlayer_num,out_num);
    for(int i=0;i<in_num;i++){
        //no bias for inputs
      for(int j=0;j<hidden_num;j++){
        fprintf(file,"%lf ",nodes[0][i].weights[j]);
      }
      fprintf(file,"\n");
    }
    for(int x=1;x<hlayer_num;x++){
      for(int i=0;i<hidden_num;i++){
        fprintf(file,"%lf\n",nodes[x][i].bias);
        for(int j=0;j<hidden_num;j++){
          fprintf(file,"%lf ",nodes[x][i].weights[j]);
        }
        fprintf(file,"\n");
      }
    }
    for(int i=0;i<hidden_num;i++){
      fprintf(file,"%lf\n",nodes[hlayer_num][i].bias);
      for(int j=0;j<out_num;j++){
        fprintf(file,"%lf ",nodes[hlayer_num][i].weights[j]);
      }
      fprintf(file,"\n");
    }
    for(int i=0;i<out_num;i++){
        fprintf(file,"%lf\n",nodes[hlayer_num+1][i].bias); //no weights for outputs
    }
    fclose(file);
}

int main() {
  Node** nodes=(Node**) malloc(sizeof(Node*)*(hlayer_num+2));
  for(int i=0;i<hlayer_num+2;i++){
    nodes[i]=(Node*) malloc(sizeof(Node)*msize);
    for(int j=0;j<msize;j++){
      nodes[i][j]=Node(i,j); //it doesn't like me having different sized lists... but the logic is there that it will only use the ones that it needs to
    }
  }
  int num;
  double des[out_num];
  double* res;
  srand((int)time(0)); //seed for random numbers
  for(int i=0;i<1000;i++){
    num=rand()%2;
    nodes[0][0].val=num;
    res=activate(nodes,0);
    des[0]=num%2;
    adjust(nodes,des,10);
    if(i%10==0) std::cout<<*res<<" "<<*des<<"\n";
  }
  return 0;
}
