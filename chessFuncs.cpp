//chess functionality written in c++ meant to be used for chess ai
//kinda like my own little chess library
//I will have a different program for the actual ai functions though (on top of the base neural network mini library that I wrote). Ah, with all of these libraries, I'm basically an academic :P or a librarian lol

#include <stdio.h>
#include <iostream>

//figured out the memory stuff yay!! pretty sure at the very least :) so, nice
/*
I wanna be super efficient, probably to the point where it is going too far,
so im gonna use ints instead of chars for the piece values at a relative cost of
clarity, but otherwise I would have to have a 2d char array and that's tbh kinda
annoying and this way I can have some stuff built in so yeah, itll be good I think
 
 Using SHORT FOR MAXIMUM EFFICIENCY LMAO (I think only memory but maybe idk lmaoooo)
 Actually char would technically be even less memory... only 8 bits... but nah idk it's w/e. That's just too weird
 I might do it
 Memory accesses are apparently the limiting factor, so using chars or shorts vs ints should be faster!!
 lmaooooo after testing, it is about .2 seconds faster for 1 million calls of getAllMoves with shorts vs ints (or chars vs ints i dont remember) which is the heftiest portion. Honestly not a bad advantage. Not huge, but decent.
 So yes I am using chars instead of shorts, and I couldn't be happier

pieces, for reference
 
 empty: 0
 pawn: 1
 bishop: 2
 knight: 3
 rook: 4
 queen: 5
 king: 6
 
 black: +6
 
 examples:
 bp=7
 wr=4
 bK=12
 
Special keys:
 1=en passant
 2-5=promotion to corresponding piece
 6=long castle
 7=short castle
 8=double pawn move
*/

char*** setUp(){
    char** board=new char*[8];
    char* ti=new char[64];
    for(int i=0;i<8;i++){
        board[i]=ti+i*8;
        for(int j=0;j<8;j++){
            board[i][j]=0;
        }
    }
    
    /*Back Rows*/
    
    //white
    board[0][0]=4; //rook
    board[0][1]=3; //knight
    board[0][2]=2; //bishop
    board[0][3]=5; //queen
    board[0][4]=6; //king
    board[0][5]=2;
    board[0][6]=3;
    board[0][7]=4;
    
    //black
    board[7][0]=10;
    board[7][1]=9;
    board[7][2]=8;
    board[7][3]=11;
    board[7][4]=12;
    board[7][5]=8;
    board[7][6]=9;
    board[7][7]=10;
    
    /*Pawns*/
    
    //white
    board[1][0]=1;
    board[1][1]=1;
    board[1][2]=1;
    board[1][3]=1;
    board[1][4]=1;
    board[1][5]=1;
    board[1][6]=1;
    board[1][7]=1;
    
    //black
    board[6][0]=7;
    board[6][1]=7;
    board[6][2]=7;
    board[6][3]=7;
    board[6][4]=7;
    board[6][5]=7;
    board[6][6]=7;
    board[6][7]=7;
    
    bool** hasMoved=new bool*[8]; //haha 1 bit per square fight me >:) (stored as 1 byte unfortunately)
    bool* thi=new bool[64];
    for(int i=0;i<8;i++){
        hasMoved[i]=thi+i*8;
        for(int j=0;j<8;j++){
            hasMoved[i][j]=1;
        }
    }
    //pawns haven't moved
    hasMoved[1][0]=0;
    hasMoved[1][1]=0;
    hasMoved[1][2]=0;
    hasMoved[1][3]=0;
    hasMoved[1][4]=0;
    hasMoved[1][5]=0;
    hasMoved[1][6]=0;
    hasMoved[1][7]=0;
    
    hasMoved[6][0]=0;
    hasMoved[6][1]=0;
    hasMoved[6][2]=0;
    hasMoved[6][3]=0;
    hasMoved[6][4]=0;
    hasMoved[6][5]=0;
    hasMoved[6][6]=0;
    hasMoved[6][7]=0;
    
    //other pieces haven't moved yet
    hasMoved[0][0]=0;
    hasMoved[0][1]=0;
    hasMoved[0][2]=0;
    hasMoved[0][3]=0;
    hasMoved[0][4]=0;
    hasMoved[0][5]=0;
    hasMoved[0][6]=0;
    hasMoved[0][7]=0;
    
    hasMoved[7][0]=0;
    hasMoved[7][1]=0;
    hasMoved[7][2]=0;
    hasMoved[7][3]=0;
    hasMoved[7][4]=0;
    hasMoved[7][5]=0;
    hasMoved[7][6]=0;
    hasMoved[7][7]=0;
    
    char*** both=new char**[2];
    both[0]=board;
    both[1]=(char**) hasMoved;
    return both;
}

