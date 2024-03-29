
## GstPadDirection 
The direction of a pad.

### Members
* GST_PAD_UNKNOWN (0) – direction is unknown.
* GST_PAD_SRC (1) – the pad is a source pad.
* GST_PAD_SINK (2) – the pad is a sink pad.


## GstPadProbeType 

The different probing types that can occur. When either one of GST_PAD_PROBE_TYPE_IDLE or GST_PAD_PROBE_TYPE_BLOCK is used, the probe will be a blocking probe.

### Members
* GST_PAD_PROBE_TYPE_INVALID (0) – invalid probe type
* GST_PAD_PROBE_TYPE_IDLE (1) – probe idle pads and block while the callback is called
* GST_PAD_PROBE_TYPE_BLOCK (2) – probe and block pads
* GST_PAD_PROBE_TYPE_BUFFER (16) – probe buffers
* GST_PAD_PROBE_TYPE_BUFFER_LIST (32) – probe buffer lists
* GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM (64) – probe downstream events
* GST_PAD_PROBE_TYPE_EVENT_UPSTREAM (128) – probe upstream events
* GST_PAD_PROBE_TYPE_EVENT_FLUSH (256) – probe flush events. This probe has to be explicitly enabled and is not included in the @GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM or @GST_PAD_PROBE_TYPE_EVENT_UPSTREAM probe types.
* GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM (512) – probe downstream queries
* GST_PAD_PROBE_TYPE_QUERY_UPSTREAM (1024) – probe upstream queries
* GST_PAD_PROBE_TYPE_PUSH (4096) – probe push
* GST_PAD_PROBE_TYPE_PULL (8192) – probe pull
* GST_PAD_PROBE_TYPE_BLOCKING (3) – probe and block at the next opportunity, at data flow or when idle
* GST_PAD_PROBE_TYPE_DATA_DOWNSTREAM (112) – probe downstream data (buffers, buffer lists, and events)
* GST_PAD_PROBE_TYPE_DATA_UPSTREAM (128) – probe upstream data (events)
* GST_PAD_PROBE_TYPE_DATA_BOTH (240) – probe upstream and downstream data (buffers, buffer lists, and events)
* GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM (114) – probe and block downstream data (buffers, buffer lists, and events)
* GST_PAD_PROBE_TYPE_BLOCK_UPSTREAM (130) – probe and block upstream data (events)
* GST_PAD_PROBE_TYPE_EVENT_BOTH (192) – probe upstream and downstream events
* GST_PAD_PROBE_TYPE_QUERY_BOTH (1536) – probe upstream and downstream queries
* GST_PAD_PROBE_TYPE_ALL_BOTH (1776) – probe upstream events and queries and downstream buffers, buffer lists, events and queries
* GST_PAD_PROBE_TYPE_SCHEDULING (12288) – probe push and pull


## GstPadPresence 
Indicates when this pad will become available.

### Members
* GST_PAD_ALWAYS (0) – the pad is always available
* GST_PAD_SOMETIMES (1) – the pad will become available depending on the media stream
* GST_PAD_REQUEST (2) – the pad is only available on request with gst_element_request_pad.

## GstPadProbeReturn 
Different return values for the GstPadProbeCallback.

### Members

* GST_PAD_PROBE_DROP (0) – drop data in data probes. For push mode this means that the data item is not sent downstream. For pull mode, it means that the data item is not passed upstream. In both cases, no other probes are called for this item and GST_FLOW_OK or TRUE is returned to the caller.
* GST_PAD_PROBE_OK (1) – normal probe return value. This leaves the probe in place, and defers decisions about dropping or passing data to other probes, if any. If there are no other probes, the default behaviour for the probe type applies ('block' for blocking probes, and 'pass' for non-blocking probes).
* GST_PAD_PROBE_REMOVE (2) – remove this probe, passing the data. For blocking probes this will cause data flow to unblock, unless there are also other blocking probes installed.
* GST_PAD_PROBE_PASS (3) – pass the data item in the block probe and block on the next item. Note, that if there are multiple pad probes installed and any probe returns PASS, the data will be passed.
* GST_PAD_PROBE_HANDLED (4) – Data has been handled in the probe and will not be forwarded further. For events and buffers this is the same behaviour as GST_PAD_PROBE_DROP (except that in this case you need to unref the buffer or event yourself). For queries it will also return TRUE to the caller. The probe can also modify the GstFlowReturn value by using the GST_PAD_PROBE_INFO_FLOW_RETURN() accessor. Note that the resulting query must contain valid entries. Since: 1.6


## GstEventType 
GstEventType lists the standard event types that can be sent in a pipeline.

The custom event types can be used for private messages between elements that can't be expressed using normal GStreamer buffer passing semantics. Custom events carry an arbitrary GstStructure. Specific custom events are distinguished by the name of the structure.

### Members

