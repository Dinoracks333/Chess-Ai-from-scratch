//
//  neural_network_base.h
//  
//
//  Created by Alejandro Casillas on 4/9/23.
//

#ifndef neural_network_base_h
#define neural_network_base_h

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <cstring>


class Node{
  public:
    double bias;
    double val;
    double activation;
    double delta_a;
    double* weights;
    Node(int, double);
    void backProp(Node**, double, int);
};

/*
 normal flow is something like set input vals (can either use setVals or can manually set them. If
 you choose to manually set them, make sure to skip the first node as that is reserved for meta
 knowledge)
 then call activate on the network to get the result (need to free the result as it does dynamically
 allocate memory for the ouput)
 then call adjust on the network with your desired values and the learning rate. For now, I have
 not written an optimizer to handle all of the learning rate stuff and adjust it as necessary, so
 that can hurt performance a bit
 I also have not written code to handle data in minibatches, but I might do that soon
 
 example:
 setVals(nodes,inps);
 res=activate(nodes);
 adjust(nodes,desired,.01);
 delete[] res;
 
 side note: call getCost before adjust if you want data from that as adjust flushes out the data in the network
 */
void adjust(Node**, double*, double);
double* activate(Node**, int layer=0);
void setVals(Node**,double*);
double getCost(Node**,double*);

/*Activation Functions*/
void sigmoid(Node*);
void logit(Node*);
void relu(Node*);
void ntanh(Node*);
void softmax(Node*);
void noAct(Node*);
//Activation Derivatives
void dsigmoid(Node*);
void dlogit(Node*);
void drelu(Node*);
void dtanh(Node*);
/* //okay so basically this I just will not use... it will never be used outside of the output layer, and it doesnt make sense to anyways. I will just put a null into the dAct if this is used. But this is here just so it's known that it exists
double** dsoftmax(Node* x);
*/

/*Cost Functions*/
double diffSquared(Node*, double*);
double sparseCCE(Node*, double*);
double binaryCCE(Node*, double*);

//Cost Derivatives
void ddiffsquared(Node*, double*);
void dsparseCCE(Node*, double*);
void dbinaryCCE(Node*, double*);
void dSoftsparse(Node*, double*);
void dSoftbinary(Node*, double*);

typedef void (*actFunc) (Node*);
typedef double (*costFunc) (Node*, double*);
typedef void (*dCostFunc) (Node*, double*);

//functions to help make certain parts of the network (Node**) so that it can be cleanly returned to the programmer. makeLayer especially is where the magic happens
void makeStart(Node*, int, actFunc, actFunc, costFunc, dCostFunc);
Node** makeLayer(int, const char* actF="", const char* costF="");
//makeLayer is the most important function. It allows you to very conveniently construct the network
//layer by layer with the actvation functions that you want. The function holds onto
//the parts of the layer that have been created until you give it a cost function, signalling
//the end of the layer, at which point it returns the whole layer, and yes it has contiguous memory
//at least for the pointers to each layer. Individual layers might have gaps between them, which is fine
//You should use this instead of trying to make your own Node** because I incorporate meta knowledge
//into the layer which is used in every function. This was done so that you only have to worry about
//one singular variable

Node** brainRead(char[]); //creates a network from a file output created by brainWrite
void brainWrite(char[], Node**);

#endif /* neural_network_base_h */
