/**
 * @file otter-serial-stub.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Defines pre-processor tokens which stand in for the actual otter-serial API.
 * @version 0.2.0
 * @date 2022-06-28
 * 
 * This file defines pre-processor tokens which stand in for the actual
 * otter-serial API, for use by users of sofware which includes Otter but does
 * not require it's users to install Otter.
 * 
 * THE FOLLOWING LICENSE TEXT APPLIES TO THIS FILE ONLY.
 * 
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. * 
 */

#if !defined(OTTER_SERIAL_STUB_H)
#define OTTER_SERIAL_STUB_H

#define otter_sync_children
#define otter_sync_descendants
#define OTTER_SRC_ARGS()

#define otterTraceInitialise()
#define otterTraceFinalise()
#define otterTraceStart()
#define otterTraceStop()
#define otterThreadsBegin()
#define otterThreadsEnd()
#define otterTaskBegin()
#define otterTaskEnd()
#define otterLoopBegin()
#define otterLoopEnd()
#define otterLoopIterationBegin()
#define otterLoopIterationEnd()
#define otterSynchroniseTasks()
#define otterSynchroniseDescendantTasksBegin()
#define otterSynchroniseDescendantTasksEnd()
#define otterPhaseBegin()
#define otterPhaseEnd()
#define otterPhaseSwitch()

#endif // OTTER_SERIAL_STUB_H
