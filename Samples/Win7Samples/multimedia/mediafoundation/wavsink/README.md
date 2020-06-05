---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: WavSink sample
urlFragment: wavSink-sample
description: Demonstrates how to write a custom media sink in Media Foundation.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# WavSink sample

This sample implements a media sink that writes audio .wav files. It demonstrates how to write a custom media sink in Media Foundation. It also shows how to use work queues to implement asynchronous methods.

The sample contains two project files:

- *WavSink*: Implements the media sink.
- *WriteWavSink*: Console application that writes a *.wav* file from a WMA or MP3 audio file.

This sample does not support protected content.

This sample requires Windows Vista or later.

 
## Implementation Notes

The media sink is a rateless archiving sink with a fixed number of streams (one stream). Most of the work is done by the stream sink. Asynchronous operations are performed using a Media Foundation work queue.

The media sink supports the following interfaces:

- `IMFMediaSink`: Required for all media sinks.
- `IMFClockStateSink`: Required for all media sinks. Notifies the media sink when the presentation changes state.
- `IMFFinalizableMediaSink`: Used to complete the archiving operation. In this sample, this interface is used to write the RIFF file header.

The stream sink supports the following interfaces:

- `IMFStreamSink`: Required for all stream sinks.
- `IMFMediaEvent Generator`: Inherited through `IMFStreamSink`.
- `IMFMediaTypeHandler`: Enables the `IMFStreamSink::GetMediaTypeHandler` method to return a pointer to the stream object itself, instead of creating a separate helper object.

Asynchronous operations are implemented as follows:
    
1. Create a `CAsyncOperation` object. This is a helper object that stores the operation type plus a VARIANT for additional data.
    
1. Call `MFPutWorkItem` to put a work item on the work queue. The `CAsyncOperation` object is passed in as the state object.

1. The work queue thread calls the stream sink's `OnDispatchWorkQueue` method. This method performs the asnchronous operation.

## Markers

When `PlaceMarker` is called, the stream sink sends an `MEStreamSinkMarker` event *after* it has processed all of the samples that it received prior to the marker. 

To handle this, the stream sink places `ProcessSample` operations and `PlaceMarker` operations on a queue. Whenever the asynchronous `BeginWrite` method completes, the stream sink pulls the associated `ProcessSample` operation from the queue. Then it pulls the next batch of `PlaceMarker` operations from the queue (if any) and sends the marker events.

For example, theoretically the queue could look like this, although this is an unlikely case:

```Sample, Marker, Marker, Sample, Sample, Marker```

When the first sample is done, the stream sink sends the next two marker events. The `Flush` method pulls *all* of the marker operationss from the queue at once and sends marker events with `hr = E_ABORT`.


THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.