#pragma once
#define FCVAR_ARCHIVE 1
struct cvar_t { float value; };
struct event_args_s { float origin[3]; };
typedef float vec3_t[3];
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
