LOCAL_PATH := $(call my-dir)
#LOCAL_PATH := $(subst //,/,$(call my-dir))

include $(CLEAR_VARS)

LOCAL_MODULE	:= Gore
REAL_LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/Source/*.cpp)
LOCAL_SRC_FILES := $(subst jni/, , $(REAL_LOCAL_SRC_FILES))
#Main.cpp
#LOCAL_SRC_FILES	:= $(wildcard Source/*.cpp)

LOCAL_ARM_MODE	:= arm

LOCAL_LDLIBS			:= -lstdc++ -lc -llog -landroid -ldl -lGLESv2 -lEGL
LOCAL_STATIC_LIBRARIES	:= android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)

