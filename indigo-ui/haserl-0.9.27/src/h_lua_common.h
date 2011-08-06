#ifndef _H_LUA_COMMON_H
#define _H_LUA_COMMON_H

void lua_common_setup(char *shell, list_t *env);
void lua_common_destroy(void);

#define HASERL_LIBRARY "/usr/local/lib/lua/5.1/haserl_lualib.lua"

#endif
