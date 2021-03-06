#include "mini_stdint.h"
#include "../buffer_t.h"
#include "HalideRuntime.h"

extern "C" {

extern int atoi(const char *);
extern char *getenv(const char *);
extern int64_t halide_current_time_ns(void *user_context);
extern void *malloc(size_t);
extern int snprintf(char *, size_t, const char *, ...);

#ifndef DEBUG
#define CHECK_CALL(c,str) c
#define TIME_CALL(c,str) c
#else
#define CHECK_CALL(c,str) do {\
    halide_printf(user_context, "Do %s\n", str);        \
    CUresult status = (c);                              \
    if (status != CUDA_SUCCESS)                                         \
        halide_printf(user_context, "CUDA: %s returned non-success: %d\n", str, status); \
    halide_assert(user_context, status == CUDA_SUCCESS);                \
} while(0)
#define TIME_CALL(c,str) do {\
    cuEventRecord(__start, 0);                              \
    CHECK_CALL((c),(str));                                  \
    cuEventRecord(__end, 0);                                \
    cuEventSynchronize(__end);                              \
    float msec;                                             \
    cuEventElapsedTime(&msec, __start, __end);              \
    halide_printf(user_context, "   (took %fms, t=%lld)\n", msec,   \
                  (long long)halide_current_time_ns(user_context)); \
} while(0)
#endif //DEBUG

// TODO: On windows this needs __stdcall
//#define CUDAAPI __stdcall
#define CUDAAPI

// API version > 3020
#define cuCtxCreate                         cuCtxCreate_v2
#define cuMemAlloc                          cuMemAlloc_v2
#define cuMemFree                           cuMemFree_v2
#define cuMemcpyHtoD                        cuMemcpyHtoD_v2
#define cuMemcpyDtoH                        cuMemcpyDtoH_v2
// API version >= 4000
#define cuCtxDestroy                        cuCtxDestroy_v2
#define cuCtxPopCurrent                     cuCtxPopCurrent_v2
#define cuCtxPushCurrent                    cuCtxPushCurrent_v2
#define cuStreamDestroy                     cuStreamDestroy_v2
#define cuEventDestroy                      cuEventDestroy_v2

#ifdef BITS_64
typedef unsigned long long CUdeviceptr;
#else
typedef unsigned int CUdeviceptr;
#endif

