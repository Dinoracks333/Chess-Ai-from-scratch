//
//  pgnRead.cpp
//  
//
//  Created by Alejandro Casillas on 3/30/23.
//

#include "chessFuncs.cpp"
#include <ctype.h>
#include <string.h>


char* fullLower(char* word){
    int n=strlen(word);
    char* low=new char[n+1];
    for(int i=0;i<n;i++)
        low[i]=tolower(word[i]);
    low[n]=0;
    return low;
}

int playPGN(char* seq){ //make sure not to have newline character in seq (will probably still work but return value might be wrong)
    bool turn=0; //just setup stuff
    char** move;
    char** board=setUpBoard();
    bool** hasMoved=setUpHM();
    int x=0;
    char enPas=10; //default value of 10 cause can't be accessed
    char pgn[10];
    int m=0;
    int i;
    for(i=0;seq[m]!=' ' && seq[m];i++,m++) //really glad I made this little thing. So much better than strtok (and doesnt change the original string)
        pgn[i]=seq[m];
    pgn[i]=0;
    m++;
    while(seq[m-1]){
        //std::cout<<pgn<<"\n"; //if you want to see the pgns as it goes through them
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
        movePiece(board,hasMoved,move,turn);
        destroyMove(move);
        turn^=1;
        x++;
        for(i=0;seq[m]!=' ' && seq[m];i++,m++)
            pgn[i]=seq[m];
        pgn[i]=0;
        m++;
    }
    
    //displayBoard(board); //if you want to see the final board state
    //moves done, check for result
    if(strcmp(pgn,"1/2-1/2")==0){
        //std::cout<<"Draw\n";
        destroyBoard(board);
        destroyHM(hasMoved);
        return 2;
    }
    else if(strcmp(pgn,"1-0")==0){
        //std::cout<<"White wins\n";
        destroyBoard(board);
        destroyHM(hasMoved);
        return 0;
    }
    else if(strcmp(pgn,"0-1")==0){
        //std::cout<<"Black wins\n";
        destroyBoard(board);
        destroyHM(hasMoved);
        return 1;
    }
    else if(strcmp(pgn,"*")==0){
        //std::cout<<"No result\n";
        destroyBoard(board);
        destroyHM(hasMoved);
        return -1;
    }
    destroyBoard(board);
    destroyHM(hasMoved);
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

int main(){
    FILE* fin;
    FILE* fnames=fopen("chess_names.txt","r"); //can enter in a file with a bunch of different file names if you want
    char temp[8000]; //8kb. Somehow 2kb is not enough
    char tname[50];
    char* lowname;
    int fct=0; //total count of pgn files opened (not just number of games)
    int games=0; //total game count
    while(!feof(fnames)){
        fgets(tname,50,fnames);
        /* //formatting stuff. Was necessary for my case
        rep(tname);
        int i;
        for(i=0;tname[i] && tname[i]!='\n';i++);
        tname[i]='.';
        tname[i+1]='p';
        tname[i+2]='g';
        tname[i+3]='n';
        tname[i+4]='\0';
        lowname=fullLower(tname);
        */
        lowname=tname;
        fin=fopen(lowname,"r");
        if(!fin){
            std::cout<<lowname<<" couldnt be opened\n";
            continue;
        }
        std::cout<<lowname<<" "<<++fct<<"\n";
        delete[] lowname;
        int res;
        while(!feof(fin)){
            fscanf(fin,"%[^\n]",temp);
            fgetc(fin);
            if(temp[0]=='1'){
                games++;
                res=playPGN(temp);
                //std::cout<<res<<"\n"; //if you want to know the result
                if(res==-2){
                    std::cout<<temp<<"\n";
                }
                fgetc(fin);
            }
        }
        fclose(fin);
        std::cout<<games<<"\n";
    }
    return 0;
}
