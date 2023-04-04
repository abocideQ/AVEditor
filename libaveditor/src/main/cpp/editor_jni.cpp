#include <jni.h>

#include "timeline.h"
#include "repack.h"
#include "recode.h"
#include "filtering.h"

void native_ave_repack(JNIEnv *env, jobject *, jstring inUrl, jstring outUrl) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    {
        LOGE("_______________start repack_______________");
        auto *p_repack = new repack();
        int err = p_repack->repack_fmt(inUrl_str, outUrl_str);
        delete p_repack;
        LOGE("_______________end repack_______________%d", err);
    }
}

void native_ave_recode(JNIEnv *env, jobject *, jstring inUrl, jstring outUrl) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    {
        LOGE("_______________start get time line_______________");
        vector<long long> vec_dts = timeline::get_time_stamps(inUrl_str);
        LOGE("_______________end get time line_______________size=%lu", vec_dts.size());
        LOGE("_______________start recode_______________");
        auto *p_recode = new recode();
        AVConfig config = AVConfig();
        config.dts_left = vec_dts[(vec_dts.size() / 3)];
        config.dts_right = vec_dts[(vec_dts.size() / 2)];
        int err = p_recode->recode_codec(inUrl_str, outUrl_str, config);
        delete p_recode;
        LOGE("_______________end recode_______________%d", err);
    }
}

void native_ave_filter(JNIEnv *env, jobject *, jstring inUrl, jstring outUrl, jstring filterDesc) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    std::string filterDesc_str = std::string((char *) env->GetStringUTFChars(filterDesc, &jCopy));
    {
        LOGE("_______________start filter_______________");
        auto *p_filter = new filtering();
        delete p_filter;
        LOGE("_______________end filter_______________");
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
        {
                "native_ave_filter",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                (void *) native_ave_filter
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