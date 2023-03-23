#include <jni.h>
#include "remix_timestamp.h"

void native_ave_run(JNIEnv *env, jobject *obj, jstring inUrl, jstring outUrl) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    {
        LOGE("_______________start remix_______________");
        auto *remix = new remix_timestamp();
        std::map<long, long> stamp_map = remix->get_stamp(inUrl_str);
        for (auto &it: stamp_map) {
            it.second = it.first;
        }
        remix->remix_stamp(inUrl_str, outUrl_str, &stamp_map);
        LOGE("_______________end remix_______________");
    }
    {
        LOGE("_______________start recode_______________");
        LOGE("_______________end recode_______________");
    }
}

#define JNI_LENGTH(n) (sizeof(n) / sizeof((n)[0]))
const char *JNI_CLASS_AVEditor[] = {
        "com/av/editor/AVEditor",
};
JNINativeMethod JNI_METHODS_AVEditor[] = {
        {"native_ave_run", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) native_ave_run},
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