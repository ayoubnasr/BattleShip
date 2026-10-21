#include "../include/Bot.h"
#include "../include/Attacks.h"
#include "../include/Player.h"
#include "../include/ShipPlacement.h"
#include "../include/CalcProbs.h"

#include <time.h>

static int CountSunkFlags(Player * player);
static int HasOpenHits(Player * player);



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


        int getCoord = -1;

        if (bot->riskFactor < HIGH_RISK){

            int count = 0;
            while (opponent->probabilityHeapCategoryLists[bot->currTargetCategory].size == 0 && count < PROB_CATEGORYCOUNT)
            {
                bot->currTargetCategory += 1;
                bot->currTargetCategory %= PROB_CATEGORYCOUNT;
                count++;
            }

            if (count < PROB_CATEGORYCOUNT){
                //Get a randome cell from the opponent's category list of index bot->currTargetCategory:
                //(the heaps only hold cells that were not shot yet, so the cell is always a new one)
                getCoord = GetRandomProbCoordinateFromCategory(&row, &col, &opponent->probabilityHeapCategoryLists[bot->currTargetCategory]);
            }

            bot->currTargetCategory += 1;
            bot->currTargetCategory %= PROB_CATEGORYCOUNT; //Keeping track of the next category to target.

        }
        else {
            if (bot->riskFactor == EXTREMELYHIGH_RISK){
                getCoord = GetHighestProbability(&row, &col, opponent);
            }
            else {
                getCoord = GetRandomHighestProbCell(&row, &col, opponent);
            }
        }

        if (getCoord < 0 || opponent->grid[row][col] == HIT || opponent->grid[row][col] == MISS){
            //All categories are empty: the formulas give no cell a chance anymore (it can happen late in the game when misses pile up
            //around the last ships). Fall back to any cell that was not shot yet.
            PickRandomCell(&row, &col, opponent, false);
        }


        //Now I must attack the target:
        #pragma region [Firing]
        //For now, we'll stick to the Fire() function:

        BotFireHelper(row, col, bot, opponent);

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

        freeTask(topTask); //The task is done (or impossible), either way it's off the stack now

        //if res <= 0 then the task was either invalid or could not be performed (eg. the cell was already shot), so move on to the next one

        if (res <= 0){
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

    if (heapCategory == NULL){
        Println_Centered("Cannot pick probability from binomial heap. Heap is NULL.", strlen("Cannot pick probability from binomial heap. Heap is NULL."), RED);
        return -1;
    }

    if (heapCategory->size <= 0){
        Println_Centered("Invalid array size for heapCategory!", strlen("Invalid array size for heapCategory!"), RED);
        return -1;
    }

    if (row == NULL || col == NULL){
        Println_Centered("Cannot return coordinate from category when row or column addresses are NULL.", strlen("Cannot return coordinate from category when row or column addresses are NULL."), RED);
        return -1;
    }

    //(no srand() here: reseeding with the time on every call returned the same "random" index for a whole second)
    int randIndex = rand() % heapCategory->size;

    D_ListNode * curr = get_first(heapCategory);

    for (int i = 0; i < randIndex; i++)
    {
        curr = curr->next;
    }
    

    int * elem = BinHeap_FindHighestProbabilityCell((BinomialHeap*)(curr->data));

    if (elem == NULL) return -1; //Empty heaps are kept out of the category lists, so this should not happen

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
    if (player->probabilityHeapCategoryLists[HIGHPROB_CATEGORY].size > 0){
        return GetRandomProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[HIGHPROB_CATEGORY]);
    }
    else if (player->probabilityHeapCategoryLists[AVGPROB_CATEGORY].size > 0){
        return GetRandomProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[AVGPROB_CATEGORY]);
    }
    else if (player->probabilityHeapCategoryLists[LOWPROB_CATEGORY].size > 0){
        return GetRandomProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[LOWPROB_CATEGORY]);
    }

    //All grid cells have 0 probability: the caller falls back to another way of picking a cell.
    return -1;
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

    int maxProb = -1;

    while (curr != NULL)
    {
        int currProb = BinHeap_FindHighestProbabilityCell((BinomialHeap*)(curr->data))[0];

        if (currProb > maxProb){
            maxProbNode = curr;
            maxProb = currProb;
        }

        curr = curr->next;
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
    if (player->probabilityHeapCategoryLists[HIGHPROB_CATEGORY].size > 0){
        return GetHighestProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[HIGHPROB_CATEGORY]);
    }
    else if (player->probabilityHeapCategoryLists[AVGPROB_CATEGORY].size > 0){
        return GetHighestProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[AVGPROB_CATEGORY]);
    }
    else if (player->probabilityHeapCategoryLists[LOWPROB_CATEGORY].size > 0){
        return GetHighestProbCoordinateFromCategory(row, col, &player->probabilityHeapCategoryLists[LOWPROB_CATEGORY]);
    }

    //All grid cells have 0 probability: the caller falls back to another way of picking a cell.
    return -1;
}

