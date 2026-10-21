#include "../include/Bot.h"
#include "../include/Attacks.h"
#include "../include/Player.h"
#include "../include/ShipPlacement.h"
#include "../include/CalcProbs.h"

#include <time.h>



/**
 * The smart bot uses a combination of different tactics to minimize the number of operations it has to do accross turns and maximize its 
 * hit accuracy.
 * 
 * On one end, the robot keeps track of a probability distribution of the cells on the opponent's grid. Using mathematical formulas implemented in functions
 * in the CalcProbs.c file, it can dynamically update the opponent's probability grid to adapt to the changes (Hits or Misses) that have occured.
 * Essentially, the bot will target higher probability cells (cells where there is a higher number of possible ship placements). This creates a
 * more statistically driven approach with higher chances of success than a completely randomized targeting algorithm.
 * 
 * However, too much statistics in this sense would deem the bot extremely predictable. Always targetting the highest probability cell would yield
 * very similar paths over and over every game. If the opponent could analyze the bot's strategy, it would easily be able to predict where not to place
 * its ships. Then it would delay being hit for a while as the bot follows a predetermined path from high probability to low.
 * 
 * To solve that problem, we decided to randomize, but along certain lines. We still want the bot to favor higher probability, but not too often.
 * Our first thought was to keep track of the next highest probability node somehow. Priority queue? Well, that would give us O(1) retrieval time
 * for the highest probability cell, but it won't hold as O(1). After attacking, the probability grid will change, which means the priority queue
 * must be re-sorted. This would take O(n) or O(m^2) where m is the width and height of the grid.
 * O(m^2) seemed a bit much to handle on every turn, especially if we wanted to expand the game borders for a wider grid.
 * 
 * Fortunately, it came to mind again that we don't really need to target the highest probability every time, so it's fine to find a way to 
 * ...
 * 
 * We came up with the idea of subdividing the probability grid into different probability regions. These regions are categorized as
 * follows: HIGHPROBABILITY, AVERAGEPROBABILITY, and LOWPROBABILITY, depending on the highest probability cell in that region. (could've depended
 * on the average probability of all cells, but it wasn't necessary for our purposes)
 * 
 * The bot subdivides the grid into these regions and keeps track of them in binomial heaps. Binomial heaps weren't necessary, but they interested
 * us from 214, so we implemented them for fun.
 * 
 * The idea of heapifying the regions is important for maintaining effiecient access to the highest probability cells. The randomization part comes
 * from the non ordered region categories. The bot could randomly pick from the list of high probability regions, the average, or the low.
 * 
 * Region size can be changed by changing its respective macro. Working with region size becomes like working with the resolution of an image. The
 * smaller the pixels, the more there are, then the higher the resolution. The smallest one could go is regions of size 1x1.
 * 
 * 
 * Using this method we can lock our cell retrieval time to O(1), while keeping our algorithm random and non predictable. Actually, using binomial heaps
 * it's O(logn) to get the maximum, but n here is a constant. (size of regions is a constant) 
 * Not only that, but also updating the probability grid would only require updating a certain number of regions. Assuming that number is K, then we would have O(k) per region.
 * (but number of regions we are updating is a constant)
 * So if both the size of the region and the number of regions updated are both constants, that gives us O(1) for all these operations.
 * 
 * The time complexity depends as well on how small regions are compared to ships. After targeting, the bot should update the region surrounding the hit
 * only so far as to where the largest ship could be and still reach the target point. So, typically there will be multiple regions intersecting with
 * the area surrounding the target, all of their heaps must be updated.
 * 
 * 
 * BOT'S BRAIN
 * 
 * 
 * In order to carry chained decisions and keep track of the next moves, we gave the bot a stack brain that keeps track of tasks to perform every other
 * turn. When the bot hits a target for example, it must hit around the target until it sinks the ship. So every hit we assign a new task with a function
 * pointer and the necessary arguments. The next turn, the task is popped from the stack and the function is called, and the arguments are passed.
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */












