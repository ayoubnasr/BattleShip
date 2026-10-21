/**
 * Tests for the bot (Bot.c): its stack memory of tasks ("the brain"), firing, and full games played by the bots.
 */
#include <stdlib.h>
#include "test.h"
#include "fixtures.h"
#include "../include/Bot.h"

static int taskCalls = 0;
static int lastTaskArgument = 0;

static int RecordingTask(void **args){
    taskCalls++;
    lastTaskArgument = *(int *)args[0];
    return 1;
}

static void **OneIntArgument(int value){
    void **args = malloc(sizeof(void *));
    int *v = malloc(sizeof(int));
    *v = value;
    args[0] = v;
    return args;
}

void test_bot_task_stack_order(void){
    Player *bot = NewTestPlayer("bot", true, SMART);
    CHECK(bot->stackMemory != NULL);
    CHECK(is_empty(bot->stackMemory));

    //High priority goes on top of the stack, low priority at the bottom:
    AssignNewTask(TASKFLAG_LOWPRIORITY, bot, RecordingTask, OneIntArgument(1), 1, NULL, 0);
    AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, RecordingTask, OneIntArgument(2), 1, NULL, 0);
    AssignNewTask(TASKFLAG_HIGHPRIORITY, bot, RecordingTask, OneIntArgument(3), 1, NULL, 0);
    CHECK_EQ_INT(AssignNewTask(42, bot, RecordingTask, NULL, 0, NULL, 0), 0); //unknown priority flag
    CHECK_EQ_INT(bot->stackMemory->size, 3);

    int expectedOrder[3] = {3, 2, 1};
    taskCalls = 0;
    for (int i = 0; i < 3; i++){
        BotTask *task = GetNextTask(bot);
        CHECK(task != NULL);
        if (task == NULL) break;
        CHECK_EQ_INT(PerformTask(task), 1);
        CHECK_EQ_INT(lastTaskArgument, expectedOrder[i]);
        CHECK_EQ_INT(freeTask(task), 1);
    }
    CHECK_EQ_INT(taskCalls, 3);
    CHECK(GetNextTask(bot) == NULL);
}

void test_bot_task_creation(void){
    void **args = OneIntArgument(7);
    BotTask *task = CreateTask(RecordingTask, args, 1, NULL, 0);
    CHECK(task != NULL);
    if (task == NULL) return;
    CHECK(task->function == RecordingTask);
    CHECK(task->arguments == args);
    CHECK_EQ_INT(task->argumentCount, 1);
    freeTask(task);

    BotTask *empty = CreateTask(NULL, NULL, 0, NULL, 0);
    CHECK_EQ_INT(PerformTask(empty), 0); //a task with no function does nothing
    freeTask(empty);
}
