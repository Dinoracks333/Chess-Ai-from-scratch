//
//  neural_network_bylayer.cpp
//  
//
//  Created by Alejandro Casillas on 4/1/23.
//


/*
 NOTE:
 NEED TO CHANGE UP HOW I PROPOGATE VIA THE WEIGHTS BECAUSE I CHANGED UP NODES FROM STORING WEIGHTS TO NEXT LAYER INSTEAD TO PREVIOUS LAYER
 ALSO NEED TO CHANGE FROM HOW I WAS PREVIOUSLY USING GLOBALS TO NOT THAT NOW
 */


#include "neural_network_base.h"


/*Activation Functions*/
void sigmoid(Node* x){ //deriv: s*(1-s)
    int size=*((int*) x); //readout layer size from memory of first node in layer which is reserved for that
    for(int i=1;i<size+1;i++){
        x[i].val+=x[i].bias;
        x[i].activation=1/(1+pow(M_E,-x[i].val));
    }
}
void logit(Node* x){ //deriv: 1/(x*(1-x))
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].val+=x[i].bias;
        x[i].activation=log(x[i].val/(1-x[i].val));
    }
}
void relu(Node* x){ //deriv 1 for x>0 else 0
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].val+=x[i].bias;
        x[i].activation=x[i].val*(x[i].val>0); //slightly faster than the ternary operator hehe :P
    }
}
void noAct(Node* x){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].val+=x[i].bias;
        x[i].activation=x[i].val; //assuming no bias either but can change that. Mostly this is used for inputs.
    }
}
//(e^x-e^-x)/(e^x+e^-x)  deriv: 1-tanh^2=sech^2     can also be represented as (e^2x-1)/(e^2x+1)
void ntanh(Node* x){
    int size=*((int*) x);
    double p;
    for(int i=1;i<size+1;i++){
        x[i].val+=x[i].bias;
        p=pow(M_E,2*x[i].val);
        x[i].activation=(p-1)/(p+1);
    }
}
void softmax(Node* x){ //can either integrate this into the actual activation part or use this function
    //deriv is a jacobian, and anyways youll use it with certain cost functions which I will note later but diags are s[i]*(1-s[i]) otherwise -s[i]*s[j]
    int size=*((int*) x);
    double s=0;
    for(int i=1;i<size+1;i++){
        x[i].val+=x[i].bias;
        s+=pow(M_E,x[i].val);
    }
    for(int i=1;i<size+1;i++){
        x[i].activation=pow(M_E,x[i].val)/s;
    }
}
//Derivatives
void dsigmoid(Node* x){
    int size=*((int*) x);
    double t;
    for(int i=1;i<size+1;i++){
        t=1/(1+pow(M_E,-x[i].activation));
        x[i].delta_a*=t*(1-t);
    }
}
void dlogit(Node* x){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].delta_a*=1/(x[i].activation*(1-x[i].activation));
    }
}
void drelu(Node* x){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].delta_a*=x[i].activation>0;
    }
}
void dtanh(Node* x){ //just 1-tanh^2, very nice (literally a well-known derivative... damn me. Even I knew it. yet i didnt know it. Im so much smarter than myself lol)
    int size=*((int*) x);
    double p,d;
    for(int i=1;i<size+1;i++){
        p=pow(M_E,2*x[i].activation);
        d=(p-1)/(p+1); //tanh
        x[i].delta_a*=1-d*d;
    }
}

/* //okay so basically this I just will not use... it will never be used outside of the output layer, and it doesnt make sense to anyways. I will just put a null into the dAct if this is used
double** dsoftmax(double* vals){ //so this is kind of a tricky one to use since the partial derivs are not 0. So basically, you gotta know the cost function or something to put it together... I need to think
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
      out[i][j]=softs[i]*((i==j)-softs[j]);
    }
  }
  return out;
}
*/

