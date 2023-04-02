# Chess-Ai-from-scratch
Chess AI that I wrote in c++ (for speed). All AI functionality is written by me and not from a library
The reason for this project is just for me to learn about AI, not to do anything groundbreaking.

My basic idea is pretty much monte carlo tree search using two different AI models. 
Both recieve the 64 squares of the board as input plus a 65th input for the turn.
I label pieces with certain values when I feed them into the first 64 input nodes (you can find these at the top of the chessFuncs.cpp file)
The first model has one output, from 0 to 1 which I then transform with logits to get negative infinity to positive infinity. 
This is a valuation of whether the current board state favors black or white. I will take more later about what this is used for.
The second ai has an output vector of length 4112, every possible move in chess (irrespective of the board state).
For instance, a move like pawn from 1,1 to 3,1 (using 0 indexing and computer coordinates, because it is much easier to represent than PGN notation, aka Ke6 type moves)
The ai assigns a value from 0 to 1 to each of these moves (note that this does not use softmax because you could have two moves which are essentially winning,
so there is never just one right or classified move.
I wrote a special, incredibly optimized function to find the index of a move within this array. From here, I take only the legal moves and find the best
k among them (usually 2 or 3). Essentially, instead of simulating and searching every branch of every possible game state, you limit yourself to a heuristic (the AI)
which selects what it thinks are the best few game states for you to look at. So, instead of branching 20 or so times per move, leading to
a time complexity of 20^d (where d is the depth or how many moves ahead it looks), it only has k^d time complexity (and then times the
complexity of running the ai which is n^2*k with a very low constant factor). Parrallel computing could speed this up so that you can search many states at once, but I only have just one little computer.
Now, the simulations run in a depth first search manner, prioritizing the highest rated move. For every depth, another heuristic "cut-off" value is created
based on the valuation of the first ai. Essentially, if a branch becomes rated too low, it cuts it off, not wasting extra computation
on a state without any potential. Using the concept of minimax (essentially, assuming that my opponent always plays the best move, what is the best
that I can do?), I update the valuations for each of the branches based on the best response and the resulting board state to determine which move
should actually be played.

***
I have yet to determine:

Will I store branches which I have looked at on previous moves? This is a ton of memory, and while it saves computation, I also have to destroy all of
the removed branches (a majority, precisely (k-1)/k proportion of the branches) whenever a new move is started.

Will I have a fixed k and depth or will I give the program a time limit and then it branches out either increasing k or d (keep in mind that the cost of
increasing d by 1 is much less than increasing k by 1, but I could do a smaller depth for new k) or something else altogether like a cutoff for desired
board state (that is a shaky idea because you can never guarantee anything. You in fact are just trying to figure out that info)

What optimizer I will use for the ais. I have gotten smaller ones to learn pretty well, but have struggled with larger ones
Also what cost and activation functions I will use. I presume I will use sigmoid for the ai which decides the ratings of the moves in conjunction with binary sparse crossentropy as the loss function. For the valuation ai, I might use relu and then just sigmoid on the last layer. Something simple like mean squared error might be sufficient as the loss function. Also, I will transform the ouput from a probability to a real number value using logits for easier comparison and human view, but I am not sure if that is totally necessary.
***
