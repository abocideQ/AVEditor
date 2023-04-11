#include <jni.h>

#include "timeline.h"
#include "repack.h"
#include "recode.h"
#include "filtering.h"
#include "merger.h"

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
        vector<long double> vec_dts = timeline::get_time_stamps(inUrl_str);
        LOGE("_______________end get time line_______________size=%lu", vec_dts.size());
        LOGE("_______________start recode_______________");
        auto *p_recode = new recode();
        recode::AVConfig config = recode::AVConfig();
        config.dts_left = vec_dts[(vec_dts.size() / 4)];
        config.dts_right = vec_dts[(vec_dts.size() / 2)];
        int err = p_recode->recode_codec(inUrl_str, outUrl_str, config);
        delete p_recode;
        LOGE("_______________end recode_______________%d", err);
    }
}

void native_ave_filter(JNIEnv *env, jobject *,
                       jstring inUrl,
                       jstring outUrl,
                       jstring filterVideo,
                       jstring filterAudio) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    std::string filterVideo_str = std::string((char *) env->GetStringUTFChars(filterVideo, &jCopy));
    std::string filterAudio_str = std::string((char *) env->GetStringUTFChars(filterAudio, &jCopy));
    {
        LOGE("_______________start filter_______________");
        auto *p_filtering = new filtering();
        int err = p_filtering->go_filter(inUrl_str, outUrl_str, filterVideo_str, filterAudio_str);
        delete p_filtering;
        LOGE("_______________end filter_______________%d", err);
    }
}


void native_ave_merge(JNIEnv *env, jobject *,
                      jobjectArray inUrls,
                      jstring outUrl) {
    jboolean jCopy = false;
    std::vector<std::string> inUrl_vec = std::vector<std::string>();
    for (size_t i = 0; i < env->GetArrayLength(inUrls); i++) {
        auto jString = (jstring) (env->GetObjectArrayElement(inUrls, (int) i));
        std::string inUrl_str = std::string((char *) env->GetStringUTFChars(jString, &jCopy));
        inUrl_vec.push_back(inUrl_str);
    }
    std::string outUrl_str = std::string((char *) env->GetStringUTFChars(outUrl, &jCopy));
    {
        LOGE("_______________start merge_______________");
        auto *p_merger = new merger();
        p_merger->merger_merge(inUrl_vec, outUrl_str);
        delete p_merger;
        LOGE("_______________end merge_______________");
    }
}

void native_ave_write_time_line(JNIEnv *env, jobject *, jstring inUrl, jstring outFolderUrl) {
    jboolean jCopy = false;
    std::string inUrl_str = std::string((char *) env->GetStringUTFChars(inUrl, &jCopy));
    std::string outFolderUrl_str = std::string(
            (char *) env->GetStringUTFChars(outFolderUrl, &jCopy));
    {
        LOGE("_______________start write time line_______________");
        timeline::write_time_line_jpg(inUrl_str, outFolderUrl_str, 5);
        LOGE("_______________end write time line_________________");
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
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                (void *) native_ave_filter
        },
        {
                "native_ave_merge",
                "([Ljava/lang/Object;Ljava/lang/String;)V",
                (void *) native_ave_merge
        },
        {
                "native_ave_write_time_line",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                (void *) native_ave_write_time_line
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