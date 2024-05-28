//
//  chessAiTrain.cpp
//  
//  Need major experimenation with different model types/activation functions. Currently not training well.
//  The code does work fine though.
//
//  Created by Alejandro Casillas on 4/15/23.
// 

#include "chessFuncs.cpp"
#include "neural_network_bylayer.cpp"
#include "perfectHash.cpp"
#include <ctype.h>
#include <string.h>

double cost=0;
double wrong=0;
int mct=0;

char* fullLower(char* word){
    int n=strlen(word);
    char* low=new char[n+1];
    for(int i=0;i<n;i++)
        low[i]=tolower(word[i]);
    low[n]=0;
    return low;
}

int playPGN(char* seq, Node** nodes){ //make sure not to have newline character in seq
    //works!!
    bool turn=0; //just setup stuff
    char** move;
    char** board=setUpBoard();
    bool** hasMoved=setUpHM();
    int x=0;
    char enPas=10; //default value of 10 cause can't be accessed
    char pgn[10];
    double* res;
    double des[4112]={};
    int spot;
    int prev=0;
    int m=0;
    int i;
    int ts;
    double th;
    double a;
    for(i=0;seq[m]!=' ' && seq[m];i++,m++) //really glad I made this little thing. So much better than strtok (and doesnt change the original string)
        pgn[i]=seq[m];
    pgn[i]=0;
    m++;
    while(seq[m-1]){
        if(x%3==0){
            x++;
            for(i=0;seq[m]!=' ' && seq[m];i++,m++)
                pgn[i]=seq[m];
            pgn[i]=0;
            m++;
            continue;
        }
        move=pgnConvert(pgn,board,hasMoved,turn,enPas);
        if(move==(char**) -1){ //I made this the bad value cause I made null be just a skipped turn...
            std::cout<<"Invalid board\n";
            return -2;
        }
        if(!move){
            x++;
            for(i=0;seq[m]!=' ' && seq[m];i++,m++)
                pgn[i]=seq[m];
            pgn[i]=0;
            m++;
            turn^=1;
            continue;
        }
        if(move[2][0]==8){ //change en passant
            enPas=move[1][1];
        }
        else{
            enPas=10;
        }
        
        for(int i=0;i<8;i++){
            for(int j=0;j<8;j++){
                nodes[0][i*8+j+1].val=board[i][j];
            }
        }
        nodes[0][65].val=(int) turn;
        res=activate(nodes);
        spot=allHash(move);
        des[prev]=0;
        des[spot]=1; //only saying the right move is the one that was played... has drawbacks for sure
        prev=spot;
        
        ts=0;
        th=0;
        for(int i=0;i<4112;i++){
            if(nodes[6][i].activation>th){
                th=nodes[6][i].activation;
                ts=i;
            }
        }
        if(ts!=spot)
            wrong++;
        a=getCost(nodes,des);
        cost+=a;
        mct++;
        
        adjust(nodes,des,.01); //fixed learn rate for now... generally not too happy about that
        delete[] res; //welp im not using this. oh well
        
        movePiece(board,hasMoved,move,turn);
        destroyMove(move);
        turn^=1;
        x++;
        for(i=0;seq[m]!=' ' && seq[m];i++,m++)
            pgn[i]=seq[m];
        pgn[i]=0;
        m++;
    }

    //moves done, check for result
    if(strcmp(pgn,"1/2-1/2")==0){
        //std::cout<<"Draw\n";
        destroyBoard(board);
        destroyBoard(hasMoved);
        return 2;
    }
    else if(strcmp(pgn,"1-0")==0){
        //std::cout<<"White wins\n";
        destroyBoard(board);
        destroyBoard(hasMoved);
        return 0;
    }
    else if(strcmp(pgn,"0-1")==0){
        //std::cout<<"Black wins\n";
        destroyBoard(board);
        destroyBoard(hasMoved);
        return 1;
    }
    else if(strcmp(pgn,"*")==0){
        //std::cout<<"No result\n";
        destroyBoard(board);
        destroyBoard(hasMoved);
        return -1;
    }
    destroyBoard(board);
    destroyBoard(hasMoved);
    return -1;
}

void rep(char* word){
    for(int i=0;word[i];i++){
        if(word[i]=='.'){
            if(word[i+1]=='\n')
                word[i]=0;
            else
                word[i]=' ';
        }
    }
}

//TOTAL GAME COUNT: 807765
int main(){
    /*
    makeLayer(65); //64 squares + turn (i wont do en pas bc that will naturally be filtered out)
    for(int i=0;i<5;i++){
        makeLayer(100,"relu");
    }
    Node** nodes=makeLayer(4112,"softmax","binary crossentropy"); //debating between sigmid and softmax...
     */
    
    Node** nodes=brainRead((char*)"chessBrainSmall3.txt");
    FILE* fin;
    FILE* fnames=fopen("chess_names.txt","r");
    char temp[8000];
    char tname[50];
    char* lowname;
    int fct=0;
    int games=0;
    while(!feof(fnames)){
        fgets(tname,50,fnames);
        rep(tname);
        int i;
        for(i=0;tname[i] && tname[i]!='\n';i++);
        tname[i]='.';
        tname[i+1]='p';
        tname[i+2]='g';
        tname[i+3]='n';
        tname[i+4]='\0';
        lowname=fullLower(tname);
        fin=fopen(lowname,"r");
        if(!fin){
            std::cout<<lowname<<" couldnt be opened\n";
            continue;
        }
        std::cout<<lowname<<" "<<++fct<<"\n";
        delete[] lowname;
        if(fct<45) //36 for sigmoid version. 45 for softmax version
            continue;
        //int tgames=0;
        int res;
        while(!feof(fin)){
            fscanf(fin,"%[^\n]",temp);
            fgetc(fin);
            if(temp[0]=='1'){
                res=playPGN(temp,nodes);
                //std::cout<<res<<"\n";
                if(res==-2){
                    std::cout<<temp<<"\n";
                }
                fgetc(fin);
                //std::cout<<++tgames<<"\n";
                if(++games%1000==0)
                    std::cout<<games<<" games trained so far\n";
            }
        }
        fclose(fin);
        std::cout<<games<<"\n";
        std::cout<<cost/mct<<" "<<100*wrong/mct<<"\n";
        cost=0;
        wrong=0;
        mct=0;
        brainWrite((char*)"chessBrainSmall3.txt",nodes); //writing out the progress after each file in case I have to stop
    }
    return 0;
}

