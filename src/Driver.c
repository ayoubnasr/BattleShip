#include "../include/Driver.h"
#include "../include/CalcProbs.h"

int currPlayer;
int currOpponent;

Player **playersArray;

//Most complicated function:
void Quit()
{
    exit(0);
}

/**
 * This method will return the game mode index according to the user input.
 * Indices are returned according to how the modes are placed in the GameModeStrings array.
 */
int SelectGameMode(char *modeInput)
{

    return StringToEnumIndex(modeInput, GameModeStrings, GAMEMODECOUNT);
}

int SetDifficulty(char *difficultyInput)
{
    int diff = StringToEnumIndex(difficultyInput, DifficultyStrings, DIFFICULTYCOUNT);

    if (diff < 0)
        return -1;

    DifficultyValue = diff;

    return diff;
}

int PerformOperation(char **inputPtr, char ** outputMsg)
{

    // It will check the operation:
    // If invalid throw warning
    // If valid pass input to specific operation function.

readoperation:

    char *operationStr = next(inputPtr);

    int operationIndex = StringToEnumIndex(operationStr, InstructionSet, TOTALINSTRUCTIONCOUNT);

    if (operationIndex < 0 || strlen(operationStr) == 0)
    {
        if (outputMsg != NULL) *outputMsg = CreateString_alloc(1, INVALID_OPERATION_WARNING);
        free(operationStr);
        return -1;
    }

    free(operationStr);

    char *coords = next(inputPtr);

    int res;

    switch (operationIndex)
    {
    case START:
        res = 1;
        break;
    case QUIT:
        Quit();
        res = 2;
        break;
    case NEXTURN:
        char * oppName = playersArray[currOpponent % PlayerCount]->name;
        //int textLen = strlen("Skipping turn. Turn passed to ") + strlen(oppName);
        //Print_Centered("Skipping turn. Turn passed to ", textLen, WHITE);
        //PrintlnClr(oppName, playersArray[currOpponent % PlayerCount]->UIColor);

        if (outputMsg != NULL) *outputMsg = CreateString_alloc(2, "Skipping turn. Turn passed to ", oppName);

        res = 3;
        break;
    case FIRE:
        int fire = Fire(coords, playersArray[(currPlayer + 1) % PlayerCount]->grid, DifficultyValue, outputMsg);
        if (fire > 0)
            res = 4;
        else
            res = fire;
        break;
    case RADAR:
        int radar = performRadarSweep(coords, playersArray[(currPlayer + 1) % PlayerCount], outputMsg);
        if (radar > 0)
            res = 5;
        else
            res = radar;
        break;
    case SMOKE:
        int smokescreen = applySmokeScreen(coords, playersArray[currPlayer], playersArray[currOpponent], outputMsg);
        if (smokescreen > 0)
            res = 6;
        else
            res = smokescreen;
        break;
    case ARTILLERY:
        int artillery = Artillery(coords, playersArray[currPlayer],playersArray[currOpponent], DifficultyValue, outputMsg);
        if (artillery > 0)
            res = 7;
        else res = artillery;
        break;
    case TORPEDO:
        int torpedo = Torpedo(coords, playersArray[currPlayer],playersArray[currOpponent],DifficultyValue, outputMsg);
        if (torpedo > 0)
            res = 8;
        else res = torpedo;
        break;

    default:
        break;
    }

    free(coords);
    return res;

    // Map the operationStr to the right operation
    // Choose the right function to call
}



void CheckForQuit(char *input)
{
    if (strcmpi(input, "quit") == 0)
    {
        Quit();
    }

    return;
}

int SetUpShip(int playerIndex, char *shipName, char shipChar, int shipWidth)
{

    char *input;
    char *inpPtr;

    Print_Centered("Place the ", strlen("Place the ") + strlen(shipName), WHITE);
    PrintClr(shipName, WHITE);
    Println("");

start:

    inpPtr = alloc_Input(REQUEST_COORDINATE, &input);

    char *coords = next(&inpPtr);

    CheckForQuit(coords);

    free(input);

    inpPtr = alloc_Input(REQUEST_ORIENTATION, &input);

    char *orientation = next(&inpPtr);

    CheckForQuit(orientation);

    free(input);

    Player *player = playersArray[playerIndex];

    int shipSize[2] = {shipWidth, 0};

    char * outputMsg = NULL;

    int placement = PlaceShipOnGridHelper(player, shipChar, (*player).grid, coords, orientation, shipSize, &outputMsg);

    if (placement < 0)
    {
        Println_Centered(outputMsg, strlen(outputMsg), RED);
        if (outputMsg != NULL) free(outputMsg);
        goto start;
    }

    if (outputMsg != NULL) free(outputMsg); //In case any function outputs a message (for debugging or whatever)

    return 1;
}

