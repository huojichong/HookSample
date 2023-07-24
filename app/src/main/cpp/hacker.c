#include <android/log.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <inttypes.h>
#include <jni.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "bytehook.h"

#define HACKER_JNI_VERSION    JNI_VERSION_1_6
#define HACKER_JNI_CLASS_NAME "com/bytedance/android/bytehook/sample/NativeHacker"
#define HACKER_TAG            "bytehook_tag"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, HACKER_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop

typedef void *(*malloc_t)(int);

typedef void *(*calloc_t)(int , int);

typedef void (*free_t)(void *);

struct mem_control_block {
    int is_available; //这是一个标记？
    int size; //这是实际空间的大小
};

#define HOOK_DEF(fn)                                                                                         \
  static fn##_t fn##_prev = NULL;                                                                            \
  static bytehook_stub_t fn##_stub = NULL;                                                                   \
  static void fn##_hooked_callback(bytehook_stub_t task_stub, int status_code, const char *caller_path_name, \
                                   const char *sym_name, void *new_func, void *prev_func, void *arg) {       \
    if (BYTEHOOK_STATUS_CODE_ORIG_ADDR == status_code) {                                                     \
      fn##_prev = (fn##_t)prev_func;                                                                         \
      LOG(">>>>> save original address: %" PRIxPTR, (uintptr_t)prev_func);                                   \
    } else {                                                                                                 \
      LOG(">>>>> hooked. stub: %" PRIxPTR                                                                    \
          ", status: %d, caller_path_name: %s, sym_name: %s, new_func: %" PRIxPTR ", prev_func: %" PRIxPTR   \
          ", arg: %" PRIxPTR,                                                                                \
          (uintptr_t)task_stub, status_code, caller_path_name, sym_name, (uintptr_t)new_func,                \
          (uintptr_t)prev_func, (uintptr_t)arg);                                                             \
    }                                                                                                        \
  }

HOOK_DEF(malloc)
HOOK_DEF(calloc)
HOOK_DEF(free)

static void debug(const char *sym, const char *pathname, int flags, int fd, void *lr) {
    Dl_info info;
    memset(&info, 0, sizeof(info));
    dladdr(lr, &info);

    LOG("proxy %s(\"%s\", %d), return FD: %d, called from: %s (%s)", sym, pathname, flags, fd,
        info.dli_fname,
        info.dli_sname);
}


static bool allow_filter(const char *caller_path_name, void *arg) {
    (void) arg;

//    if (NULL != strstr(caller_path_name, "libbase.so")) return false;
//    if (NULL != strstr(caller_path_name, "libc.so")) return false;
    if (NULL != strstr(caller_path_name, "libhooksample.so")) return false;

    return true;
}

static void *malloc_proxy(int size) {
    void *res = (void *) BYTEHOOK_CALL_PREV(malloc_proxy, malloc_t , size);

    LOG("malloc size:%u", size);
    BYTEHOOK_POP_STACK();
    return res;
}

static void *calloc_proxy(int count, int size) {
    void *res = (void *) BYTEHOOK_CALL_PREV(calloc_proxy, calloc_t, count, size);

    LOG("calloc count:%u, size:%u", count, size);
    BYTEHOOK_POP_STACK();
    return res;
}

static void free_proxy(void *p) {
    LOG("free size:%p",p);
    BYTEHOOK_CALL_PREV(free_proxy,free_t,p);
    BYTEHOOK_POP_STACK();
}

static int hacker_hook(JNIEnv *env, jobject thiz) {
    (void) env, (void) thiz;

    if (NULL != malloc_stub || NULL != calloc_stub || NULL != free_stub) return -1;

//  void *malloc_proxy = (void *) malloc_func;
  malloc_stub = bytehook_hook_partial(allow_filter, NULL, NULL, "malloc",
                                      malloc_proxy, malloc_hooked_callback,NULL);

//  void *calloc_proxy = (void *) calloc_proxy;
    calloc_stub = bytehook_hook_partial(allow_filter, NULL, NULL, "malloc",
                                        calloc_proxy, calloc_hooked_callback, NULL);

//  void* free_proxy = (void*) free_func;
  free_stub = bytehook_hook_partial(allow_filter,NULL,NULL,"free",
                                    free_proxy,free_hooked_callback,NULL);

    return 0;
}

static int hacker_unhook(JNIEnv *env, jobject thiz) {
    (void) env, (void) thiz;

    if (NULL != malloc_stub) {
        bytehook_unhook(malloc_stub);
        malloc_stub = NULL;
    }

    if (NULL != calloc_stub) {
        bytehook_unhook(calloc_stub);
        calloc_stub = NULL;
    }

    if (NULL != free_stub) {
        bytehook_unhook(free_stub);
        free_stub = NULL;
    }

    return 0;
}

static void hacker_dump_records(JNIEnv *env, jobject thiz, jstring pathname) {
    (void) thiz;

    const char *c_pathname = (*env)->GetStringUTFChars(env, pathname, 0);
    if (NULL == c_pathname) return;

    int fd = open(c_pathname, O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC | O_APPEND,
                  S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        bytehook_dump_records(fd, BYTEHOOK_RECORD_ITEM_ALL);
        close(fd);
    }

    (*env)->ReleaseStringUTFChars(env, pathname, c_pathname);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void) reserved;

    if (NULL == vm) return JNI_ERR;

    JNIEnv *env;
    if (JNI_OK != (*vm)->GetEnv(vm, (void **) &env, HACKER_JNI_VERSION)) return JNI_ERR;
    if (NULL == env || NULL == *env) return JNI_ERR;

    jclass cls;
    if (NULL == (cls = (*env)->FindClass(env, HACKER_JNI_CLASS_NAME))) return JNI_ERR;

    JNINativeMethod m[] = {{"nativeHook",        "()I",                   (void *) hacker_hook},
                           {"nativeUnhook",      "()I",                   (void *) hacker_unhook},
                           {"nativeDumpRecords", "(Ljava/lang/String;)V", (void *) hacker_dump_records}};
    if (0 != (*env)->RegisterNatives(env, cls, m, sizeof(m) / sizeof(m[0]))) return JNI_ERR;

    return HACKER_JNI_VERSION;
}