void BotSmartAttack(Player * bot, Player * opponent){

    start:

    //First we must check if the stack is empty:
    if (is_empty(bot->stackMemory)){

        //printf("stack empty, continuing pattern: \n");

        //Normal Randomized Probabilistic Pattern:

        //First: Get the next target
        
        int row = 0;
        int col = 0;


        if (bot->riskFactor < HIGH_RISK){

            int count = 0;
            while (opponent->probabilityHeapCategoryLists[bot->currTargetCategory].size == 0)
            {
                if (count >= PROB_CATEGORYCOUNT){
                    //Something is definetely going wrong, all lists appear empty!
                    perror("Something is wrong! All probability categories are empty.");
                    exit(EXIT_FAILURE);
                }

                bot->currTargetCategory += 1;
                bot->currTargetCategory %= PROB_CATEGORYCOUNT;
                count++;
            }
            
            
            pickCoords:
            //Get a randome cell from the opponent's category list of index bot->currTargetCategory:
            int getCoord = GetRandomProbCoordinateFromCategory(&row, &col, &opponent->probabilityHeapCategoryLists[bot->currTargetCategory]);
            //printf("got random coord %d,%d\n", row, col);
            //printf("getCoord: %d\n", getCoord);

            if (opponent->grid[row][col] == HIT || (opponent->grid[row][col] == MISS)){
                goto pickCoords;
            }

            bot->currTargetCategory += 1;
            bot->currTargetCategory %= PROB_CATEGORYCOUNT; //Keeping track of the next category to target.

        }
        else {
            if (bot->riskFactor == EXTREMELYHIGH_RISK){
                GetHighestProbability(&row, &col, opponent);
            }
            else {
                GetRandomHighestProbCell(&row, &col, opponent);
            }
        }


        //Now I must attack the target:
        #pragma region [Firing]
        //For now, we'll stick to the Fire() function:

        //Transform the indexes to coordinates: because attack methods take in user coords

        char * error = NULL;


        int fire = BotFireHelper(row, col, bot, opponent);


        if (error != NULL) free(error);


        #pragma endregion

    }
    else {

        //In this case the stack is non empty, so we must pop out from it and perform the required task.


        //When it's a success (a HIT), then we must add new stuff onto the stack. Whether the stuff added are at the top or at the bottom depends on
        //their importance.

        //There's a costy path of operations when sinking a ship, we'll have to check the nodes surrounding the ship

        doTask:

        if (is_empty(bot->stackMemory)){
            goto start;
        }  


        BotTask * topTask = (BotTask*)removeFirst(bot->stackMemory);
        //printf("get funcptr: %p\n", topTask->function);
        int res = PerformTask(topTask);

        //if res = 0 then the task was either invalid or 

        if (res <= 0){
            freeTask(topTask);
            goto doTask;
        }

    }
}


#pragma region [Accessing Probability Cells]
/**
 * Input:
 *      - heapCategory: The binomial heap array that the function will randomly choose from.
 *      - categorySize
 * 
 * Output:
 *      - row: the address where the function will store the row index
 *      - col: the address where the function will store the column index
 */
int GetRandomProbCoordinateFromCategory(int * row, int * col, D_LinkedList * heapCategory){

    if (heapCategory->size <= 0){
        Println_Centered("Invalid array size for heapCategory!", strlen("Invalid array size for heapCategory!"), RED);
        return -1;
    }

    if (row == NULL || col == NULL){
        Println_Centered("Cannot return coordinate from category when row or column addresses are NULL.", strlen("Cannot return coordinate from category when row or column addresses are NULL."), RED);
        return -1;
    }
    if (heapCategory == NULL){
        Println_Centered("Cannot pick probability from binomial heap. Heap is NULL.", strlen("Cannot pick probability from binomial heap. Heap is NULL."), RED);
        return -1;
    }

    srand(time(0));
    int randIndex = rand() % heapCategory->size;

    D_ListNode * curr = get_first(heapCategory);

    for (int i = 0; i < randIndex; i++)
    {
        curr = curr->next;
    }
    

    int * elem = BinHeap_FindHighestProbabilityCell((BinomialHeap*)(curr->data));

    //printf("max prob in heap: %d\n", ((int*)elem)[0]);
    //printf("hmm: %d\n", *(int*)(heapCategory[randIndex]->head->rightSibling->value));

    *row = elem[1];
    *col = elem[2];

    return 1;

}

