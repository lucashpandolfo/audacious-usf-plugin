#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <libaudcore/plugin.h>
#include <libaudcore/i18n.h>
#include <libaudcore/preferences.h>

class USFPlugin : public InputPlugin
{
    public:
    static const char * const exts[];
    static constexpr PluginInfo info = {
        N_("USF plugin. Adapted from the old USF audacious plugin."),
        ""
    };
    static constexpr auto iinfo = InputInfo ()
        .with_exts (exts);

    constexpr USFPlugin () : InputPlugin (info, iinfo) {}

    bool init ();
    void cleanup ();
    bool is_our_file (const char * filename, VFSFile & file);
    Tuple read_tuple (const char * filename, VFSFile & file);
    bool play (const char * filename, VFSFile & file);


    void open_sound();
    void add_buffer(unsigned char *buf, unsigned int length);
    void ai_len_changed();
    unsigned ai_read_length(void);
    void ai_dacrate_changed(unsigned int value);
};

extern bool usf_playing;
extern int32_t SampleRate;
extern int16_t samplebuf[16384];
extern USFPlugin* context;

#endif /* _PLUGIN_H_ */
