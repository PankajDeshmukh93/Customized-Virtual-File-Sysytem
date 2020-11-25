/*
Customized Virtual File System
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define MAX_FILES 100
#define FILESIZE 1024

#define REGULAR 1
#define SPECIAl 2

#define READ 4
#define WRITE 2

struct Super_Block
{   
    int Total_Inodes;
    int Free_Inodes;

}Super_obj;

struct inode
{
    int Inode_number;
    char File_Name[30];
    int File_Size;
    int Actual_File_Size;
    int File_Type;
    int Link_Count;
    int Refrence_Count;
    //int permission;
    char * Data;
    struct inode * next;
};

typedef struct inode INODE;
typedef struct inode * PINODE;

PINODE Head = NULL;                //declaring Head varaible which is inode pointer as global

struct File_Table                   //Structre of File table
{
    int ReadOffset;
    int WriteOffset;
    int Count;
    PINODE iptr;
    int Mode;           //or permission
};

typedef struct File_Table FILETABLE;
typedef struct File_Table * PFILETABLE;                 //pointer of the file table structre


struct UFDT                     //Structre of UAREA
{
    PFILETABLE ufdt[MAX_FILES];

}UFDT_obj;


void CreateDILB()
{
    PINODE newn = NULL;
    PINODE temp = NULL;
    int i = 0;

    while( i < MAX_FILES )
    {
        newn = (PINODE)malloc(sizeof(INODE));                   //alocating memory to inode structre dynamically

        newn->Inode_number = 0;
        newn->File_Size = FILESIZE;
        newn->Actual_File_Size = 0;
        newn->File_Type = 0;
        newn->Link_Count = 0;
        newn->Refrence_Count = 0;
        //newn->permission = 0;
        newn->Data = NULL;
        newn->next = NULL;

        if(Head == NULL)
        {
            Head = newn;
            temp = Head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }

        i++;
    }
    printf("DILB is created Successfully \n");
}

void Create_SuperBlock()
{
    Super_obj.Total_Inodes = MAX_FILES;
    Super_obj.Free_Inodes = MAX_FILES;
    printf("Super Block Created Successfully \n ");
}

void CreateUFDT()
{
    int i = 0;
    for( i = 0; i < MAX_FILES; i++ )
    {

        UFDT_obj.ufdt[i] = NULL;

    }
    printf("UFDT created successfully\n");
}


bool ChkFile(char * name)
{
    PINODE temp = Head;

    while(temp != NULL)
    {
        if( temp->File_Type != 0)              
        {

            if( strcmp( temp->File_Name , name ) == 0)
            {
                break;
            }
        }
        temp = temp->next;

    }

    if(temp == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }

}

int CreateFile(char * name , int permission)
{
    bool bret = false;

    if( ( name == NULL ) || ( permission > READ+WRITE ) || ( permission < WRITE ) )   //filter 1
    {
        printf(" Invalid Permission or invalid file name enetrd \n");
        return -1;
    }

    
    bret = ChkFile(name);
    if(bret == true)                                                                //filter 2
    {
        printf("File with same name is already present \n");
        return -1;
    }

    if(Super_obj.Free_Inodes == 0)
    {
        printf("There are no inodes left to create a File \n");
        return -1;
    }

    int i = 0;

    for( i = 0; i < MAX_FILES ; i++)
    {
        if(UFDT_obj.ufdt[i] == NULL)
        {
            break;
        }
    }

    //Alocate memeory to file table sstructre

    UFDT_obj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
    printf("Memory for File Table structre is allocated succesfuly \n");
    //Intialize members of File Table
    UFDT_obj.ufdt[i]->ReadOffset = 0;
    UFDT_obj.ufdt[i]->WriteOffset = 0;
    UFDT_obj.ufdt[i]->Count = 1;
    UFDT_obj.ufdt[i]->Mode = permission; 

    //Alocate memory to the inode strcutre

    PINODE temp = Head;

    while(temp != NULL)
    {
        if(temp->File_Type == 0)
        {
            break;
        }
        temp = temp->next;
    }

    UFDT_obj.ufdt[i]->iptr = temp;
    printf("Memory for inode structre is allocated successfully \n");
    //intialize the members of inode structre
    strcpy( UFDT_obj.ufdt[i]->iptr->File_Name , name );
    UFDT_obj.ufdt[i]->iptr->Actual_File_Size = 0;
    UFDT_obj.ufdt[i]->iptr->File_Type = REGULAR;    //If it is 0 File is deleted
    UFDT_obj.ufdt[i]->iptr->Link_Count = 1;          //Link Count is just place holder in our project //practically it is associated with Hard Link ... if in project it was implemented we will have to write logic separately for LINK_COUNT().
    UFDT_obj.ufdt[i]->iptr->Refrence_Count = 1;
   // UFDT_obj.ufdt[i]->iptr->permission = permission;
    //Allocate memory for files data
    UFDT_obj.ufdt[i]->iptr->Data = (char *)malloc(sizeof(FILESIZE));

    //Since a New file is created so now inode present on the DILB will bew reduced since the newly created file will us it
    Super_obj.Free_Inodes--;
    return i;// i is returned to the function due which we get the file descritor value

}


void DeleteFile(char * name)                //delete a file from VFS
{
    if( name == NULL)                       //filter 1
    {
        printf("There is no such file present in VFS. nothing to delete \n");
        return;
    }

    bool bret = false;
    bret = ChkFile(name);

    if(bret == false)                      //filter 2
    {
        printf("After passing the name to ChkFile() functions its says : There is no such file present inn Virtual File System \n");
        printf("\n");
        return;
    }

    if(Super_obj.Free_Inodes == 0)
    {
        printf("There are no free inodes. So there is no file that can be deleted\n");
        return;
    }

    //Search UFDT entry
    int i = 0;
    for( i = 0 ; i < MAX_FILES ; i++ )
    {
        if( strcmp(UFDT_obj.ufdt[i]->iptr->File_Name , name ) == 0)
        {
            break;
        }
    }

    PINODE temp = Head;

    printf("The file : %s will be  deleted from Virtual File System \n" ,name);
    
    strcpy(UFDT_obj.ufdt[i]->iptr->File_Name , "");
    UFDT_obj.ufdt[i]->iptr->Refrence_Count = 0;
    UFDT_obj.ufdt[i]->iptr->Link_Count = 0;
    UFDT_obj.ufdt[i]->iptr->File_Type = 0; //If it is 0 File is deleted
    UFDT_obj.ufdt[i]->iptr->Actual_File_Size = 0;
    
    free(UFDT_obj.ufdt[i]->iptr->Data );            //free Data chi memory ji file sati allocate keli hoti ---> bcoze file is being deleted now
    free(UFDT_obj.ufdt[i]);                         //file table chi memory free kara

    UFDT_obj.ufdt[i] = NULL; 

    //Since the file is deleted inode associated with the file will be reinstated in the DILB
    Super_obj.Free_Inodes++;

}

void LS()                       //list the files in VFS
{
    PINODE temp = Head;
    

    // if(temp->File_Name == NULL)
    // {
    //     printf("There are no files in Virtual File system to display");
    //     return;
    // }  
    // else
    // {
        
    // }
    printf("The list of files in the Virtual File System are : \n");
    while(temp != NULL)
    {
        if(temp->File_Type != 0)
        {
            
            printf("%s \n" , temp->File_Name);
            //printf("The permission for the current listed file in VFS is : %d \n", temp->permission);
        }
        temp = temp->next;
    }

}



//Get file descriptor from file name
int GetFd_From_Name(char * name)
{
    int i = 0;

    for( i = 0; i < MAX_FILES; i++)
    {
        if(UFDT_obj.ufdt[i] != NULL)
        {

            if(strcmp(UFDT_obj.ufdt[i]->iptr->File_Name , name) == 0)
            {
                break;
            }

        }
    }

    if(i == 100)
    {
        return -1;
    }
    else
    {
        return i;
    }
}

PINODE Get_Inode(char * name)
{

    PINODE temp = Head;
    if(name == NULL)
    {
        printf("Error : Occured when user is trying to enter the name of the file \n");
        return NULL;
    }

    while (temp != NULL)                //travel entrie inode link list to see if file name entered by user is present in Inode Link List
    {
        if(strcmp(temp->File_Name , name) == 0)
        {
            break;                      //if the file name is found break from the while loop 
        }
        temp = temp->next;
    }
    return temp;                        //return the adress asscoited with file name

}

//file descriptor                       //function Overloading
//Data
//Size of data
int WriteFile1(char * name , char * buffer , int size) //program C++ aslyamule apan same name ch function declare karu shakto. This is Function Overloading
{
    //Search fd from ' name ' ch logic lihayla lagel
    int fd = GetFd_From_Name(name);
    if(fd == 100)
    {
        printf("Wrong File descriptor \n");
    }
    else
    {
        printf("File descriptor derived from the file name is : %d \n" ,fd);
    }

    strncpy(  ( UFDT_obj.ufdt[fd]->iptr->Data  + UFDT_obj.ufdt[fd]->WriteOffset ) , buffer , size);
    UFDT_obj.ufdt[fd]->WriteOffset = UFDT_obj.ufdt[fd]->WriteOffset + size;

    return size;
}

int WriteFile(int fd , char * buffer , int size)                        //function Overloading
{
    printf("Inside Writefile function \n");
    //printf("Data enterd in the file is : %s \n",buffer);
    printf("the file descriptor of the newly created file in which data is to be wriiten is : %d \n" , fd);
    if(UFDT_obj.ufdt[fd] == NULL)
    {
        printf("Invalid File descriptor \n");
        return -1;
    }

    if(UFDT_obj.ufdt[fd]->Mode == READ)         //filter 2
    {
        printf("There is no write permission \n");
        return -1;
    }
    //data gets copied in buffer
    //starting point shodla , alela paramter, kiti data lihaycha ahe
    //UFDT chya array madun file table la point kele ; titun inode madey gelo ani data la point kele; ani tya data madey offset add kel ; means ata data madey kutun lihayla start karayche ahe
    strncpy( ( ( UFDT_obj.ufdt[fd]->iptr->Data ) + ( UFDT_obj.ufdt[fd]->WriteOffset )), buffer ,size) ;
                                            //Destination                             , Source , Kiti data copy karaycha ahe
    //WriteOffset update kela ; WriteOffset chya pude lihla data
    UFDT_obj.ufdt[fd]->WriteOffset = UFDT_obj.ufdt[fd]->WriteOffset + size;

    return size;
}

void fstat_file(int fd)
{
    //from fd acces file table
    printf("From the fd : %d ; below information is retrived \n" , fd);
    printf("The file name  is : %s \n", UFDT_obj.ufdt[fd]->iptr->File_Name);
    printf("The file type is : %d \n" ,UFDT_obj.ufdt[fd]->iptr->File_Type );
    printf("Link Count of the file is  : %d \n " , UFDT_obj.ufdt[fd]->iptr->Link_Count);
    printf("The size of the file is : %d \n", UFDT_obj.ufdt[fd]->iptr->File_Size);
    printf("The data enterd in the file is : %s\n" , UFDT_obj.ufdt[fd]->iptr->Data);

}

void stat_file(char * name)
{
    PINODE temp = Get_Inode(name);
    // if( temp != NULL)
    // {
    //     printf("inode assscociated with the file name is : %ld" , temp);
    // }
    printf("Displaying the stats of file ; by retriven the inode associated with the file \n");
    printf("name of the file : %s \n " , temp->File_Name);
    printf("Size of the file is : %d \n " ,temp->File_Size);
    printf("Type of the file is : %d\n " ,temp->File_Type);
    printf("Link_Count of the file is : %d \n " , temp->Link_Count);
    printf("Refrnce count of the file is : %d \n " , temp->Refrence_Count);
    printf("Data enetred in the file is : %s \n " , temp->Data);

}

int OpenFile(char * name , int mode)
{

    PINODE temp = NULL;
    
    if((name == NULL) || ( mode > READ+WRITE ) || ( mode < WRITE ) )
    {
        printf("Inavlid mode or invalid file name entered by user \n");
        return -1;
    }
    

    temp = Head;
    while(temp != NULL)
    {
        if(strcmp(temp->File_Name , name) == 0)
        {
            break;
        }

        temp = temp ->next;
    }

    if(temp == NULL)
    {
        printf("File Name enetered is not present in inode Link list \n");
        return -2;
    }

    //check if the file is present in inode link list
    // temp = Get_Inode(name);
    // if(temp != NULL)
    // {
    //     printf("Inode asscoited with the file name is found \n");
    // }

    int i = 0;
    for( i = 0; i < MAX_FILES; i++)
    {
        if(UFDT_obj.ufdt[i] == NULL)
        {
            break;
        }
    }

    //allocate memory to file table
    UFDT_obj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
    //intialize members
    UFDT_obj.ufdt[i]->Count = 1;
    UFDT_obj.ufdt[i]->Mode = mode;

    if(mode == READ + WRITE)
    {
        UFDT_obj.ufdt[i]->ReadOffset = 0;
        UFDT_obj.ufdt[i]->WriteOffset = 0;
    }
    if(mode == READ)
    {
        UFDT_obj.ufdt[i]->ReadOffset = 0;
    }
    else if( mode == WRITE)
    {
        UFDT_obj.ufdt[i]->WriteOffset = 0;
    }
    UFDT_obj.ufdt[i]->iptr = temp;
    (UFDT_obj.ufdt[i]->iptr->Refrence_Count)++;

    printf("The refrnce count of the current opened file is : %d " , temp->Refrence_Count);

    printf("\n");

    return i;
}

int ReadFile(int fd , char * arr ,int isize)
{
    int read_size = 0;
    if(UFDT_obj.ufdt[fd] == NULL)
    {
        return -1;
    }
    if( (UFDT_obj.ufdt[fd]->Mode > READ+WRITE) && (UFDT_obj.ufdt[fd]->Mode < WRITE) )
    {
        return -2;
    }
    // if( ( UFDT_obj.ufdt[fd]->ReadOffset ) == ( UFDT_obj.ufdt[fd]->iptr->Actual_File_Size )  )
    // {
    //     return -3;
    // }
    //filter if file is not regular
    //
    // read_size = (UFDT_obj.ufdt[fd]->iptr->Actual_File_Size) - (UFDT_obj.ufdt[fd]->ReadOffset);
    // if(read_size < isize)
    // {
    //     strncpy(arr , ( ( UFDT_obj.ufdt[fd]->iptr->Data ) + (UFDT_obj.ufdt[fd]->ReadOffset) ) , read_size);
    //        UFDT_obj.ufdt[fd]->ReadOffset = UFDT_obj.ufdt[fd]->ReadOffset + read_size;
    // }
    // else
    //{
        strncpy(arr , ( (UFDT_obj.ufdt[fd]->iptr->Data) + (UFDT_obj.ufdt[fd]->ReadOffset) ) , isize);
        UFDT_obj.ufdt[fd]->ReadOffset = UFDT_obj.ufdt[fd]->ReadOffset + isize;
    //}
    return isize;
}

void CloseFileByName(int fd)
{
    //Remove the details of the file
    //Refrnce count of the file will be reduced by 1
    UFDT_obj.ufdt[fd]->ReadOffset = 0;
    UFDT_obj.ufdt[fd]->WriteOffset = 0;
    (UFDT_obj.ufdt[fd]->iptr->Refrence_Count)--;

    printf("Current Updated Refrnce count of the file that is closed by fd  is : %d \n" , UFDT_obj.ufdt[fd]->iptr->Refrence_Count);
}

void CloseFileByName(char * name)
{
    //call function GetFdBYName
    int fd = GetFd_From_Name(name);
    if(fd >= 0)
    {
        printf("The file that user wants to close can be closed now. \n Since We have the file descriptor : %d which is associated with the file name " ,fd);\
        printf("\n");
    }
    else
    {
        printf("Invalid File name. File descriptor not retrived \n");
    }

    UFDT_obj.ufdt[fd]->ReadOffset = 0;
    UFDT_obj.ufdt[fd]->WriteOffset = 0;
    (UFDT_obj.ufdt[fd]->iptr->Refrence_Count)--;
    printf("Current Updated Refrnce count of the file that is closed by name is : %d \n" , UFDT_obj.ufdt[fd]->iptr->Refrence_Count);
    printf("\n");
}

void CloseAllFile()
{
    int i = 0;
    for( i = 0; i < MAX_FILES ; i++)
    {
        if(UFDT_obj.ufdt[i] != NULL)
        {
            UFDT_obj.ufdt[i]->ReadOffset = 0;
            UFDT_obj.ufdt[i]->WriteOffset = 0;
            (UFDT_obj.ufdt[i]->iptr->Refrence_Count)--;
            break;
        }

    }
    printf("All files are closed successfully \n");
    printf("\n");
}

void SetEnvironment()
{
    CreateDILB();
    CreateUFDT();
    Create_SuperBlock();

    printf("Virtual File System  Environmet is set successfully .... \n");
    printf("\n");
}



void DisplayHelp(char * str)
{
    printf("open : It is used to open an existing file \n");
    printf("read : It is used to read contnts/ data of an existing opened file \n");
    printf("write : It is used to write data in an existing opened file \n");
    printf("close : It is used to close an opened file \n");
    printf("ls : It is used to list files in the currrent directory \n");
    printf("rm : It is used to remove / delete a file from Virtual file system \n");
    printf("fstat : Display information asscoiated with file \n");
    printf("stat : Display all the information associted with file \n");

    printf("\n");  
    
}

void Manpage(char * str)
{
    if(strcmp(str , "open") == 0)
    {
        printf("Definition : open - It is used to open an existing file\n");
        printf("usage : open File_Name mode\n");
        
    }
    else if(strcmp(str, "read") == 0)
    {
        printf("Description : It is used to read contnts/ data of an existing opened file \n");
        printf("usage : read File_Name  No_Of_Bytes_to_read \n");
    }
    else if(strcmp (str , "write") == 0)
    {
        printf("Description : It is used to write data in an existing opened file \n");
        printf("usage : write File_Desscriptor  \n");
        printf("After the command please enter the data  \n"); //command enter kelyavr user kadun data ghyaycha ahe
    }
    else if(strcmp(str , "creat") == 0)
    {
        printf("Description : It is used to create a new file \n");
        printf("usage : creat File_Name permission \n");
    }
    else if(strcmp(str , "close") == 0)
    {
        printf("Description : It is used to close an opened file \n");
        printf("usage : close File_Decriptor \n");
    }
    else if( strcmp(str , "rm") == 0)
    {
        printf("Description : It is used to remove a file from virtual File system \n");
        printf("usage : rm File_Name\n ");
    }
    else if(strcmp( str , "closeall" ) == 0)
    {
        printf("Descriptionn : It will close all the files that are previously opened in the VFS \n");
        printf("usage : closeall \n");
    }
    else if(strcmp (str , "ls") == 0)
    {
        printf("Description : It is used to list files in the currrent directory \n");
        printf("usage : ls \n");
    }
    else if(strcmp(str,"fstat") == 0)
    {
        printf("Description : Display information asscoiated with file \n");
        printf("usage : fstat File_Descritor \n");
    }
    else if( strcmp(str , "stat") == 0)
    {
        printf("Description :  Display information asscoiated with file \n");
        printf("usage : stat File_Name \n");
    }
}




int main()
{
    char str[80];
    char command[4][80];
    int count = 0;
    printf("Customized Virtual File System \n");

    SetEnvironment();

    while(1)
    {
        printf("Marvellous vfs>: ");
        fgets(str, 80, stdin);  
        fflush(stdin);  //flushing the enter in stdin due which loop goes in infinte state

        printf("\n");  

        count = sscanf(str , " %s %s %s %s " , command[0], command[1], command[2], command[3]);    //string tokenization

        if(count == 1)
        {
            if(strcmp(command[0] , "help") == 0)
            {
                //call function 
                DisplayHelp(command[0]);
            }
            else if(strcmp(command[0] , "exit") == 0)
            {
                printf("Thank you for using Marvellous Virtual File System \n");  
                printf("\n");                       
                break;
            }
            else if(strcmp(command[0] , "clear") == 0)
            {
                system("clear");                //inbulit function use
                printf("\n");                       
            }
            else if( strcmp( command[0] , "ls") == 0)
            {
                LS();                                   //function call
            }
            else if(strcmp(command[0] , "closeall") == 0)
            {
                CloseAllFile();

            }

            else
            {
                printf("Command not found \n");
            }
        }
        else if(count == 2)
        {
            if(strcmp(command[0] , "man") == 0)
            {

                Manpage(command[1]);

            }
            else if( strcmp(command[0] , "rm") == 0)
            {

                DeleteFile(command[1]);

            }
            else if(strcmp(command[0] , "write1") == 0)                            
            {
                char arr[1024];
                printf("Enter the data to be written in the file : \n");
                fgets(arr , 1024 ,stdin);
                fflush(stdin);
                //Deriving file descriptor by the file name which is created already in VFS                  
               int ret =  WriteFile1( command[1] , arr , strlen(arr)-1 );                    //function Overloading
               if(ret != -1)
               {
                   printf(" %d Bytes gets written in the file \n" ,ret);
               }

            }
            else if( strcmp(command[0], "write") == 0)
            {
                char buffer[1024];
                printf("Enter the data to be written in the file : ");
                fgets(buffer , 1024 , stdin);
                fflush(stdin);

                int ret = WriteFile(atoi(command[1]) , buffer , strlen(buffer)-1);          //function Overloading

                if(ret != -1)
                {
                    printf(" %d bytes gets written successfully in the file \n" , ret);

                }
            }
            else if(strcmp(command[0] , "fstat") == 0)
            {
                fstat_file(atoi(command[1]));

            }
            else if(strcmp(command[0] , "stat") == 0)
            {
                stat_file(command[1]);
            }
            else if(strcmp(command[0] , "close") == 0)
            {
                //CloseFileByFD(atoi(command[1]));
                //if( CloseFileByName(command[1] || CloseFileByFD(atoi(command[1]) ) );
                CloseFileByName(command[1]);

            }
            else
            {

                printf("Command not found \n");
                printf("\n");

            }

        }
        else if( count == 3 )
        {

            if(strcmp(command[0] , "creat") == 0)
            {
                int fd = 0;
                fd = CreateFile( command[1] , atoi(command[2]) );
                
                if(fd == -1)
                {

                    printf("Error : Unable to creat a new file \n");
                    printf("\n");

                }
                else
                {

                    printf("File Created successfully with File Descriptor : %d \n" , fd);
                    printf("\n");

                }
            }
            else if(strcmp(command[0] , "open") == 0)
            {
                int ret = 0;
                ret = OpenFile(command[1] , atoi(command[2]));
                if(ret >= 0)
                {
                    printf("File is successfully opened with file descriptor : %d \n" , ret);
                }
               
            }
            else if(strcmp(command[0] , "read") == 0)
            {
                int fd = 0, ret = 0;
                char * ptr = NULL;

                fd = GetFd_From_Name(command[1]);
                if (fd == -1)
                {
                    printf("Error : Invalid parameter \n");
                    
                }
                
                ptr = (char *)malloc(sizeof( atoi( command[2] ) ) + 1);
                if(ptr == NULL)
                {
                    printf("Error : Memory allocation failed \n");
                   
                }
                ret = ReadFile(fd , ptr , atoi(command[3]) );
                printf("The no. of bytes read from the file is : %d\n " ,ret);
                //filters and ture condtion comes what to do ???
            }
            else
            {

                printf("Command Not Found \n");

            }

        }
        else
        {
            printf("Bad command or file name entered is wrong \n");
        }
        

    }
    return 0;
}