#pragma endregion

/**
 * This function updates a specified binomial heap: the heap is reassembled from the current probabilities of the cells of its region,
 * then moved to the right probability category.
 * 
 * Only cells that can still be targeted go in the heap: cells that were not shot (neither HIT nor MISS) and whose probability is above 0.
 * The heap is rebuilt from the grid rather than from its own elements so that a cell whose probability dropped to 0 earlier (and was left
 * out) comes back once its probability rises again (eg. after a hit next to it).
 * 
 * Input:
 *      - player: the player of which a heap will be updated
 *      - heapIndex: the index of the heap that will be updated. This index should be a valid index within the size of the heap hashmap.
 * 
 * Time complexity: O(k log k) for a region of k cells, constant since the region size is constant. Moving the heap to its category is O(1).
 */
int UpdateHeap(Player * player, int heapIndex){

    if (heapIndex >= PROB_REGION_COUNT || heapIndex < 0) return -1;

    BinomialHeap * targetHeap = player->probabilityHeapSet[heapIndex];

    //First I empty the heap, freeing its old elements:
    while (targetHeap->head != NULL)
    {
        free(deleteMin(targetHeap));
    }

    //Then I re-insert every cell of the region with its new probability. The region's top-left cell is found from its index the same
    //way HashRegion() numbers the regions: row by row from left to right.
    int row0 = (heapIndex / PROB_REGIONS_PER_ROW) * PROB_REGION_HEIGHT;
    int col0 = (heapIndex % PROB_REGIONS_PER_ROW) * PROB_REGION_WIDTH;

    for (int row = row0; row < row0 + PROB_REGION_HEIGHT && row < GRIDSIZE; row++)
    {
        for (int col = col0; col < col0 + PROB_REGION_WIDTH && col < GRIDSIZE; col++)
        {
            //Now I need to make sure that the probability is not 0 and the coordinate is neither a hit nor a miss:
            if (player->probabilityGrid[row][col] > 0 && player->grid[row][col] != MISS && player->grid[row][col] != HIT){
                BinHeap_Insert_ProbElement(targetHeap, player->probabilityGrid[row][col], row, col);
            }
        }
    }

    //Finally, I must move this heap to the right category. The region's list node is stored with the region, so this is O(1):
    return PlaceHeapInCategory(player, heapIndex);
}


/**
 * This function will scan an area on the graph and update, once only, the binomial heap tied to each probability region.
 * The area is given by its inclusive bounds and is clipped to the grid.
 */
int UpdateHeapsWithinBounds(Player * player, int row0, int row1, int col0, int col1){

    //I could keep track of every region I've already updated so that I don't update it again.
    //But I don't see no easy way of doing that without using a data structure
    //Or, better way: I could save the hash indices of the regions updated and check if the array contains the one I'm trying to update

    //Even better way: I could set up a coordinate point that jumps between regions. I can access the region size from the PROB_REGION_SIZE macro.
    //Jumping a whole region at a time means every region that intersects the area is visited exactly once: I go from the region that
    //holds the top-left corner of the area to the region that holds its bottom-right corner, row of regions by row of regions.

    row0 = MAX(0, row0);
    col0 = MAX(0, col0);
    row1 = MIN(GRIDSIZE - 1, row1);
    col1 = MIN(GRIDSIZE - 1, col1);

    if (row0 > row1 || col0 > col1) return 0;

    for (int regionRow = row0 / PROB_REGION_HEIGHT; regionRow <= row1 / PROB_REGION_HEIGHT; regionRow++)
    {
        for (int regionCol = col0 / PROB_REGION_WIDTH; regionCol <= col1 / PROB_REGION_WIDTH; regionCol++)
        {
            UpdateHeap(player, regionRow * PROB_REGIONS_PER_ROW + regionCol);
        }
    }

    return 1;
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

    return task;
}

/**
 * Frees a task together with its arguments and flags. The task owns every argument and flag it holds (each one is its own allocation),
 * so that this function can free them all the same way.
 */
int freeTask(BotTask * task){

    if (task == NULL) return 0;

    if (task->arguments != NULL){
        for (int i = 0; i < task->argumentCount; i++)
        {
            free(task->arguments[i]);
        }
        free(task->arguments);
    }

    if (task->flags != NULL){
        for (int i = 0; i < task->flagCount; i++)
        {
            free(task->flags[i]);
        }
        free(task->flags);
    }

    free(task);

    return 1;
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

    return task->function(task->arguments);

}