bool inside(char x, char y){
    return x>=0 && x<8 && y>=0 && y<8;
}

char** createMove(char ystart, char xstart, char yend, char xend, char spKey, char piece){ //wrote this just bc it was getting annoying and honestly kinda bad to create the move thing every time and it'll be really bad for passing into functions and stuff
    char** move = new char*[3];
    char* ti=new char[6]; //contiguous memory faster to allocate and destroy (by quite a lot) (and probably to retrieve from memory)
    for(int i=0;i<3;i++){
        move[i]=ti+i*2;
    }
    move[0][0]=ystart;
    move[0][1]=xstart;
    move[1][0]=yend;
    move[1][1]=xend;
    move[2][0]=spKey;
    move[2][1]=piece; //might as well use unused spot for the piece
    return move;
}

void destroyMove(char** move){ //note that it is actually quite a bit faster (almost twice as fast for this case) to do contiguous memory on the 1d part of the array
    delete[] move[0];
    delete[] move; //just destroy contents, not actual pointer to move.
}

void destroyBoard(char** board){
    //writing it all out so I don't have to use a for loop. Very very minor time save lol at the sacrifice of some mutability (really in the board size) but that won't happen anyways, so assumptions are valid ig.
    delete[] board[0]; //so because i am technically representing the 1d part as contiguous memory, I don't have to call delete on each index. In fact, it breaks if I do because only the first one really deletes.
    delete[] board;
}

bool kingInCheck(char** board, bool turn, bool destroy){ //destroy parameter decides whether or not we should get rid of the board memory we were given. Should be used whenever a simMove output is fed directly into this function (which I do a lot). Once again, makes it easier bc don't have to create as many variables and is honestly more readable (debateable) so I'm fine w/ it for now (forever).
    const char opcolor=(turn^1)*6;
    char king[2]; //finding king
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(board[i][j]==turn*6+6){
                king[0]=i;
                king[1]=j;
                break;
            }
        }
    }
    
    //knights
    const char knightMove[2][2]={{2,1},{1,2}};
    char a,b;
    for(int i=0;i<2;i++){
        for(int x=-1;x<2;x+=2){
            for(int y=-1;y<2;y+=2){
                a=king[0]+x*knightMove[i][0];
                b=king[1]+y*knightMove[i][1];
                if(inside(a,b) && board[a][b]==opcolor+3){
                    if(destroy)
                        destroyBoard(board);
                    return true;
                }
            }
        }
    }
    
    //pawns
    const char s=-2*turn+1;
    if((inside(king[0]+s,king[1]+1) && board[king[0]+s][king[1]+1]==opcolor+1) || (inside(king[0]+s,king[1]-1) && board[king[0]+s][king[1]-1]==opcolor+1)){
        if(destroy)
            destroyBoard(board);
        return true;
    }
    
    //bishops
    char tx,ty;
    for(int i=-1;i<2;i+=2){
        for(int j=-1;j<2;j+=2){
            for(ty=king[0]+i,tx=king[1]+j;inside(ty,tx) && !board[ty][tx];ty+=i,tx+=j); //succint and beautiful :P these for loops are the bomb
            if(inside(ty,tx) && (board[ty][tx]==opcolor+2 || board[ty][tx]==opcolor+5)){
                if(destroy)
                    destroyBoard(board);
                return true;
            }
        }
    }
    
    //rooks
    for(int a=-1;a<2;a+=2){
        for(int i=0;i<2;i++){
            for(int j=0;j<2;j++){
                for(ty=king[0]+a*i,tx=king[1]+a*j;inside(ty,tx) && !board[ty][tx];ty+=a*i,tx+=a*j);
                if(inside(ty,tx) && (board[ty][tx]==opcolor+4 || board[ty][tx]==opcolor+5)){
                    if(destroy)
                        destroyBoard(board);
                    return true;
                }
            }
        }
    }
    
    //king
    for(int i=-1;i<2;i++){
        for(int j=-1;j<2;j++){
            if(inside(king[0]+i,king[1]+j) && board[king[0]+i][king[1]+j]==opcolor+6){
                if(destroy)
                    destroyBoard(board);
                return true;
            }
        }
    }
    if(destroy)
        destroyBoard(board);
    return false;
}

