/**
 *  @file        BeeSDK.java
 *  @brief       BeeSDK通用接口声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;
import android.os.Build;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

import org.webrtc.EglBase;
import org.webrtc.Logging;
import org.webrtc.NetworkMonitor;

import java.util.Vector;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import static com.sohu.tv.bee.BeeSystemFuncion.toBeeErrorCode;

/// BeeSDK通用接口类.
public class BeeSDK {
    private final static String TAG = BeeSDK.class.getSimpleName();
    private static volatile BeeSDK beeSDK = null;
    private final static ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();
    private static boolean beeInitialized = false;
    private EglBase rootEglBase = null;
    private EglBase encEglbase = null;
    private EglBase decEglbase = null;

    static { System.loadLibrary("bee");}

    private BeeSDK() {}

    public static ScheduledExecutorService getExecutor() {
        return BeeSDK.executor;
    }

    /**
     *  @brief  单例方法.
     *  @return BeeSDK全局唯一单例对象.
     */
    public static BeeSDK sharedInstance() {
        if (beeSDK == null) {
            synchronized(BeeSDK.class) {
                if (beeSDK == null) {
                    beeSDK = new BeeSDK();
                }
            }
        }
        return beeSDK;
    }

    /**
     *  @brief  获取eglbase，内部使用.
     *  @return eglbase对象.
     */
    public EglBase getRootEglBase() {
        return rootEglBase;
    }

    /**
     * @brief 初始化BeeSDK.
     * @param context    android上下文.
     * @param beeParam   初始化参数.
     * @param timeout    初始化方法的超时时间，单位ms.
     * @param beeSDKSink 接收通知的对象.
     * @param handler    初始化结果异步回调.
     * @note  必须在调用其他API前调用本方法，本方法为异步，必须传入handler才能获得异步调用结果.
     */
    public void init(final Context context, final BeeSystemParam beeParam, final int timeout, final BeeSDKSink beeSDKSink, final BeeAsyncHandler handler) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                initialize(context, beeParam, timeout, beeSDKSink, handler);
            }
        });
    }

    /**
     *  @brief  反初始化BeeSDK，释放资源.
     *  @param  handler     反初始化结果异步回调.
     *  @note   本方法为异步，必须传入handler才能获得异步调用结果.
     */
    public void unit(final BeeAsyncHandler handler) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                unInitialize(handler);
            }
        });
    }

    /**
     *  @brief  打开一个会话，每个业务都必须绑定到一个会话中.
     *  @param  handler     打开会话结果异步回调.
     *  @note   本方法为异步，必须传入handler才能获得异步调用结果.
     */
    public void openSession(final BeeAsyncHandler handler) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                openSessionInternal(handler);
            }
        });
    }

    /**
     *  @brief  关闭一个会话，关闭会话后，会话上绑定的所有业务接口不再可用.
     *  @param  handle      会话句柄.
     *  @param  handler     关闭会话结果异步回调.
     *  @note   本方法为异步，必须传入handler才能获得异步调用结果.
     */
    public void closeSession(final int handle, final BeeAsyncHandler handler) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                closeSessionInternal(handle, handler);
            }
        });
    }

    /**
     *  @brief  注册网络变化状态回调.
     *  @param  observer     监听网络状态.
     */
    public void registerNetMonitor(final NetworkMonitor.NetworkObserver observer) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (NetworkMonitor.isInitialized()) {
                    NetworkMonitor.addNetworkObserver(observer);
                }
            }
        });
    }

    /**
     *  @brief  反注册网络变化状态回调.
     *  @param  observer     监听网络状态.
     */
    public void unRegisterNetMonitor(final NetworkMonitor.NetworkObserver observer) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (NetworkMonitor.isInitialized()) {
                    NetworkMonitor.removeNetworkObserver(observer);
                }
            }
        });
    }

    private void initialize(Context context, BeeSystemParam beeParam, int timeout, BeeSDKSink sink, BeeAsyncHandler beeAsyncHandler) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (beeInitialized) {
                break;
            }

            Logging.d(TAG, "Initializing BeeLib.");
            NetworkMonitor.init(context);

            ret = toBeeErrorCode(nativeBeeSyncInit(context, beeParam, timeout, sink));
            if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                Logging.e(TAG, "BeeLib initialize failed, error code " + ret + ".");
                break;
            }

            // must be after nativeBeeSyncInit.
            eglBaseCreate();

            beeInitialized = true;
            Logging.d(TAG, "Initialize BeeLib success.");
        } while (false);

        if (beeAsyncHandler != null) {
            beeAsyncHandler.InitHandler(ret);
        }
    }

    private void unInitialize(BeeAsyncHandler beeAsyncHandler) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (!beeInitialized) {
                Logging.d(TAG, "BeeLib not initialized.");
                break;
            }

            elgBaseDispose();

            Logging.d(TAG, "UnInitializing BeeLib.");
            ret = toBeeErrorCode(nativeBeeSyncUninit());
            if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                Logging.e(TAG, "BeeLib unInitialize failed, error code " + ret + ".");
                break;
            }
        } while (false);

        if (beeAsyncHandler != null) {
            beeAsyncHandler.UnInitHandler(ret);
        }
    }

    private void openSessionInternal(BeeAsyncHandler beeAsyncHandler) {
        int handle = -1;
        Vector<BeeOpenSessionParam.BeeSDKCapability> capabilitiesInternal = new Vector<>();

        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (!beeInitialized) {
                Logging.d(TAG, "BeeLib not initialized.");
                break;
            }

            NetworkMonitor.setAutoDetectConnectivityState(true);

            BeeOpenSessionParam beeOpenSessionParam = nativeBeeSyncOpenSession();
            ret = toBeeErrorCode(beeOpenSessionParam.getErrorCode());
            if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                Logging.e(TAG, "openSession failed, error code " + ret + ".");
                break;
            }

            handle = beeOpenSessionParam.getHandle();
            if (handle == -1) {
                ret = BeeErrorCode.kBeeErrorCode_Session_Not_Opened;
                break;
            }

            capabilitiesInternal = beeOpenSessionParam.getCapabilities();
            if (capabilitiesInternal.isEmpty()) {
                ret = BeeErrorCode.kBeeErrorCode_Not_Implemented;
                break;
            }

        } while (false);

        if (beeAsyncHandler != null) {
            beeAsyncHandler.OpenSessionHandler(ret, handle, capabilitiesInternal);
        }

    }

    private void closeSessionInternal(int handle, BeeAsyncHandler beeAsyncHandler) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (!beeInitialized) {
                Logging.d(TAG, "BeeLib not initialized.");
                break;
            }

            NetworkMonitor.setAutoDetectConnectivityState(false);

            ret = toBeeErrorCode(nativeBeeSyncCloseSession(handle));
            if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                Logging.e(TAG, "BeeLib CloseSession failed, error code " + ret + ".");
                break;
            }
        } while (false);

        if (beeAsyncHandler != null) {
            beeAsyncHandler.CloseSessionHandler(ret);
        }
    }

    private void eglBaseCreate() {
        this.rootEglBase = EglBase.create();
        //For render and codec.
        setVideoHwAccelerationOptions(rootEglBase.getEglBaseContext(), rootEglBase.getEglBaseContext());
    }

    private void elgBaseDispose() {
        if (encEglbase != null) {
            encEglbase.release();
            encEglbase = null;
        }

        if (decEglbase != null) {
            decEglbase.release();
            decEglbase = null;
        }

        if (rootEglBase != null) {
            rootEglBase.release();
            rootEglBase = null;
        }
    }

    private void setVideoHwAccelerationOptions(EglBase.Context encEglContext, EglBase.Context decEglContext) {
        if (encEglbase != null) {
            Logging.w(TAG, "Egl context already set.");
            encEglbase.release();
            encEglbase = null;
        }

        if (decEglbase != null) {
            Logging.w(TAG, "Egl context already set.");
            decEglbase.release();
            decEglbase = null;
        }

        if (encEglContext != null) {
            encEglbase = EglBase.create(encEglContext);
        }

        if (decEglContext != null) {
            decEglbase = EglBase.create(decEglContext);
        }

        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
            //Render to texture won't work correctly in some platform, so fallback to
            //I420Frame mode manually by return null for now. 2018/02/28.
            nativeSetCodecEglContext(encEglbase.getEglBaseContext(), null);
        } else {
            nativeSetCodecEglContext(encEglbase.getEglBaseContext(), decEglbase.getEglBaseContext());
        }

    }

    public static String errorToString(BeeErrorCode error) {
        return "";
    }

    private static native int nativeBeeSyncInit(Context context, BeeSystemParam param, int timeout, BeeSDKSink sink);
    private static native int nativeBeeSyncUninit();
    private static native BeeOpenSessionParam nativeBeeSyncOpenSession();
    private static native int nativeBeeSyncCloseSession(int handle);
    private static native int nativeSetCodecEglContext(EglBase.Context encEglBase, EglBase.Context decEglBase);
}