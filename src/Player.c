#include "../include/Player.h"
#include "../include/ShipPlacement.h"
#include "../include/Bot.h"
#include "../include/CalcProbs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

char * PlayerColors[playerColorCount] = {RED, GREEN, YELLOW, BLUE, MAGENTA};


Player ** alloc_InitializePlayerArray(int playerCount, Player *** playersArray){
    *playersArray = (Player**)(malloc(sizeof(Player*) * playerCount));

    for (int i = 0; i < playerCount; i++)
    {
        (*playersArray)[i] = NULL;
    }
    
}

/**
 * We pass a pointer to a pointer here not just a pointer because the function would create a copy of the pointer and have the copy point at the allocated
 * memory. Then I would have to set the orignal pointer equal to the one returned here. I decided to pass a pointer to a pointer to not have to do all that,
 * just call the function without needing a left side of an expression.
 */
Player* alloc_InitializePlayer(Player** output, char* playerName, int isBot, BotIQ botIQ){
    
    

    *output = (Player *)(malloc(sizeof(Player)));

    //Initialize Grid and name:

    (*output)->name = (char*)(malloc(sizeof(char) * MAXINPUTLENGTH));

    strcpy((*output)->name, playerName);

    (*output)->grid = (char**)(malloc(sizeof(char*) * GRIDSIZE));
    for (int i = 0; i < GRIDSIZE; i++)
    {
        ((*output)->grid)[i] = (char*)(malloc(sizeof(char) * GRIDSIZE));
    }

    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            ((*output)->grid)[i][j] = WATER_C;
        }
        
    }
    
    //initializing artilleryused, smokegrid, and torpedoused to false

    (*output)->smokeGrid = (bool**)(malloc(sizeof(bool*) * GRIDSIZE));
    for (int i = 0; i < GRIDSIZE; i++) {
        (*output)->smokeGrid[i] = (bool*)malloc(sizeof(bool) * GRIDSIZE);
        for (int j = 0; j < GRIDSIZE; j++) {
            (*output)->smokeGrid[i][j] = false; // initializing each cell to false
        }
    }
    
    (*output)->usedsmokes = 0;
    // Initialize sweepsLeft to 3
    (*output)->sweepsLeft = 3;
    (*output)->prevSunk = 0;
    (*output)->currSunkShips = 0;



    //initialize player color
    srand(time(0));
    (*output)->UIColor = PlayerColors[rand() % playerColorCount]; //Need a better way so that no two players have same color.
    

    (*output)->isBot = isBot;
    (*output)->botIQ = botIQ;


    //Initialize the probability distribution grid:
    InitializeProbabilities(*output);

    DisplayIntGrid((*output)->probabilityGrid, GRIDSIZE);

    InitializeProbabilityHeaps(*output);
    //printf("adsff\n");

    if (isBot == 1){

        //If Bot: Initialize Bot Stack Memory:
        InitializeBotStackMemory(*output);
    }

    (*output)->currTargetCategory = PROB_CATEGORYCOUNT - 1;



    //Initializing Risk Factor:
    (*output)->riskFactor = NORMAL_RISK;

    return *output;
}

#pragma region [PROBABILITY INITIALIZATION]

/**
 * This function passes over the entire grid and calculates the starting probabilities of each cell.
 */
int InitializeProbabilities(Player * player){

    player->probabilityGrid = (int**)(malloc(sizeof(int*) * GRIDSIZE));

    for (int i = 0; i < GRIDSIZE; i++)
    {
        player->probabilityGrid[i] = (int*)(malloc(sizeof(int) * GRIDSIZE));
    }

    //I need to calculate the cutoff probability for each square on the grid:
    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            int ij[2] = {i,j}; 
            InitalizeCutOffProb(player, ij);
        }
        
    }
    
    return 1;

}

#pragma region [Binomial Heap Specific Functions]
int compareProbabilities(void * elem1, void * elem2){

    //The void pointer will be an integer pointer representing an array of 3 integers
    //The integer at index 0 is the key and the indices 1 and 2 represent the coordinates i and j respectively

    return ((int*)elem2)[0] - ((int*)elem1)[0];//returns elem2 - elem1 so that it would act as a maxheap
}

