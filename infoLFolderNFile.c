

#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <winternl.h>
#include <ntstatus.h>
#include <winioctl.h>

/* MinGW defines the size macro but not the structure */
#ifndef _MY_REPARSE_DATA_BUFFER_DEFINED
#define _MY_REPARSE_DATA_BUFFER_DEFINED
typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;

        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;

        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#endif

void printfFileAttributes(DWORD attrs){
    printf(" [");
    if(attrs == FILE_ATTRIBUTE_NORMAL) printf("N");
    else{
        printf("%c", (attrs & FILE_ATTRIBUTE_DIRECTORY) ? 'D' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_READONLY) ? 'R' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_HIDDEN) ? 'H' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_SYSTEM) ? 'S' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_ARCHIVE) ? 'A' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_COMPRESSED) ? 'C' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_ENCRYPTED) ? 'E' : '-');
        printf("%c", (attrs & FILE_ATTRIBUTE_REPARSE_POINT) ? 'L' : '-');
    }
    printf("] ");
}

void covertToSystemTime(FILETIME fileTime, const char *s){
    SYSTEMTIME st;
    FILETIME localFt;
    if(FileTimeToLocalFileTime(&fileTime, &localFt)){
        if(FileTimeToSystemTime(&localFt, &st)){
            printf(" | %s: %04u-%02u-%02u %02u:%02u ",
                   s,
                   st.wYear,
                   st.wMonth,
                   st.wDay,
                   st.wHour,
                   st.wMinute
                   );
        }
    }    
}

//Return -1 on failure
int printHardLinks(const wchar_t *filePath){
    HANDLE hFind;
    int countHardLink = 0;
    WCHAR fileNameBuffer[MAX_PATH + 1] = {0};
    DWORD nameLength = MAX_PATH + 1;
    
    hFind = FindFirstFileNameW(filePath, 0, &nameLength, fileNameBuffer);
    
    if(hFind == INVALID_HANDLE_VALUE){
        wprintf(L"FindFirstFileNameW failed: %lu\n", GetLastError());
        return -1;
    }
    
    countHardLink++;
   
    while(1){
        nameLength = MAX_PATH + 1; // MUST reset
        if(!FindNextFileNameW(hFind, &nameLength, fileNameBuffer)){
            DWORD err = GetLastError();
            if(err != ERROR_HANDLE_EOF){
                printf("FindNextFileNameW error: %lu\n", err);
                FindClose(hFind);
                return -1;
            }
            break;
        }
        countHardLink++;
    }
 
    FindClose(hFind);
    return countHardLink;
}

/*Return 0 on failure
  Detailed, Costly, FileSystem-specific
  Where does this actually go?
  How do I reconstructure the target?
*/
ULONG FindSpecialExtraInfo(HANDLE hFile, const wchar_t *filePath){
    BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    DWORD bytesReturned;

    if(!DeviceIoControl(hFile,
                        FSCTL_GET_REPARSE_POINT,
                        NULL, 0, buffer,
                        sizeof(buffer),
                        &bytesReturned, NULL))
        {
            return 0;
        }

    REPARSE_DATA_BUFFER *rdb = (REPARSE_DATA_BUFFER *)buffer;

    if(rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK){

        BOOLEAN isRelative =
            (rdb->SymbolicLinkReparseBuffer.Flags & SYMLINK_FLAG_RELATIVE) != 0;

        if (isRelative) {
            printf(" (relative)\n");

            printf("Flags = 0x%lx\n",
                   rdb->SymbolicLinkReparseBuffer.Flags);

            WCHAR *printName = (WCHAR *)(
                                         (BYTE *)rdb
                                         + FIELD_OFFSET(REPARSE_DATA_BUFFER,
                                                        SymbolicLinkReparseBuffer.PathBuffer)
                                         + rdb->SymbolicLinkReparseBuffer.PrintNameOffset
                                         );

            WCHAR *substName = (WCHAR *)(
                                         (BYTE *)rdb
                                         + FIELD_OFFSET(REPARSE_DATA_BUFFER,
                                                        SymbolicLinkReparseBuffer.PathBuffer)
                                         + rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset
                                         );

            wprintf(L" PrintName: %.*s | ", rdb->SymbolicLinkReparseBuffer.PrintNameLength / 2,
                    printName);

            wprintf(L" SubstituteName: %.*s | \n", rdb->SymbolicLinkReparseBuffer.SubstituteNameLength / 2,
                    substName);

            if (!filePath || !*filePath)
                return rdb->ReparseTag;

            const wchar_t *sourceName = wcsrchr(filePath, L'\\');
            sourceName = sourceName ? sourceName + 1 : filePath;

            wchar_t linkDir[MAX_PATH];
            wcsncpy(linkDir, filePath, MAX_PATH - 1);
            linkDir[MAX_PATH - 1] = L'\0';

            wchar_t *lastSlash = wcsrchr(linkDir, L'\\');
            if(lastSlash)
                *lastSlash = L'\0';

            wchar_t resolved[MAX_PATH];
            swprintf(resolved, MAX_PATH, L"%ls\\%.*ls", linkDir,
                     rdb->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR),
                     printName);

            wchar_t finalPath[MAX_PATH];
            GetFullPathNameW(resolved, MAX_PATH, finalPath, NULL);

            wchar_t *targetName = wcsrchr(finalPath, L'\\');
            targetName = targetName ? targetName + 1 : finalPath;

            wprintf(L"%ls -> %ls\n", sourceName, targetName);

        }
    }
    return rdb->ReparseTag;
}