/**
 * This function will return a random grid cell coordinate from the highest probability category that is nonempty.
 * 
 * Output: 
 *      - integer array of i and j coordinates
 */
int GetRandomHighestProbCell(int * row, int* col, Player * player){

    //First we check if the high-probability category contains anything:
    if (player->probabilityHeapCategoryLists[2].size > 0){
        GetRandomProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[2]);
    }
    else if (player->probabilityHeapCategoryLists[1].size > 0){
        GetRandomProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[1]);
    }
    else if (player->probabilityHeapCategoryLists[0].size > 0){
        GetRandomProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[0]);
    }
    else {
        printf("Something must have gone wrong. All grid cells have 0 probability.\n");
        return -1;
    }

    return 1;
}


/**
 * This function searches in the specified probability category array and stores the row and column coordinates of the highest probability cell
 * in row and col respectively.
 */
int GetHighestProbCoordinateFromCategory(int * row, int * col, D_LinkedList * heapCategory){

    if (heapCategory->size <= 0){
        Println_Centered("Invalid array size for heapCategory!", strlen("Invalid array size for heapCategory!"), RED);
        return -1;
    }

    if (row == NULL || col == NULL){
        Println_Centered("Cannot return coordinate from category when row or column addresses are NULL.", strlen("Cannot return coordinate from category when row or column addresses are NULL."), RED);
        return -1;
    }
    if (heapCategory == NULL){
        Println_Centered("Cannot pick probability from binomial heap. Heap is NULL.", strlen("Cannot pick probability from binomial heap. Heap is NULL."), RED);
        return -1;
    }

    int highestIndex = 0;

    //Recall, we're working with linked lists of binomial heaps, so the D_ListNode stores a binomial heap struct pointer as data.

    D_ListNode * curr = get_first(heapCategory);
    D_ListNode * maxProbNode = curr;

    while (curr != NULL)
    {
        int currProb = BinHeap_FindHighestProbabilityCell((BinomialHeap*)(curr->data))[0];
        int maxProb = BinHeap_FindHighestProbabilityCell((BinomialHeap*)(maxProbNode->data))[0];
        
        if (currProb > maxProb){
            maxProbNode = curr;
        }
    }

    if (maxProbNode == NULL){
        printf("No probability node was found in this category list.");
        return -1;
    }

    int * res = BinHeap_FindHighestProbabilityCell((BinomialHeap*)(maxProbNode->data));

    *row = res[1];
    *col = res[2];

    return 1;
    

}

/**
 * This function searches across all probability categories for the highest probability cell and saves its coordinate values in row and col.
 */
int GetHighestProbability(int * row, int* col, Player * player){

    //First we check if the high-probability category contains anything:
    if (player->probabilityHeapCategoryLists[2].size > 0){
        GetHighestProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[2]);
    }
    else if (player->probabilityHeapCategoryLists[1].size > 0){
        GetHighestProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[1]);
    }
    else if (player->probabilityHeapCategoryLists[0].size > 0){
        GetHighestProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[2]);
    }
    else {
        printf("Something must have gone wrong. All grid cells have 0 probability.\n");
        return -1;
    }

    return 1;
}

#pragma endregion

/**
 * This function updates a specified binomial heap by updating each element's probability and re-inserting it into the heap.
 * 
 * Input:
 *      - player: the player of which a heap will be updated
 *      - heapIndex: the index of the heap that will be updated. This index should be a valid index within the size of the heap hashmap.
 */
