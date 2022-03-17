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
    free(cHolder.commands);
    return 0;
}


void checkInputValidity(int argc){
    if(argc != INPUT_COUNT){
        printf("hata\n");
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
        printf("%c argument\n", argument[i]);
        if(argument[i] == '/' && (i == 0 || argument[i - 1] == ';')){
            if(commandHolder.commandCount >= commandHolder.cap){
                printf("%s\n", argument);
                commandHolder.commands = (Command*)realloc(commandHolder.commands, commandHolder.cap * 2 * sizeof(Command));
                
                commandHolder.cap *= 2;
            }
            
            commandHolder.commands[commandHolder.commandCount++] = parseCommand(argument, &i);
            printf("%c argument2\n", argument[i]);
            if(i <= argumentLen - 1 && argument[i] == 'i'){
                printf("%c argument3\n", argument[i]);
                commandHolder.commands[commandHolder.commandCount - 1].insensitive = 1;
                if(i < argumentLen - 1 && argument[i+1] == ';'){
                    if(i+1 < argumentLen)
                        i+=1;
                    else
                        fprintf(stderr, "Invalid syntax for argument.\n");
                }
            }
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
    if(command->oldStr.str[*commandIndex] == ']' && command->oldStr.str[*commandIndex + 1] == '*'){
        returnVal = 1;
    }
    if(*commandIndex >= command->oldStr.strLen || command->oldStr.str[*commandIndex] != ']')
        returnVal = -1;
    return returnVal;
}

int isCharMatch(char c1, char c2, int insensitive){
    int gap = 'a' - 'A';
    return (c1==c2 || (insensitive && (c1 == c2 + gap || c1 == c2 - gap)));
}

int isAnyCharacterMatchReverse(Command* command, int commandIndex, char c){
    int returnVal = 0;
    if(command->oldStr.str[commandIndex] == ']'){
        while(commandIndex < command->oldStr.strLen && command->oldStr.str[commandIndex] != '['){
            printf("%c\n", command->oldStr.str[commandIndex]);
            if(isCharMatch(command->oldStr.str[commandIndex], c, command->insensitive))
                return 1;
            --(commandIndex);
        }
    }

    if(commandIndex <= 0 || command->oldStr.str[commandIndex] != '[')
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
       printf("%c %c\n", command->oldStr.str[commandIndex ], readBuffer[i]);
        if(commandIndex >= command->oldStr.strLen)
            break;
        
        if(command->oldStr.str[commandIndex] == '['){
            isMatch = isAnyCharacterMatch(command, &commandIndex, readBuffer[i]);
            printf("readbuf %c command %c ismathc %d\n", readBuffer[*readBufferIndex], command->oldStr.str[commandIndex], isMatch);
        }
        else if(command->oldStr.str[commandIndex] == '*'){
            printf("%c\n", readBuffer[i]);
            if(commandIndex <= 0)
                isMatch = -1;
            
            if(isCorrectCharacter(command->oldStr.str[commandIndex - 1])){
                
                while(i < strlen(readBuffer) && isCharMatch(readBuffer[i], command->oldStr.str[commandIndex - 1], command->insensitive)){
                    i++;
                }
                i--;
            }
            else if(command->oldStr.str[commandIndex - 1] == ']'){
                int f = 0;
                printf("214\n");
                while(i < strlen(readBuffer) && isAnyCharacterMatchReverse(command, commandIndex - 1, readBuffer[i])){
                    f++;
                    i++;
                }
                i--;
                if(f == 0)
                    i--;
                printf("test %c  %c\n", readBuffer[i], command->oldStr.str[commandIndex]);
            }
        }
        else if(isNextCharMatch(command->oldStr.str, '*', commandIndex, command->oldStr.strLen)){
            --i;
        }
        else if(isCorrectCharacter(command->oldStr.str[commandIndex])){
            
            
            if(!isCharMatch(command->oldStr.str[commandIndex], readBuffer[i], command->insensitive))
                isMatch = 0;
        }
        else if(command->oldStr.str[commandIndex] == '^'){
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
    int fileStart = 1;
    char tmpFileName[] = "newfile-XXXXXX";
    int tmpFd = mkstemp(tmpFileName);
    if(tmpFd <= 0){
        perror("mkstemp: ");
        exit(-1);
    }
    while(!fileFinish){
        while((bytesRead = read(fd, readBuffer, BLKSIZE) == -1) && (errno == EINTR));
        if(bytesRead < 0)
            fileFinish = 1;
        if(!fileFinish){
            writeBufferSize = translateFile(commands, readBuffer, writeBuffer, fileStart, bytesRead == 0);
            write(tmpFd, writeBuffer, writeBufferSize);
            printf("%s\n", writeBuffer);
        }
        if(bytesRead == 0)
            fileFinish = 1;
        fileStart = 0;
    }
    rename(tmpFileName, filename);
}