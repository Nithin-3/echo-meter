#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include "tool.h"
#include "conf.h"
#include "path.h"

static float clamp(float v){ return v<0?0:v>1?1:v; }

// ---------- Sysfs brightness ----------
static int catFile(const char *p,int *v){ 
    FILE *f=fopen(p,"r");
    if(!f) return 0;
    int r=fscanf(f,"%d",v);
    fclose(f);
    return r==1;
}
static float getBri(void){
    DIR *d=opendir("/sys/class/backlight/"); if(!d)return 0;
    struct dirent *dir; char path[128]; int max,crnt;
    while((dir=readdir(d))){
        if(strcmp(dir->d_name,".")==0||strcmp(dir->d_name,"..")==0) continue;
        snprintf(path,sizeof(path),"/sys/class/backlight/%s/max_brightness",dir->d_name);
        if(!catFile(path,&max)) continue;
        snprintf(path,sizeof(path),"/sys/class/backlight/%s/actual_brightness",dir->d_name);
        if(!catFile(path,&crnt)) continue;
        closedir(d); return (float)crnt/max;
    }
    closedir(d); return 0;
}
static int setBri(float level){
    level=clamp(level); DIR *d=opendir("/sys/class/backlight/"); if(!d)return 0;
    struct dirent *dir; char path[128]; int max,val;
    while((dir=readdir(d))){
        if(strcmp(dir->d_name,".")==0||strcmp(dir->d_name,"..")==0) continue;
        snprintf(path,sizeof(path),"/sys/class/backlight/%s/max_brightness",dir->d_name);
        if(!catFile(path,&max)) continue;
        val=(int)(level*max);
        snprintf(path,sizeof(path),"/usr/share/echo-meter/write-brightness %s %d",dir->d_name,val);
         if(!system(path)) continue;
        closedir(d); return 1;
    }
    closedir(d); return 0;
}

// ---------- ALSA helper ----------
static int mixerElem(int isPlay,int isGet,float *val,int isMute,int muteState){
    snd_mixer_t *h=NULL; snd_mixer_elem_t *e=NULL;
    if(snd_mixer_open(&h,0)<0)return 0;
    if(snd_mixer_attach(h,"default")<0){snd_mixer_close(h);return 0;}
    if(snd_mixer_selem_register(h,NULL,NULL)<0){snd_mixer_close(h);return 0;}
    if(snd_mixer_load(h)<0){snd_mixer_close(h);return 0;}
    int ok=0;
    for(e=snd_mixer_first_elem(h);e;e=snd_mixer_elem_next(e)){
        if(!snd_mixer_selem_is_active(e)) continue;
        if(isGet && val){
            if(isPlay && snd_mixer_selem_has_playback_volume(e)){
                long min,max,v; snd_mixer_selem_get_playback_volume_range(e,&min,&max);
                snd_mixer_selem_get_playback_volume(e,SND_MIXER_SCHN_FRONT_LEFT,&v);
                *val=(float)(v-min)/(max-min); ok=1; break;
            } else if(!isPlay && snd_mixer_selem_has_capture_volume(e)){
                long min,max,v; snd_mixer_selem_get_capture_volume_range(e,&min,&max);
                snd_mixer_selem_get_capture_volume(e,SND_MIXER_SCHN_FRONT_LEFT,&v);
                *val=(float)(v-min)/(max-min); ok=1; break;
            }
        }
        if(!isGet && val){
            long min,max,v;
            if(isPlay && snd_mixer_selem_has_playback_volume(e)){
                snd_mixer_selem_get_playback_volume_range(e,&min,&max);
                v = (long)(clamp(*val) * (max - min) + min);
                snd_mixer_selem_set_playback_volume(e,SND_MIXER_SCHN_FRONT_LEFT,v);
                snd_mixer_selem_set_playback_volume(e,SND_MIXER_SCHN_FRONT_RIGHT,v); ok=1; break;
            } else if(!isPlay && snd_mixer_selem_has_capture_volume(e)){
                snd_mixer_selem_get_capture_volume_range(e,&min,&max);
                v = (long)(clamp(*val) * (max - min) + min);
                snd_mixer_selem_set_capture_volume(e,SND_MIXER_SCHN_FRONT_LEFT,v);
                snd_mixer_selem_set_capture_volume(e,SND_MIXER_SCHN_FRONT_RIGHT,v); ok=1; break;
            }
        }
        if(isMute>=0){
            int sw=!muteState;
            if(isPlay && snd_mixer_selem_has_playback_switch(e)){
                snd_mixer_selem_set_playback_switch(e,SND_MIXER_SCHN_FRONT_LEFT,sw);
                snd_mixer_selem_set_playback_switch(e,SND_MIXER_SCHN_FRONT_RIGHT,sw); ok=1; break;
            } else if(!isPlay && snd_mixer_selem_has_capture_switch(e)){
                snd_mixer_selem_set_capture_switch(e,SND_MIXER_SCHN_FRONT_LEFT,sw);
                snd_mixer_selem_set_capture_switch(e,SND_MIXER_SCHN_FRONT_RIGHT,sw); ok=1; break;
            }
        }
    }
    snd_mixer_close(h);
    return ok;
}

// ---------- Public API ----------
float getVol(void){ float v=0; return mixerElem(1,1,&v,-1,0)?v:-1.0f; }
int setVol(float l){ return mixerElem(1,0,&l,-1,0); }
int getMute(void){ return mixerElem(1,1,NULL,0,0)?0:1; }
int setMute(int s){ return mixerElem(1,-1,NULL,1,s); }

float getMicVol(void){ float v=0; return mixerElem(0,1,&v,-1,0)?v:-1.0f; }
int setMicVol(float l){ return mixerElem(0,0,&l,-1,0); }
int getMicMute(void){ return mixerElem(0,1,NULL,0,0)?0:1; }
int setMicMute(int s){ return mixerElem(0,-1,NULL,1,s); }


static Sys *getCachedSys() {
    static Sys sys;
    static int loaded = 0;
    if (!loaded) {
        char *path = findPath("conf.json");
        if (path == NULL || !catSys(path, &sys)) {
            return NULL;
        }
        loaded = 1;
    }
    return &sys;
}

float getVal(const Type mode) {
    switch (mode) {
        case AUD: return getVol();
        case MIC: return getMicVol();
        case BRI: return getBri();
        default: return 0.0f;
    }
}

void setVal(const Type mode, float val) {
    val = clamp(val);
    switch (mode) {
        case AUD: setVol(val); break;
        case MIC: setMicVol(val); break;
        case BRI: setBri(val); break;
        default: break;
    }
}
void step(const Type mode, bool direction, float stepVal) {
    if (mode == INVALIDT) {
        return;
    }
    Sys *s = getCachedSys();
    if (!s) return;

    float current = getVal(mode);
    if (current < 0.0f) return;

    if (stepVal == 0.0) {
        switch (mode) {
            case AUD: stepVal = s->volumeStep; break;
            case BRI: stepVal = s->brightnessStep; break;
            case MIC: stepVal = s->micStep; break;
            default: return;
        }
    }
    float newVal = direction ? current + stepVal/100.0 : current - stepVal/100.0;
    setVal(mode, newVal);
}