//What kind of Reparse Behavior does it have?
void printfReparseTagString(ULONG reparseTag){
    switch(reparseTag){
        case IO_REPARSE_TAG_SYMLINK:{
            puts("SYMLINK");
            break; 
        }
        case IO_REPARSE_TAG_MOUNT_POINT:{
            puts("MOUNT_POINT");
            break; 
        }
        case IO_REPARSE_TAG_APPEXECLINK:{
            puts("APPEXECLINK");
            break; 
        }
        case IO_REPARSE_TAG_CLOUD:{
            puts("CLOUD");
            break; 
        }
        case IO_REPARSE_TAG_DFS:{
            puts("DFS");
            break; 
        }
        case IO_REPARSE_TAG_WIM:{
            puts("WIM");
            break; 
        }
        case IO_REPARSE_TAG_SIS:{
            puts("SIS");
            break; 
        }
        default:
            printf("UNKNOWN(Ox%08lx)", reparseTag);
            break;
    } 
}

void getFileNtLvlInfo(const wchar_t *filePath){

    HANDLE hFile = CreateFileW(filePath,
                               0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                               NULL);
    
    if(hFile == INVALID_HANDLE_VALUE){
        printf("CreateFileW Failed: %lu\n", GetLastError());
        return;
    }
    
    //Win32 API approach
    LARGE_INTEGER readEOFSize;
    BOOL ok = GetFileSizeEx(hFile, &readEOFSize);
    if(ok){
        printf("ReadEOFSize = %lld | ", readEOFSize.QuadPart);
    }else{
        puts("Error You don't have correct readEOFSize.");
    }

    DWORD high = 0;
    DWORD low = GetCompressedFileSizeW(filePath, &high);
    if(low == INVALID_FILE_SIZE && GetLastError() != NO_ERROR){
        printf("GetCompressedFileSizeW failed : %lu\n", GetLastError());
    }else{
        ULONGLONG allocateSize = ((ULONGLONG)high << 32) | low;
        printf("ASize = %llu\n", allocateSize);
    }

    BY_HANDLE_FILE_INFORMATION fileInformation;
    BOOL okay = GetFileInformationByHandle(hFile, &fileInformation);
    if(okay){
        DWORD attribute = fileInformation.dwFileAttributes;
        printfFileAttributes(attribute);
        FILETIME createTime = fileInformation.ftCreationTime;
        FILETIME lastAccessTime = fileInformation.ftLastAccessTime;
        FILETIME lastWriteTime = fileInformation.ftLastWriteTime;

        char timeRepresent[100];
        snprintf(timeRepresent, sizeof(timeRepresent),
                 "%s", "createTime");
        covertToSystemTime(createTime, timeRepresent);
        
        snprintf(timeRepresent, sizeof(timeRepresent),
                 "%s", "lastAccessTime");
        covertToSystemTime(lastAccessTime, timeRepresent);
        
        snprintf(timeRepresent, sizeof(timeRepresent),
                 "%s", "lastWriteTime");
        covertToSystemTime(lastWriteTime, timeRepresent);

        uint64_t fileIndex = ((uint64_t)fileInformation.nFileIndexHigh << 32) |
            fileInformation.nFileIndexLow;
        
        int hardLink = printHardLinks(filePath);
        if(hardLink == -1){
            puts("Error occurred to find hard link");
        }

        if(hardLink != (int)fileInformation.nNumberOfLinks){
            printf(
                   "WARNING: Hard link enumeration incomplete. "
                   "Kernel reports %lu links, enumeration found %d. "
                   "Possible causes: access restrictions, concurrent modification, or filesystem filters.\n",
                   fileInformation.nNumberOfLinks,
                   hardLink
                   );
        }
        printf(" | VolSerialNumber: %lu | FileId: %llu | nLinks: %lu | HardLink:  %d ",
               fileInformation.dwVolumeSerialNumber,
               fileIndex,
               fileInformation.nNumberOfLinks,
               hardLink);
        
    }else{
        printf("Error status code = %lu\n", GetLastError());
    }
    printf("\n\n +++++++++++ NT APIs ++++++++++++++++\n");

    //NT Lvl appraoch
    IO_STATUS_BLOCK ioSB;
    ULONG size = sizeof(FILE_ALL_INFORMATION) + 1024;
    PFILE_ALL_INFORMATION fAI = malloc(size);
    
    NTSTATUS ntStatus = NtQueryInformationFile(hFile, &ioSB, fAI,
                                               size,
                                               FileAllInformation
                                               );
 
    if(NT_SUCCESS(ntStatus)){
        
        FILE_BASIC_INFORMATION fbInfo = fAI->BasicInformation;
        FILE_STANDARD_INFORMATION fsInfo = fAI->StandardInformation;
        
        if(fsInfo.Directory){
            printf("[Type: DIR] | ");
        }else{
            printf("[Type: File] | ");
        }
            
        //File Attribute
        DWORD attri = (DWORD)fbInfo.FileAttributes;
  
        //This is not internal part of FILE_ALL_INFORMATION struct
        if(fbInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT){
            ULONG reparseTag = FindSpecialExtraInfo(hFile, filePath);
            printfReparseTagString(reparseTag);
        }

        printfFileAttributes(attri);

        LARGE_INTEGER Allocation_Size = fsInfo.AllocationSize;
        printf( " AllocationSize: %lld | ", Allocation_Size.QuadPart);

        LARGE_INTEGER EOF_Size = fsInfo.EndOfFile;
        printf( " EOF_Size: %lld | ", EOF_Size.QuadPart);

        ULONG hardLinks = fsInfo.NumberOfLinks;
        printf("HardLink: %lu | ", hardLinks);
        
        
        FILETIME ft;
        char timeRepresent[100];

        // Creation time
        ft.dwLowDateTime  = fbInfo.CreationTime.LowPart;
        ft.dwHighDateTime = fbInfo.CreationTime.HighPart;
        snprintf(timeRepresent, sizeof(timeRepresent), "%s", "createTime");
        covertToSystemTime(ft, timeRepresent);

        // Last access time
        ft.dwLowDateTime  = fbInfo.LastAccessTime.LowPart;
        ft.dwHighDateTime = fbInfo.LastAccessTime.HighPart;
        snprintf(timeRepresent, sizeof(timeRepresent), "%s", "lastAccessTime");
        covertToSystemTime(ft, timeRepresent);

        // Last write time
        ft.dwLowDateTime  = fbInfo.LastWriteTime.LowPart;
        ft.dwHighDateTime = fbInfo.LastWriteTime.HighPart;
        snprintf(timeRepresent, sizeof(timeRepresent), "%s", "lastWriteTime");
        covertToSystemTime(ft, timeRepresent);

        // Change time
        ft.dwLowDateTime  = fbInfo.ChangeTime.LowPart;
        ft.dwHighDateTime = fbInfo.ChangeTime.HighPart;
        snprintf(timeRepresent, sizeof(timeRepresent), "%s", "ChangeTime");
        covertToSystemTime(ft, timeRepresent);

        FILE_INTERNAL_INFORMATION fIInfo = fAI->InternalInformation;
        LARGE_INTEGER FileId = fIInfo.IndexNumber;
        printf(" | FileId: %lld", FileId.QuadPart);

    }
    else if(ntStatus == STATUS_INFO_LENGTH_MISMATCH){
        puts("ERROR: STATUS_INFO_LENGTH_MISMATCH");
    }
    else if(ntStatus == (long int)STATUS_INVALID_PARAMETER){
        puts("ERROR: STATUS_INVALID_PARAMETER");
    }
    else{
        printf("ERROR: NtQueryInformationFile failed: 0x%08lx\n", ntStatus);
    }

    printf("\n");

    free(fAI);
    CloseHandle(hFile);
}