void SetUpNewPlayer(int index)
{
    char *input;
    char *inpPtr;

    playername:

    inpPtr = alloc_Input(REQUEST_PLAYERNAME, &input);

    char *name = next(&inpPtr);

    CheckForQuit(name);

    if (strlen(name) == 0){
        Println_Centered(INVALID_INPUT_WARNING, strlen(INVALID_INPUT_WARNING), RED);
        free(input);
        free(name);
        goto playername;
    }

    for (int i = 0; i < PlayerCount; i++)
    {
        if (playersArray[i] != NULL){
            if (strcmp(playersArray[i]->name, name) == 0){
                Println_Centered("Name is taken. Choose another name.", strlen("Name is taken. Choose another name."), RED);
                free(input);
                free(name);
                goto playername;
            }
        }
    }
    


    alloc_InitializePlayer(&(playersArray[index]), name, false, DUMB);

    free(input);
    free(name);    

    RefreshScreen();

    Print_Centered("Set up ", strlen("Set up 's grid:") + strlen(playersArray[index]->name), WHITE);
    PrintClr(playersArray[index]->name, playersArray[index]->UIColor);
    PrintlnClr("'s grid:", WHITE);
    
    DisplayGrid(playersArray[index]->grid, GRIDSIZE);
    SetUpShip(index, "Battleship", BATTLESHIP_C, BATTLESHIP_LENGTH);

    RefreshScreen();
    Print_Centered("Set up ", strlen("Set up 's grid:") + strlen(playersArray[index]->name), WHITE);
    PrintClr(playersArray[index]->name, playersArray[index]->UIColor);
    PrintlnClr("'s grid:", WHITE);

    DisplayGrid(playersArray[index]->grid, GRIDSIZE);
    SetUpShip(index, "Carrier", CARRIER_C, CARRIER_LENGTH);

    RefreshScreen();
    Print_Centered("Set up ", strlen("Set up 's grid:") + strlen(playersArray[index]->name), WHITE);
    PrintClr(playersArray[index]->name, playersArray[index]->UIColor);
    PrintlnClr("'s grid:", WHITE);

    DisplayGrid(playersArray[index]->grid, GRIDSIZE);
    SetUpShip(index, "Destroyer", DESTROYER_C, DESTROYER_LENGTH);

    RefreshScreen();
    Print_Centered("Set up ", strlen("Set up 's grid:") + strlen(playersArray[index]->name), WHITE);
    PrintClr(playersArray[index]->name, playersArray[index]->UIColor);
    PrintlnClr("'s grid:", WHITE);

    DisplayGrid(playersArray[index]->grid, GRIDSIZE);
    SetUpShip(index, "Submarine", SUBMARINE_C, SUBMARINE_LENGTH);
    DisplayGrid(playersArray[index]->grid, GRIDSIZE);

    RefreshScreen();
    Print_Centered("Set up ", strlen("Set up 's grid:") + strlen(playersArray[index]->name), WHITE);
    PrintClr(playersArray[index]->name, playersArray[index]->UIColor);
    PrintlnClr("'s grid:", WHITE);
}





void SetUpBot(int index){

    char * input;

    askBotDifficulty:

    alloc_Input("Please choose the bot difficulty (easy, hard)", &input);

    CheckForQuit(input);

    int isEasy = strcmpi(input, "easy");
    int isHard = strcmpi(input, "hard");

    if (isEasy != 0 && isHard != 0){
        free(input);
        Println_Centered("Invalid input! Please enter a valid bot difficulty.", strlen("Invalid input! Please enter a valid bot difficulty."), RED);
        goto askBotDifficulty;
    }


    int botIQ = DUMB;

    if (isHard == 0) botIQ = SMART;

    free(input);

    alloc_InitializePlayer(&(playersArray[index]), "Botteyi", true, botIQ);

    PlaceBotShips(playersArray[index]);
    RefreshScreen();

}







int PickRandomPlayer(int playerCount)
{
    int r = rand() % playerCount;

    return r;
}

