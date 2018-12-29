/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtc;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.EnumSet;
import java.util.logging.Level;
import java.util.logging.Logger;

/** Java wrapper for WebRTC logging. */
public class Logging {
  private static final Logger fallbackLogger = Logger.getLogger("org.webrtc.Logging");
  private static volatile boolean tracingEnabled;
  private static volatile boolean loggingEnabled;
  private static volatile boolean nativeLibLoaded;

  static {
      fallbackLogger.setLevel(Level.ALL);
  }

  // Keep in sync with webrtc/common_types.h:TraceLevel.
  public enum TraceLevel {
    TRACE_NONE(0x0000),
    TRACE_STATEINFO(0x0001),
    TRACE_WARNING(0x0002),
    TRACE_ERROR(0x0004),
    TRACE_CRITICAL(0x0008),
    TRACE_APICALL(0x0010),
    TRACE_DEFAULT(0x00ff),
    TRACE_MODULECALL(0x0020),
    TRACE_MEMORY(0x0100),
    TRACE_TIMER(0x0200),
    TRACE_STREAM(0x0400),
    TRACE_DEBUG(0x0800),
    TRACE_INFO(0x1000),
    TRACE_TERSEINFO(0x2000),
    TRACE_ALL(0xffff);

    public final int level;
    TraceLevel(int level) {
      this.level = level;
    }
  }

  // Keep in sync with webrtc/base/logging.h:LoggingSeverity.
  public enum Severity { LS_SENSITIVE, LS_VERBOSE, LS_INFO, LS_WARNING, LS_ERROR, LS_NONE }

  public static void enableLogThreads() {

  }

  public static void enableLogTimeStamps() {

  }

  // Enable tracing to |path| of messages of |levels|.
  // On Android, use "logcat:" for |path| to send output there.
  // Note: this function controls the output of the WEBRTC_TRACE() macros.
  public static synchronized void enableTracing(String path, EnumSet<TraceLevel> levels) {

  }

  // Enable diagnostic logging for messages of |severity| to the platform debug
  // output. On Android, the output will be directed to Logcat.
  // Note: this function starts collecting the output of the LOG() macros.
  public static synchronized void enableLogToDebugOutput(Severity severity) {

  }

  public static void log(Severity severity, String tag, String message) {
    // Fallback to system log.
    Level level;
    switch (severity) {
      case LS_ERROR:
        level = Level.SEVERE;
        break;
      case LS_WARNING:
        level = Level.WARNING;
        break;
      case LS_INFO:
        level = Level.INFO;
        break;
      default:
        level = Level.FINE;
        break;
    }
    fallbackLogger.log(level, tag + ": " + message);
  }

  public static void d(String tag, String message) {
    log(Severity.LS_INFO, tag, message);
  }

  public static void e(String tag, String message) {
    log(Severity.LS_ERROR, tag, message);
  }

  public static void w(String tag, String message) {
    log(Severity.LS_WARNING, tag, message);
  }

  public static void e(String tag, String message, Throwable e) {
    log(Severity.LS_ERROR, tag, message);
    log(Severity.LS_ERROR, tag, e.toString());
    log(Severity.LS_ERROR, tag, getStackTraceString(e));
  }

  public static void w(String tag, String message, Throwable e) {
    log(Severity.LS_WARNING, tag, message);
    log(Severity.LS_WARNING, tag, e.toString());
    log(Severity.LS_WARNING, tag, getStackTraceString(e));
  }

  public static void v(String tag, String message) {
    log(Severity.LS_VERBOSE, tag, message);
  }

  private static String getStackTraceString(Throwable e) {
    if (e == null) {
      return "";
    }

    StringWriter sw = new StringWriter();
    PrintWriter pw = new PrintWriter(sw);
    e.printStackTrace(pw);
    return sw.toString();
  }
}
