#include "../source/camera_adapter.h"
#include <stdio.h>
int main(void){double a=123; if(mr_camera_bind()||mr_camera_angle(&a)||a!=123)return 1;puts("PASS: unrelated host rejected; no camera access permitted.");return 0;}
