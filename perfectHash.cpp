//
//  perfectHash.cpp
//  
//
//  Created by Alejandro Casillas on 3/26/23.
//

//my overoptimized masterpiece!
//only took 3-4 days and a couple of *literal* painful headaches
//I am proud of myself for writing code that would be considered so shitty in any other circumstance
//like seriously I coulda done binary search over it and it would have taken me like 5 minutes to write, but this is probably at least 20x faster than that, if I had to guess. Maybe better.

/* NUMBER OF MOVES FOR EACH PIECE*/
//Pawn: 440
//Bishop: 560
//Knight: 336
//Rook: 896
//Queen: 1456
//King: 424
//Total: 4112
int allHash(char** move){ //using perfect hash function to find index of move within array of every possible move in chess (does not include check or takes or any other extraneous bloat). This function is written to be incredibly fast by not searching the array at all and only using addition, subtraction, multiplication, and some conditionals, but pretty minor ones (switch statements are also pretty fast, plus ive tried to optimize how I use them to get the full use out of the jump statements.
    //will partition into sections based on what piece. I think this should overall save memory because certain special keys are only used by certain ones so yeah man
    //maybe this can't really be called a hash function but whatever (bc it doesn't take input of any size. It is more just a pretty fast, specific function mapping the set of moves to array indexes. But im gonna keep the name allHash)
    //the reason I started using the name allHash anyways was because I was originally thinking of using a hashmap and then I was like ok maybe I can make my own hash to get the perfect keys and then I was like wait I can go and get the perfect location essentially and so it basically works because of perfect information of the data I am working with and yeah this is what we call overoptimization. Optimization at the cost of sanity and versatility/the ability to change the code or the data at all
    int start;
    char x,y,x2,y2;
    y=move[0][0];
    x=move[0][1];
    y2=move[1][0];
    x2=move[1][1];
    switch(move[2][1]){
        case 1: //pawn 1-5,8
            start=0;
            //somehow I did this one and it actually wasn't even that awful. I am amazed (probably because I had to practice 5 other ones first)
            switch(y){
                case 2:
                    start+=118; //118
                    break;
                case 3:
                    start+=162; //44
                    break;
                case 4:
                    start+=220;  //58
                    break;
                case 5:
                    start+=278; //58
                    break;
                case 6:
                    start+=322; //44
                    break;
                case 7:
                    start+=440; //118
                    break;
            }
            //At start of row now. Count to start of square from here.
            switch(y){
                case 1:
                case 6:
                    start+=(x!=0)*(16*x-5); //im sure it makes an insignificant difference, but I think doing just the conditional x!=0 might be a little faster than converting x to bool and having it convert back to int to perform the multiplication. Idk tbh
                    break;
                case 2:
                case 5:
                    start+=(x!=0)*(6*x-2);
                    break;
                case 3:
                case 4:
                    start+=(x!=0)*(8*x-3);
                    break;
            }
            //at the start of the square now. I guess that wasn't *too* bad. Kinda just like king (sorta). Knights make this look like a joke.
            switch(y){
                case 1:
                    //bottom row closest to promotion. All of the first ones here are promotion after all. After that, you have double move and normal stuff otherwise
                    if(y2>y) //upside
                        start+=12-4*(x==0||x==7) + x2-x+(x!=0)+(1+(x!=7))*(y2-y-1); //managed to make it concise lmao. Left part is handling how many promotion moves were before
                    else //bottom edge, promotion
                        start+=move[2][0]-2+4*(x2-x+(x!=0));
                    break;
                case 2:
                case 5: //same case because no promotions or en passant or double move from these two rows.
                    if(y2>y)
                        start+=3-(x==7) + x2-x+1-2*(x==0); //note that -(x==0||x==7) +(x!=0) becomes 2*(x==0)-(x==7)-1. Can replace if too confusing. But technically a little faster since you aren't checking something redundant
                    else
                        start+=x2-x+(x!=0);
                    break;
                case 3:
                    if(y2>y)
                        start+=5-2*(x==7) + x2-x+1-3*(x==0); //same optimization as before
                    else
                        start+=-2*(x==0)-((((x2-x)-4)*((x2-x)+1))>>1) + (move[2][0]!=0); //-0.5x^2+1.5x+2.0 to start before en Passant where x is (x2-x) factors to -1/2*(x-4)(x+1)
                    break;
                case 4:
                    if(y2>y)
                        start+=3-(x==7) - 3*(x==0) -((((x2-x)-4)*((x2-x)+1))>>1) + (move[2][0]!=0); //combined -(x==0||x==7) -2*(x==0) into -(x==7)-3*(x==0)
                    else
                        start+=x2-x+(x!=0);
                    break;
                case 6:
                    if(y2>y) //top edge, promotion
                        start+=4-(x==0||x==7) + move[2][0]-2+4*(x2-x+(x!=0));
                    else //downside, normal moves and double step
                        start+=y2-y+2+x2-x+(x!=0 && y2==y-1);
                    break;
            }
            //there's no way this works and was this clean right? Like that was way easier to write than I had feared.
            break;
        case 2:{ //bishop None
            //OKAY PRETTY SURE THIS ONE IS FINISHED!!!!! YAY!!!! THAT WAS HARD!!!! (pawns are gonna be terrible. but some of these others should be okay)
            start=440;
            //rings (like 8,7,6 yknow)
            //will count rings from outside in so ring 1 is outermost ring
            //ring 1: 7
            //ring 2: 9
            //ring 3: 11
            //ring 4: 13
            start+=(y*8+x)*7; //gets you to the beginning of actual square but ignores extra moves for other rings.
            switch(y){ //did have something cool going on before, but I decided that this was just kinda easier/a little faster
                case 1:
                    start+=12;
                    break;
                case 2:
                    start+=32;
                    break;
                case 3:
                    start+=56;
                    break;
                case 4:
                    start+=80;
                    break;
                case 5:
                    start+=100;
                    break;
                case 6:
                case 7:
                    start+=112;
            }
            //so that counted to the end of the row. Now adjust based on what was overcounted.
            switch(y){
                case 3:
                case 4:
                    if(x<5) //each subtracts their own little subset
                        start-=2*(5-(x>2?x:3));
                case 2:
                case 5:
                    if(x<6)
                        start-=2*(6-(x>1?x:2));
                case 1:
                case 6:
                    if(x<7)
                        start-=2*(7-(x>0?x:1));
            }
            //so now we are at start of square and must figure out from here which move we are within the square.
            //So basically you have restrictions on both sides of x relative to you both up and down. They each have some limiting factor given by the following:
            int lu=(x<y?x:y);
            int ld=(x<7-y?x:7-y);
            int ru=(7-x<y?7-x:y);
            int rd=(7-x<7-y?7-x:7-y); //essentially just the distance from up down left right of board closest bc diag
            int dist=(x2<x?x-x2:x2-x); //dist is same in both dirs so
            switch(2*(y2>y)+(x2>x)){ //LFG LFGGGGGGGG
                case 0: //left up
                    start+=lu-dist+(ru>dist?ru-dist:0);
                    break;
                case 1: //right up
                    start+=ru-dist+(lu>=dist?lu-dist+1:0);
                    break;
                case 2: //left down
                    start+=lu+ru-1+dist+(dist>rd?rd:dist-1);
                    break;
                case 3: //right down
                    start+=lu+ru-1+dist+(dist>=ld?ld:dist);
                    break;
            }
            break;
        }
        case 3:{ //knight None
            //middle is easy but corners and other edge spots I need to think about
            //ending up being pretty hard and ugly, especially the switch statement in the middle. But thank god. It works. I did it and it will be nice and fast and efficient yay
            start=1000;
            switch(y){
                case 1:
                    start+=26; //26
                    break;
                case 2:
                    start+=64; //38
                    break;
                case 3:
                    start+=116; //52
                    break;
                case 4:
                    start+=168; //52
                    break;
                case 5:
                    start+=220; //52
                    break;
                case 6:
                    start+=272; //52
                    break;
                case 7:
                    start+=310; //38
                    break;
            }
            //yeah im just gonna statically type a lot of this... way too hard to come up with a general way that is efficient. There might not be one honestly
            //lmaooooo this is such demon code. Literally demonic. (I make up for it later)
            switch(x){
                case 7:
                    switch(y){
                        case 0:
                        case 7:
                            start+=3;
                            break;
                        case 1:
                        case 6:
                            start+=4;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            start+=6;
                            break;
                    }
                case 6:
                case 5:
                case 4:
                case 3:
                    switch(y){
                        case 0:
                        case 7:
                            start+=4*(x<7?x:6)-8;
                            break;
                        case 1:
                        case 6:
                            start+=6*(x<7?x:6)-12;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            start+=8*(x<7?x:6)-16;
                            break;
                    }
                case 2:
                    switch(y){
                        case 0:
                        case 7:
                            start+=3;
                            break;
                        case 1:
                        case 6:
                            start+=4;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            start+=6;
                            break;
                    }
                case 1:
                    switch(y){
                        case 0:
                        case 7:
                            start+=2;
                            break;
                        case 1:
                        case 6:
                            start+=3;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            start+=4;
                            break;
                    }
            }
            //okay now I just have to figure out which move it is within the square.
            //same masking idea as with the king. Each of these numbers represents how much distance they remove for the given direction. 2 is closest then 1 then 0 is nothing
            //LMAO WE DON'T HAVE TO CONSIDER UP COLLISION!!! POG!!! (bc it will always cover the last parts of the moves, and those and anything past them can't be accessed, so no need to remove anything or check if greater cause youll never get over there)
            //also that means down is hella easy to handle because same idea applies there. All the bottom parts will be removed, so you can just subtract a static part without having to worry about anything.
            //this part is pretty smart, pretty nice. Really glad I took the time to get past the ugly logic of the awful for loop I had before. This really fits the setting of everything else I was doing and more of it being a true function/hash function (even though idek if hash applies for this kinda thing)
            //really glad I got all if statements out too lol. I was just able to make a few conditional to bool numbers which is pretty nice
            int d=(2-y>0?2-y:0);
            int l=(2-x>0?2-x:0);
            int r=(x-5>0?x-5:0);
            int ct=(y2-y+2-(y2>y))*2+(x2>x); //total move spot before masking
            switch(d*3+l+r){ //in my comments, the ternary represents first how much d and then how much l or r
                case 1: //01
                    ct-=((ct-(r==1&&ct))>>1)-(ct>5+(r==1)); //bit shifts coming in clutch for floor division (and faster than normal division)
                    break;
                case 2: //02
                    ct-=(ct+2-(r==2))>>1;
                    break;
                case 3: //10
                    ct-=2;
                    break;
                case 4: //11
                    ct-=2+((ct-(r==1))>>1)-(ct>5+(r==1));
                    break;
                case 5: //12
                    ct-=1+((ct+2-(r==2))>>1);
                    break;
                case 6: //20
                    ct-=4;
                    break;
                case 7: //21
                    ct-=4+(ct>(4+(r==1)));
                    break;
                case 8: //22
                    ct-=4+((ct-2-(r==2))>>1);
                    break;
            }
            start+=ct;
            break;
        }
        case 4: //rook None (castling is considered a king move not a rook move)
            start=1336;
            //super easy. 14 moves everywhere!!
            start+=(y*8+x)*14;
            //this gets you to start of square. Now just need to calculate array poss from here (should be straightforward, probably)
            if(y2<y)
                start+=y2;
            else if(y2==y)
                start+=y+x2-(x2>x);
            else
                start+=6+y2;
             //wow so much easier than the knight one and the bishop one. Honeslty, a breeze. Thank god.
            break;
        case 5:{ //queen None
            //OKAY QUEEN DONE I THINK
            //That was a lot harder than I thought it would be. Was mostly just making the bishop move logic within the squares. Had to modify is for diff coords but I got it and it prob took longer than it should have
            start=2232;
            //can just combine rook and bishop logic. Shouldn't be too bad maybe
            start+=(y*8+x)*21; //gets you to the beginning of actual square but ignores extra moves for other rings.
            switch(y){ //reused bishop code
                case 1:
                    start+=12;
                    break;
                case 2:
                    start+=32;
                    break;
                case 3:
                    start+=56;
                    break;
                case 4:
                    start+=80;
                    break;
                case 5:
                    start+=100;
                    break;
                case 6:
                case 7:
                    start+=112;
            }
            switch(y){
                case 3:
                case 4:
                    if(x<5)
                        start-=2*(5-(x>2?x:3));
                case 2:
                case 5:
                    if(x<6)
                        start-=2*(6-(x>1?x:2));
                case 1:
                case 6:
                    if(x<7)
                        start-=2*(7-(x>0?x:1));
            }
            //at the right square. Now have to find move.
            int lu=(x<y?x:y);
            int ld=(x<7-y?x:7-y);
            int ru=(7-x<y?7-x:y);
            int rd=(7-x<7-y?7-x:7-y);
            int dist=(x2<x?x-x2:x2-x);
            if(x2==x){
                if(y2>y){
                    start+=lu+ru+(y2<y+ld?y2-y:ld)+(y2<=y+rd?y2-y-1:rd); //this is probably correct
                }
                else
                    start+=(y-lu<=y2?y2-y+lu+1:0)+(y-ru<y2?y2-y+ru:0); //this might be working now
            }
            else if(y2==y){ //we get full scope for lu and ru
                //basically this part just lu+ru adding all of the up stuff cause no intersect now.
                start+=lu+ru;
            }
            else{
                start+=x2>x; //rook counts one fewer move if not its turn and bishop move is to the right of it
                switch(2*(y2>y)+(x2>x)){
                    case 0: //left up
                        start+=lu-dist+(ru>dist?ru-dist:0);
                        break;
                    case 1: //right up
                        start+=ru-dist+(lu>=dist?lu-dist+1:0);
                        break;
                    case 2: //left down
                        start+=lu+ru-1+dist+(dist>rd?rd:dist-1);
                        break;
                    case 3: //right down
                        start+=lu+ru-1+dist+(dist>=ld?ld:dist);
                        break;
                }
            }
            if(y2<y)
                start+=y2;
            else if(y2==y)
                start+=y+x2-(x2>x);
            else
                start+=6+y2;
            break;
        }
        case 6:{ //king 6,7 I can honestly put castling at the end so that it is super easy to search and so that the hashing is easier
            start=3688;
            //castling I put at the end because it is an absolute pain to deal with otherwise.
            if(move[2][0]==6){
                start=4108+2*(y==7);
                break;
            }
            else if(move[2][0]==7){
                start=4109+2*(y==7);
                break;
            }
            //easy except I need to put castling as special moves at the end, I think. Only exceptional squares are corner and edges.
            //3 on corners. 5 on edges. 8 in the middle
            if(y==0 || y==7){ //put the extra conditionals just into the operation
                start+=(y==7)*384 + (x!=0)*(5*x-2); //3+5*(x-1)
            }
            else{
                start+=y*58-22 + (x!=0)*(8*x-3); //36+58*(y-1), goes to start of row //5+8*(x-1) start of square
            }
            //now at start of square. Find move within
            int ct;
            ct=(y2-y)*3+x2-x+4; //ternary diffs. just row and column of 3x3 box with hole in the middle
            ct-=ct>3;
            int u,l,r,d; //technically not all used so minorly inefficient but this seems easier to just type them out here and worry about implementing them within for less clutter (lol have you seen your knight section, man?) (okok fair but... ugh do you really want me to do it? ill type out the cleaner way and then decide. Should be very quick regardless.)
            d=(ct>2?3:ct); //basically masks for the directions. Couldn't actually do it with bits because it's ternary (so, how fitting that im using the "ternary" operator)
            l=(ct>5?3:ct/3+1);
            r=((ct+1)/3);
            u=(ct>4?ct-4:0);
            switch(3*(2*(x==0)+(x==7))+(2*(y==0)+(y==7))){ //this is probably what I like most about switch statements. You can create really nice binary nums of condtionals like this and simplify if else statements immensely. Also jump statements are just goated. (actually this is kinda a ternary state lol. surprised i figured this one out)
                    
                //case 0: middle but no masking so I actually don't need to handle this case at all
                case 1: //top egde of the board only U
                    ct-=u;
                    break;
                case 2: //bottom edge of the board only D
                    ct-=d;
                    break;
                case 3: //right edge of the board only R
                    ct-=r;
                    break;
                case 4: //top right corner UR
                    ct-=u+r-(u>0);
                    break;
                case 5: //bottom right corner DR
                    ct-=d+r-(r>0);
                    break;
                case 6: //left edge only L
                    ct-=l;
                    break;
                case 7: //top left corner UL
                    ct-=u+l-(u>0);
                    break;
                case 8: //bottom left corner DL
                    ct-=d+l-(d>0);
                    break;
            }
            start+=ct;
            break;
        }
    }
    return start;
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