/*Cost Functions*/
double diffSquared(Node* x, double* des){ //deriv: 2*(acts[i]-des[i])
    int size=*((int*) x);
    double tot=0;
    double p;
    for(int i=1;i<size+1;i++){
        p=x[i].activation-des[i-1];
        tot+=p*p;
    }
    return tot;
}
double sparseCCE(Node* x, double* des){ //meant to be used in labelling/categorization, especially with softmax with outputs 0-1
    int size=*((int*) x);
    double tot=0;
    for(int i=1;i<size+1;i++){
        tot-=des[i-1]*log(x[i].activation);
    }
    return tot;
}
double binaryCCE(Node* x, double* des){
    int size=*((int*) x);
    double tot=0;
    for(int i=1;i<size+1;i++){
        tot-=des[i-1]*log(x[i].activation)+(1-des[i-1])*log(1-x[i].activation);
    }
    return tot;
}
//Derivatives
void ddiffsquared(Node* x, double* des){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].delta_a=2*(x[i].activation-des[i-1]);
    }
}
void dsparseCCE(Node* x, double* des){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].delta_a=-des[i-1]/x[i].activation;
    }
}
void dbinaryCCE(Node* x, double* des){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].delta_a=-des[i-1]/x[i].activation+(1-des[i-1])/(1-x[i].activation);
    }
}
void dSoftsparse(Node* x, double* des){
    int size=*((int*) x);
    for(int i=1;i<size+1;i++){
        x[i].delta_a=x[i].activation-des[i-1];
    }
}
void dSoftbinary(Node* x, double* des){
    int size=*((int*) x);
    double temp[size];
    double s=0;
    for(int i=0;i<size;i++){
        if(!des[i]){
            temp[i]=(des[i]-1)*x[i+1].activation/(1-x[i+1].activation);
        }
        else{
            temp[i]=0;
        }
        s+=temp[i];
    }
    for(int i=0;i<size;i++){
        x[i+1].delta_a=x[i+1].activation-des[i]+(1-des[i])*x[i+1].activation+x[i+1].activation*(s-temp[i]);
        //magic O_O (tbh I'd have to look at my notes to see how I derived this. Think once, worry never)
    }
}


//Node and network functions

Node::Node(int num_weights, double initpsize){
    weights=new double[num_weights]; //can actually have variable lengths like this. less wasteful and tbh more freedom
    for(int i=0;i<num_weights;i++) weights[i]=(((double)rand())/RAND_MAX)*initpsize;
    bias=(((double)rand())/RAND_MAX)*initpsize;
    val=0;
    activation=0;
    delta_a=0;
}

void Node::backProp(Node** nodes, double learn_rate, int layer){ //function should not be called for layer=0
    int size=*((int*) nodes[layer-1]);
    //delta_a will serve essentially as delta bias here. Just ended up being easier to write it like this with how the functions work
    bias-=delta_a*learn_rate;
    for(int i=0;i<size;i++){
        nodes[layer-1][i+1].delta_a+=weights[i]*delta_a; //im excluding delta_a from learn_rate weight cause i think unecessarily propogating that might be bad and just mitigate the change
        weights[i]-=nodes[layer-1][i+1].activation*delta_a*learn_rate; //weight are stored backwards so that's why the ordering seems weird to be changing the current layer's weights
    }
    //reset registers
    delta_a=0;
    val=0;
    activation=0;
}

void adjust(Node** nodes, double* desired, double learn_rate){ //can return if you want to dynamically update the learning rate using gradient data
    
    int num_layers=*((int*) (*nodes)+1); //get number of layers from meta knowledge
    dCostFunc dCF=*((dCostFunc*) nodes[num_layers-1]+4);
    dCF(nodes[num_layers-1],desired); //apply cost derivatives to last node
    
    for(int i=num_layers-1;i>0;i--){ //not used on last layer
        int size=*((int*) nodes[i]); //get num nodes in layer
        actFunc dfunc=*((actFunc*) nodes[i]+2); //grab the delta activation function from layer
        if(dfunc){ //ignore softmax and other potentially invalid ones cause they'll just break it and are improper usage. was set up earlier
            dfunc(nodes[i]);
        }
        for(int j=0;j<size;j++){
            nodes[i][j+1].backProp(nodes,learn_rate,i);
        }
    }
}

double* activate(Node** nodes, int layer/*=0*/){ //returns dynamic memory, so make sure to delete
    //note: I coded it so that the activation functions add the bias to the nodes which makes this part way easier (and a bit more efficient)
    //so much more succint than earlier code. This is awesome
    actFunc aF=*((actFunc*) nodes[layer]+1);
    if(aF) //no activation for input!!
        aF(nodes[layer]); //add bias and apply activation function to layer
    else
        noAct(nodes[layer]);
    int size=*((int*) nodes[layer]); //get # nodes in current layer
    if(layer==*((int*) (*nodes)+1)-1){ //last layer, return values without propogating
        double* out=new double[size];
        for(int i=0;i<size;i++){
            out[i]=nodes[layer][i+1].activation;
        }
        return out;
    }
    //propogate values to next layer
    int sizeNext=*((int*) nodes[layer+1]); //get # nodes in next layer
    for(int i=1;i<size+1;i++){
        for(int j=1;j<sizeNext+1;j++){
            nodes[layer+1][j].val+=nodes[layer+1][j].weights[i-1]*nodes[layer][i].activation;
        }
    }
    return activate(nodes,layer+1);
}

