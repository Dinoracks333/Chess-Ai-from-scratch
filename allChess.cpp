#include <chrono>
#include <cstdint>
#include <iostream>
#include "perfectHash.cpp"

uint64_t time() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
/*
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
*/
int compareMoves(char** move1,char** move2){ //assuming same piece for now. Technically could just compare element by element (looking at greatest elements first) but...
    int t1=move1[0][0]*72+move1[0][1]*9+move1[1][0]*4608+move1[1][1]*576+move1[2][0];
    int t2=move2[0][0]*72+move2[0][1]*9+move2[1][0]*4608+move2[1][1]*576+move2[2][0];
    return t1-t2;
    //note: not comparing pieces
}

void sortMoves(char*** moves){ //bubble sort because im sorting only on a small scale and again just way easier to write.
    char** temp;
    for(int i=0;moves[i]!=NULL;i++){
        for(int j=i;moves[j]!=NULL;j++){
            if(compareMoves(moves[i],moves[j])>0){
                temp=moves[i];
                moves[i]=moves[j];
                moves[j]=temp;
            }
        }
    }
}

char*** getTrueMoves(char pos[2], char piece){
    //remove non positional conditions
    //I haven't written this in here yet, but everytime that I am adding a new move, I need to check that the king isn't in check
    char*** moves=new char**[30]; //should be about right? cause only one pos at a time remember. 28 should be fine but 30 to be safe.
    char ind=0;
    switch(piece){ //switch is supposed to be faster with nested ifs like I have here bc you declare the jumps yourself.
        case 1:{ //pawn
            for(int s=1;s>-2;s-=2){
                bool turn=-(s-1)/2;
                if(pos[0]==turn*6+(turn^1)){ //double move from start
                    moves[ind++]=createMove(pos[0],pos[1],pos[0]+2*s,pos[1],8,piece);
                }
                if(pos[0]+s<8 && pos[0]+s>=0){ //one space
                    if(pos[0]+s==(turn^1)*7){ //promotion
                        for(int i=2;i<6;i++){
                            moves[ind++]=createMove(pos[0],pos[1],pos[0]+s,pos[1],i,piece);
                        }
                    }
                    else{
                        moves[ind++]=createMove(pos[0],pos[1],pos[0]+s,pos[1],0,piece);
                    }
                    for(int i=-1;i<2;i+=2){ //captures
                        if(pos[1]+i<8 && pos[1]+i>=0){
                            if(pos[0]+s==(turn^1)*7){
                                for(int j=2;j<6;j++){
                                    moves[ind++]=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,j,piece);
                                }
                            }
                            else{
                                moves[ind++]=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,0,piece);
                            }
                            if(pos[0]==4-turn){ //en passant
                                moves[ind++]=createMove(pos[0],pos[1],pos[0]+s,pos[1]+i,1,piece); //note that normal capture is considered different from en passant
                            }
                        }
                    }
                }
            }
            break;
        }
        case 2:{ //bishop
            char tx,ty;
            for(int i=-1;i<2;i+=2){
                for(int j=-1;j<2;j+=2){
                    for(ty=pos[0]+i,tx=pos[1]+j;inside(ty,tx);ty+=i,tx+=j){
                        moves[ind++]=createMove(pos[0],pos[1],ty,tx,0,piece);
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
                        if(inside(a,b)){
                            moves[ind++]=createMove(pos[0],pos[1],a,b,0,piece);
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
                    for(ty=pos[0]+a*i,tx=pos[1]+a*j;inside(ty,tx);ty+=a*i,tx+=a*j){
                        moves[ind++]=createMove(pos[0],pos[1],ty,tx,0,piece);
                    }
                }
            }
            break;
        }
        case 5:{ //queen
            char*** temp=getTrueMoves(pos,2); //simulate as bishop
            for(int i=0;i<14 && temp[i]!=NULL;i++){
                temp[i][2][1]=piece;
                moves[ind++]=temp[i];
            }
            temp=getTrueMoves(pos,4); //simulate as rook
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
                    if(inside(pos[0]+i,pos[1]+j)){
                        moves[ind++]=createMove(pos[0],pos[1],pos[0]+i,pos[1]+j,0,piece);
                    }
                }
            }
            /* //removing for now cause I just wanna put them at the end
            if((pos[0]==7 || pos[0]==0) && pos[1]==4){ //castling
                for(int i=-1;i<2;i+=2){ //lol so much simpler when I don't have to look for checks
                    moves[ind++]=createMove(pos[0],pos[1],pos[0],pos[1]+2*i,6+(i+1)/2,6);
                }
            }
            */
            break;
        }
    }
    moves[ind]=NULL; //NULL pointer. I am using this to denote the end of the array instead of having to pass the length in. This may or may not have some issues... I think actually it will be perfectly fine. The one potential problem is if a random pointer value is set to null when uninitialized but I don't think that that can happen
    return moves;
}

