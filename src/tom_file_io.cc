namespace tom
{

fn bool write_file(const char* file_path, szt buf_sz, void* buf)
{
    b32 success = false;

    HANDLE file_handle = CreateFileA(file_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if (WriteFile(file_handle, buf, (DWORD)buf_sz, &bytes_written, 0)) {
            success = (bytes_written == buf_sz);
        } else {
            printf("ERROR-> Failed to write file contents!\n");
        }
        CloseHandle(file_handle);
    } else {
        printf("ERROR-> Failed to oepn file handle!\n");
    }

    return success;
}

fn ReadFileResult read_file(const char* file_path)
{
    ReadFileResult file = {};

    HANDLE file_handle { CreateFileA(file_path, GENERIC_READ, 0, 0, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL, 0) };
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_sz;
        if (GetFileSizeEx(file_handle, &file_sz)) {
            file.buf = plat_malloc(file_sz.QuadPart);
            if (file.buf) {
                DWORD bytesRead;
                if (ReadFile(file_handle, file.buf, (DWORD)file_sz.QuadPart, &bytesRead, 0) &&
                    (u32)file_sz.QuadPart == bytesRead) {
                    file.size = file_sz.QuadPart;
                } else {
                    plat_free(file.buf);
                    file.buf = nullptr;
                }
            } else {
                printf("ERROR-> Failed to read file contents!\n");
            }
        } else {
            printf("ERROR-> Failed to open file handle!\n");
        }
        CloseHandle(file_handle);
    }
    return file;
}

}  // namespace tom