double getCost(Node** nodes, double* desired){
    int size=*((int*) *nodes+1);
    costFunc cost=*((costFunc*) nodes[size-1]+3);
    return cost(nodes[size-1],desired);
}

/*
 Mental sketch for meta knowledge stored in first node in every layer:
 a Node object has 40 bytes total
 0-3 (int): # nodes in layer (not counting meta node)
 4-7 (int): # layer in network
 8-15 (actFunc): address of activation function for layer
 16-23 (actFunc): address of derivative of activation function for layer
 24-31 (costFunc): if last layer, address of cost function for network. Otherwise, 0, signalling that this layer is not the last layer
 32-39 (dCostFunc): if last layer, address of derivative of cost function for network. Otherwise, the layer number for the current layer
 */
//basically this is just me explicitly assigning the memory address with certain info so that I can
//make the first element of each layer be info about the layer (costFunc only used in last layer tbf)
void makeStart(Node* layer, int lSize, actFunc aF, actFunc dAF, costFunc cF, dCostFunc dCF){
    //sizeof(Node)==40, so i have 40 bytes of memory to work with
    *((int*) layer)=lSize;
    //skipping an ints memory because i will eventually put total number of layers in there
    *((actFunc*) layer+1)=aF;
    *((actFunc*) layer+2)=dAF;
    *((costFunc*) layer+3)=cF;
    *((dCostFunc*) layer+4)=dCF;
}

Node** makeLayer(int lSize, const char* actF/*=""*/, const char* costF/*=""*/){
    static int l_num;
    static int prev;
    static Node** prevLayer;
    
    //temp variables for functions
    actFunc aF;
    actFunc dAF;
    costFunc cF;
    dCostFunc dCF;
    
    //Check for activation functions
    if(l_num==0){
        aF=0x0; //no activation function for input layer
        dAF=0x0;
    }
    else if(strcmp(actF,"sigmoid")==0){
        aF=sigmoid;
        dAF=dsigmoid;
    }
    else if(strcmp(actF,"logit")==0){
        aF=logit;
        dAF=dlogit;
    }
    else if(strcmp(actF,"relu")==0){
        aF=relu;
        dAF=drelu;
    }
    else if(strcmp(actF,"tanh")==0){
        aF=ntanh;
        dAF=dtanh;
    }
    else if(strcmp(actF,"softmax")==0){
        aF=softmax;
        dAF=0x0; //setting this as null because softmax doesnt make sense outside of the context of activation functions, particularly sparse cce or binary cce, so I will only integrate them in that way
        //I could maybe throw an error if softmax is used outside of the last layer even
    }
    else{ //invalid function
        char err[100];
        int i;
        for(i=0;i<100-35 && actF[i];i++)
            err[i]=actF[i];
        strcpy(err+i," is not a valid function\n");
        throw std::invalid_argument(err);
    }
    
    //Check for cost functions
    if(costF[0]==0){ //no cost func because not the end of the layer
        cF=0x0; //setting this to NULL to indicate that it is not the start of the network
        dCF=(dCostFunc) l_num; //store layer num here. Should be more efficient in terms of access and memory this way
    }
    else if(strcmp(costF,"difference squared")==0){
        cF=diffSquared;
        dCF=ddiffsquared;
    }
    else if(strcmp(costF,"sparse crossentropy")==0){
        cF=sparseCCE;
        dCF=dsparseCCE;
        if(aF==softmax){
            dCF=dSoftsparse;
        }
    }
    else if(strcmp(costF,"binary crossentropy")==0){
        cF=binaryCCE;
        dCF=dbinaryCCE;
        if(aF==softmax){
            dCF=dSoftbinary;
        }
    }
    else{
        char err[100];
        int i;
        for(i=0;i<100-35 && actF[i];i++)
            err[i]=costF[i];
        strcpy(err+i," is not a valid function\n");
        throw std::invalid_argument(err);
    }
    
    
    //Actually make the node layer!!!!!!
    Node* nodes=(Node*) malloc(sizeof(Node)*(lSize+1)); //don't want to call constructor, so malloc
    makeStart(nodes,lSize,aF,dAF,cF,dCF); //set the values for the first, meta node
    for(int i=1;i<lSize+1;i++){
        nodes[i]=Node(prev,1.0/prev); //making initial weight value max 1/prev so max add up to 1. Not a terrible idea I think
        if(l_num==0)
            nodes[i].bias=0; //no bias for input
    }
    
    //make linked list to hold all of this info until the layer is complete
    Node** temp=(Node**) malloc(sizeof(Node*)*2);
    temp[0]=nodes;
    temp[1]=(Node*) prevLayer; //have to cast it down so that I can store it
    
    //change static vars as needed
    l_num++;
    prev=lSize;
    prevLayer=temp;
    if(costF[0]){ //i.e not an empty string, meaning the last layer
        //reset static variables when you have completed a layer so that you can make a new model now!
        //i dont think I need any meta knowledge about the layers outside of them, so I can just store the pointers to them and nothing else
        Node** network=(Node**) malloc(sizeof(Node*)*l_num);
        Node** tp;
        int num_layers=l_num;
        for(int i=l_num-1;i>=0;i--){
            network[i]=prevLayer[0];
            *((int*)network[i]+1)=l_num; //storing the total number of layers in the network in the meta knowledge
            tp=(Node**) prevLayer[1];
            delete[] prevLayer;
            prevLayer=tp;
        }
        l_num=0;
        prev=0;
        //prevLayer auto becomes null again by how ive set it up
        return network;
    }
    return NULL; //returns NULL when network is not yet complete
}