int UpdateHeap(Player * player, int heapIndex){

    //printf("\nINSIDE UPDATEHEAP FUNC:\n");

    if (heapIndex >= PROB_REGION_COUNT || heapIndex < 0) return -1;

    //printf("is it null: %d\n", player->probabilityHeapSet[heapIndex] == NULL);

    //First I need to intialize a temporary binomial heap:
    BinomialHeap temp;
    temp.head = NULL;
    temp.compare = player->probabilityHeapSet[heapIndex]->compare;
    
    //printf("heree!\n");

    BinomialHeap * targetHeap = player->probabilityHeapSet[heapIndex];


    while (targetHeap->head != NULL)
    {
        int * element = (int*)deleteMin(targetHeap);

        int row = element[1];
        int col = element[2];

        //Updating the probability to the new one after probGrid got updated:
        element[0] = player->probabilityGrid[row][col];

        //Now I need to make sure that the probability is not 0 and the coordinate is neither a hit nor a miss:
        if (element[0] > 0 && player->grid[row][col] != MISS && player->grid[row][col] != HIT){
            insert(&temp, element);
        }
    }
    
    //Connect the head of the temporary heap to the initial, now empty, heap:
    targetHeap->head = temp.head;

    //NOT ENOUGH TIME TO THINK OF AND IMPLEMENT A BETTER WAY THAN THE ONE BELOW.

    //Finally, I must move this heap to the right category:
    //I'll traverse all the categories till I find this heap:
    D_ListNode * curr = NULL; 
    int CategoryNumber = 0;
    while (CategoryNumber < PROB_CATEGORYCOUNT)
    {
        curr = get_first(&player->probabilityHeapCategoryLists[CategoryNumber]);
        while (curr != NULL)
        {
            BinomialHeap * currHeap = (BinomialHeap*)(curr->data);

            int row = ((int*)(currHeap->head->value))[1];
            int col = ((int*)(currHeap->head->value))[2];

            if (HashRegion(row, col) == heapIndex){//We don't compare the nodes because they are not the same
                int currHeapProb = BinHeap_FindHighestProbabilityCell(targetHeap)[0];
                if ((currHeapProb >= HIGHPROB_BASE && CategoryNumber == 2) || (currHeapProb >= AVGPROB_BASE && CategoryNumber == 1)
                 || (currHeapProb >= LOWPROB_BASE && CategoryNumber == 0)){
                    return 1; //This means that the heap is at the right location.
                 }

                goto endLoop;
            }

            curr = curr->next;
        }
        CategoryNumber++;
    }
    endLoop:

    if (curr == NULL || CategoryNumber >= PROB_CATEGORYCOUNT){
        printf("Something is wrong, cannot find heap in any category!");
        return -1;
    }

    disconnectNode(&player->probabilityHeapCategoryLists[CategoryNumber], curr);

    int highestProb = BinHeap_FindHighestProbabilityCell(targetHeap)[0];

        //InOrderTraversal_ProbNode_Print(&binHeap);
    //printf("highestProb: %d\n", highestProb);

    if (highestProb >= HIGHPROB_BASE){

        //Place in list of index 2 for high probabilities:
        addLast(&player->probabilityHeapCategoryLists[2], targetHeap);
    }
    else if (highestProb >= AVGPROB_BASE){
        //Place in array of index 1 for avg probabilities:
        addLast(&player->probabilityHeapCategoryLists[1], targetHeap);
    }
    else {
        //Place in array of index 0 for low probabilities:
        addLast(&player->probabilityHeapCategoryLists[0], targetHeap);
    }

    return 1;
}


/**
 * This function will scan an area on the graph and update, once only, the binomial heap tied to each probability region.
 */
