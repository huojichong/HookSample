#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_classic_hooksample_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

//    int* p = (int *) calloc(10,sizeof (int));
//    p[1] = 10;
//    p[2] = 30;
//    free(p);
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_classic_hooksample_MainActivity_nativeCallFunc(JNIEnv *env, jobject thiz) {
    int* p = (int *) calloc(10,sizeof (int));
    p[1] = 10;
    p[2] = 30;
    free(p);


}