/**
 * Input:
 *      - Prob: the probability at this grid coordinate
 *      - i: the row index
 *      - j: the column index
 */
int BinHeap_Insert_ProbElement(BinomialHeap * binHeap, int Prob, int row, int col){

    int * element = (int*)(malloc(sizeof(int) * 3));
    element[0] = Prob;
    element[1] = row;
    element[2] = col;

    insert(binHeap, (void*)element);

    return 1;
}

/**
 * This function returns an pointer which represents an array of three integers (probability, i, j).
 */
int* BinHeap_FindHighestProbabilityCell(BinomialHeap * binHeap){

    void * topVal = findMin(binHeap);

    return (int*)(topVal);

}

void InOrderTraversalTree_ProbNode(Node * root){
    if (root == NULL) return;

    
    InOrderTraversalTree_INT(root->leftChild);
    printf("%d, ", ((int*)(root->value))[0]);
    
    InOrderTraversalTree_INT(root->rightSibling);
}

void InOrderTraversal_ProbNode_Print(BinomialHeap * heap){

    if (heap == NULL) return;

    Node * curr = heap->head;


    InOrderTraversalTree_INT(curr);
    
    printf("\n");

}

#pragma endregion


/**
 * 
 * This function takes in two integers i and j representing coordinates of a region in the grid.
 * It rounds the two coordinates to the region origin and hashes that origin to an index. The hashing is simple, we just number the regions
 * row by row from left to right, and according to i and j we can calculate that number.
 * 
 */
int HashRegion(int i, int j){

    //Step 1: Get region coordinate
    //The function should first transform i and j into the region corner coordinate which will be then hashed.
    //This way all coordinates within the same region will be hashed to the same index.

    double regionSize[2] = PROB_REGION_SIZE;


    //We can't just add i and j then hash the sum because (1,0) and (0,1) would then hash to the same index

    //Step 2: Hash the resultant value:

    //This is very simple, we are just numbering the regions on the grid row by row from left to right

    double di = (double)i;
    double dj = (double)j;
    double GRDSZ = (double)GRIDSIZE;

    double rows = floor(di / regionSize[0]);
    double cols = ceil(GRIDSIZE / regionSize[1]);
    double colIdx = floor(dj / regionSize[1]);

    int regionIndex = (int)(rows * cols + colIdx);
    return regionIndex;
}


/**
 * This function initializes the binomial heaps stored inside the player struct. For every region of the probability graph, the function creates
 * a heap that stores the coordinates of a region and orders them according to probability. The heap is a maximum heap, it prioritizes higher probabilities.
 * 
 * Heaps could fall into one of three categories:
 *      - High probability: Heaps of regions with highest probability >= HIGHPROB_BASE macro are placed here
 *      - Average probability: Heaps of regions with highest probability >= AVGPROB_BASE macro but < HIGHPROB_BASE are placed here
 *      - Low probability: Heaps of regions with highest probability >= LOWPROB_BASE macro but < AVGPROB_BASE are placed here
 *      - Null chance: Heaps of regions with highest probability = 0 are placed here
 * 
 * Maximum memory is allocated for each category (the total number of regions) not to have to realloc every time.
 */
int InitializeProbabilityHeaps(Player * player){

    //Initializing the binomial heap hashset:
    player->probabilityHeapSet = (BinomialHeap**)(malloc(sizeof(BinomialHeap*) * PROB_REGION_COUNT));

    for (int i = 0; i < PROB_REGION_COUNT; i++)
    {
        player->probabilityHeapSet[i] = (BinomialHeap * )(malloc(sizeof(BinomialHeap)));
        player->probabilityHeapSet[i]->head = NULL;
        player->probabilityHeapSet[i]->compare = compareProbabilities;
    }
    

    //Initializing the array of linked lists to categorize the probability binomial heaps:
    player->probabilityHeapCategoryLists = (D_LinkedList*)(malloc(sizeof(D_LinkedList) * PROB_CATEGORYCOUNT));
    for (int i = 0; i < PROB_CATEGORYCOUNT; i++)
    {
        initialize_empty_DList(&player->probabilityHeapCategoryLists[i]);
    }
    


    //We will pass over every cell on the grid and insert it into the right Binomial Heap in the heap hashset using the HashRegion() function.
    //This way is better than looping over regions using math and stuff because the latter would create problems with the Hash values 
    //(trust me on this, i hurt my soul debugging that)

    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            int hashIndex = HashRegion(i,j);
            //player->probabilityGrid[i][j] = hashIndex;
            //printf("el: %d,%d | hash: %d\n",i,j,hashIndex);
            BinHeap_Insert_ProbElement(player->probabilityHeapSet[hashIndex], player->probabilityGrid[i][j], i, j);
        }
        
    }

    DisplayIntGrid(player->probabilityGrid, GRIDSIZE);
    
    //Now here, we go over the hashset and we categorize the heaps into the three category arrays:

    for (int i = 0; i < PROB_REGION_COUNT; i++)
    {
        BinomialHeap * targetHeap = player->probabilityHeapSet[i];

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
        
    }
    

