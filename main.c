#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define INPUT_COUNT 3
#define BLKSIZE 1024

typedef struct inlineStr{
    char *str;
    int strLen;
}InlineStr;

typedef struct command{
    struct inlineStr oldStr;
    struct inlineStr newStr;

    int insensitive;
    int multipleCharSupport;
    int onlyLineStart;
    int onlyLineEnd;
    int repetitionCharSupport;
}Command;

typedef struct commandHolder{
    struct command *commands;
    int commandCount;
    int cap;
}CommandHolder;

void checkInputValidity(int argc);
CommandHolder parseArgument(const char *argument);
void translateFile(CommandHolder commands, const char* readBuffer, char* writeBuffer);


int main(int argc, char const *argv[])
{
    checkInputValidity(argc);
    CommandHolder cHolder = parseArgument(argv[1]);
    printf("40\n");
    char testInput[256] = "testtzzzzytr1str2";
    char testOutput[256] = {'\0'};
    translateFile(cHolder, testInput, testOutput);
    printf("%s\n", testOutput);

    return 0;
}


void checkInputValidity(int argc){
    if(argc != INPUT_COUNT){
        fprintf(stderr, "Usage: ./hmw1 ‘/str1/str2/‘ inputFilePath");
        exit(-1);
    }
}

Command parseCommand(const char *argument, int *argumentIndex){
    int slashCount = 0;
    int strLen;
    Command command;

    while(*argumentIndex < strlen(argument) && argument[*argumentIndex] != ';'){
        strLen = 0;
        if(argument[*argumentIndex] == '/')
            ++slashCount;
        ++(*argumentIndex);

        if(slashCount == 1)
            command.oldStr.str = &argument[*argumentIndex];
        else if(slashCount == 2)
            command.newStr.str = &argument[*argumentIndex];
        else{
            break;
        }
        printf("%c %d\n", command.oldStr.str[0], slashCount);
        while(*argumentIndex < strlen(argument) && argument[*argumentIndex] != '/'){
            ++strLen;
            if(argument[*argumentIndex] == '[')
                command.multipleCharSupport = 1;
            else if(argument[*argumentIndex] == '^')
                command.onlyLineStart = 1;
            else if(argument[*argumentIndex] == '$')
                command.onlyLineEnd = 1;
            else if(argument[*argumentIndex] == '*'){
                command.repetitionCharSupport = 1;
            }
            ++(*argumentIndex);
        }
        
        if(slashCount == 1)
            command.oldStr.strLen = strLen;
        else if(slashCount == 2)
            command.newStr.strLen = strLen;
        
    }
printf("%d\n", *argumentIndex);
    return command;
}

CommandHolder parseArgument(const char *argument){
    int isCorrectSyntax = 1;
    const int argumentLen = strlen(argument);
    CommandHolder commandHolder;
    commandHolder.commandCount = 0;
    commandHolder.commands = (Command*)calloc(8, sizeof(Command));
    /*error check*/
    commandHolder.cap = 8;
    printf("%s\n", argument);
    
    
    for (int i = 0; i < argumentLen && isCorrectSyntax; ++i){
        if(argument[i] == '/' && (i == 0 || argument[i - 1] == ';')){
            if(commandHolder.commandCount >= commandHolder.cap){
                printf("%s\n", argument);
                commandHolder.commands = (Command*)realloc(commandHolder.commands, commandHolder.cap * 2 * sizeof(Command));
                
                commandHolder.cap *= 2;
            }
            
            commandHolder.commands[commandHolder.commandCount++] = parseCommand(argument, &i);
            printf("118\n");
            if(i < argumentLen - 1 && argument[i] == 'i')
                commandHolder.commands[commandHolder.commandCount - 1].insensitive = 1;
        }
        else{
            isCorrectSyntax = 0;
        }
    }

    if(!isCorrectSyntax){
        fprintf(stderr, "Usage: ./hmw1 ‘/str1/str2/‘ inputFilePath");
        exit(-1);
    }
    return commandHolder;
}