int UpdateHeapsWithinBounds(Player * player, int row0, int row1, int col0, int col1){

    //I could keep track of every region I've already updated so that I don't update it again.
    //But I don't see no easy way of doing that without using a data structure
    //Or, better way: I could save the hash indices of the regions updated and check if the array contains the one I'm trying to update

    //Even better way: I could set up a coordinate point that jumps between regions. I can access the region size from the PROB_REGION_SIZE macro.
    //First I initialize the coordinate point to be the top right of the target area. Then I add to the index (region width + 1) 
    //in the horizontal direction and update until it goes out of bounds of the target area.
    //I do the same vertically.

    int curr[2] = {row0, col0};

    int regionSize[2] = PROB_REGION_SIZE;

    
    while (IndexWithinRange(curr[0])){

        while (IndexWithinRange(curr[0]) && IndexWithinRange(curr[1])){
            
            UpdateHeap(player, HashRegion(curr[0], curr[1]));

            curr[1] += regionSize[1] + 1;
        }
        curr[1] = col0;
        curr[0] += regionSize[0] + 1;

    }
        

}






/**
 * This function assigns a new task. It creates a new task structure, fills it with a pointer to the function it must perform and with the arguments
 * necessary. It then pushes the new task onto the bot's stack memory.
 */
BotTask * CreateTask(int (*function)(void**), void** arguments, int argumentCount, void** flags, int flagCount){

    BotTask * task = (BotTask*)(malloc(sizeof(BotTask)));

    //printf("creat funcptr: %p\n", function);

    task->function = function;
    task->arguments = arguments;
    task->argumentCount = argumentCount;
    task->flags = flags;
    task->flagCount = flagCount;


}

int freeTask(BotTask * task){

    if (task->arguments != NULL){
        for (int i = 0; i < task->argumentCount; i++)
        {
            free(task->arguments[i]);
        }
    }

    if (task->flags != NULL){
        for (int i = 0; i < task->flagCount; i++)
        {
            free(task->flags[i]);
        }
        
    }

    free(task);    

}

int AssignNewTask(int priorityFlag,  Player * bot, int (*function)(void**), void** arguments, int argumentCount, void** flags, int flagCount){

    BotTask * task = CreateTask(function, arguments, argumentCount, flags, flagCount);

    //printf("func ptr: %p\n", function);

    switch (priorityFlag)
    {
    case TASKFLAG_HIGHPRIORITY:
        //We add the task at the top of the stack for high importance
        addFirst(bot->stackMemory, (void*)task);
        break;

    case TASKFLAG_LOWPRIORITY:
        //We add the task at the bottom of the stack for low importance
        addLast(bot->stackMemory, (void*)task);
        break;
    
    default:
        return 0;
    }


    return 1;


}


BotTask * GetNextTask(Player * bot){

    if (is_empty(bot->stackMemory)) return NULL;

    return (BotTask*)removeFirst(bot->stackMemory);

}

/**
 * This function takes in a BotTask, performs it and returns an integer showing whether the task was successfully performed or not.
 * 
 * Output:
 *      - The function returns 1 if task was successful
 *      - It returns 0 if task was not successful
 * 
 */
int PerformTask(BotTask * task){

    /**
     * We must call the function specified in the task and pass it the arguments 
     */

    if (task->function == NULL) return 0;

    printf("func ptr in perfTask: %p\n", task->function);
    return task->function(task->arguments);

}

#pragma region [Bot Firing Systems]
/**
 * Input:
 *      - int row: the row number of the targeted cell
 *      - int col: the column number of the targeted cell
 *      - Player * bot
 *      - Player * opponent
 */
int BotFire(void** args){
    
    printf("YOUY!\n");
    if (args == NULL) return 0;

    int row = *(int*)args[0];
    int col = *(int*)args[1];

    Player * bot = (Player*)args[2];

    Player * opponent = (Player*)args[3];

    BotFireHelper(row, col, bot, opponent);

}