typedef int CUdevice;                                     /**< CUDA device */
typedef struct CUctx_st *CUcontext;                       /**< CUDA context */
typedef struct CUmod_st *CUmodule;                        /**< CUDA module */
typedef struct CUfunc_st *CUfunction;                     /**< CUDA function */
typedef struct CUstream_st *CUstream;                     /**< CUDA stream */
typedef struct CUevent_st *CUevent;                       /**< CUDA event */
typedef enum {
    CUDA_SUCCESS                              = 0,
    CUDA_ERROR_INVALID_VALUE                  = 1,
    CUDA_ERROR_OUT_OF_MEMORY                  = 2,
    CUDA_ERROR_NOT_INITIALIZED                = 3,
    CUDA_ERROR_DEINITIALIZED                  = 4,
    CUDA_ERROR_PROFILER_DISABLED           = 5,
    CUDA_ERROR_PROFILER_NOT_INITIALIZED       = 6,
    CUDA_ERROR_PROFILER_ALREADY_STARTED       = 7,
    CUDA_ERROR_PROFILER_ALREADY_STOPPED       = 8,
    CUDA_ERROR_NO_DEVICE                      = 100,
    CUDA_ERROR_INVALID_DEVICE                 = 101,
    CUDA_ERROR_INVALID_IMAGE                  = 200,
    CUDA_ERROR_INVALID_CONTEXT                = 201,
    CUDA_ERROR_CONTEXT_ALREADY_CURRENT        = 202,
    CUDA_ERROR_MAP_FAILED                     = 205,
    CUDA_ERROR_UNMAP_FAILED                   = 206,
    CUDA_ERROR_ARRAY_IS_MAPPED                = 207,
    CUDA_ERROR_ALREADY_MAPPED                 = 208,
    CUDA_ERROR_NO_BINARY_FOR_GPU              = 209,
    CUDA_ERROR_ALREADY_ACQUIRED               = 210,
    CUDA_ERROR_NOT_MAPPED                     = 211,
    CUDA_ERROR_NOT_MAPPED_AS_ARRAY            = 212,
    CUDA_ERROR_NOT_MAPPED_AS_POINTER          = 213,
    CUDA_ERROR_ECC_UNCORRECTABLE              = 214,
    CUDA_ERROR_UNSUPPORTED_LIMIT              = 215,
    CUDA_ERROR_CONTEXT_ALREADY_IN_USE         = 216,
    CUDA_ERROR_INVALID_SOURCE                 = 300,
    CUDA_ERROR_FILE_NOT_FOUND                 = 301,
    CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND = 302,
    CUDA_ERROR_SHARED_OBJECT_INIT_FAILED      = 303,
    CUDA_ERROR_OPERATING_SYSTEM               = 304,
    CUDA_ERROR_INVALID_HANDLE                 = 400,
    CUDA_ERROR_NOT_FOUND                      = 500,
    CUDA_ERROR_NOT_READY                      = 600,
    CUDA_ERROR_LAUNCH_FAILED                  = 700,
    CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES        = 701,
    CUDA_ERROR_LAUNCH_TIMEOUT                 = 702,
    CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING  = 703,
    CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED = 704,
    CUDA_ERROR_PEER_ACCESS_NOT_ENABLED    = 705,
    CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE         = 708,
    CUDA_ERROR_CONTEXT_IS_DESTROYED           = 709,
    CUDA_ERROR_UNKNOWN                        = 999
} CUresult;

#define CU_POINTER_ATTRIBUTE_CONTEXT 1

CUresult CUDAAPI cuInit(unsigned int Flags);
CUresult CUDAAPI cuDeviceGetCount(int *count);
CUresult CUDAAPI cuDeviceGet(CUdevice *device, int ordinal);
CUresult CUDAAPI cuCtxCreate(CUcontext *pctx, unsigned int flags, CUdevice dev);
CUresult CUDAAPI cuCtxDestroy(CUcontext pctx);
CUresult CUDAAPI cuModuleLoadData(CUmodule *module, const void *image);
CUresult CUDAAPI cuModuleUnload(CUmodule module);
CUresult CUDAAPI cuModuleGetFunction(CUfunction *hfunc, CUmodule hmod, const char *name);
CUresult CUDAAPI cuMemAlloc(CUdeviceptr *dptr, size_t bytesize);
CUresult CUDAAPI cuMemFree(CUdeviceptr dptr);
CUresult CUDAAPI cuMemcpyHtoD(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount);
CUresult CUDAAPI cuMemcpyDtoH(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount);
CUresult CUDAAPI cuLaunchKernel(CUfunction f,
                                unsigned int gridDimX,
                                unsigned int gridDimY,
                                unsigned int gridDimZ,
                                unsigned int blockDimX,
                                unsigned int blockDimY,
                                unsigned int blockDimZ,
                                unsigned int sharedMemBytes,
                                CUstream hStream,
                                void **kernelParams,
                                void **extra);
CUresult CUDAAPI cuCtxSynchronize();

CUresult CUDAAPI cuCtxPushCurrent(CUcontext ctx);
CUresult CUDAAPI cuCtxPopCurrent(CUcontext *pctx);