int isCorrectCharacter(char c){
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

int isAnyCharacterMatch(Command* command, int *commandIndex, char c){
    int returnVal = 0;
    if(command->oldStr.str[*commandIndex] == '['){
        while(*commandIndex < command->oldStr.strLen && command->oldStr.str[*commandIndex] != ']'){
            printf("146");
            if(command->oldStr.str[*commandIndex] == c)
                returnVal = 1;
            ++(*commandIndex);
        }
    }

    if(*commandIndex >= command->oldStr.strLen || command->oldStr.str[*commandIndex] != ']')
        returnVal = -1;
    return returnVal;
}

int isNextCharMatch(const char *buffer, char c, int ind, int bufferLen){
    return (ind + 1 < bufferLen && buffer[ind + 1] == c);
}


int isCommandMatch(Command* command, const char* readBuffer, int* readBufferIndex, int firstBuffer){
    int i;
    int iBak;
    int commandIndex = 0;
    int isMatch = 1;
    char oldChar;
    for(i = *readBufferIndex; i < strlen(readBuffer) && isMatch == 1; ++i){
       
        if(commandIndex >= command->oldStr.strLen)
            break;
        
        if(command->oldStr.str[commandIndex] == '['){
            isMatch = isAnyCharacterMatch(command, &commandIndex, readBuffer[*readBufferIndex]);
        }
        else if(command->oldStr.str[commandIndex] == '*'){
            
            if(commandIndex != 0 && isCorrectCharacter(command->oldStr.str[commandIndex - 1])){
                
                while(i < strlen(readBuffer) && readBuffer[i] == command->oldStr.str[commandIndex - 1]){
                    i++;
                }
                i--;
            }
            else
                isMatch = -1;
        }
        else if(isNextCharMatch(command->oldStr.str, '*', commandIndex, command->oldStr.strLen)){
            --i;
        }
        else if(isCorrectCharacter(command->oldStr.str[commandIndex])){
            

            if(command->oldStr.str[commandIndex] != readBuffer[i])
                isMatch = 0;
        }
        else if(command->oldStr.str[commandIndex] == '^'){
            if(firstBuffer && i == 0){
                --i;
            }
            else if(i != 0 && readBuffer[i-1] == '\n'){
                --i;
            }
            else{
                isMatch = 0;
            }
        }
        else if(command->oldStr.str[commandIndex] == '^' || command->oldStr.str[commandIndex] == '$'){
            --i;
        }
        else{
            isMatch = -1;
        }
        printf("%c %c %d %d\n", command->oldStr.str[commandIndex], readBuffer[i], i, isMatch);
        ++commandIndex;
    }

    if(isMatch == 1 && commandIndex == command->oldStr.strLen)
        *readBufferIndex = i;
    if(commandIndex != command->oldStr.strLen)
        isMatch = 0;

    return isMatch;
}

void translateFile(CommandHolder commands, const char* readBuffer, char* writeBuffer){
    int readBufferInd = 0;
    int writeBufferInd = 0;
    int commandMatchVal;
    
    while(readBufferInd < strlen(readBuffer)){
        
        for(int i = 0; i < commands.commandCount; ++i){
            
            commandMatchVal = isCommandMatch(&(commands.commands[i]), readBuffer, &readBufferInd, 1);
            printf("%d 215", readBufferInd);
            if(commandMatchVal == 1){
                for(int j = 0; j < commands.commands[i].newStr.strLen; ++j)
                    writeBuffer[writeBufferInd++] = commands.commands[i].newStr.str[j];
                break;
            }
            else if(commandMatchVal == -1){
                //error handling
                exit(-1);
            }
        }
        if(commandMatchVal == 0){
            writeBuffer[writeBufferInd++] = readBuffer[readBufferInd++];
        }
    }

}

void changeOccurences(int fd, CommandHolder commands){
    int fileFinish = 0;
    int bytesRead;
    char readBuffer[BLKSIZE];
    char *writeBuffer = (char*)calloc(2*BLKSIZE,sizeof(char));
    int writeBufferCap = 2*BLKSIZE;


    int tmpFd = mkstemp("newfile-XXXXXX");
    if(tmpFd < 0){
        //error handling
    }

    while(!fileFinish){
        while((bytesRead = read(fd, readBuffer, BLKSIZE) == -1) && (errno == EINTR));
        if(bytesRead <= 0)
            fileFinish = 1;
        if(!fileFinish){

        }
    }
}