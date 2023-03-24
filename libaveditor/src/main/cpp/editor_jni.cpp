#include <jni.h>

#include "repack.h"
#include "recode.h"

void native_ave_repack(JNIEnv *env, jobject *obj, jstring inUrl, jstring outUrl) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    {
        LOGE("_______________start repack_______________");
        auto *p_repack = new repack();
        int err = p_repack->repack_fmt(inUrl_str, outUrl_str);
        LOGE("_______________end repack_______________%d", err);
    }
}

void native_ave_recode(JNIEnv *env, jobject *obj, jstring inUrl, jstring outUrl) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    {
        LOGE("_______________start recode_______________");
        auto *p_recode = new recode();
        int err = p_recode->recode_codec(inUrl_str, outUrl_str);
        LOGE("_______________end recode_______________%d", err);
    }
}


/**
 * JNI_OnLoad
 */

#define JNI_LENGTH(n) (sizeof(n) / sizeof((n)[0]))
const char *JNI_CLASS_AVEditor[] = {
        "com/av/editor/AVEditor",
};
JNINativeMethod JNI_METHODS_AVEditor[] = {
        {
                "native_ave_repack",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                (void *) native_ave_repack
        },
        {
                "native_ave_recode",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                (void *) native_ave_recode
        },
};

jint JNI_OnLoad(JavaVM *jvm, void *) {
    JNIEnv *env = nullptr;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass jClassAVEditor = env->FindClass(JNI_CLASS_AVEditor[0]);
    if (env->RegisterNatives(jClassAVEditor,
                             JNI_METHODS_AVEditor,
                             JNI_LENGTH(JNI_METHODS_AVEditor)) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}