#pragma region [Bot Firing Systems]

/**
 * This function pushes a task onto the bot's stack memory to fire at the cell (row, col) on a later turn.
 *
 * Every argument gets its own allocation, including the two player pointers (stored in a Player** box), so that the task owns all of
 * its arguments and freeTask() can free them the same way without ever freeing the players themselves.
 */
static void AssignFireTask(int priorityFlag, Player * bot, Player * opponent, int row, int col){

    int argCount = 4;
    void ** args = (void**)(malloc(sizeof(void*) * argCount));

    int* rowPtr = (int*)(malloc(sizeof(int)));
    *rowPtr = row;
    int* colPtr = (int*)(malloc(sizeof(int)));
    *colPtr = col;
    Player** botPtr = (Player**)(malloc(sizeof(Player*)));
    *botPtr = bot;
    Player** opponentPtr = (Player**)(malloc(sizeof(Player*)));
    *opponentPtr = opponent;

    //Setting the arguments:
    args[0] = rowPtr;
    args[1] = colPtr;
    args[2] = botPtr;
    args[3] = opponentPtr;

    AssignNewTask(priorityFlag, bot, BotFire, args, argCount, NULL, 0);
}

/**
 * Task function: fires at a cell. The arguments are the ones packed by AssignFireTask():
 *      - args[0]: int * row of the targeted cell
 *      - args[1]: int * column of the targeted cell
 *      - args[2]: Player ** bot
 *      - args[3]: Player ** opponent
 *
 * Returns 1 if the bot fired, and 0 if the task is no longer valid (the cell was already shot since the task was assigned, which happens
 * when two hits next to each other both queue the cell between them).
 */
int BotFire(void** args){
    
    if (args == NULL) return 0;

    int row = *(int*)args[0];
    int col = *(int*)args[1];

    Player * bot = *(Player**)args[2];

    Player * opponent = *(Player**)args[3];

    if (opponent->grid[row][col] == HIT || opponent->grid[row][col] == MISS) return 0;

    return BotFireHelper(row, col, bot, opponent) > 0;

}

int BotFireHelper(int row, int col, Player * bot, Player * opponent){

    char * coords = alloc_GetCoordsFromIndices(row, col, GRIDSIZE, startingCoordinate_1, startingCoordinate_2,
     endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);

    //The bot always fires in easy mode (difficulty 0) so that its misses are written on the grid: that's how the bot remembers them.
    //On hard mode they stay hidden from the player because DisplayOpponentGrid() only shows misses on easy mode.
    char * error = NULL;
    int fireRes = Fire(coords, opponent->grid, 0, &error);

    free(coords);
    if (error != NULL) free(error);

    if (fireRes < 0){
        //The cell was already hit: nothing was fired. The caller must pick another target.
        return fireRes;
    }

    BotAnalyzeShot(row, col, bot, opponent);

    return fireRes;

}

/**
 * This is the "analyze results and changes, then assign operations for next turn" part of the bot's brain. It runs after every cell the
 * bot shoots at (row, col), whatever the weapon:
 *      - If the shot sank a ship, the entire probability grid and every region heap are recalculated. (presentation, 2.1.2)
 *        When no damaged ship is left after that, the tasks left on the stack (cells around the sunk ship) are dropped.
 *      - Otherwise the probabilities around the cell, and the heaps of the regions around it, are updated.
 *      - If the shot was a HIT on a ship that is still afloat, tasks are assigned to target the surrounding cells.
 */
void BotAnalyzeShot(int row, int col, Player * bot, Player * opponent){

    int sunkBefore = CountSunkFlags(opponent);
    int sunkAfter = countSunkShips(opponent); //Also updates the IsSunk flags the probability formulas rely on

    if (sunkAfter > sunkBefore){

        RecalculateAllProbabilities(opponent);

        if (!HasOpenHits(opponent)){
            ClearStackMemory(bot);
        }
    }
    else {
        int target[2] = {row,col};

        //Updating the probability distribution:
        UpdateSurroundingProbabilities(opponent, target);

        //Update the probability heaps for all the probability regions that surround the target.
        UpdateHeapsWithinBounds(opponent, row - LONGEST_SHIPLENGTH, row + LONGEST_SHIPLENGTH,
         col - LONGEST_SHIPLENGTH, col + LONGEST_SHIPLENGTH);
    }

    //Need to check if the target was a HIT or a MISS. If it's a HIT then we assign new tasks to target the surrounding cells:
    if(opponent->grid[row][col] == HIT && !IsCellOfSunkShip(opponent, row, col)){

        int neighbours[4][2] = {{row + 1, col}, {row - 1, col}, {row, col + 1}, {row, col - 1}};

        for (int n = 0; n < 4; n++)
        {
            int r = neighbours[n][0];
            int c = neighbours[n][1];

            if (IndexWithinRange(r) && IndexWithinRange(c) && opponent->grid[r][c] != HIT && opponent->grid[r][c] != MISS){
                AssignFireTask(TASKFLAG_HIGHPRIORITY, bot, opponent, r, c);
            }
        }
    }

}

