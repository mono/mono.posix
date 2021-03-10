#ifndef NL_H
#define NL_H
#include <glib.h>

G_BEGIN_DECLS
void* CreateNLSocket (void);
int ReadEvents (void* sock, void* buffer, int32_t count, int32_t size);
void* CloseNLSocket (void* sock);
G_END_DECLS

#endif