void ShowTurnStats(){
    Print_Centered("", strlen("'s turn.") + strlen(playersArray[currPlayer%PlayerCount]->name), WHITE);
    PrintClr(playersArray[currPlayer % PlayerCount]->name, playersArray[currPlayer % PlayerCount]->UIColor);
    PrintlnClr("'s turn.", WHITE);

    Print_Centered("", strlen(" is attacking ") + strlen(playersArray[currPlayer % PlayerCount]->name) + strlen(playersArray[currOpponent % PlayerCount]->name), WHITE);
    PrintClr(playersArray[currPlayer % PlayerCount]->name, playersArray[currPlayer % PlayerCount]->UIColor);
    PrintClr(" is attacking ", WHITE);
    PrintlnClr(playersArray[currOpponent % PlayerCount]->name, playersArray[currOpponent % PlayerCount]->UIColor);

    Show_2_PlayerStats(playersArray[currPlayer % PlayerCount], playersArray[currOpponent % PlayerCount]);
}

/**
 * Tells the player where the bot fired this turn, by comparing the opponent's grid with how it was before the bot's turn.
 * (On hard mode the misses are not drawn on the grid, so this is the only way to know where the bot missed.)
 */
void AnnounceBotShots(Player * bot, char before[GRIDSIZE][GRIDSIZE])
{
    Player * opponent = playersArray[currOpponent];
    int shots = 0, hits = 0, firstRow = 0, firstCol = 0;

    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            if (opponent->grid[i][j] == before[i][j]) continue;

            if (shots == 0){
                firstRow = i;
                firstCol = j;
            }
            shots++;
            if (opponent->grid[i][j] == HIT) hits++;
        }
    }

    if (shots == 0) return;

    char * msg;
    char * coords = alloc_GetCoordsFromIndices(firstRow, firstCol, GRIDSIZE, startingCoordinate_1, startingCoordinate_2,
     endingCoordinate_1, endingCoordinate_2, coord_1_shift, coord_2_shift);

    if (shots == 1){
        msg = CreateString_alloc(4, bot->name, " fired at ", coords, (hits > 0) ? ": Hit!" : ": Miss.");
    }
    else {
        //Artillery or torpedo
        char count[16];
        sprintf(count, "%d", hits);
        msg = CreateString_alloc(5, bot->name, " used a special attack starting at ", coords, ", hits: ", count);
    }

    Println_Centered(msg, strlen(msg), (hits > 0) ? RED : WHITE);

    free(coords);
    free(msg);
}

