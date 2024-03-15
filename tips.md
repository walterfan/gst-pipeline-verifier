
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


## GstPadProbeReturn 
Different return values for the GstPadProbeCallback.

### Members

* GST_PAD_PROBE_DROP (0) – drop data in data probes. For push mode this means that the data item is not sent downstream. For pull mode, it means that the data item is not passed upstream. In both cases, no other probes are called for this item and GST_FLOW_OK or TRUE is returned to the caller.
* GST_PAD_PROBE_OK (1) – normal probe return value. This leaves the probe in place, and defers decisions about dropping or passing data to other probes, if any. If there are no other probes, the default behaviour for the probe type applies ('block' for blocking probes, and 'pass' for non-blocking probes).
* GST_PAD_PROBE_REMOVE (2) – remove this probe, passing the data. For blocking probes this will cause data flow to unblock, unless there are also other blocking probes installed.
* GST_PAD_PROBE_PASS (3) – pass the data item in the block probe and block on the next item. Note, that if there are multiple pad probes installed and any probe returns PASS, the data will be passed.
* GST_PAD_PROBE_HANDLED (4) – Data has been handled in the probe and will not be forwarded further. For events and buffers this is the same behaviour as GST_PAD_PROBE_DROP (except that in this case you need to unref the buffer or event yourself). For queries it will also return TRUE to the caller. The probe can also modify the GstFlowReturn value by using the GST_PAD_PROBE_INFO_FLOW_RETURN() accessor. Note that the resulting query must contain valid entries. Since: 1.6
