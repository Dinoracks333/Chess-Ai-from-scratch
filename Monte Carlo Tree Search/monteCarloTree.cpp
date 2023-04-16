//
//  monteCarloTree.cpp
//
//
//  Created by Alejandro Casillas on 3/31/23.
//

//WARNING: UNTESTED

#include "chessFuncs.cpp"
#include "perfectHash.cpp"
#include "neural_network_base.cpp"
#include <float.h>

//may or may not have depth integrated into mcts (monte carlo tree search)
//might be best just to let it dfs and run for a while, but idk
//I should probably also sketch out kinda what I want to do for this, especially in regards to the heuristics and how each parent node integrates the values of its child nodes (actually that part is simple, just take the best opponent response, i.e. the worst of them)


double abss(double x){ //one line functions aren't great... kinda pointless but whatever. Gets the message across
    return (x<0?-x:x);
}

//so basically template is part of the function signature...
template <typename T> //using template so I dont have to retype same version of insert for char**
void insert(T* arr, T x, int spot, int size){
    for(int i=size-1;i>spot;i--){
        arr[i]=arr[i-1];
    }
    arr[spot]=x;
}

char*** bestMove(Node** valAi, Node** moveAi, char** board, bool** hasMoved, bool turn, char enPas, bool sturn, int k, int depth, int n=0){
    static double* best=new double[depth];
    if(n==0){ //only set array initially
        best=new double[depth]; //basically marks the best valuation at each depth so that the current branch can be compared and killed if necessary
        for(int i=0;i<depth;i++){
            best[i]=0; //this should actually be declared to +- DBL_MAX
        }
    }
    
    const int s=(turn==sturn)*2-1; //this is used essentially to flip the valuation checks when simulating enemy turn
    char*** moves=getAllMoves(board,hasMoved,turn,enPas);
    char*** out=new char**[2]; //returning [next move, board val]
    double* val=new double[1]; //need dynamic memory so that I can return it outside of this instance
    char** tmoves[k]; //actual best moves
    double bmoves[k];
    out[1]=(char**) val; //setting it like this so that it changes automatically.
    if(moves[0]==NULL){ //no moves so game has ended
        if(kingInCheck(board,turn)){
            *val=(turn*-2+1)*DBL_MAX; //white wins is max double. Black wins is -max double (or like min double sorta but theres a distinction)
        }
        else{
            val=0;
        }
        out[0]=NULL; //no next move
        return out;
    }
    else{
        *val=*activate(valAi);
    }
    if(n==depth){
        out[0]=NULL;
        return out;
    }
    if(n!=0){ //check cut-off value for current depth (0 depth excluded)
        if(s*(*val)>s*best[n-1]){ //but what about turn!=sturn??? then these should be flipped right??
            best[n-1]=*val;
        }
        else if(s*(*val)<s*(best[n-1]-sqrt(abss(best[n-1])))){ //I think sqrt is a good heuristic cut-off, but I might change my mind
            //maybe I could do some sort of sigmoid?? or maybe like tanh??
            out[0]=NULL;
            *val=-DBL_MAX*s;
            return out; //prune branch
        }
    }
    
    double* mvals=activate(moveAi); //4112 is the number of legal moves in chess (see perfectHash for move of an explanation) and also the size of the ouput vector of moveAi
    double tval;
    for(int m=0;moves[m];m++){ //setting initial guesses for best k moves
        tval=logit(mvals[allHash(moves[m])]);
        for(int j=0;j<k;j++){
            if(tval>bmoves[j]){
                insert(bmoves,tval,j,k);
                insert(tmoves,moves[m],j,k);
                break;
            }
        }
    }
    
    //create a copy of the board and hasMoved to pass onto next branch so that it isn't mutated unecessarily
    char** tboard=new char*[8];
    tboard[0]=new char[64];
    bool** tHM=new bool*[8];
    tHM[0]=new bool[64];
    for(int i=0;i<8;i++){
        tboard[i]=tboard[0]+i*8;
        tHM[i]=tHM[0]+i*8;
    }
    char*** tout;
    for(int i=0;i<k;i++){
        copyBoard(board,tboard);
        copyBoard(hasMoved,tHM);
        tout=bestMove(valAi,moveAi,tboard,tHM,turn^1,(tmoves[i][2][0]==8?tmoves[i][1][1]:10),sturn,k,depth,n+1);
        tval=*((double*)tout[1]);
        if(s*tval>s*(*val)){
            *val=tval;
            out[0]=tmoves[i];
        }
        destroyMove(tout[0]); //no memory leaks plox
        delete tout[1]; //maybe delete &tval
        delete[] tout;
    }
    /* //unsure if this is good or not. Basically, updating the best depth to be full information... very, very unsure
    if(n!=0 && s*(*val)>s*best[n-1]){
        best[n-1]=val;
    }
     */
    destroyBoard(tboard);
    destroyBoard(tHM);
    if(n==0)
        delete[] best;
    return out;
}
