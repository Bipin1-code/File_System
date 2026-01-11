

#include <stdio.h>
#include <windows.h>
#include <stdint.h>


void printfFileAttributes(DWORD attrs){
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

    // const wchar_t *filePath = L"D:\\SoftwareFoundationInfo\\WindowsProjects\\winAppLauncher.c";

    /* //Only present what the kernel enforces, */
    /* //the object actually is, behavior is guaranteed */
    /* DWORD attrV = GetFileAttributesW(filePath); */
    /* if(attrV == INVALID_FILE_ATTRIBUTES){ */
    /*     puts("File does not exist or path is incorrect."); */
    /*     return 1; */
    /* } */
    /* printfFileAttributes(attrV); */

    //const wchar_t *filePath = L"C:\\Users\\bipin\\NTUSER.DAT";
 
    const wchar_t *filePath =
        L"D:\\SoftwareFoundationInfo\\WindowsProjects\\winAppLauncher.c";
    HANDLE hFile = CreateFileW(filePath,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    
    if(hFile == INVALID_HANDLE_VALUE){
        printf("CreateFileW Failed: %lu\n", GetLastError());
        return 1;
    }
    
    /* //Win32 API approach */
    /* LARGE_INTEGER readEOFSize; */
    /* BOOL ok = GetFileSizeEx(hFile, &readEOFSize); */
    /* if(ok){ */
    /*     printf("ReadEOFSize = %lld | ", readEOFSize.QuadPart); */
    /* }else{ */
    /*     puts("Error You don't have correct readEOFSize."); */
    /* } */

    /* DWORD high = 0; */
    /* DWORD low = GetCompressedFileSizeW(filePath, &high); */
    /* if(low == INVALID_FILE_SIZE && GetLastError() != NO_ERROR){ */
    /*     printf("GetCompressedFileSizeW failed : %lu\n", GetLastError()); */
    /* }else{ */
    /*     ULONGLONG allocateSize = ((ULONGLONG)high << 32) | low; */
    /*     printf("ASize = %llu\n", allocateSize); */
    /* } */

    /* BY_HANDLE_FILE_INFORMATION fileInformation; */
    /* BOOL okay = GetFileInformationByHandle(hFile, &fileInformation); */
    /* if(okay){ */
    /*     DWORD attribute = fileInformation.dwFileAttributes; */
    /*     printfFileAttributes(attribute); */
    /*     FILETIME createTime = fileInformation.ftCreationTime; */
    /*     FILETIME lastAccessTime = fileInformation.ftLastAccessTime; */
    /*     FILETIME lastWriteTime = fileInformation.ftLastWriteTime; */

    /*     char timeRepresent[100]; */
    /*     snprintf(timeRepresent, sizeof(timeRepresent), */
    /*              "%s", "createTime"); */
    /*     covertToSystemTime(createTime, timeRepresent); */
        
    /*     snprintf(timeRepresent, sizeof(timeRepresent), */
    /*              "%s", "lastAccessTime"); */
    /*     covertToSystemTime(lastAccessTime, timeRepresent); */
        
    /*     snprintf(timeRepresent, sizeof(timeRepresent), */
    /*                      "%s", "lastWriteTime"); */
    /*     covertToSystemTime(lastWriteTime, timeRepresent); */

    /*     uint64_t fileIndex = ((uint64_t)fileInformation.nFileIndexHigh << 32) | */
    /*         fileInformation.nFileIndexLow; */
        
    /*     int hardLink = printHardLinks(filePath); */
    /*     if(hardLink == -1){ */
    /*         puts("Error occurred to find hard link"); */
    /*     } */

    /*     if(hardLink != (int)fileInformation.nNumberOfLinks){ */
    /*         printf( */
    /*                "WARNING: Hard link enumeration incomplete. " */
    /*                "Kernel reports %lu links, enumeration found %d. " */
    /*                "Possible causes: access restrictions, concurrent modification, or filesystem filters.\n", */
    /*                fileInformation.nNumberOfLinks, */
    /*                hardLink */
    /*                ); */
    /*     } */
    /*     printf(" | VolSerialNumber: %lu | FileId: %llu | nLinks: %lu | HardLink:  %d ", */
    /*            fileInformation.dwVolumeSerialNumber, */
    /*            fileIndex, */
    /*            fileInformation.nNumberOfLinks, */
    /*            hardLink); */
        
    /* }else{ */
    /*     printf("Error status code = %lu\n", GetLastError()); */
    /* } */
    //printf("\n");

    //NT Lvl appraoch
    IO_STATUS_BLOCK ioSB;
    FILE_BASIC_INFORMATION fBInfo;
    
    NTSTATUS ntStatus = NtQueryInformationFile(hFile, &ioSB, &fBInfo,
                                               sizeof(fBInfo),
                                               FileBasiceInformation
                                               );
    if(NT_SUCCESS(ntStatus)){
        //We do real work here
        //To do use FileAllInformation and parse needed data correctly
    }else if(ntStatus == STATUS_INFO_LENGTH_MISMATCH){
        puts("ERROR: Status information length mismatch");
    }else if(ntStatus == STATUS_INVALID_PARAMETER){
        puts("ERROR: Status invalid parameter");
    }else{
        puts("ERROR: STATUS_ACCESS_DENIED or else error");
    }
    
    CloseHandle(hFile);
    
    return 0;
}