Node** brainRead(char fname[]){
    Node** network;
    FILE* file=fopen(fname,"r");
    int num_layers;
    fscanf(file,"%d",&num_layers);
    
    int sizes[num_layers];
    int size;
    char actF[100];
    char costF[100];
    
    for(int i=0;i<num_layers;i++){ //create the network first with the proper number of layers, sizes, and functions
        fscanf(file,"%d %s %[^\n]",&size,actF,costF);
        if(costF[0]=='0')
            costF[0]=0;
        if(actF[0]=='0')
            actF[0]=0;
        network=makeLayer(size,actF,costF);
        sizes[i]=size;
    }
    
    //now, read in the weight and bias information for each node (so this info is separated from the actual data for each layer
    //which has some notable advantages, especially in read-in efficiency and also human readability as the meta info about
    //the layer is at the top of the file rather than hidden within the data)
    for(int i=1;i<num_layers;i++){ //skip input layer because there is no relevant info there
        for(int j=1;j<sizes[i]+1;j++){
            fscanf(file,"%lf",&network[i][j].bias);
            for(int k=0;k<sizes[i-1];k++){
                fscanf(file,"%lf",&network[i][j].weights[k]);
            }
        }
    }
    
    fclose(file);
    return network;
}

void brainWrite(char fname[], Node** nodes){
    FILE* file=fopen(fname,"w");
    int num_layers=*((int*) (*nodes)+1);
    fprintf(file,"%d\n",num_layers);
    
    for(int i=0;i<num_layers;i++){
        fprintf(file,"%d ",*((int*) nodes[i])); //print number of nodes in layer
        actFunc aF=*((actFunc*) nodes[i]+1);
        if(aF==sigmoid){
            fprintf(file,"sigmoid ");
        }
        else if(aF==logit){
            fprintf(file,"logit ");
        }
        else if(aF==relu){
            fprintf(file,"relu ");
        }
        else if(aF==ntanh){
            fprintf(file,"tanh ");
        }
        else if(aF==softmax){
            fprintf(file,"softmax ");
        }
        else{
            fprintf(file,"0 ");
        }
        
        costFunc cF=*((costFunc*) nodes[i]+3);
        if(cF==0){
            fprintf(file,"0\n");
        }
        else if(cF==diffSquared){
            fprintf(file,"difference squared\n");
        }
        else if(cF==sparseCCE){
            fprintf(file,"sparse crossentropy\n");
        }
        else if(cF==binaryCCE){
            fprintf(file,"binary crossentropy\n");
        }
    }
    for(int i=1;i<num_layers;i++){ //no info to print out for input layer, so skip it
        int size=*((int*) nodes[i]);
        int sizePrev=*((int*) nodes[i-1]);
        for(int j=1;j<size+1;j++){
            fprintf(file,"%lf\n",nodes[i][j].bias);
            for(int k=0;k<sizePrev;k++){
                fprintf(file,"%lf ",nodes[i][j].weights[k]);
            }
            fprintf(file,"\n");
        }
    }
    
    fclose(file);
}