int PlayTurn()
{
    int showMiss = (DifficultyValue == 0) ? 1 : 0;

    currOpponent = (currPlayer + 1) % PlayerCount;

    ShowTurnStats();

    DisplayOpponentGrid((playersArray[currOpponent])->grid, GRIDSIZE, showMiss);

    if (playersArray[currPlayer % PlayerCount]->isBot == 0){


    startofoperation:

        char *input;
        char *inpPtr;

        inpPtr = alloc_Input(REQUEST_OPERATION, &input);

        char * outputMsg = NULL;

        int hitsBefore = countHitCells(playersArray[currOpponent]);

        int operation = PerformOperation(&inpPtr, &outputMsg);

        //A bot keeps track of how well its opponent attacks (fire = 4, artillery = 7, torpedo = 8) to adjust its risk:
        if (playersArray[currOpponent]->isBot && (operation == 4 || operation == 7 || operation == 8)){
            RecordOpponentAttack(playersArray[currOpponent], countHitCells(playersArray[currOpponent]) > hitsBefore);
        }

        if (operation < 0)
        {
            RefreshScreen();
            ShowTurnStats();
            DisplayOpponentGrid((playersArray[currOpponent])->grid, GRIDSIZE, showMiss);

            if (outputMsg != NULL){
                Println_Centered(outputMsg, strlen(outputMsg), RED);//strlen breaks with NULL
                free(outputMsg);
            }
            
            free(input);
            goto startofoperation;
        }

        if (operation != 3)
        {
            RefreshScreen();

            if (outputMsg != NULL){
                Print_Centered("", strlen(outputMsg), WHITE);
                PrintlnClr(outputMsg, WHITE);
            }

            Print_Centered("", strlen(playersArray[currPlayer % PlayerCount]->name) + strlen("'s turn ended."), WHITE);
            PrintClr(playersArray[currPlayer % PlayerCount]->name, playersArray[currPlayer%PlayerCount]->UIColor);
            PrintClr("'s turn ended.", WHITE);

            DisplayOpponentGrid((playersArray[currOpponent])->grid, GRIDSIZE, showMiss);
        }

        if (outputMsg != NULL) free(outputMsg);
        free(input);
    }
    else {

        #pragma region [BOTS TURN]

        Player * bot = playersArray[currPlayer % PlayerCount];

        //Remember the opponent's grid to tell the player what the bot did:
        char before[GRIDSIZE][GRIDSIZE];
        for (int i = 0; i < GRIDSIZE; i++)
        {
            for (int j = 0; j < GRIDSIZE; j++)
            {
                before[i][j] = playersArray[currOpponent]->grid[i][j];
            }
        }

        //The bot reassesses the risk from how the opponent has been doing:
        UpdateRiskFactor(bot, playersArray[currOpponent]);

        //Choose Bot attack depending on bot level:
        switch (bot->botIQ)
        {
        case SMART:
            BotSmartAttack(bot, playersArray[currOpponent]);
            break;

        default:
            BotDumbAttack(bot, playersArray[currOpponent]);
            break;
        }

        RefreshScreen();
        ShowTurnStats();
        DisplayOpponentGrid((playersArray[currOpponent])->grid, GRIDSIZE, showMiss);

        AnnounceBotShots(bot, before);
        //DisplayIntGrid(playersArray[1]->probabilityGrid, GRIDSIZE);

        #pragma endregion


    }

    // Check if opponent lost:
    //...
    int oppSunkShips = countSunkShips(playersArray[currOpponent]);

    //If a player loses:
    if (oppSunkShips == SHIPCOUNT){

        char * win = CreateString_alloc(4, playersArray[currPlayer]->name, " sunk all of ", playersArray[currOpponent]->name, "'s ships!");

        Println_Centered(win, strlen(win), BLUE);

        //Note to self: MAKE PRINT FUNCTIONS VARIADIC!!!
        char * congrats = CreateString_alloc(3, "Well done ", playersArray[currPlayer]->name, "!");

        Println_Centered(congrats, strlen(congrats), BLUE);
        //free(input);
        free(win);
        free(congrats);
        return -1;
    }

    playersArray[currPlayer % PlayerCount]->prevSunk = (playersArray[currOpponent]->currSunkShips == oppSunkShips)?0:1;

    playersArray[currOpponent]->currSunkShips = oppSunkShips;
    

    currPlayer = (currPlayer + 1) % PlayerCount;
    
    //if (outputMsg != NULL) free(outputMsg); //In case any function returns an output (Debug or whatever)

    //free(input);

    return 1;
}

void RefreshScreen(){

    ClearScreen();

    SetBold();
    Println_Centered(TITLE, strlen(TITLE), BLUE);
    ResetFormat();
    Println("");
    Println_Centered(BYGROUPNAME, strlen(BYGROUPNAME), BRIGHT_CYAN);
    Println("");
    Println("");
    Println("");
}

void RunGame_PVP()
{
    alloc_InitializePlayerArray(2, &playersArray);

    for (int i = 0; i < PlayerCount; i++)
    {
        SetUpNewPlayer(i);
        RefreshScreen();
    }

    Println_Centered(ANNOUNCE_RANDOMPLAYER, strlen(ANNOUNCE_RANDOMPLAYER), WHITE);

    currPlayer = PickRandomPlayer(PlayerCount);

startturn:

    int turn = PlayTurn();

    if (turn > 0)
    {
        
        char * input;
        char * inpPtr = alloc_Input(REQUEST_ANYKEY, &input);

        free (input);

        RefreshScreen();
        goto startturn;
    }

    for (int i = 0; i < PlayerCount; i++)
    {
        free(playersArray[i]);
    }

    free(playersArray);
    return;
}

// Phase 2
void RunGame_PVE()
{
    alloc_InitializePlayerArray(2, &playersArray);

    //To make the game work for multiple players this initialization process should be changed:
        //Set Player:
        SetUpNewPlayer(0);
        RefreshScreen();
        
        //Set Bot:
        SetUpBot(1);

        RefreshScreen();


    Println_Centered(ANNOUNCE_RANDOMPLAYER, strlen(ANNOUNCE_RANDOMPLAYER), WHITE);

    currPlayer = PickRandomPlayer(PlayerCount);

startturn:

    int turn = PlayTurn();

    if (turn > 0)
    {
        
        char * input;
        char * inpPtr = alloc_Input(REQUEST_ANYKEY, &input);

        free (input);

        RefreshScreen();
        goto startturn;
    }

    for (int i = 0; i < PlayerCount; i++)
    {
        free(playersArray[i]);
    }

    free(playersArray);
    return;
}
//

