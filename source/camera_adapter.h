#ifndef MR_CAMERA_ADAPTER_H
#define MR_CAMERA_ADAPTER_H
/* Call only on the game's UI/render thread during an active match.
   This module never patches memory or installs hooks. */
int mr_camera_bind(void);
int mr_camera_angle(double *screen_rotation_radians);
#endif