//    DisplayIntGrid(player->probabilityGrid, GRIDSIZE);
            
//    InOrderTraversal_ProbNode_Print(&binHeap);


    //exit(0);
    

    return 1;
}

#pragma endregion



#pragma region [Bot-Specific Initializations]

int InitializeBotStackMemory(Player * bot){

    //To be continued...
    if (bot->isBot == true)
        bot->stackMemory = create_empty_Dlist();
    else bot->stackMemory = NULL;

    return 1;
}

#pragma endregion











void DisplayGrid(char ** grid, int gridSize){

    //I need to calculate proper indentation between grid squares. It depends on the length of the numerals:
    int base = endingCoordinate_1 - startingCoordinate_1 + 1;
    int TopNumLen = 0;
    int k = gridSize;
    do
    {
        TopNumLen++;
        k/=base;
    } while (k > 0);
    

    printf("\n");
    int h = (getConsoleWidth() - (TopNumLen * gridSize) - gridSize);
    Indent(h/2);

    //I need a way to convert integer to numeral according to my numeral system.
    for (int i = 0; i < gridSize; i++)
    {
        char* num = alloc_IntegerToNumeral(i, gridSize, startingCoordinate_1, endingCoordinate_1);
        printf("%s ", num);
        free(num);
    }

    Println("");

    for (int i = 1; i <= gridSize; i++)
    {
        char* num = alloc_IntegerToNumeral(i, gridSize, startingCoordinate_2, endingCoordinate_2);
        
        Indent(h/2 - 7 - strlen(num) - 1);
        printf("%s%*s", num, 8, "");
        free(num);

        for (int j = 0; j < gridSize; j++)
        {
            char * color = WHITE;

            if (grid[i-1][j] == HIT) color = RED;
            printf("%s", color);
            printf("%c", grid[i-1][j]);
            printf(RESET);
            for (int l = 0; l < (TopNumLen); l++)
            {
                printf(" ");
            }
            
        }
        Println("");
    }
    Println("");

}

void DisplayIntGrid(int ** grid, int gridSize){

    //I need to calculate proper indentation between grid squares. It depends on the length of the numerals:
    int base = endingCoordinate_1 - startingCoordinate_1 + 1;
    int TopNumLen = 0;
    int k = gridSize;
    do
    {
        TopNumLen++;
        k/=base;
    } while (k > 0);
    

    printf("\n");
    int h = (getConsoleWidth() - (TopNumLen * gridSize) - gridSize);
    Indent(h/2);

    //I need a way to convert integer to numeral according to my numeral system.
    for (int i = 0; i < gridSize; i++)
    {
        char* num = alloc_IntegerToNumeral(i, gridSize, startingCoordinate_1, endingCoordinate_1);
        printf("%s ", num);
        free(num);
    }

    Println("");

    for (int i = 1; i <= gridSize; i++)
    {
        char* num = alloc_IntegerToNumeral(i, gridSize, startingCoordinate_2, endingCoordinate_2);
        
        Indent(h/2 - 7 - strlen(num) - 1);
        printf("%s%*s", num, 8, "");
        free(num);

        for (int j = 0; j < gridSize; j++)
        {
            char * color = WHITE;

            if (grid[i-1][j] == HIT) color = RED;
            printf("%s", color);
            printf("%d", grid[i-1][j]);
            printf(RESET);
            for (int l = 0; l < (TopNumLen); l++)
            {
                printf(" ");
            }
            
        }
        Println("");
    }
    Println("");

}