* GST_EVENT_UNKNOWN (0) – unknown event.
* GST_EVENT_FLUSH_START (2563) – Start a flush operation. This event clears all data from the pipeline and unblock all streaming threads.
* GST_EVENT_FLUSH_STOP (5127) – Stop a flush operation. This event resets the running-time of the pipeline.
* GST_EVENT_STREAM_START (10254) – Event to mark the start of a new stream. Sent before any other serialized event and only sent at the start of a new stream, not after flushing seeks.
* GST_EVENT_CAPS (12814) – GstCaps event. Notify the pad of a new media type.
* GST_EVENT_SEGMENT (17934) – A new media segment follows in the dataflow. The segment events contains information for clipping buffers and converting buffer timestamps to running-time and stream-time.
* GST_EVENT_STREAM_COLLECTION (19230) – A new GstStreamCollection is available (Since: 1.10)
* GST_EVENT_TAG (20510) – A new set of metadata tags has been found in the stream.
* GST_EVENT_BUFFERSIZE (23054) – Notification of buffering requirements. Currently not used yet.
* GST_EVENT_SINK_MESSAGE (25630) – An event that sinks turn into a message. Used to send messages that should be emitted in sync with rendering.
* GST_EVENT_STREAM_GROUP_DONE (26894) – Indicates that there is no more data for the stream group ID in the message. Sent before EOS in some instances and should be handled mostly the same. (Since: 1.10)
* GST_EVENT_EOS (28174) – End-Of-Stream. No more data is to be expected to follow without either a STREAM_START event, or a FLUSH_STOP and a SEGMENT event.
* GST_EVENT_TOC (30750) – An event which indicates that a new table of contents (TOC) was found or updated.
* GST_EVENT_PROTECTION (33310) – An event which indicates that new or updated encryption information has been found in the stream.
* GST_EVENT_SEGMENT_DONE (38406) – Marks the end of a segment playback.
* GST_EVENT_GAP (40966) – Marks a gap in the datastream.
* GST_EVENT_INSTANT_RATE_CHANGE (46090) – Notify downstream that a playback rate override should be applied as soon as possible. (Since: 1.18)
* GST_EVENT_QOS (48641) – A quality message. Used to indicate to upstream elements that the downstream elements should adjust their processing rate.
* GST_EVENT_SEEK (51201) – A request for a new playback position and rate.
* GST_EVENT_NAVIGATION (53761) – Navigation events are usually used for communicating user requests, such as mouse or keyboard movements, to upstream elements.
* GST_EVENT_LATENCY (56321) – Notification of new latency adjustment. Sinks will use the latency information to adjust their synchronisation.
* GST_EVENT_STEP (58881) – A request for stepping through the media. Sinks will usually execute the step operation.
* GST_EVENT_RECONFIGURE (61441) – A request for upstream renegotiating caps and reconfiguring.
* GST_EVENT_TOC_SELECT (64001) – A request for a new playback position based on TOC entry's UID.
* GST_EVENT_SELECT_STREAMS (66561) – A request to select one or more streams (Since: 1.10)
* GST_EVENT_INSTANT_RATE_SYNC_TIME (66817) – Sent by the pipeline to notify elements that handle the instant-rate-change event about the running-time when the rate multiplier should be applied (or was applied). (Since: 1.18)
* GST_EVENT_CUSTOM_UPSTREAM (69121) – Upstream custom event
* GST_EVENT_CUSTOM_DOWNSTREAM (71686) – Downstream custom event that travels in the data flow.
* GST_EVENT_CUSTOM_DOWNSTREAM_OOB (74242) – Custom out-of-band downstream event.
* GST_EVENT_CUSTOM_DOWNSTREAM_STICKY (76830) – Custom sticky downstream event.
* GST_EVENT_CUSTOM_BOTH (79367) – Custom upstream or downstream event. In-band when travelling downstream.
* GST_EVENT_CUSTOM_BOTH_OOB (81923) – Custom upstream or downstream out-of-band event.

### reference
* https://gstreamer.freedesktop.org/documentation/gstreamer/gstevent.html?gi-language=c#GST_EVENT_STREAM_START


## GstEventTypeFlags 
GstEventTypeFlags indicate the aspects of the different GstEventType values. You can get the type flags of a GstEventType with the gst_event_type_get_flags function.

### Members
* GST_EVENT_TYPE_UPSTREAM (1) – Set if the event can travel upstream.
* GST_EVENT_TYPE_DOWNSTREAM (2) – Set if the event can travel downstream.
* GST_EVENT_TYPE_SERIALIZED (4) – Set if the event should be serialized with data flow.
* GST_EVENT_TYPE_STICKY (8) – Set if the event is sticky on the pads.
* GST_EVENT_TYPE_STICKY_MULTI (16) – Multiple sticky events can be on a pad, each identified by the event name.


## GstQueryType 
Standard predefined Query types

### Members

* GST_QUERY_UNKNOWN (0) – unknown query type
* GST_QUERY_POSITION (2563) – current position in stream
* GST_QUERY_DURATION (5123) – total duration of the stream
* GST_QUERY_LATENCY (7683) – latency of stream
* GST_QUERY_JITTER (10243) – current jitter of stream
* GST_QUERY_RATE (12803) – current rate of the stream
* GST_QUERY_SEEKING (15363) – seeking capabilities
* GST_QUERY_SEGMENT (17923) – segment start/stop positions
* GST_QUERY_CONVERT (20483) – convert values between formats
* GST_QUERY_FORMATS (23043) – query supported formats for convert
* GST_QUERY_BUFFERING (28163) – query available media for efficient seeking.
* GST_QUERY_CUSTOM (30723) – a custom application or element defined query.
* GST_QUERY_URI (33283) – query the URI of the source or sink.
* GST_QUERY_ALLOCATION (35846) – the buffer allocation properties
* GST_QUERY_SCHEDULING (38401) – the scheduling properties
* GST_QUERY_ACCEPT_CAPS (40963) – the accept caps query
* GST_QUERY_CAPS (43523) – the caps query
* GST_QUERY_DRAIN (46086) – wait till all serialized data is consumed downstream
* GST_QUERY_CONTEXT (48643) – query the pipeline-local context from downstream or upstream (since 1.2)
* GST_QUERY_BITRATE (51202) – the bitrate query (since 1.16)
* GST_QUERY_SELECTABLE (53763) – Query stream selection capability. (Since: 1.22)