CUresult CUDAAPI cuEventRecord(CUevent hEvent, CUstream hStream);
CUresult CUDAAPI cuEventCreate(CUevent *phEvent, unsigned int Flags);
CUresult CUDAAPI cuEventDestroy(CUevent phEvent);
CUresult CUDAAPI cuEventSynchronize(CUevent hEvent);
CUresult CUDAAPI cuEventElapsedTime(float *pMilliseconds, CUevent hStart, CUevent hEnd);
CUresult CUDAAPI cuPointerGetAttribute(void *result, int query, CUdeviceptr ptr);

}
extern "C" {

// A cuda context defined in this module with weak linkage
CUcontext WEAK weak_cuda_ctx = 0;

// A pointer to the cuda context to use, which may not be the one above. This pointer is followed at init_kernels time.
CUcontext WEAK *cuda_ctx_ptr = NULL;

WEAK void halide_set_cuda_context(CUcontext *ctx_ptr) {
    cuda_ctx_ptr = ctx_ptr;
}

// Structure to hold the state of a module attached to the context.
// Also used as a linked-list to keep track of all the different
// modules that are attached to a context in order to release them all
// when then context is released.
struct _module_state_ WEAK *state_list = NULL;
typedef struct _module_state_ {
    CUmodule module;
    _module_state_ *next;
} module_state;

static CUevent __start, __end;

WEAK bool halide_validate_dev_pointer(void *user_context, buffer_t* buf) {
// The technique using cuPointerGetAttribute and CU_POINTER_ATTRIBUTE_CONTEXT
// requires unified virtual addressing is enabled and that is not the case
// for 32-bit processes on Mac OS X. So for now, as a total hack, just return true
// in 32-bit. This could of course be wrong the other way for cards that only
// support 32-bit addressing in 64-bit processes, but I expect those cards do not
// support unified addressing at all.
// TODO: figure out a way to validate pointers in all cases if strictly necessary.
#ifdef BITS_32
    return true;
#else
    if (buf->dev == 0)
        return true;

    CUcontext ctx;
    CUresult result = cuPointerGetAttribute(&ctx, CU_POINTER_ATTRIBUTE_CONTEXT, buf->dev);
    if (result) {
        halide_printf(user_context, "Bad device pointer %p: cuPointerGetAttribute returned %d\n",
                      (void *)buf->dev, result);
        return false;
    }
    return true;
#endif
}

WEAK void halide_dev_free(void *user_context, buffer_t* buf) {
    // halide_dev_free, at present, can be exposed to clients and they
    // should be allowed to call halide_dev_free on any buffer_t
    // including ones that have never been used with a GPU.
    if (buf->dev == 0)
        return;

    #ifdef DEBUG
    halide_printf(user_context, "In dev_free of %p - dev: 0x%p\n", buf, (void*)buf->dev);
    halide_assert(user_context, halide_validate_dev_pointer(user_context, buf));
    #endif

    CHECK_CALL( cuMemFree(buf->dev), "cuMemFree" );
    buf->dev = 0;

}

WEAK void* halide_init_kernels(void *user_context, void *state_ptr, const char* ptx_src, int size) {
    // If the context pointer isn't hooked up yet, point it at this module's weak-linkage context.
    if (cuda_ctx_ptr == NULL) {
        cuda_ctx_ptr = &weak_cuda_ctx;
    }

    // Initialize one shared context for all Halide compiled instances
    if (*cuda_ctx_ptr == 0) {
        // Initialize CUDA
        CHECK_CALL( cuInit(0), "cuInit" );

        // Make sure we have a device
        int deviceCount = 0;
        CHECK_CALL( cuDeviceGetCount(&deviceCount), "cuDeviceGetCount" );
        halide_assert(user_context, deviceCount > 0);

        char *device_str = getenv("HL_GPU_DEVICE");

        CUdevice dev;
        // Get device
        CUresult status;
        if (device_str) {
            status = cuDeviceGet(&dev, atoi(device_str));
        } else {
            // Try to get a device >0 first, since 0 should be our display device
            // For now, don't try devices > 2 to maintain compatibility with previous behavior.
            if (deviceCount > 2)
                deviceCount = 2;
            for (int id = deviceCount - 1; id >= 0; id--) {
                status = cuDeviceGet(&dev, id);
                if (status == CUDA_SUCCESS) break;
            }
        }

        halide_assert(user_context, status == CUDA_SUCCESS && "Failed to get device\n");

        #ifdef DEBUG
        halide_printf(user_context, "Got device %d, about to create context (t=%lld)\n",
                      dev, (long long)halide_current_time_ns(user_context));
        #endif


        // Create context
        CHECK_CALL( cuCtxCreate(cuda_ctx_ptr, 0, dev), "cuCtxCreate" );
    } else {
        //CHECK_CALL( cuCtxPushCurrent(*cuda_ctx_ptr), "cuCtxPushCurrent" );
    }

    // Create the module state if necessary
    module_state *state = (module_state*)state_ptr;
    if (!state) {
        state = (module_state*)malloc(sizeof(module_state));
        state->module = NULL;
        state->next = state_list;
        state_list = state;
    }

    // Initialize a module for just this Halide module
    if ((!state->module)) {
        // Create module
        CHECK_CALL( cuModuleLoadData(&state->module, ptx_src), "cuModuleLoadData" );

        #ifdef DEBUG
        halide_printf(user_context, "-------\nCompiling PTX:\n%s\n--------\n",
                      ptx_src);
        #endif
    }

    // Create two events for timing
    if (!__start) {
        cuEventCreate(&__start, 0);
        cuEventCreate(&__end, 0);
    }

    return state;
}

#ifdef DEBUG
#define CHECK_CALL_DEINIT_OK(c,str) do {\
    halide_printf(user_context, "Do %s\n", str); \
    CUresult status = (c); \
    if (status != CUDA_SUCCESS && status != CUDA_ERROR_DEINITIALIZED) \
        halide_printf(user_context, "CUDA: %s returned non-success: %d\n", str, status); \
    halide_assert(user_context, status == CUDA_SUCCESS || status == CUDA_ERROR_DEINITIALIZED); \
} while(0)
#else
#define CHECK_CALL_DEINIT_OK(c,str) c
#endif

