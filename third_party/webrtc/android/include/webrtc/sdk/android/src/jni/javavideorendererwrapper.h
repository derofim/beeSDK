#ifndef __JAVA_VIDEO_RENDERER_WRAPPER_H__
#define __JAVA_VIDEO_RENDERER_WRAPPER_H__

#include "webrtc/sdk/android/src/jni/classreferenceholder.h"
#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include "webrtc/sdk/android/src/jni/native_handle_impl.h"

namespace webrtc_jni {

// Wrapper dispatching rtc::VideoSinkInterface to a Java VideoRenderer
// instance.
class JavaVideoRendererWrapper
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  JavaVideoRendererWrapper(JNIEnv* jni, jobject j_callbacks)
      : j_callbacks_(jni, j_callbacks),
        j_render_frame_id_(GetMethodID(
            jni, GetObjectClass(jni, j_callbacks), "renderFrame",
            "(Lorg/webrtc/VideoRenderer$I420Frame;)V")),
        j_frame_class_(jni,
                       FindClass(jni, "org/webrtc/VideoRenderer$I420Frame")),
        j_i420_frame_ctor_id_(GetMethodID(
            jni, *j_frame_class_, "<init>", "(III[I[Ljava/nio/ByteBuffer;J)V")),
        j_texture_frame_ctor_id_(GetMethodID(
            jni, *j_frame_class_, "<init>",
            "(IIII[FJ)V")),
        j_byte_buffer_class_(jni, FindClass(jni, "java/nio/ByteBuffer")) {
    CHECK_EXCEPTION(jni);
  }

  virtual ~JavaVideoRendererWrapper() {}

  void OnFrame(const webrtc::VideoFrame& video_frame) override {
    ScopedLocalRefFrame local_ref_frame(jni());
    jobject j_frame =
        (video_frame.video_frame_buffer()->native_handle() != nullptr)
            ? CricketToJavaTextureFrame(&video_frame)
            : CricketToJavaI420Frame(&video_frame);
    // |j_callbacks_| is responsible for releasing |j_frame| with
    // VideoRenderer.renderFrameDone().
    jni()->CallVoidMethod(*j_callbacks_, j_render_frame_id_, j_frame);
    CHECK_EXCEPTION(jni());
  }

 private:
  // Make a shallow copy of |frame| to be used with Java. The callee has
  // ownership of the frame, and the frame should be released with
  // VideoRenderer.releaseNativeFrame().
  static jlong javaShallowCopy(const webrtc::VideoFrame* frame) {
    return jlongFromPointer(new webrtc::VideoFrame(*frame));
  }

  // Return a VideoRenderer.I420Frame referring to the data in |frame|.
  jobject CricketToJavaI420Frame(const webrtc::VideoFrame* frame) {
    jintArray strides = jni()->NewIntArray(3);
    jint* strides_array = jni()->GetIntArrayElements(strides, NULL);
    strides_array[0] = frame->video_frame_buffer()->StrideY();
    strides_array[1] = frame->video_frame_buffer()->StrideU();
    strides_array[2] = frame->video_frame_buffer()->StrideV();
    jni()->ReleaseIntArrayElements(strides, strides_array, 0);
    jobjectArray planes = jni()->NewObjectArray(3, *j_byte_buffer_class_, NULL);
    jobject y_buffer = jni()->NewDirectByteBuffer(
        const_cast<uint8_t*>(frame->video_frame_buffer()->DataY()),
        frame->video_frame_buffer()->StrideY() *
            frame->video_frame_buffer()->height());
    size_t chroma_height = (frame->height() + 1) / 2;
    jobject u_buffer = jni()->NewDirectByteBuffer(
        const_cast<uint8_t*>(frame->video_frame_buffer()->DataU()),
        frame->video_frame_buffer()->StrideU() * chroma_height);
    jobject v_buffer = jni()->NewDirectByteBuffer(
        const_cast<uint8_t*>(frame->video_frame_buffer()->DataV()),
        frame->video_frame_buffer()->StrideV() * chroma_height);

    jni()->SetObjectArrayElement(planes, 0, y_buffer);
    jni()->SetObjectArrayElement(planes, 1, u_buffer);
    jni()->SetObjectArrayElement(planes, 2, v_buffer);
    return jni()->NewObject(
        *j_frame_class_, j_i420_frame_ctor_id_,
        frame->width(), frame->height(),
        static_cast<int>(frame->rotation()),
        strides, planes, javaShallowCopy(frame));
  }

  // Return a VideoRenderer.I420Frame referring texture object in |frame|.
  jobject CricketToJavaTextureFrame(const webrtc::VideoFrame* frame) {
    NativeHandleImpl* handle = reinterpret_cast<NativeHandleImpl*>(
        frame->video_frame_buffer()->native_handle());
    jfloatArray sampling_matrix = handle->sampling_matrix.ToJava(jni());

    return jni()->NewObject(
        *j_frame_class_, j_texture_frame_ctor_id_,
        frame->width(), frame->height(),
        static_cast<int>(frame->rotation()),
        handle->oes_texture_id, sampling_matrix, javaShallowCopy(frame));
  }

  JNIEnv* jni() {
    return AttachCurrentThreadIfNeeded();
  }

  ScopedGlobalRef<jobject> j_callbacks_;
  jmethodID j_render_frame_id_;
  ScopedGlobalRef<jclass> j_frame_class_;
  jmethodID j_i420_frame_ctor_id_;
  jmethodID j_texture_frame_ctor_id_;
  ScopedGlobalRef<jclass> j_byte_buffer_class_;
};

} //namespace webrtc_jni

#endif //__JAVA_VIDEO_RENDERER_WRAPPER_H__