void genAll(){
    FILE* fout=fopen("allChessMoves.txt","w");
    char pos[2]={7,0};
    char*** moves;
    int tot=0;
    int tpiece;
    for(int piece=1;piece<7;piece++){
        tpiece=0;
        for(int i=0;i<8;i++){
            pos[0]=i;
            for(int j=0;j<8;j++){
                if(piece==1 && (i==0 || i==7))
                    continue;
                pos[1]=j;
                moves=getTrueMoves(pos,piece);
                sortMoves(moves);
                for(int i=0;moves[i]!=NULL;i++,tot++,tpiece++){
                    fprintf(fout,"%d %d %d %d %d %d\n",moves[i][0][0],moves[i][0][1],moves[i][1][0],moves[i][1][1],moves[i][2][0],moves[i][2][1]); //could change order if you want, but I thought it might be good to keep some of the consistency
                    std::cout<<(int)moves[i][0][0]<<" "<<(int)moves[i][0][1]<<" "<<(int)moves[i][1][0]<<" "<<(int)moves[i][1][1]<<" "<<(int)moves[i][2][0]<<" "<<tot<<"\n";
                    destroyMove(moves[i]);
                }
            }
        }
        std::cout<<"total moves for "<<piece<<": "<<tpiece<<"\n";
    }
    moves=new char**[4];
    moves[0]=createMove(0,4,0,2,6,6);
    moves[1]=createMove(0,4,0,6,7,6);
    moves[2]=createMove(7,4,7,2,6,6);
    moves[3]=createMove(7,4,7,6,7,6);
    for(int i=0;i<4;i++,tot++){
        fprintf(fout,"%d %d %d %d %d %d\n",moves[i][0][0],moves[i][0][1],moves[i][1][0],moves[i][1][1],moves[i][2][0],moves[i][2][1]);
        std::cout<<(int)moves[i][0][0]<<" "<<(int)moves[i][0][1]<<" "<<(int)moves[i][1][0]<<" "<<(int)moves[i][1][1]<<" "<<(int)moves[i][2][0]<<" "<<tot<<"\n";
        destroyMove(moves[i]);
    }
    delete[] moves;
    std::cout<<"total moves for all pieces: "<<tot<<"\n";
}

int main(){
    //okay so I got all of it after modifying the getmoves to remove all non-positional conditions. Now, I just have to figure out how I can extract info from it for my perfect hash function!
    FILE* fin=fopen("allChessMoves.txt","r");
    char** move=createMove(0,0,0,0,0,0);
    int h;
    for(int i=0;i<4112;i++){
        fscanf(fin,"%d %d %d %d %d %d",move[0],move[0]+1,move[1],move[1]+1,move[2],move[2]+1);
        h=allHash(move);
        std::cout<<i<<" "<<h<<" "<<(int)move[0][0]<<" "<<(int)move[0][1]<<" "<<(int)move[1][0]<<" "<<(int)move[1][1]<<" "<<(int)move[2][0]<<"\n";
        if(h!=i)
            std::cout<<"WRONG WRONG WRONG WRONG WRONG!!!!!\n";
    }
    return 0;
}
//13.3 seconds no deletion except for boards and stuff being used internally
//14.2 seconds for total deletion
//over 1 million calls to getAllMoves (not getTrueMoves)
//so yeah obviously the deletion does take a bit to do but yknow. Whatever... It's fine. I'ts like 14% slower :( but it's not that bad. I hope. I can try to do as much micro efficiencies as I want later but I think this part won't be too too bad to do anyways. Ah idk I am a bit sad that it is slower w/ deletion but obv it's gonna be. It's fine.
//tried using the temp hash function that I have right now and it literally didn't add any time. Like the normal fluctuations were greater.
//it still is really fast!! Okay, Alex?? There's no need to worry :) so much faster than the python version and to think that the full working version with deletion is not all that much slower is great!
//okay so down to 13.3 with better deletion! damn that's crazy how using contiguous memory is so much easier for it to allocate and deallocate even on a very small scale. So very nice.
//I can certainly accept this :)