char** simMove(char** board, char** move, bool turn, bool destroy){
    char color=turn*6;
    char** tb=new char*[8];
    char* ti=new char[64]; //finally figured out the notation or way to do static 2d pointers. Just had to make next level contiguous.
    //I actually don't know if the memory will be good or not though... oh well...
    //don't use static here. Either that or don't destroy the boards, but I feel a bit safer with destroying the boards. So yeah, anyways.
    for(int i=0;i<8;i++){
        tb[i]=ti+i*8;
        for(int j=0;j<8;j++){
            tb[i][j]=board[i][j];
        }
    }
    if(move[2][0]==1){ //En Passant
        tb[move[0][0]][move[0][1]]=0;
        tb[move[1][0]][move[1][1]]=color+move[2][1];
        tb[move[1][0]+2*turn-1][move[1][1]]=0;
    }
    else if(move[2][0]>=2 && move[2][0]<6){ //Promotion keys. It's just the piece number :P hehe cheeky me
        tb[move[0][0]][move[0][1]]=0;
        tb[move[1][0]][move[1][1]]=color+move[2][0];
    }
    else if(move[2][0]==6){ //Left side castle (long)
        tb[move[0][0]][move[0][1]]=0;
        tb[move[1][0]][move[1][1]]=color+move[2][1];
        tb[move[1][0]][move[1][1]+1]=tb[move[0][0]][move[0][1]-4];
        tb[move[0][0]][move[0][1]-4]=0;
    }
    else if(move[2][0]==7){ //Right side castle (short)
        tb[move[0][0]][move[0][1]]=0;
        tb[move[1][0]][move[1][1]]=color+move[2][1];
        tb[move[1][0]][move[1][1]-1]=tb[move[0][0]][move[0][1]+3];
        tb[move[0][0]][move[0][1]+3]=0;
    }
    else{
        tb[move[0][0]][move[0][1]]=0;
        tb[move[1][0]][move[1][1]]=color+move[2][1];
    }
    if(destroy){ //used when I'm passing things into functions and just don't want to make extra variables and clutter. Not too hard or unintuitive of a fix, and I feel that it fits somewhat well with simMove, so I think I'll accept it for now.
        destroyMove(move);
    }
    return tb;
}