int IsShipChar(char c){

    return (c == BATTLESHIP_C || c == SUBMARINE_C || c == DESTROYER_C || c == CARRIER_C);

}

/**
 * Does the same job as DisplayGrid() function but hides ships on the gird and shows misses according to an integer passed.
 */
void DisplayOpponentGrid(char ** grid, int gridSize, int showMiss){

    //I need to calculate proper indentation between grid squares. It depends on the length of the numerals:
    int base = endingCoordinate_1 - startingCoordinate_1 + 1;
    int TopNumLen = 0;
    int k = gridSize;
    do
    {
        TopNumLen++;
        k/=base;
    } while (k > 0);

    //I need a way to convert integer to numeral according to my numeral system.
    printf("\n");
    int h = (getConsoleWidth() - (TopNumLen * gridSize) - gridSize);
    Indent(h/2);
    for (int i = 0; i < gridSize; i++)
    {
        char* num = alloc_IntegerToNumeral(i, gridSize, startingCoordinate_1, endingCoordinate_1);
        printf("%s ", num);
        free(num);
    }

    Println("");

    for (int i = 1; i <= gridSize; i++)
    {
        char* num = alloc_IntegerToNumeral(i, gridSize, startingCoordinate_2, endingCoordinate_2);
        Indent(h/2 - 7 - strlen(num) - 1);
        printf("%s%*s", num, 8, "");
        free(num);

        for (int j = 0; j < gridSize; j++)
        {

            char * color = WHITE;

            if (grid[i-1][j] == HIT) color = RED;

            printf("%s", color);

            char c = grid[i-1][j];

            if (!IsShipChar(c)){
                if (c == MISS){
                    if (showMiss == 0){
                        printf("%c", WATER_C);
                        continue;
                    } 
                    else {
                        printf("%c", c);
                    }
                }
                else{
                    printf("%c", c);
                }
            }
            else {
                printf("%c", WATER_C);
            }

            //Indent the correct amount of times:
            for (int l = 0; l < (TopNumLen); l++)
            {
                printf(" ");
            }
            
        }
        Println("");
    }
    Println("");

}



//I hate this function, for some reason I debugged it for 2 hrs
int countSunkShips(Player* player){

    int res = MAX(0, checkIfSunk(player, &(player->battleshipBounds)));
    res += MAX(0, checkIfSunk(player, &(player->carrierBounds)));
    res += MAX(0, checkIfSunk(player, &(player->destroyerBounds)));
    res += MAX(0, checkIfSunk(player, &(player->submarineBounds)));

    return MAX(0,res);

}

int checkIfSunk(Player *player, ShipBounds *shipBounds) {

    for (int i = shipBounds->startRow; i <= shipBounds->endRow; i++) {
        for (int j = shipBounds->startCol; j <= shipBounds->endCol; j++) {
            if (player->grid[i][j] != HIT) {
                shipBounds->IsSunk=0;
                return -1;
            }
        }
    }
    shipBounds->IsSunk=1;
    return 1;
}


void ShowPlayerStats(Player * player){
    Println("");
    char * txt = CreateString_alloc(2, player->name, "'s STATS:");
    Println_Centered(txt, strlen(txt), player->UIColor);

    free(txt);

    int shipsLeft = 4 - player->currSunkShips;

    Print_Centered("Ships left: ", strlen("Ships left: 1"), player->UIColor);
    
    SetColor(player->UIColor);
    printf("%d", shipsLeft);
    ResetFormat();
    Println("");

    Print_Centered("Artillery: ", strlen("Artillery: 1"), player->UIColor);
    SetColor(player->UIColor);
    printf("%d", player->prevSunk);
    ResetFormat();
    Println("");

}

void Show_2_PlayerStats(Player * player, Player * opponent){
    ShowPlayerStats(player);
    Print_Centered("Torpedo: ", strlen("Torpedo: 1"), player->UIColor);
    SetColor(player->UIColor);
    printf("%d", (player->prevSunk && opponent->currSunkShips >= 3)?(1):0);
    ResetFormat();
    Println("");

    Println("");
    ShowPlayerStats(opponent);
}