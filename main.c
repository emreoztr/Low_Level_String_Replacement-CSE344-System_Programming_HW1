#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>


#define INPUT_COUNT 3
#define BLKSIZE 1024

typedef struct inlineStr{
    const char *str;
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
void changeOccurences(int fd, const char* filename, CommandHolder commands);


int main(int argc, char const *argv[])
{
    checkInputValidity(argc);
    CommandHolder cHolder = parseArgument(argv[1]);
    printf("40\n");
    int fd = open(argv[2], O_RDWR);
    printf("%d\n", fd);
    changeOccurences(fd, argv[2], cHolder);
    char testInput[256] = "testtzzzzytr1str2";
    char testOutput[256] = {'\0'};
    //translateFile(cHolder, testInput, testOutput);
    //printf("%s\n", testOutput);
    free(cHolder.commands);
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
    command.onlyLineStart = 0;
    command.onlyLineEnd = 0;

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


int isCommandMatch(Command* command, const char* readBuffer, int* readBufferIndex, int firstBuffer, int endBuffer){
    int i;
    int iBak;
    int commandIndex = 0;
    int isMatch = 1;
    int isStartMatch = 1;
    int isEndMatch = 1;
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
            printf("test %d\n", firstBuffer);
            if(firstBuffer && i == 0 || (i != 0 && readBuffer[i-1] == '\n')){
                
            }
            else{
                isStartMatch = 0;
            }
            --i;
        }
        else if(command->oldStr.str[commandIndex] == '$'){
            if(!((endBuffer && i == strlen(readBuffer) - 1) || (i != strlen(readBuffer) - 1 && readBuffer[i] == '\n')))
                isEndMatch = 0;
            
            --i;
        }
        else{
            isMatch = -1;
        }
        ++commandIndex;
    }
    if((command->onlyLineStart && !isStartMatch) && (command->onlyLineEnd && !isEndMatch))
        isMatch = 0;
    if(command->onlyLineStart && !command->onlyLineEnd && !isStartMatch)
        isMatch = 0;
    if(command->onlyLineEnd && !command->onlyLineStart && !isEndMatch)
        isMatch = 0;

    if(isMatch == 1 && command->oldStr.str[commandIndex] == '$')
        ++commandIndex;
    if(isMatch == 1 && commandIndex == command->oldStr.strLen)
        *readBufferIndex = i;
    if(commandIndex != command->oldStr.strLen)
        isMatch = 0;

    return isMatch;
}

int translateFile(CommandHolder commands, const char* readBuffer, char* writeBuffer, int firstBuffer, int endBuffer){
    int readBufferInd = 0;
    int writeBufferInd = 0;
    int commandMatchVal;
    
    while(readBufferInd < strlen(readBuffer)){
        for(int i = 0; i < commands.commandCount; ++i){
            commandMatchVal = isCommandMatch(&(commands.commands[i]), readBuffer, &readBufferInd, firstBuffer, endBuffer);
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
    return writeBufferInd;
}

void changeOccurences(int fd, const char* filename, CommandHolder commands){
    int fileFinish = 0;
    int bytesRead = -1;
    char readBuffer[BLKSIZE] = {'\0'};
    char *writeBuffer = (char*)calloc(2*BLKSIZE,sizeof(char));
    int writeBufferSize;
    int writeBufferCap = 2*BLKSIZE;

    char tmpFileName[] = "newfile-XXXXXX";
    int tmpFd = mkstemp(tmpFileName);
    if(tmpFd < 0){
        //error handling
    }
    while(!fileFinish){
        while((bytesRead = read(fd, readBuffer, BLKSIZE) == -1) && (errno == EINTR));
        if(bytesRead < 0)
            fileFinish = 1;
        if(!fileFinish){
            writeBufferSize = translateFile(commands, readBuffer, writeBuffer, 1, bytesRead == 0);
            write(tmpFd, writeBuffer, writeBufferSize);
            printf("%s\n", writeBuffer);
        }
        if(bytesRead == 0)
            fileFinish = 1;
    }
    rename(tmpFileName, filename);
}