char*** getMoves(char** board, bool** hasMoved, char pos[2], char piece, bool turn, char enPas){
    //I haven't written this in here yet, but everytime that I am adding a new move, I need to check that the king isn't in check
    char*** moves=new char**[28]; //I believe this is the most amount of moves from any one piece (27+1 for null)
    char** tempMove;
    char color=turn*6;
    char ind=0;
    switch(piece){ //switch is supposed to be faster with nested ifs like I have here bc you declare the jumps yourself.
        case 1:{ //pawn
            const char s=-2*turn+1;
            if(pos[0]==6*turn+(turn^1) && !hasMoved[pos[0]][pos[1]] && !board[pos[0]+2*s][pos[1]] && !board[pos[0]+s][pos[1]]){ //double move from start
                tempMove=createMove(pos[0],pos[1],pos[0]+2*s,pos[1],8,piece);
                if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                    moves[ind++]=tempMove;
                else
                    destroyMove(tempMove);
            }
            if(pos[0]+s<8 && pos[0]+s>=0){
                if(!board[pos[0]+s][pos[1]]){ //normal one space move
                    if(pos[0]+s==(turn^1)*7){ //promotion
                        for(int i=2;i<6;i++){
                            tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1],i,piece);
                            if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                    }
                    else{
                        tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1],0,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
                for(int i=-1;i<2;i+=2){ //captures
                    if(pos[1]+i<8 && pos[1]+i>=0 && board[pos[0]+s][pos[1]+i] && board[pos[0]+s][pos[1]+i]/7==(turn^1)){
                        if(pos[0]+s==(turn^1)*7){
                            for(int j=2;j<6;j++){
                                tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,j,piece);
                                if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                                    moves[ind++]=tempMove;
                                else
                                    destroyMove(tempMove);
                            }
                        }
                        else{
                            tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,0,piece);
                            if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                    }
                    if(pos[1]+i==enPas && pos[0]==4-turn){ //en passant
                        tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,1,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
            }
            break;
        }
        case 2:{ //bishop
            char tx,ty;
            for(int i=-1;i<2;i+=2){
                for(int j=-1;j<2;j+=2){
                    for(ty=pos[0]+i,tx=pos[1]+j;inside(ty,tx) && !board[ty][tx];ty+=i,tx+=j){
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                    if(inside(ty,tx) && board[ty][tx]/7==(turn^1)){
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
            }
            break;
        }
        case 3:{ //knight
            const char knightMove[2][2]={{2,1},{1,2}};
            char a,b;
            for(int i=0;i<2;i++){
                for(int x=-1;x<2;x+=2){
                    for(int y=-1;y<2;y+=2){
                        a=pos[0]+x*knightMove[i][0];
                        b=pos[1]+y*knightMove[i][1];
                        if(inside(a,b) && (!board[a][b] || board[a][b]/7==(turn^1))){ //nice boolean simplification
                            tempMove=createMove(pos[0],pos[1],a,b,0,piece);
                            if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                    }
                }
            }
            break;
        }
        case 4:{ //rook
            char ty,tx,j;
            for(int a=-1;a<2;a+=2){
                for(int i=0;i<2;i++){
                    j=i^1;
                    for(ty=pos[0]+a*i,tx=pos[1]+a*j;inside(ty,tx) && !board[ty][tx];ty+=a*i,tx+=a*j){
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                    if(inside(ty,tx) && board[ty][tx]/7==(turn^1)){
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
            }
            break;
        }
        case 5:{ //queen
            char*** temp=getMoves(board,hasMoved,pos,2,turn,enPas); //simulate as bishop
            for(int i=0;i<14 && temp[i]!=NULL;i++){
                temp[i][2][1]=piece;
                moves[ind++]=temp[i];
            }
            temp=getMoves(board,hasMoved,pos,4,turn,enPas); //simulate as rook
            for(int i=0;i<14 && temp[i]!=NULL;i++){
                temp[i][2][1]=piece;
                moves[ind++]=temp[i];
            }
            break;
        }
        case 6:{ //king
            for(int i=-1;i<2;i++){ //normal one space move
                for(int j=-1;j<2;j++){
                    if(i==0 && j==0)
                        continue;
                    if(inside(pos[0]+i,pos[1]+j) && (!board[pos[0]+i][pos[1]+j] || board[pos[0]+i][pos[1]+j]/7==(turn^1))){
                        tempMove=createMove(pos[0],pos[1],pos[0]+i,pos[1]+j,0,piece);
                        if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
            }
            if(!hasMoved[pos[0]][pos[1]]){
                char rpos;
                bool clear;
                for(int i=-1;i<2;i+=2){
                    rpos=3.5*i-.5; //-4 when -1 else 3
                    clear=true;
                    if(hasMoved[pos[0]][pos[1]+rpos])
                        continue;
                    for(int j=pos[1]+i;j!=pos[1]+rpos;j+=i){ //check that the board is clear
                        if(board[pos[0]][j]){
                            clear=false;
                            break;
                        }
                    }
                    if(clear){ //check that the king isn't moving through check
                        if(!kingInCheck(board,turn,false) && !kingInCheck(simMove(board,createMove(pos[0],pos[1],pos[0],pos[1]+i,0,6),turn,true),turn,true) && !kingInCheck(simMove(board,createMove(pos[0],pos[1],pos[0],pos[1]+2*i,0,6),turn,true),turn,true)){
                            tempMove=createMove(pos[0],pos[1],pos[0],pos[1]+2*i,6+(i+1)/2,6);
                            if(!kingInCheck(simMove(board,tempMove,turn,false),turn,true))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                    }
                }
            }
            break;
        }
    }
    moves[ind]=NULL; //NULL pointer. I am using this to denote the end of the array instead of having to pass the length in. This may or may not have some issues... I think actually it will be perfectly fine. The one potential problem is if a random pointer value is set to null when uninitialized but I don't think that that can happen
    return moves;
}

char*** getAllMoves(char** board, bool** hasMoved, bool turn, char enPas){
    char*** moves=new char**[256]; //I am assuming 218 as a more than reasonable upper bound, especially given that nobody has managed to contruct a position with more than 218, and anything even remotely close to that is extremely unlikely to naturally occur. Putting 256 cause char
    char*** tempMoves;
    char tpos[2];
    char tind=0;
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(board[i][j] && board[i][j]/7==turn){
                tpos[0]=i;
                tpos[1]=j;
                tempMoves=getMoves(board,hasMoved,tpos,(board[i][j]-1)%6+1,turn,enPas); //error is here when a piece has no moves because check
                for(int x=0;tempMoves[x]!=NULL;x++){
                    moves[tind++]=tempMoves[x];
                }
            }
        }
    }
    return moves;
}

void movePiece(char** board, bool** hasMoved, char** move, bool turn){
    char color=turn*6;
    if(move[2][0]==1){ //En Passant
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+move[2][1];
        board[move[1][0]+2*turn-1][move[1][1]]=0;
    }
    else if(move[2][0]>=2 && move[2][0]<6){ //Promotion keys. It's just the piece number :P hehe cheeky me
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+move[2][0];
    }
    else if(move[2][0]==6){ //Left side castle (long)
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+6;
        board[move[1][0]][move[1][1]+1]=color+4;
        board[move[0][0]][move[0][1]-4]=0;
        hasMoved[move[0][0]][move[0][1]-4]=1;
    }
    else if(move[2][0]==7){ //Right side castle (char)
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+6;
        board[move[1][0]][move[1][1]-1]=color+4;
        board[move[0][0]][move[0][1]+3]=0;
        hasMoved[move[0][0]][move[0][1]+3]=1;
    }
    else{
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+move[2][1];
    }
    hasMoved[move[0][0]][move[0][1]]=1;
}
/*
char** pgnConvert(char* pgn, char n, char** board, bool** hasMoved, bool turn, char enPas){ //not yet complete
    char piece;
    char* pgnClean=new char[n];
    char** move=new char*[3];
    for(int i=0;i<3;i++){
        move[i]=new char[2];
    }
    
    if(pgn[0]=='-'){ //move was skipped for whatever reason. Only ever saw one example of this
        return 0;
    }
    if(pgn[0]=='O'){ //castling
        if(pgn[3]){ //long
            move=createMove(7*turn,4,7*turn,2,6,6);
        }
        else{ //short
            move=createMove(7*turn,4,7*turn,6,7,6);
        }
        return move;
    }
    
    char relstart=1;
    if(pgn[0]=='K'){
        piece=6;
    }else if(pgn[0]=='Q'){
        piece=5;
    }else if(pgn[0]=='R'){
        piece=4;
    }else if(pgn[0]=='N'){
        piece=3;
    }else if(pgn[0]=='B'){
        piece=2;
    }else{
        piece=1;
        relstart=0;
    }
    
    char tind=0;
    for(int i=relstart;i<n;i++){
        if(pgn[i]=='x' || pgn[i]=='=' || pgn[i]=='+' || pgn[i]=='#')
            continue;
        pgnClean[tind++]=pgn[i];
    }
    //after this it is only 2-4 characters representing (start y) (start x) end y end x (promote piece)
    char ylook,xlook,spKey;
    bool isy=0,isx=0;
    if((pgnClean[1]>='a' && pgnClean[1]<='h') || (tind>4 && pgnClean[2]>='a' && pgnClean[2]<='h')){
        ylook=pgnClean[0]-'a'; //first character is extra info for row
        isy=1;
        if(pgnClean[1]>='1' || pgnClean[1]<='8'){
            xlook=pgnClean[0]-'1'; //second char is extra info for col
            isx=1;
        }
    }
    if(pgnClean[0]>='1' || pgnClean[0]<='8'){ //not sure I can put an else on this statement, so im leaving it like this.
        xlook=pgnClean[0]-'1';
        isx=1;
    }
    if(pgnClean[tind-1]=='Q'){
        spKey=5;
    }else if(pgnClean[tind-1]=='R'){
        spKey=4;
    }else if(pgnClean[tind-1]=='N'){
        spKey=3;
    }else if(pgnClean[tind-1]=='B'){
        spKey=2;
    }else{
        spKey=0; //no promotion
    }
    char*** moves;
    char pos[2];
    switch(isy+isx*2){
        case 0: //no restriction
            for(int i=0;i<8;i++){
                for(int j=0;j<8;j++){
                    if(board[i][j]==piece+turn*6){
                        pos[0]=i;
                        pos[1]=j;
                        moves=getMoves(board,hasMoved,pos,piece,turn,enPas);
                    }
                }
            }
            
    }
    //finish this. Not complete yet
    return move;
}
*/
bool validSearch(char*** allMoves, char** move, char size){ //check if given move is within all valid moves for a turn.
    //relies on the assumption that allMoves is generated by my getAllMoves functions. Namely, this means that it assumes that moves are generated row by row, left to right. I.e, spatially sorted. So if it meets the second criterium, inputs are valid
    //do binary search to find lower bound  over start pos and then linear search until end pos or match. I can't really think of a way in general to reduce the searching within, but this should honestly be okay anyways because it won't exceed 27 moves, and having some protracted method to abstract some "sorted" quality from how I generate end points isn't worth it for such a small search space. For the input space where it is very clear and can get a bit bigger (a normal maximum likely around 50 ish) is roughly worth it.
    //tbh I doubt ill ever use this function. It's really just the reverse, finding the index of a certain valid move within an array of every possible move in the game. That, I will try to sort at minimum by piece and then likely by start, end, spkey. And that will be very useful to bin search cause itll be around 4000 elements.
    char low=0;
    char high=size;
    char mid;
    while(low<=high){
        mid=(low+high)/2;
        if(mid==0){ //Would get out of bounds error otherwise and would never find this move (the piece is the first valid piece w/ moves
            if(allMoves[mid][0][0]==move[0][0] && allMoves[mid][0][1]==move[0][1])
                break;
            else
                return 0; //ain't no chance. Not sure if necessary
        }
        else if(allMoves[mid][0][0]>move[0][0] || (allMoves[mid][0][0]==move[0][0] && allMoves[mid][0][1]>move[0][1])){ //move less than current pos
            high=mid-1;
        }
        else if(allMoves[mid][0][0]<move[0][0] || (allMoves[mid][0][0]==move[0][0] && allMoves[mid][0][1]<move[0][1])){ //move greater than current pos
            low=mid+1;
        } //now the implication is that the start point is the same. So just need to check if bottom neighbor is different.
        else if(allMoves[mid][0][0]!=allMoves[mid-1][0][0] || allMoves[mid][0][1]!=allMoves[mid-1][0][1]){ //match for lower point.
            break;
        }
        else{ //have starting pos but not lower bound...
            high=mid-1;
        }
    }
    if(low>high){
        return 0;
    }
    for(int i=mid;allMoves[i][0][0]==move[0][0] && allMoves[i][0][1]==move[0][1];i++){
        if(allMoves[i][1][0]==move[1][0] && allMoves[i][1][1]==move[1][1] && allMoves[i][2][0]==move[2][0])
            return 1; //total match!
    }
    return 0;
}

void displayBoard(char** board){
    char tcolor;
    for(int i=7;i>-1;i--){
        for(int j=0;j<8;j++){
            if(board[i][j]==0){
                printf("   ");
                continue;
            }
            if(board[i][j]>6){
                tcolor='b';
            }
            else{
                tcolor='w';
            }
            switch((board[i][j]-1)%6){
                case 0:
                    printf("%cp",tcolor);
                    break;
                case 1:
                    printf("%cb",tcolor);
                    break;
                case 2:
                    printf("%ck",tcolor);
                    break;
                case 3:
                    printf("%cr",tcolor);
                    break;
                case 4:
                    printf("%cQ",tcolor);
                    break;
                case 5:
                    printf("%cK",tcolor);
                    break;
            }
            printf(" ");
        }
        printf("\n");
    }
}

void playGame(){
    //prob use printf cause chars will be auto printed as ascii characters rather than as numerical values as we want (can case in std::cout)
    //NOTE: NEED TO ADD THREEFOLD REP AND FIFTY MOVE RULE
    char*** t=setUp();
    char** board=t[0];
    bool** hasMoved=(bool**) t[1];
    char*** moves;
    displayBoard(board);
    int option, i;
    bool turn=0;
    char enPas=-3;
    while(1){
        moves=getAllMoves(board,hasMoved,turn,enPas); //ignoring en passant for now...
        for(i=0;moves[i]!=NULL;i++){
            printf("%d:  %d %d    %d %d\n",i,moves[i][0][1],moves[i][0][0],moves[i][1][1],moves[i][1][0]);
        }
        if(i==0){ //no moves available. Game is over either due to checkmate or stalemate
            int res=2;
            if(kingInCheck(board,turn,false)){
                res=turn^1;
            }
            if(res==0){
                std::cout<<"White wins\n";
            }
            else if(res==1){
                std::cout<<"Black Wins\n";
            }
            else{
                std::cout<<"Draw\n";
            }
            break;
        }
        printf("Choose a move from the menu above or -1 for quit: ");
        scanf("%d",&option);
        if(option>=i || option<-1){
            std::cout<<"Invalid choice\n";
            continue;
        }
        if(option==-1){
            break;
        }
        movePiece(board, hasMoved, moves[option], turn);
        if(moves[option][2][0]==8){ //double pawn move
            enPas=moves[option][1][1];
        }
        displayBoard(board);
        turn^=1;
        for(i=0;moves[i]!=NULL;i++){ //clean-up
            destroyMove(moves[i]);
        }
    }
    //final clean-up
    destroyBoard(board);
    destroyBoard((char**)hasMoved); //have to convert like this to char** bc that's the input type for the function, but it just deletes the memory unbiasedly, and I allocatted them in the same way, so it is fine. I should be a little wary of what I did, but I think this should be completely fine and no errors.
    delete[] t;
}
/*
int main(){ //if you want to compile this on its own you have to include main. But if you want to use it elsewhere then you have to remove main.
    //playGame();
    char*** m;
    char** move;
    int x,y,x2,y2;
    printf("Enter 4: ");
    scanf("%d %d %d %d",&y,&x,&y2,&x2);
    move=createMove(y,x,y2,x2,2,1);
    std::cout<<allHash(move)<<"\n";
    return 0;
}
 */
//takes about 12.5 seconds to run getAllMoves 1000000 times without deletion. Pretty good actually, I think. Would be pretty hard to improve as I wrote it quite efficiently.
//NOTE: need to retest because I changed to switch statement, which I think will be a bit faster, but I also added in deletion, so that might be something. Although I was talking more about deleting all of the moves.
//13.3 with deletion after making it a bit more efficient so that's pretty good


//I think that I have determined that searching through the set of all moves will be most efficient with just a hash map (likely one for which I create the hashing so that there are no collisions)
//I already have perfect information about what elements are within the set and certain qualities of the elements, so rather than manipulating the ordering so that I can do binary search, why not just do an instantaneous lookup from a hashmap? And actually, I can store it as an array in general and still order the elements in such a way that the desired hashing is preservered or smth. Like I could do (naively) start coords (like 0 to 63)*64*8+end coords*8 + spKey (that actually fits within an int pretty comfortably... but it would be nice not to have the array of all possible moves be bigger than necessary. There is definitely a way that I can manipulate this... Maybe I can do start to end coords first and spkey at the front or smth. Idk. Eh, it might just be best to make a hashmap anyways (aka std::unordered_map), but like I need to look at exactly how it allocates memory for that to see if it just creates a bigger space and then doesn't use it or allocates for keys dynamically or what
//okay so after some research, it basically is what I expected it to be. You have an array of size #keys in each of which is a bucket/linked list of all of the elements which have been mapped to corresponding to that key
//I feel that there must be a more efficient memory way to do it dynamically without taking up as much memory but I might be wrong...
//nah I think this is gonna have to be the way. (you could I suppose choose to allocate a pointer key or whatever for each spot dynamically but eh anyways who the hell cares. Especially for this it ain't that bad... and the speedup is insane. Guaranteed perfect hash hell yeah)
