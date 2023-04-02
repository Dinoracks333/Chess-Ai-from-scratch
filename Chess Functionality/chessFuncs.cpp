//chess functionality written in c++ meant to be used for chess ai
//kinda like my own little chess library
//I will have a different program for the actual ai functions though (on top of the base neural network mini library that I wrote). Ah, with all of these libraries, I'm basically an academic :P or a librarian lol

#include <stdio.h>
#include <iostream>

//figured out the memory stuff yay!! pretty sure at the very least :) so, nice
/*
I wanna be super efficient, probably to the point where it is going too far,
so im gonna use chars (aka 1 byte numbers, 0 to 255) for most of the variables. 
Saves memory and limits cache misses though, so it does speed up the program
 

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

char** setUpBoard(){
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
    
    return board;
}
//originally had setup returning both hasMoved and board and was just casting, but there were some weird issues, and so i decided to change things around and make it explicit and not trying to toe the line at all
bool** setUpHM(){
    bool** hasMoved=new bool*[8];
    bool* ti=new bool[64];
    for(int i=0;i<8;i++){
        hasMoved[i]=ti+i*8;
        for(int j=0;j<8;j++){
            hasMoved[i][j]=0;
        }
    }
    return hasMoved;
}

bool inside(char x, char y){
    return x>=0 && x<8 && y>=0 && y<8;
}

char** createMove(char ystart, char xstart, char yend, char xend, char spKey, char piece){ //simplifies creating a new move
    char** move = new char*[3];
    char* ti=new char[6]; //contiguous memory faster to allocate and deallocate (by quite a lot) (and probably to retrieve from memory)
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
    delete[] board[0]; //so because i am technically representing the 1d part as contiguous memory, I don't have to call delete on each index.
    delete[] board;
}

void destroyHM(bool** hasMoved){ //again just being clear and creating a separate function for this even though I could cast to char** and use destroyBoard
    delete[] hasMoved[0];
    delete[]hasMoved;
}

bool kingInCheck(char** board, bool turn){
    //basically inverses piece moves to find if enemies can move onto itself
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
                if(inside(a,b) && board[a][b]==opcolor+3)
                    return true;
            }
        }
    }
    
    //pawns
    const char s=-2*turn+1;
    if((inside(king[0]+s,king[1]+1) && board[king[0]+s][king[1]+1]==opcolor+1) || (inside(king[0]+s,king[1]-1) && board[king[0]+s][king[1]-1]==opcolor+1))
        return true;
    
    //bishops
    char tx,ty;
    for(int i=-1;i<2;i+=2){
        for(int j=-1;j<2;j+=2){
            for(ty=king[0]+i,tx=king[1]+j;inside(ty,tx) && !board[ty][tx];ty+=i,tx+=j); //succint and beautiful :P these for loops are the bomb
            if(inside(ty,tx) && (board[ty][tx]==opcolor+2 || board[ty][tx]==opcolor+5))
                return true;
        }
    }
    
    //rooks
    char j;
    for(int a=-1;a<2;a+=2){
        for(int i=0;i<2;i++){
            j=i^1;
            for(ty=king[0]+a*i,tx=king[1]+a*j;inside(ty,tx) && !board[ty][tx];ty+=a*i,tx+=a*j);
            if(inside(ty,tx) && (board[ty][tx]==opcolor+4 || board[ty][tx]==opcolor+5))
                return true;
        }
    }
    
    //king
    for(int i=-1;i<2;i++){
        for(int j=-1;j<2;j++){
            if(inside(king[0]+i,king[1]+j) && board[king[0]+i][king[1]+j]==opcolor+6)
                return true;
        }
    }
    return false;
}

bool simMove(char** board, char** move, bool turn){
    //much faster to move then move back rather than creating new board
    char color=turn*6;
    char temp;
    bool res;
    char piece=board[move[0][0]][move[0][1]]; //can't trust piece value being fed in when simulating as bishop or rook while queen
    if(move[2][0]==1){ //En Passant
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+move[2][1];
        temp=board[move[1][0]+2*turn-1][move[1][1]];
        board[move[1][0]+2*turn-1][move[1][1]]=0;
        res=kingInCheck(board,turn);
        board[move[1][0]+2*turn-1][move[1][1]]=temp;
        board[move[0][0]][move[0][1]]=piece;
        board[move[1][0]][move[1][1]]=0;
    }
    else if(move[2][0]>=2 && move[2][0]<6){ //Promotion keys. It's just the piece number :P hehe cheeky me
        board[move[0][0]][move[0][1]]=0;
        temp=board[move[1][0]][move[1][1]];
        board[move[1][0]][move[1][1]]=color+move[2][0];
        res=kingInCheck(board,turn);
        board[move[1][0]][move[1][1]]=temp;
        board[move[0][0]][move[0][1]]=piece;
    }
    else if(move[2][0]==6){ //Left side castle (long)
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+6;
        board[move[1][0]][move[1][1]+1]=color+4;
        board[move[0][0]][move[0][1]-4]=0;
        res=kingInCheck(board,turn);
        board[move[0][0]][move[0][1]]=color+6;
        board[move[1][0]][move[1][1]]=0;
        board[move[1][0]][move[1][1]+1]=0;
        board[move[0][0]][move[0][1]-4]=color+4;
    }
    else if(move[2][0]==7){ //Right side castle (short)
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+6;
        board[move[1][0]][move[1][1]-1]=color+4;
        board[move[0][0]][move[0][1]+3]=0;
        res=kingInCheck(board,turn);
        board[move[0][0]][move[0][1]]=color+6;
        board[move[1][0]][move[1][1]]=0;
        board[move[1][0]][move[1][1]-1]=0;
        board[move[0][0]][move[0][1]+3]=color+4;
    }
    else{
        board[move[0][0]][move[0][1]]=0;
        temp=board[move[1][0]][move[1][1]];
        board[move[1][0]][move[1][1]]=color+move[2][1];
        res=kingInCheck(board,turn);
        board[move[1][0]][move[1][1]]=temp;
        board[move[0][0]][move[0][1]]=piece;
    }
    return res;
}

char*** getMoves(char** board, bool** hasMoved, char pos[2], char piece, bool turn, char enPas){
    char*** moves=new char**[28]; //I believe this is the most amount of moves from any one piece (27+1 for null)
    char** tempMove;
    char color=turn*6;
    char ind=0;
    switch(piece){ //switch is supposed to be faster with nested ifs like I have here bc you declare the jumps yourself.
        case 1:{ //pawn
            const char s=-2*turn+1;
            if(pos[0]==6*turn+(turn^1) && !hasMoved[pos[0]][pos[1]] && !board[pos[0]+2*s][pos[1]] && !board[pos[0]+s][pos[1]]){ //double move from start
                tempMove=createMove(pos[0],pos[1],pos[0]+2*s,pos[1],8,piece);
                if(!simMove(board,tempMove,turn))
                    moves[ind++]=tempMove;
                else
                    destroyMove(tempMove);
            }
            if(pos[0]+s<8 && pos[0]+s>=0){
                if(!board[pos[0]+s][pos[1]]){ //normal one space move
                    if(pos[0]+s==(turn^1)*7){ //promotion
                        for(int i=2;i<6;i++){
                            tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1],i,piece);
                            if(!simMove(board,tempMove,turn))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                    }
                    else{
                        tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1],0,piece);
                        if(!simMove(board,tempMove,turn))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
                for(int i=-1;i<2;i+=2){ //captures
                    if(pos[1]+i<8 && pos[1]+i>=0 && board[pos[0]+s][pos[1]+i] && board[pos[0]+s][pos[1]+i]/7==(turn^1)){
                        if(pos[0]+s==(turn^1)*7){
                            for(int j=2;j<6;j++){ //promotion
                                tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,j,piece);
                                if(!simMove(board,tempMove,turn))
                                    moves[ind++]=tempMove;
                                else
                                    destroyMove(tempMove);
                            }
                        }
                        else{
                            tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,0,piece);
                            if(!simMove(board,tempMove,turn))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                    }
                    if(pos[1]+i==enPas && pos[0]==4-turn){ //en passant
                        tempMove=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,1,piece);
                        if(!simMove(board,tempMove,turn))
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
                    for(ty=pos[0]+i,tx=pos[1]+j;inside(ty,tx) && !board[ty][tx];ty+=i,tx+=j){ //go until hits a piece or edge of the board
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!simMove(board,tempMove,turn))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                    if(inside(ty,tx) && board[ty][tx]/7==(turn^1)){
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!simMove(board,tempMove,turn))
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
                            if(!simMove(board,tempMove,turn))
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
                    for(ty=pos[0]+a*i,tx=pos[1]+a*j;inside(ty,tx) && !board[ty][tx];ty+=a*i,tx+=a*j){ //go until hits a piece or edge of board
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!simMove(board,tempMove,turn))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                    if(inside(ty,tx) && board[ty][tx]/7==(turn^1)){
                        tempMove=createMove(pos[0],pos[1],ty,tx,0,piece);
                        if(!simMove(board,tempMove,turn))
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
                        if(!simMove(board,tempMove,turn))
                            moves[ind++]=tempMove;
                        else
                            destroyMove(tempMove);
                    }
                }
            }
            if(!hasMoved[pos[0]][pos[1]]){ //castling
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
                        char** t1=createMove(pos[0],pos[1],pos[0],pos[1]+i,0,6);
                        char** t2=createMove(pos[0],pos[1],pos[0],pos[1]+2*i,0,6);
                        if(!kingInCheck(board,turn) && !simMove(board,t1,turn) && !simMove(board,t2,turn)){
                            tempMove=createMove(pos[0],pos[1],pos[0],pos[1]+2*i,6+(i+1)/2,6);
                            if(!simMove(board,tempMove,turn))
                                moves[ind++]=tempMove;
                            else
                                destroyMove(tempMove);
                        }
                        destroyMove(t1);
                        destroyMove(t2);
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
    char*** moves=new char**[125]; //I don't think anything will exceed this bound, but can always change it if there are problems...
    char*** tempMoves;
    char tpos[2];
    char tind=0;
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(board[i][j] && board[i][j]/7==turn){
                tpos[0]=i;
                tpos[1]=j;
                tempMoves=getMoves(board,hasMoved,tpos,(board[i][j]-1)%6+1,turn,enPas);
                for(int x=0;tempMoves[x]!=NULL;x++){
                    moves[tind++]=tempMoves[x];
                }
                delete[] tempMoves;
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
    else if(move[2][0]>=2 && move[2][0]<6){ //Promotion keys
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+move[2][0];
    }
    else if(move[2][0]==6){ //Left side castle (long)
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+6;
        board[move[1][0]][move[1][1]+1]=color+4;
        board[move[0][0]][move[0][1]-4]=0;
        hasMoved[move[0][0]][move[0][1]-4]=1;
        hasMoved[move[1][0]][move[1][1]+1]=1;
    }
    else if(move[2][0]==7){ //Right side castle (char)
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+6;
        board[move[1][0]][move[1][1]-1]=color+4;
        board[move[0][0]][move[0][1]+3]=0;
        hasMoved[move[0][0]][move[0][1]+3]=1;
        hasMoved[move[1][0]][move[1][1]-1]=1;
    }
    else{
        board[move[0][0]][move[0][1]]=0;
        board[move[1][0]][move[1][1]]=color+move[2][1];
    }
    hasMoved[move[0][0]][move[0][1]]=1;
    hasMoved[move[1][0]][move[1][1]]=1;
}

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
        std::cout<<(char)('1'+i)<<" ";
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
    std::cout<<"  ";
    for(int i=0;i<8;i++){
        std::cout<<(char)('a'+i)<<"  ";
    }
    std::cout<<"\n";
}

char** pgnConvert(char* pgn, char** board, bool** hasMoved, bool turn, char enPas){ //confirmed working yay!! Helped me iron out a few bugs with other functions as well
    int n=strlen(pgn);
    char end[2];
    char piece;
    char* pgnClean=new char[n];
    char** move;
    if(pgn[0]=='-'){ //move was skipped for whatever reason. Only ever saw one example of this
        return 0;
    }
    if(pgn[0]=='O'){ //castling
        if(pgn[3]=='-'){ //long
            move=createMove(7*turn,4,7*turn,2,6,6);
        }
        else{ //short
            move=createMove(7*turn,4,7*turn,6,7,6);
        }
        delete[] pgnClean;
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
    ylook=xlook=8; //dont want to go -1 and mess with weird signed vs usigned stuff, so 8 is out of the range.
    if((pgnClean[0]>='a' && pgnClean[0]<='h') && ((pgnClean[1]>='a' && pgnClean[1]<='h') || (pgnClean[2]>='a' && pgnClean[2]<='h'))){
        xlook=pgnClean[0]-'a';
        if(pgnClean[1]>='1' && pgnClean[1]<='8'){
            ylook=pgnClean[1]-'1'; //second char is extra info for col
        }
    }
    if(pgnClean[0]>='1' && pgnClean[0]<='8'){
        ylook=pgnClean[0]-'1';
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
    end[0]=pgnClean[(ylook!=8)+(xlook!=8)+1]-'1';
    end[1]=pgnClean[(ylook!=8)+(xlook!=8)]-'a';
    char*** moves;
    char pos[2];
    bool found=0;
    for(int i=0;i<8 && !found;i++){ //search through matching pieces in valid row/col until you find a piece that can move to the square
        if(ylook!=8 && i!=ylook)
            continue;
        for(int j=0;j<8 && !found;j++){
            if(xlook!=8 && j!=xlook)
                continue;
            if(board[i][j]==piece+turn*6){
                pos[0]=i;
                pos[1]=j;
                moves=getMoves(board,hasMoved,pos,piece,turn,enPas);
                for(int m=0;moves[m];m++){
                    if(moves[m][1][0]==end[0] && moves[m][1][1]==end[1]){
                        found=1;
                        if(!spKey)
                            spKey=moves[m][2][0];
                        //no break so that I delete all of the extra moves for this piece that I am checking
                    }
                    destroyMove(moves[m]);
                }
                delete[] moves;
            }
        }
    }
    if(!found){
        return (char**) -1; //error occurred and nothing was found!!!! Invalid move or my program is wrong
    }
    move=createMove(pos[0],pos[1],end[0],end[1],spKey,piece);
    delete[] pgnClean;
    return move;
}

void playGame(){
    //prob use printf cause chars will be auto printed as ascii characters rather than as numerical values as we want (or can cast in std::cout)
    //NOTE: NEED TO ADD THREEFOLD REP AND FIFTY MOVE RULE   (probably use hashmap for threefold rep and just a simple var for fifty move)
    char** board=setUpBoard();
    bool** hasMoved=setUpHM();
    char*** moves;
    displayBoard(board);
    int option, i;
    bool turn=0;
    char enPas=10;
    while(1){
        moves=getAllMoves(board,hasMoved,turn,enPas);
        for(i=0;moves[i]!=NULL;i++){
            printf("%d:  %d %d    %d %d\n",i,moves[i][0][1],moves[i][0][0],moves[i][1][1],moves[i][1][0]);
        }
        if(i==0){ //no moves available. Game is over either due to checkmate or stalemate
            int res=2;
            if(kingInCheck(board,turn)){
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
        if(moves[option][2][0]==8){ //double pawn move, change en passant space
            enPas=moves[option][1][1];
        }
        else{
            enPas=10;
        }
        displayBoard(board);
        turn^=1;
        for(i=0;moves[i]!=NULL;i++){ //clean-up
            destroyMove(moves[i]);
        }
        delete[] moves; //also have to get rid of space allocated to point to individual moves in this case
    }
    //final clean-up
    destroyBoard(board);
    destroyHM(hasMoved); //have to convert like this to char** bc that's the input type for the function, but it just deletes the memory unbiasedly, and I allocatted them in the same way, so it is fine. I should be a little wary of what I did, but I think this should be completely fine and no errors.
}

/*
int main(){ //if you want to compile this on its own you have to include main. But if you want to use it elsewhere then you have to remove main. Prob can do both if you just prototype it
    
    return 0;
}
*/