WEAK void halide_release(void *user_context) {
    // Do not do any of this if there is not context set. E.g.
    // if halide_release is called and no CUDA calls have been made.
    if (cuda_ctx_ptr != NULL) {
        // It's possible that this is being called from the destructor of
        // a static variable, in which case the driver may already be
        // shutting down. For this reason we allow the deinitialized
        // error.
        CHECK_CALL_DEINIT_OK( cuCtxSynchronize(), "cuCtxSynchronize on exit" );

        // Destroy the events
        if (__start) {
            cuEventDestroy(__start);
            cuEventDestroy(__end);
            __start = __end = 0;
        }

        // Unload the modules attached to this context
        module_state *state = state_list;
        while (state) {
            if (state->module) {
                CHECK_CALL_DEINIT_OK( cuModuleUnload(state->module), "cuModuleUnload" );
                state->module = 0;
            }
            state = state->next;
        }

        // Only destroy the context if we own it
        if (weak_cuda_ctx) {
            CHECK_CALL_DEINIT_OK( cuCtxDestroy(weak_cuda_ctx), "cuCtxDestroy on exit" );
            weak_cuda_ctx = 0;
        }

        cuda_ctx_ptr = NULL;
    }

    //CHECK_CALL( cuCtxPopCurrent(&ignore), "cuCtxPopCurrent" );
}

static CUfunction __get_kernel(void *user_context, CUmodule mod, const char* entry_name)
{
    CUfunction f;

    #ifdef DEBUG
    char msg[256];
    snprintf(msg, 256, "get_kernel %s (t=%lld)", entry_name, (long long)halide_current_time_ns(user_context) );
    #endif

    // Get kernel function ptr
    TIME_CALL( cuModuleGetFunction(&f, mod, entry_name), msg );

    return f;
}

static size_t __buf_size(void *user_context, buffer_t* buf) {
    size_t size = 0;
    for (size_t i = 0; i < sizeof(buf->stride) / sizeof(buf->stride[0]); i++) {
        size_t total_dim_size = buf->elem_size * buf->extent[i] * buf->stride[i];
        if (total_dim_size > size)
            size = total_dim_size;
    }
    halide_assert(user_context, size);
    return size;
}

