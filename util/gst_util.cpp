#include "gst_util.h"

static const std::string BLANK_STR = "";
static const std::string DOT_STR = ".";


namespace wfan {


GstPadPresence get_one_pad_presence(const char* factory_name, GstPadDirection direction) {
  const GList *pads;
  GstStaticPadTemplate *padtemplate;

  GstElementFactory* factory = gst_element_factory_find (factory_name);
  if (!factory) {
    g_print ("cannot find the factory\n");
    return GST_PAD_ALWAYS;
  }
  if (!gst_element_factory_get_num_pad_templates (factory)) {
    g_print ("no pad templates of the factory\n");
    return GST_PAD_ALWAYS;
  }

  pads = gst_element_factory_get_static_pad_templates (factory);
  while (pads) {
    padtemplate = (GstStaticPadTemplate *)pads->data;
    pads = g_list_next (pads);
    //padtemplate->name_template
    if (padtemplate->direction == direction) {
      return padtemplate->presence;
    }
  }
  return GST_PAD_ALWAYS;
}


GstCaps* get_default_caps() 
{
    
    GstCaps *caps = gst_caps_new_full (
      gst_structure_new ("video/x-raw",
            "format", G_TYPE_STRING, "I420",
             "width", G_TYPE_INT, 320,
             "height", G_TYPE_INT, 240,
             "framerate", GST_TYPE_FRACTION, 30, 1,
             NULL),
      gst_structure_new ("video/x-raw",
             "format", G_TYPE_STRING, "NV12",
             "width", G_TYPE_INT, 320,
             "height", G_TYPE_INT, 240,
             "framerate", GST_TYPE_FRACTION, 30, 1,
             NULL),
      NULL);
    
    return caps;
}

gboolean link_elements_with_filter (GstElement *element1, GstElement *element2)
{
  gboolean link_ok;
  GstCaps *caps = get_default_caps();

  link_ok = gst_element_link_filtered (element1, element2, caps);
  gst_caps_unref (caps);

  if (!link_ok) {
    g_warning ("Failed to link element1 and element2!");
  }

  return link_ok;
}


/* Functions below print the Capabilities in a human-friendly format */
gboolean print_field (GQuark field, const GValue * value, gpointer pfx) 
{
  gchar *str = gst_value_serialize (value);

  gst_println("%s  %s: %s\n", (gchar *) pfx, g_quark_to_string (field), str);
  g_free (str);
  return TRUE;
}

void print_caps (const GstCaps * caps, const gchar * pfx) 
{
  guint i;

  g_return_if_fail (caps != NULL);

  if (gst_caps_is_any (caps)) {
    gst_println("%sANY\n", pfx);
    return;
  }
  if (gst_caps_is_empty (caps)) {
    gst_println("%sEMPTY\n", pfx);
    return;
  }

  for (i = 0; i < gst_caps_get_size (caps); i++) {
    GstStructure *structure = gst_caps_get_structure (caps, i);

    gst_println("%s %s\n", pfx, gst_structure_get_name (structure));
    gst_structure_foreach (structure, print_field, (gpointer) pfx);
  }
}

void print_pad_capabilities (GstPad *pad, gchar *pad_name) 
{

  GstCaps *caps = NULL;

  /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
  caps = gst_pad_get_current_caps (pad);
  if (!caps)
    caps = gst_pad_query_caps (pad, NULL);

  /* Print and free */
  gst_println("Caps for the %s pad:\n", pad_name);
  print_caps (caps, "      ");
  gst_caps_unref (caps);

}

void check_pads(GstElement *element) {
    GstIterator *iter = gst_element_iterate_pads(element);
    GValue *elem;
    
    while (gst_iterator_next(iter, elem) == GST_ITERATOR_OK) {
        gchar * strVal = g_strdup_value_contents (elem);
        gst_println("pad: %s", strVal);
        free (strVal);
    }
    gst_iterator_free(iter);
}

/* usage of g_log and log handler:
uint handlerid = g_log_set_handler(NULL,
    G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
    log_handler,
    NULL);

if (!g_main_loop_is_running())
{
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "g_main_loop_is_running() returned 0\n");
}

if (handlerid != 0)
{
    g_log_remove_handler(NULL, handlerid);
    handlerid = 0;
}
*/
void log_handler(const gchar *log_domain,
                 GLogLevelFlags log_level,
                 const gchar *message,
                 gpointer user_data)
{
    FILE *logfile = fopen ("/tmp/debug.log", "a");
    if (logfile == NULL)
    {
        /*  Fall  back  to  console  output  if  unable  to  open  file  */
        printf ("Rerouted to console: %s", message);
        return;
    }

    fprintf (logfile, "%s", message);
    fclose (logfile);
}



/* Prints information about a Pad Template, including its Capabilities */
void print_pad_templates_information (GstElementFactory * factory) {
  const GList *pads;
  GstStaticPadTemplate *padtemplate;

  g_print ("Pad Templates for %s:\n", gst_element_factory_get_longname (factory));
  if (!gst_element_factory_get_num_pad_templates (factory)) {
    g_print ("  none\n");
    return;
  }

  pads = gst_element_factory_get_static_pad_templates (factory);
  while (pads) {
    padtemplate = (GstStaticPadTemplate *)pads->data;
    pads = g_list_next (pads);

    if (padtemplate->direction == GST_PAD_SRC)
      g_print ("  SRC template: '%s'\n", padtemplate->name_template);
    else if (padtemplate->direction == GST_PAD_SINK)
      g_print ("  SINK template: '%s'\n", padtemplate->name_template);
    else
      g_print ("  UNKNOWN!!! template: '%s'\n", padtemplate->name_template);

    if (padtemplate->presence == GST_PAD_ALWAYS)
      g_print ("    Availability: Always\n");
    else if (padtemplate->presence == GST_PAD_SOMETIMES)
      g_print ("    Availability: Sometimes\n");
    else if (padtemplate->presence == GST_PAD_REQUEST) {
      g_print ("    Availability: On request\n");
    } else
      g_print ("    Availability: UNKNOWN!!!\n");

    if (padtemplate->static_caps.string) {
      GstCaps *caps;
      g_print ("    Capabilities:\n");
      caps = gst_static_caps_get (&padtemplate->static_caps);
      print_caps (caps, "      ");
      gst_caps_unref (caps);

    }

    g_print ("\n");
  }
}

void print_pad_templates_info(const char* factory_name) {
  GstElementFactory* factory = gst_element_factory_find (factory_name);
  print_pad_templates_information(factory);
  gst_object_unref (factory);

}

std::string get_pad_name_from_link_tag(const std::string& link_tag) {
  size_t len = link_tag.size();
  if(len < 2) return BLANK_STR;
  
  std::string::size_type pos = link_tag.find(DOT_STR, 0);
  if (pos == std::string::npos) return BLANK_STR;
  if (pos == len - 1 ) return BLANK_STR;

  return link_tag.substr(pos + 1, std::string::npos);
}

bool is_probe_event(GstPadProbeInfo *info) {
  return GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM & info->type 
    || GST_PAD_PROBE_TYPE_EVENT_UPSTREAM & info->type;
}

bool is_probe_query(GstPadProbeInfo *info) {
  return GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM & info->type 
    || GST_PAD_PROBE_TYPE_QUERY_UPSTREAM & info->type;
}

bool is_probe_buffer(GstPadProbeInfo *info) {
  return GST_PAD_PROBE_TYPE_BUFFER & info->type 
    || GST_PAD_PROBE_TYPE_BUFFER_LIST & info->type;
}



}//namespace wfan