/**
 * Recalculates the probability of every cell of the grid and rebuilds every region heap. Used when a ship sinks: the ship no longer counts
 * in the formulas, which changes the probabilities everywhere.
 */
int RecalculateAllProbabilities(Player * player){

    int wholeGrid[4] = {0, GRIDSIZE - 1, 0, GRIDSIZE - 1};

    UpdateRegionProbabilities(player, wholeGrid);

    return UpdateHeapsWithinBounds(player, 0, GRIDSIZE - 1, 0, GRIDSIZE - 1);
}

//Number of the player's ships that are flagged as sunk (without re-checking the grid, unlike countSunkShips())
static int CountSunkFlags(Player * player){
    return player->carrierBounds.IsSunk + player->battleshipBounds.IsSunk + player->destroyerBounds.IsSunk + player->submarineBounds.IsSunk;
}

//Returns 1 if the player's grid has a HIT on a ship that is not sunk yet (a damaged ship the bot should finish)
static int HasOpenHits(Player * player){

    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            if (player->grid[i][j] == HIT && !IsCellOfSunkShip(player, i, j)) return 1;
        }
    }

    return 0;
}

//Drops (and frees) every task on the bot's stack memory
void ClearStackMemory(Player * bot){

    if (bot->stackMemory == NULL) return;

    while (!is_empty(bot->stackMemory))
    {
        freeTask((BotTask*)removeFirst(bot->stackMemory));
    }
}

#pragma endregion



#pragma region [Random Targeting]

/**
 * Returns 1 if at least one of the opponent's ships that are still afloat could lie on the cell (row, col), and 0 otherwise.
 * A ship could lie on the cell if, horizontally or vertically, it fits in a line through the cell with no MISS and no sunk ship in the way.
 */
int CanCellHoldShip(Player * opponent, int row, int col){

    if (opponent->grid[row][col] == MISS || IsCellOfSunkShip(opponent, row, col)) return 0;

    ShipBounds ships[] = {opponent->carrierBounds, opponent->battleshipBounds, opponent->destroyerBounds, opponent->submarineBounds};

    for (int s = 0; s < SHIPCOUNT; s++)
    {
        if (ships[s].IsSunk) continue;

        int shipLength = MAX(ships[s].endRow - ships[s].startRow, ships[s].endCol - ships[s].startCol) + 1;

        //Every horizontal (o = 0) and vertical (o = 1) placement of the ship that covers the cell:
        for (int o = 0; o < 2; o++)
        {
            for (int start = -(shipLength - 1); start <= 0; start++)
            {
                int fits = 1;

                for (int k = start; k < start + shipLength && fits; k++)
                {
                    int r = (o == 0) ? row : row + k;
                    int c = (o == 0) ? col + k : col;

                    if (!IndexWithinRange(r) || !IndexWithinRange(c) || opponent->grid[r][c] == MISS || IsCellOfSunkShip(opponent, r, c)){
                        fits = 0;
                    }
                }

                if (fits) return 1;
            }
        }
    }

    return 0;
}

/**
 * Picks a random cell of the opponent's grid that was not shot yet (neither HIT nor MISS), every such cell being equally likely.
 * When onlyCellsThatCanHoldShips is true, cells where no remaining ship fits are skipped too (see CanCellHoldShip()).
 * 
 * Returns 1 and stores the cell in row and col, or -1 if there is no such cell.
 */
int PickRandomCell(int * row, int * col, Player * opponent, int onlyCellsThatCanHoldShips){

    int candidates = 0;

    //First pass: count the candidate cells. Second pass: walk to a random one of them.
    for (int pass = 0; pass < 2; pass++)
    {
        int pick = (pass == 1) ? rand() % candidates : -1;
        int seen = 0;

        for (int i = 0; i < GRIDSIZE; i++)
        {
            for (int j = 0; j < GRIDSIZE; j++)
            {
                if (opponent->grid[i][j] == HIT || opponent->grid[i][j] == MISS) continue;
                if (onlyCellsThatCanHoldShips && !CanCellHoldShip(opponent, i, j)) continue;

                if (pass == 1 && seen == pick){
                    *row = i;
                    *col = j;
                    return 1;
                }
                seen++;
            }
        }

        candidates = seen;
        if (candidates == 0) return -1;
    }

    return -1;
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