WEAK void halide_dev_malloc(void *user_context, buffer_t* buf) {
    if (buf->dev) {
        // This buffer already has a device allocation
        return;
    }

    size_t size = __buf_size(user_context, buf);

    #ifdef DEBUG
    halide_printf(user_context, "dev_malloc allocating buffer of %zd bytes, "
                  "extents: %zdx%zdx%zdx%zd strides: %zdx%zdx%zdx%zd (%d bytes per element)\n",
                  size, buf->extent[0], buf->extent[1], buf->extent[2], buf->extent[3],
                  buf->stride[0], buf->stride[1], buf->stride[2], buf->stride[3],
                  buf->elem_size);
    #endif

    CUdeviceptr p;
    TIME_CALL( cuMemAlloc(&p, size), "dev_malloc");

    buf->dev = (uint64_t)p;
    halide_assert(user_context, buf->dev);

    #ifdef DEBUG
    halide_assert(user_context, halide_validate_dev_pointer(user_context, buf));
    #endif
}

WEAK void halide_copy_to_dev(void *user_context, buffer_t* buf) {
    if (buf->host_dirty) {
      halide_assert(user_context, buf->host && buf->dev);
        size_t size = __buf_size(user_context, buf);
        #ifdef DEBUG
        char msg[256];
        snprintf(msg, 256, "copy_to_dev (%zu bytes) %p -> %p (t=%lld)",
                 size, buf->host, (void*)buf->dev, (long long)halide_current_time_ns(user_context) );
        halide_assert(user_context, halide_validate_dev_pointer(user_context, buf));
        #endif
        TIME_CALL( cuMemcpyHtoD(buf->dev, buf->host, size), msg );
    }
    buf->host_dirty = false;
}

WEAK void halide_copy_to_host(void *user_context, buffer_t* buf) {
    if (buf->dev_dirty) {
        halide_assert(user_context, buf->dev);
        halide_assert(user_context, buf->host);
        size_t size = __buf_size(user_context, buf);
        #ifdef DEBUG
        char msg[256];
        snprintf(msg, 256, "copy_to_host (%zu bytes) %p -> %p", size, (void*)buf->dev, buf->host );
        halide_assert(user_context, halide_validate_dev_pointer(user_context, buf));
        #endif
        TIME_CALL( cuMemcpyDtoH(buf->host, buf->dev, size), msg );
    }
    buf->dev_dirty = false;
}

// Used to generate correct timings when tracing
WEAK void halide_dev_sync(void *user_context) {
    cuCtxSynchronize();
}

WEAK void halide_dev_run(
    void *user_context,
    void *state_ptr,
    const char* entry_name,
    int blocksX, int blocksY, int blocksZ,
    int threadsX, int threadsY, int threadsZ,
    int shared_mem_bytes,
    size_t arg_sizes[],
    void* args[]) {

    halide_assert(user_context, state_ptr);
    CUmodule mod = ((module_state*)state_ptr)->module;
    halide_assert(user_context, mod);
    CUfunction f = __get_kernel(user_context, mod, entry_name);

    #ifdef DEBUG
    char msg[256];
    snprintf(
        msg, 256,
        "dev_run %s with (%dx%dx%d) blks, (%dx%dx%d) threads, %d shmem (t=%lld)",
        entry_name, blocksX, blocksY, blocksZ, threadsX, threadsY, threadsZ, shared_mem_bytes,
        (long long)halide_current_time_ns(user_context)
    );
    #endif

    TIME_CALL(
        cuLaunchKernel(
            f,
            blocksX,  blocksY,  blocksZ,
            threadsX, threadsY, threadsZ,
            shared_mem_bytes,
            NULL, // stream
            args,
            NULL
        ),
        msg
    );
}

} // extern "C" linkage

#undef CHECK_ERR
#undef CHECK_CALL
#undef TIME_START
#undef TIME_CHECK