/**
 * Basic welcome function that displays the static welcome text before the main menu.
 */
void Welcome()
{
    Println("");
    SetBold();
    Println_Centered(TITLE, strlen(TITLE), BLUE);
    ResetFormat();
    Println("");
    Println_Centered(BYGROUPNAME, strlen(BYGROUPNAME), BRIGHT_CYAN);
    Println("");
    Println("");
    Println("");
    Println_Centered(WELCOME_STATEMENT, strlen(WELCOME_STATEMENT), BRIGHT_CYAN);
    Println("");
    Println_Centered("This project was built in C for CMPS 270 at AUB in Fall 2024.", strlen("This project was built in C for CMPS 270 at AUB in Fall 2024."), BRIGHT_CYAN);
    
    Println("");
    Println_Centered("Developed by:", strlen("Developed by:"), BRIGHT_CYAN);
    Println_Centered("- Ayoub Nasr", strlen("- Ayoub Nasr"), BRIGHT_CYAN);
    Println_Centered("- Ali Haddam", strlen("- Ali Haddam"), BRIGHT_CYAN);
    Println_Centered("- Fadi Bayan", strlen("- Fadi Bayan"), BRIGHT_CYAN);
    Println_Centered("- Hussein Ghaddaf", strlen("- Ayoub Nasr"), BRIGHT_CYAN);
    Println("");
    Println_Centered("About Battleship:", strlen("About Battleship:"), BRIGHT_CYAN);
    Println_Centered("Battleship is a strategic guessing game where two players try to sink each other's fleet of ships.", strlen("Battleship is a strategic guessing game where two players try to sink each other's fleet of ships."), BRIGHT_CYAN);
    Println_Centered("Each player arranges ships on their grid and takes turns guessing the location of the opponent's ships.", strlen("Battleship is a strategic guessing game where two players try to sink each other's fleet of ships."), BRIGHT_CYAN);
    Println_Centered("The goal is to sink all of the opponent’s ships by correctly guessing their coordinates.", strlen("Battleship is a strategic guessing game where two players try to sink each other's fleet of ships."), BRIGHT_CYAN);
    Println("");

    
}

int main()
{
    srand(time(NULL)); //Seed the random generator once for the whole game

    ClearScreen();
    Welcome();

    InstructionSet = PRESTARTINSTRUC;
    char *input;
    char *inpPtr;

start:

    inpPtr = alloc_Input(REQUEST_STARTINPUT, &input);

    int op = PerformOperation(&inpPtr, NULL);

    if (op < 0)
    {
        free(input);
        goto start;
    }

    free(input);

    InstructionSet = PREGAMEINSTRUC;

setdifficulty:

    inpPtr = alloc_Input(REQUEST_DIFFICULTY, &input);

    char * nextInp = next(&inpPtr);

    CheckForQuit(nextInp);

    int difficulty = SetDifficulty(nextInp);

    if (difficulty < 0)
    {
        Println_Centered(INVALID_INPUT_WARNING, strlen(INVALID_INPUT_WARNING), RED);
        free(input);
        goto setdifficulty;
    }

    free(input);
    free(nextInp);

selectgamemode:

    inpPtr = alloc_Input(REQUEST_GAMEMODE, &input);

    nextInp = next(&inpPtr);

    CheckForQuit(nextInp);

    int modeIndex = SelectGameMode(nextInp);

    if (modeIndex < 0)
    {
        Println_Centered(INVALID_INPUT_WARNING, strlen(INVALID_INPUT_WARNING), RED);
        free(input);
        goto selectgamemode;
    }

    free(input);
    free(nextInp);

    InstructionSet = INGAMEINSTRUC;
    switch (modeIndex)
    {
    case PVP:
        RunGame_PVP();
        break;
    case PVE:

        //Println_Centered("Coming soon...", strlen("Coming soon..."), RED);
        //goto selectgamemode;

        RunGame_PVE();
        break;
    default:
        break;
    }
}
