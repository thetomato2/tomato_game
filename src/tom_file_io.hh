#ifndef TOM_FILE_IO_HH
#define TOM_FILE_IO_HH

#include "tom_core.hh"

namespace tom
{

struct ReadFileResult
{
    szt size;
    void* buf;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// #DECLARES
bool write_file(const char *file_path, szt buf_sz, void *buf);
ReadFileResult read_file(const char *file_path);

}  // namespace tom

#endif