int BotFireHelper(int row, int col, Player * bot, Player * opponent){

    char * coords = alloc_GetCoordsFromIndices(row, col, GRIDSIZE, startingCoordinate_1, startingCoordinate_2,
     endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);

    startfire:

    //Make sure the bot doesn't shoot a previously shot cell

    printf("Fire Coords: %s\n", coords);

    printf("8,8 is: %s\n", alloc_GetCoordsFromIndices(8, 8, GRIDSIZE, startingCoordinate_1, startingCoordinate_2,
     endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift));

    char * error = NULL;
    int fireRes = Fire(coords, opponent->grid, DifficultyValue, &error);


    if (fireRes < 0){
        if (error != NULL) free(error);

        goto startfire;
    }


    int target[2] = {row,col};


    //Updating the probability distribution:
    UpdateSurroundingProbabilities(opponent, target);


    printf("BEF UPDHEAPWITHINBND\n");
    
    DisplayIntGrid(opponent->probabilityGrid, GRIDSIZE);

    //Update the probability heaps for all the probability regions that surround the target.
    UpdateHeapsWithinBounds(opponent, MAX(0, row - LONGEST_SHIPLENGTH), MIN(GRIDSIZE, row + LONGEST_SHIPLENGTH),
     MAX(0, col - LONGEST_SHIPLENGTH), MIN(GRIDSIZE, col + LONGEST_SHIPLENGTH));

    
    DisplayIntGrid(opponent->probabilityGrid, GRIDSIZE);

    //Need to check if the target was a HIT or a MISS. If it's a HIT then we assign 4 new tasks to target the surrounding cells:
    if(opponent->grid[row][col] == HIT){
        if (IndexWithinRange(row + 1)){
            if (opponent->grid[row+1][col] != HIT && opponent->grid[row+1][col] != MISS){

                int argCount = 4;
                void ** args = (void**)(malloc(sizeof(void*) * argCount));

                for (int i = 0; i < argCount; i++)
                {
                    args[i] = (void*)(malloc(sizeof(void*)));
                }
                
                int* rowPtr = (int*)(malloc(sizeof(int)));
                *rowPtr = row + 1;
                int* colPtr = (int*)(malloc(sizeof(int)));
                *colPtr = col;

                //Setting the arguments:
                args[0] = rowPtr;
                args[1] = colPtr;
                args[2] = bot;
                args[3] = opponent;
                

                AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, BotFire, args, argCount, NULL, 0);
            }
        }

        if (IndexWithinRange(row - 1)){
            if (opponent->grid[row-1][col] != HIT && opponent->grid[row-1][col] != MISS){
                int argCount = 4;
                void ** args = (void**)(malloc(sizeof(void*) * argCount));

                for (int i = 0; i < argCount; i++)
                {
                    args[i] = (void*)(malloc(sizeof(void*)));
                }
                
                int* rowPtr = (int*)(malloc(sizeof(int)));
                *rowPtr = row - 1;
                int* colPtr = (int*)(malloc(sizeof(int)));
                *colPtr = col;

                //Setting the arguments:
                args[0] = rowPtr;
                args[1] = colPtr;
                args[2] = bot;
                args[3] = opponent;
                

                AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, BotFire, args, argCount, NULL, 0);
            }
        }

        if (IndexWithinRange(col + 1)){
            if (opponent->grid[row][col+1] != HIT && opponent->grid[row][col+1] != MISS){
                int argCount = 4;
                void ** args = (void**)(malloc(sizeof(void*) * argCount));

                for (int i = 0; i < argCount; i++)
                {
                    args[i] = (void*)(malloc(sizeof(void*)));
                }
                
                int* rowPtr = (int*)(malloc(sizeof(int)));
                *rowPtr = row;
                int* colPtr = (int*)(malloc(sizeof(int)));
                *colPtr = col + 1;

                //Setting the arguments:
                args[0] = rowPtr;
                args[1] = colPtr;
                args[2] = bot;
                args[3] = opponent;
                

                AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, BotFire, args, argCount, NULL, 0);
            }
        }

        if (IndexWithinRange(col - 1)){
            if (opponent->grid[row][col-1] != HIT && opponent->grid[row][col-1] != MISS){
                int argCount = 4;
                void ** args = (void**)(malloc(sizeof(void*) * argCount));

                for (int i = 0; i < argCount; i++)
                {
                    args[i] = (void*)(malloc(sizeof(void*)));
                }
                
                int* rowPtr = (int*)(malloc(sizeof(int)));
                *rowPtr = row;
                int* colPtr = (int*)(malloc(sizeof(int)));
                *colPtr = col - 1;

                //Setting the arguments:
                args[0] = rowPtr;
                args[1] = colPtr;
                args[2] = bot;
                args[3] = opponent;
                

                AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, BotFire, args, argCount, NULL, 0);
            }
        }
    }

    return fireRes;

}

