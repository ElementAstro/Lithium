#define HYDROGEN_SHARED_BLOB_SUPPORT

#include <stddef.h>

#pragma once

/** \brief Allocate a buffer suitable for fast exchange over local links. Warning : the buffer will be sealed (readonly) once exchanged.
 *  \param size_t size of the memory area to allocate
 */
void *IDSharedBlobAlloc(size_t size);

/**
 * Attach to a received shared buffer by ID
 * The returned buffer cannot be realloced or sealed.
 * \return null on error + errno (invalid fd / system resources)
 */
void *IDSharedBlobAttach(int fd, size_t size);

/** \brief Free a buffer allocated using IDSharedBlobAlloc. Fall back to free for buffer that are not shared blob
 * Must be used for IBLOB.data
 */
void IDSharedBlobFree(void *ptr);

/** \brief Dettach a blob, but don't close its FD
 */
void IDSharedBlobDettach(void *ptr);

/** \brief Adjust the size of a buffer obtained using IDSharedBlobAlloc.
 *  \param size_t size of the memory area to allocate
 */
void *IDSharedBlobRealloc(void *ptr, size_t size);

/** \brief Return the filedescriptor backing up the given shared buffer.
 *  \return the filedescriptor or -1 if not a shared buffer pointer
 */
int IDSharedBlobGetFd(void *ptr);

/** \brief Seal (make readonly) a buffer allocated using IDSharedBlobAlloc. This is automatic when IDNewBlob
 *  \param size_t size of the memory area to allocate
 */
void IDSharedBlobSeal(void *ptr);

void *IDSharedBlobAlloc(size_t size);
