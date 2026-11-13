#ifndef MR_APT_BRIDGE_H
#define MR_APT_BRIDGE_H
/* UI thread only. root_path must identify the radar movie, not the full HUD. */
int mr_apt_bind(void);
int mr_apt_panel(unsigned level,const char *root_path,double width,double height,double angle);

int mr_apt_panel_rect(unsigned level,const char *root_path,double x,double y,double width,double height,double angle);

#endif