int main(){
    /*
      WIN32_FIND_DATAW findData;
      HANDLE hFind;
    
      wchar_t dirPath[MAX_PATH];
      GetCurrentDirectoryW(MAX_PATH, dirPath);

      wchar_t searchPath[MAX_PATH];
      swprintf(searchPath, MAX_PATH, L"%ls\\*", dirPath);

      hFind = FindFirstFileW(searchPath, &findData);
      if(hFind == INVALID_HANDLE_VALUE){
      wprintf(L"Failed to list directory: %ls\n", dirPath);
      return 1;
      }

      do{
      if(wcscmp(findData.cFileName, L".") == 0 ||
      wcscmp(findData.cFileName, L"..") == 0)
      continue;
      wprintf(L"Name: %ls\t", findData.cFileName);

      printf("Attr: ");
      printfFileAttributes(findData.dwFileAttributes);

      if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
      LARGE_INTEGER size;
      size.HighPart = findData.nFileSizeHigh;
      size.LowPart = findData.nFileSizeLow;
      printf("\tSize: %lld bytes", size.QuadPart);
      }
      printf("\n");
      }while(FindNextFileW(hFind, &findData));

      FindClose(hFind);
    */

    /* const wchar_t *filePath = L"D:\\SoftwareFoundationInfo\\WindowsProjects\\winAppLauncher.c"; */

    /* //Only present what the kernel enforces, */
    /* //the object actually is, behavior is guaranteed */
    /* DWORD attrV = GetFileAttributesW(filePath); */
    /* if(attrV == INVALID_FILE_ATTRIBUTES){ */
    /*     puts("File does not exist or path is incorrect."); */
    /*     return 1; */
    /* } */
    /* printfFileAttributes(attrV); */

    /* const wchar_t *filePath = L"C:\\Users\\bipin\\NTUSER.DAT"; */

    //TODO: Please make sure To check object before implementing below code because
    // some of the function must File_object if not access denied

    WIN32_FIND_DATAW data;

    const wchar_t *path =
        L"D:\\SoftwareFoundationInfo\\WindowsProjects\\*";

    HANDLE h = FindFirstFileW(path, &data);
    if(h == INVALID_HANDLE_VALUE){
        printf("FindFirstFileW failed: %lu\n",
               GetLastError());
        return 1;
    }

    do{
        if(wcscmp(data.cFileName, L".") != 0
           && wcscmp(data.cFileName, L"..") != 0){
            printf("\n------------------\n");
            wprintf(L"%ls\n", data.cFileName);

            wchar_t filePath[MAX_PATH];
            swprintf(NULL, 0, L"%ls\\%ls",
                               L"D:\\SoftwareFoundationInfo\\WindowsProjects",
                               data.cFileName);
           
            swprintf(filePath, MAX_PATH, L"%ls\\%ls",
                     L"D:\\SoftwareFoundationInfo\\WindowsProjects",
                     data.cFileName);
            
            getFileNtLvlInfo(filePath);
            
            printf("------------------\n");
        }      
    }while(FindNextFileW(h, &data));

    
    FindClose(h);
    return 0;
}