#pragma endregion



//Later on, ship placement will depend on recorded data on previous players.
//The bot will write to a folder records for each player under the player's unique name.
//Every new game, the bot accesses the player's record and analyzes the player's tactics from previous games, his attacking patterns
//and tries to place ships in accordingly.
#pragma region [BOT PLACEMENT]

int BotPickRandomIndicesWithinBounds(int * outRow, int * outCol, char** grid, int shipSize[2], char* orientation, int startRow, int endRow, int startCol, int endCol){
    

    start:
    int rowMax = endRow, colMax = endCol;

    

    if (strcmpi(orientation, "h") == 0){
        colMax -= shipSize[0];
    }
    else {
        rowMax -= shipSize[0];
    }

    printf("startRow = %d, rowMax = %d\n", startRow, rowMax);

    int row = startRow + (rand() % rowMax);
    int col = startCol + (rand() % colMax);

    printf("%d,%d\n", row, col);

    char * coords = alloc_GetCoordsFromIndices(row, col, GRIDSIZE, startingCoordinate_1, startingCoordinate_2, endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);

    printf("%s\n",coords);
    printf("%s\n",orientation);
    
    char * error = NULL;

    int * shipBounds = alloc_GridAreaFromInput(coords, orientation, shipSize[0] - 1, shipSize[1], &error);

    if (error != NULL) printf("%s\n", error);
    
    printf("huhh\n");
    printf("%d, %d,%d,%d\n", shipBounds[0], shipBounds[1], shipBounds[2], shipBounds[3]);

    if (CheckForOverlap(grid, shipBounds)){
        printf("there was an overlap\n");
        free(coords);
        free(shipBounds);
        if (error != NULL) free(error);
        goto start;
    }

    *outRow = row;
    *outCol = col;

    free(coords);
    free(shipBounds);
    if (error != NULL) free(error);

    return 1;
}



void PlaceBotShips(Player *bot) {

    srand(time(0));

    char shipTypes[] = {SUBMARINE_C, DESTROYER_C, BATTLESHIP_C, CARRIER_C};
    int ShipSizes[SHIPCOUNT][2] = {{2,0}, {3,0}, {4,0}, {5,0}};

    for (int i = 0; i < SHIPCOUNT; i++) {
        char orientation[2];
        int horizontal = rand() % 2;

        // Set orientation to 'H' for horizontal or 'V' for vertical
        orientation[0] = horizontal ? 'h' : 'v';
        orientation[1] = '\0';

        int row;
        int col;

        BotPickRandomIndicesWithinBounds(&row, &col, bot->grid, ShipSizes[i], orientation, 0, GRIDSIZE - 1, 0, GRIDSIZE - 1);

        printf("%d,%d\n", row, col);
        // Convert to user coordinates
        char * coords = alloc_GetCoordsFromIndices(row, col, GRIDSIZE, startingCoordinate_1, startingCoordinate_2, endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);
        
        //printf("%d, %d\n", ShipSizes[i][0], ShipSizes[i][1]);
        //printf("%s\n", coords);
        // Try to place the ship
        char * error = NULL;
        PlaceShipOnGridHelper(bot, shipTypes[i], bot->grid, coords, orientation, ShipSizes[i], &error);

        printf("%s\n", error);

        printf("%c\n", coords[0]);
        
        free (coords);
        if(error != NULL) free(error);
        
        //char c;

        //scanf("%c", &c);
    }
}

#pragma endregion